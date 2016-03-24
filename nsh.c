#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>

#include <sys/types.h>

#include "tok.h"
#include "vvector.h"

#define CWD_BUFF_SIZE 1000
pid_t pid;
char* cwd;
char* promptString = "# ";
char* execPath = 0;

void print_prompt()
{
    printf("%s%s",cwd,promptString);
}

void sigtstp_handler(int signum)
{
    printf("fuck you, i ain't stopping\n");
    print_prompt();
}

#define BUFFSIZE 128
char* read_line()
{
    size_t buffSize = BUFFSIZE;
    size_t pos = 0;
    char* buffer = malloc(buffSize);
    char c;
    while(1)
    {
        //Read character
        c = getchar();
        
        if(c == EOF || c == '\n')
        {
            buffer[pos] = '\0';
            return buffer;
        }
        else if (c == "\027")   //escape character detection
        {
            
        }
        else
        {
            buffer[pos] = c;
        }
        pos++;

        //Reallocate if bigger than buffer size
        if(pos >= buffSize)
        {
            size_t newSize = buffSize * 2;
            char* tempBuffer = realloc(buffer, buffSize);
            if(tempBuffer == 0) // if realloc didn't have enough space
            {
                tempBuffer = malloc(newSize);
                memcpy(tempBuffer,buffer,pos-1);
                free(buffer);
            }
            buffer = tempBuffer;    //set buffer to equal the new buffer
        }
    }
}

// function to automatically free the char* before reassigning it
void readToken(Tokenizer* tokenizer, char** strPtr)
{
    free(*strPtr);
    *strPtr = Tokenizer_next(tokenizer);
}

/////////////////////////
//  utility functions  //
/////////////////////////

void executeProgram(char* path, char** args)
{
    execPath = strdup(path);
    pid = fork();
    if(pid == 0)
    {
        printf("Forked; executing %s\n",execPath);
        execvp(execPath,args);
        free(execPath);
    }
}

void changeDirectory(char* dir)
{
    char* newPath;
    //check abs path
    if(dir[0] == '/')
    {
        newPath = strdup(dir);
    }
    else
    {
        int cwdLen = strlen(cwd);
        int dirLen = strlen(dir);
        newPath = malloc(cwdLen + dirLen + 2);    // 1 for the slash, 1 for the null
        strncpy(newPath,cwd,cwdLen);    // copy the cwd
        strcat(newPath,"/");            // add the slash
        strcat(newPath, dir);
    }
    //printf("changedir: %s",newPath);
    chdir(newPath);
    getcwd(cwd,CWD_BUFF_SIZE);  //TODO:error check
    
    free(newPath);
}

///////////////////////////////////////////
//  built in functions  and intermediates//
///////////////////////////////////////////

void cd(Tokenizer* tok)
{
    int count = Tokenizer_countTokens(tok);
    if(count == 0)  // go to home dir
    {
        changeDirectory(getenv("HOME"));
    }
    else if(count == 1) // go to target dir
    {
        char* dir = Tokenizer_next(tok);
        changeDirectory(dir);
        free(dir);
    }
    else if(count > 1)
    {
        printf("Cannot handle more than one entry");
    }
}

void ls(Tokenizer* tok)
{
    //Source the path
    char* path = 0;
    if(Tokenizer_hasTokens(tok))
    {
        path = Tokenizer_next(tok); //get it from tokens: no flags yet
    }
    else
    {
        path = strdup(cwd);         //get it from cwd
    }
    DIR *dp;
    struct dirent *ep;
    dp = opendir(path);
    if (dp != 0)
    {
        while (ep = readdir(dp))
        {
            printf("%s\t",(ep->d_name));
        }
        closedir (dp);
    }
    else
    {
        printf("Could not open directory \"%s\"",path);
    }
    free(path);

}

void echo(Tokenizer* tok)
{
    char* strOut = 0;
    int count = Tokenizer_countTokens(tok);
    int i = 0;
    for(; i < count-1; i++)
    {
        strOut = Tokenizer_next(tok);
        printf("%s ",strOut);
        free(strOut);
    }
    //To have no extra space at the end
    strOut = Tokenizer_next(tok);
    printf("%s",strOut);
    free(strOut);
}

void exec(Tokenizer* tok)
{
    char* str = Tokenizer_next(tok);
    executeProgram(str,0);
    free(str);
}

//////////////////////////
//////////////////////////

int runCommand(Tokenizer* tok)
{
    char* token = 0;
    readToken(tok, &token);
    //Check shell commands
    if(strcmp(token,"exit") == 0)
    {
        free(token);//clean up
        return 0;   //exit
    }
    else if(strcmp(token,"cd") == 0)
    {
        cd(tok);
    }
    else if(strcmp(token,"ls") == 0)
    {
        ls(tok);
    }
    else if(strcmp(token,"exec") == 0)
    {
        exec(tok);
    }
    else if(strcmp(token,"echo") == 0)
    {
        echo(tok);
    }
    else
    {
        printf("\"%s\" is not a recognized command",token);
    }

    free(token);    //clean up
    return 1;
}


int main()
{
    printf("%s\n",getenv("PATH"));  //

    //set up the cwd variable
    cwd = malloc(CWD_BUFF_SIZE * sizeof(char));
    getcwd(cwd,CWD_BUFF_SIZE);  //TODO:error check

    signal(SIGTSTP, sigtstp_handler);
    char* line;
    char** tokens;
    int numTokens;
    Tokenizer* tokenizer;
    int status = 1;
    while(status)
    {
        print_prompt();
        line = read_line();
        
        tokenizer = Tokenizer_new(line, " ");
        if(Tokenizer_hasTokens(tokenizer))
        {
            status = runCommand(tokenizer);
        }

        free(line);                     //Might keep for history?
        Tokenizer_delete(tokenizer);    //we don't need them anymore;
        
        printf("\n");   //newline
    }
}
