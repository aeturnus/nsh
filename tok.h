#ifndef __TOK_H__
#define __TOK_H__

//Provides a tokenizer struct
struct tokenizer_str;
typedef struct tokenizer_str Tokenizer;

// Allocates and initializes a tokenizer
Tokenizer* Tokenizer_new(char* str, char* delimiters);

// Frees a tokenizer
void Tokenizer_delete(Tokenizer* tokenizer);

// Get the next string in the tokenizer
// Returns null if no more tokens
// Returns pointer to newly allocated string otherwise
char* Tokenizer_next(Tokenizer* tokenizer);

// Peeks at next token, doesn't advance it
char* Tokenizer_peek(Tokenizer* tokenizer);

// Checks if there are tokens remaining
int Tokenizer_hasTokens(Tokenizer* tokenizer);

// Returns number of remaining tokens
int Tokenizer_countTokens(Tokenizer* tokenizer);

// Returns total number of tokens
int Tokenizer_numTokens(Tokenizer* tokenizer);

// Resets the tokenizer to beginning
void Tokenizer_reset(Tokenizer* tokenizer);

// Checks if the tokenizer contains a certain string
// Returns 1 if true, 0 if false
int Tokenizer_contains(Tokenizer* tokenizer, const char* str);

// Returns a deep copy of all the tokens in an array
char** Tokenizer_tokens(Tokenizer* tokenizer);

#endif
