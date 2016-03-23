#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

char* promptString = "# ";

void print_prompt()
{
    printf(promptString);
}

void sigtstp_handler(int signum)
{
    printf("\nfuck you, i ain't stopping\n");
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
                tempBuffer = malloc(buffSize);
                memcpy(tempBuffer,buffer,pos-1);
                free(buffer);
            }
            buffer = tempBuffer;    //set buffer to equal the new buffer
        }
    }
}


int main()
{
    signal(SIGTSTP, sigtstp_handler);
    char* line;
    while(1)
    {
        print_prompt();
        line = read_line();
        print_prompt();
        printf("%s\n",line);
        free(line);
    }
}
