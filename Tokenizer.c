#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Tokenizer.h"

TOKENIZER *init_tokenizer(char *string){
    //malloc a tokenizer
    TOKENIZER *tokenizer = malloc(sizeof(TOKENIZER));
    //copy string into str
    tokenizer->str = malloc(strlen(string) + 1);
    strcpy(tokenizer->str, string);
    //set pos to the start of str
    tokenizer->pos = tokenizer->str;
    return tokenizer;
}
char *get_next_token(TOKENIZER *tokenizer){
    //if current char is a delimiter, just return it
    if (strchr(" &;|<>\n", *tokenizer->pos) != NULL) {
        return tokenizer->pos++;
    }
    //else go until next char is a delimiter
    char *start = tokenizer->pos;
    while (*tokenizer->pos != '\0' && strchr(" &;|<>\n", *tokenizer->pos) == NULL) {
        tokenizer->pos++;
    }
    //return the substring without white spaces
    char *token = malloc(tokenizer->pos - start + 1);
    strncpy(token, start, tokenizer->pos - start);
    token[tokenizer->pos - start] = '\0';
    return token;
}
