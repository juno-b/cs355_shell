typedef struct tokenizer {
    char *str; //string to parse
    char *pos; //position in string
} TOKENIZER;

TOKENIZER *init_tokenizer(char *string); //mallocs and initializes a tokenizer
char *get_next_token(TOKENIZER *tokenizer); //returns next token, ignores white spaces, returns NULL when string ends
