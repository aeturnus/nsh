#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "tok.h"
#include "vvector.h"

#define CWD_BUFF_SIZE 1000
pid_t pid;
char* cwd;
char* promptString = "# ";
VVector* pathDirs;

void print_prompt()
{
    printf("%s%s",cwd,promptString);
}

void sigtstp_handler(int signum)
{
    printf("fuck you, i ain't stopping\n");
    print_prompt();
}
////////////////////////
//  helper functions  //
////////////////////////
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

char* findExecutable(char* file)
{
    int len = VVector_length(pathDirs);
    char* output = 0x0;
    int fileLen = strlen(file);
    int dirLen; //length of path directory
    char* dir;  //path directory
    char* path; //final path
    struct stat *buf = malloc(sizeof(struct stat));   //just to use stat with
    int ret;   //stat's retval; init with state of failure

    //same dir check
    if(file[0] == '.')
    {
        //Get the full path
        dir = cwd;
        dirLen = strlen(dir);
        path = malloc(dirLen + fileLen + 2);    // 1 for slash, 1 for null
        strncpy(path,dir,dirLen+1);
        strcat(path,"/");
        strcat(path,file);
        //Check if the file even exists
        ret = stat(path,buf);
        if(ret == 0)
        {
            output = path;
            free(buf);
            return output;
        }
    }

    for(int i = 0; i < len; i++)
    {
        //Get the full path
        dir = VVector_get(pathDirs,i);
        dirLen = strlen(dir);
        path = malloc(dirLen + fileLen + 2);    // 1 for slash, 1 for null
        strncpy(path,dir,dirLen+1); //+1 for nullchar
        strcat(path,"/");
        strcat(path,file);
        //Check if the file even exists
        ret = stat(path,buf);
        if(ret == 0)
        {
            output = path;
            break;
        }
        else if (ret == -1)
        {
            free(path); //path isn't good anymore
        }

    }
    free(buf);

    return output;
}


////////////////////////
////////////////////////

/////////////////////////
//  utility functions  //
/////////////////////////

void executeProgram(char* path, char** args)
{
    char* execPath = strdup(path);
    pid = fork();
    if(pid == 0)
    {
        printf("Forked; executing %s\n",execPath);
        execvp(execPath,args);
    }
    else if(pid ==-1)
    {
        printf("shit's fucked yo\n");
    }
    else
    {
        int status;
        waitpid(pid,&status,0);
    }
    free(execPath);
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
    char* path = findExecutable(str);
    if(path != 0)
    {
        executeProgram(path,0);
    }
    else
    {
        printf("\"%s\" is not a recognized command",str);
    }
    free(str);
    free(path);
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
        char* path = findExecutable(token);
        if(path != 0)
        {
            executeProgram(path,0);
        }
        else
        {
            printf("\"%s\" is not a recognized command",token);
        }
        free(path);
    }

    free(token);    //clean up
    return 1;
}


int main()
{
    //set up the cwd variable
    cwd = malloc(CWD_BUFF_SIZE * sizeof(char));
    getcwd(cwd,CWD_BUFF_SIZE);  //TODO:error check

    //setup vector for path
    Tokenizer* dirTok = Tokenizer_new(getenv("PATH"),":");  //get tokenizer
    pathDirs = VVector_new(Tokenizer_countTokens(dirTok)+1);    //+1 to account for /bin
    char* bindir = strdup("/bin");
    VVector_push(pathDirs,bindir);
    while(Tokenizer_hasTokens(dirTok))
    {
        VVector_push(pathDirs,Tokenizer_next(dirTok));  //populate the vector
    }
    Tokenizer_delete(dirTok);

    int len = VVector_length(pathDirs);
    for(int i = 0; i < len; i++)
    {
        printf("Path: %s\n",VVector_get(pathDirs,i));
    }

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
