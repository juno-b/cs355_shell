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
    if (tokenizer == NULL || *(tokenizer->pos) == '\0') // Check if the tokenizer is NULL or at the end of the string
        return NULL;

    while (isspace(*(tokenizer->pos))) // Skip leading white spaces
        (tokenizer->pos)++;

    if (*(tokenizer->pos) == '\0') // Check if the end of the string is reached
        return NULL;

    char *start = tokenizer->pos; // Start of the token
    while (*(tokenizer->pos) != '\0' && !isspace(*(tokenizer->pos))) // Find the end of the token
        (tokenizer->pos)++;

    char *token = (char *)malloc(tokenizer->pos - start + 1); // Allocate memory for the token
    if (token == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strncpy(token, start, tokenizer->pos - start); // Copy the token
    token[tokenizer->pos - start] = '\0'; // Null-terminate the token
    return token;
}
