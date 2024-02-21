#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Tokenizer.h"

TOKENIZER *init_tokenizer(char *string){
    //malloc a tokenizer
    TOKENIZER *tokenizer = malloc(sizeof(TOKENIZER));
    if (tokenizer == NULL) {
        perror("Failed to allocate memory for tokenizer");
        exit(EXIT_FAILURE);
    }
    // Allocate memory for the string and copy it
    tokenizer->str = strdup(string);
    if (tokenizer->str == NULL) {
        perror("Failed to allocate memory for string");
        free(tokenizer);
        exit(EXIT_FAILURE);
    }
    // Set the position to the start of the string
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
    if (token == NULL) {
        perror("Failed to allocate memory for token");
        exit(EXIT_FAILURE);
    }
    strncpy(token, start, tokenizer->pos - start);
    token[tokenizer->pos - start] = '\0';
    return token;
}
