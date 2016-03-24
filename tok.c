#include <stdlib.h>
#include <string.h>

#include "tok.h"
#include "vvector.h"

// Tokenizer
struct tokenizer_str
{
    int pos;            //Current position: sits on the next token
    int length;         //Number of elements
    char **elements;    //Array of strings
};

Tokenizer* Tokenizer_new(char* str, char* del)
{
    VVector* vec = VVector_new(1);
    char* tok = strtok(str,del);
    while(tok != 0x0)
    {
        VVector_push(vec,strdup(tok));  //Push the string into the vector
        tok = strtok(0x0,del);      //next token
    }
    Tokenizer* tokenizer = malloc(sizeof(Tokenizer));

    tokenizer->pos = 0;
    tokenizer->length = VVector_length(vec);    //get the size of the array returned
    tokenizer->elements = (char**)VVector_toArray(vec); //get the array of strings
    VVector_delete(vec);    //free the vector

    return tokenizer;
}

void Tokenizer_delete(Tokenizer* tok)
{
    //Free each string
    int size = tok->length;
    char **strs = tok->elements;
    for(int i = 0; i < size; i++)
    {
        free(strs[i]);
    }

    //Free the array itself;
    free(strs);

    //Free the struct
    free(tok);
}

// Get the next string in the tokenizer
// Returns null if no more tokens
// Returns pointer to newly allocated string otherwise
char* Tokenizer_next(Tokenizer* tokenizer)
{
    if(tokenizer->pos >= tokenizer->length)
    {
        return 0x0;
    }
    char* output = strdup(tokenizer->elements[tokenizer->pos]);
    tokenizer->pos++;
    return output;
}

// Peeks at next token, doesn't advance it
char* Tokenizer_peek(Tokenizer* tokenizer)
{
    int len = tokenizer->length;
    if(tokenizer->pos >= len)
    {
        return 0x0;
    }
    char* output = strdup(tokenizer->elements[tokenizer->pos]);
    return output;
}

// Checks if there are tokens remaining
int Tokenizer_hasTokens(Tokenizer* tokenizer)
{
    if(tokenizer->pos < tokenizer->length)
    {
        return 1;
    }
    return 0;
}

// Returns number of remaining tokens
int Tokenizer_countTokens(Tokenizer* tokenizer)
{
    return tokenizer->length - tokenizer->pos;
}

// Returns total number of tokens
int Tokenizer_numTokens(Tokenizer* tokenizer)
{
    return tokenizer->length;
}

// Resets the tokenizer to beginning
void Tokenizer_reset(Tokenizer* tokenizer)
{
    tokenizer->pos = 0;
}

// Checks if the tokenizer contains a certain string
// Returns 1 if true, 0 if false
int Tokenizer_contains(Tokenizer* tokenizer, const char* str)
{
    int len = tokenizer->length;
    for(int i = 0; i < len; i++)
    {
        if(strcmp(str,tokenizer->elements[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// Returns a deep copy of all the tokens in an array
char** Tokenizer_tokens(Tokenizer* tokenizer)
{
    char** output = malloc(sizeof(char*) * tokenizer->length);
    int len = tokenizer->length;
    for(int i = 0; i < len; i++)
    {
        output[i] = strdup(tokenizer->elements[i]);
    }
    return output;
}
