#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
    if (*(tokenizer->pos) == '\0' || *(tokenizer->pos) == '\n') {
        return NULL; // return NULL when string ends or reaches newline
    }
    while (isspace(*(tokenizer->pos))) // Skip leading white spaces
        (tokenizer->pos)++;

    // check for delimiters
    if (*(tokenizer->pos) == '&' || *(tokenizer->pos) == ';' || *(tokenizer->pos) == '|' || *(tokenizer->pos) == '<' || *(tokenizer->pos) == '>') {
        char *token = (char *)malloc(2 * sizeof(char));
        if (token == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        token[0] = *(tokenizer->pos);
        token[1] = '\0';
        (tokenizer->pos)++;
        return token;
    }

    // find end of token
    char *start = tokenizer->pos;
    while (*(tokenizer->pos) != '\0' && *(tokenizer->pos) != '\n' && *(tokenizer->pos) != ' ' &&
           *(tokenizer->pos) != '&' && *(tokenizer->pos) != ';' && *(tokenizer->pos) != '|' &&
           *(tokenizer->pos) != '<' && *(tokenizer->pos) != '>') {
        (tokenizer->pos)++;
    }

    // compute length of token
    int length = tokenizer->pos - start;
    
    // allocate memory for token and copy substring
    char *token = (char *)malloc((length + 1) * sizeof(char));
    if (token == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strncpy(token, start, length);
    token[length] = '\0';

    return token;
}
