
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEXEME 128

/* Check whether a word is a reserved keyword */
int is_keyword(const char *word) {
    const char *keywords[] = {
        "int", "if", "else", "while", "return"
    };

    for (int i = 0; i < 5; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;
        }
    }

    return 0;
}

/* Print a token in the required format */
void print_token(const char *lexeme, const char *type) {
    printf("%s,%s\n", lexeme, type);
}

/* Recognize the required whitespace characters */
int is_space(int ch) {
    return ch == ' ' || ch == '\t' ||
           ch == '\r' || ch == '\n';
}

int main(int argc, char **argv) {

    /* Check command-line arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: lexical_analyzer input.c\n");
        return 1;
    }

    /* Open the input file */
    FILE *input = fopen(argv[1], "rb");

    if (input == NULL) {
        perror("Input file");
        return 1;
    }

    int ch;

    /* Read the source file character by character */
    while ((ch = fgetc(input)) != EOF) {

        /* Ignore whitespace */
        if (is_space(ch)) {
            continue;
        }

        /* Identify keywords and identifiers */
        if (isalpha((unsigned char)ch) || ch == '_') {

            char lexeme[MAX_LEXEME + 1];
            int i = 0;

            do {
                if (i >= MAX_LEXEME) {
                    fprintf(stderr, "Error: lexeme too long\n");
                    fclose(input);
                    return 1;
                }

                lexeme[i++] = (char)ch;
                ch = fgetc(input);

            } while (ch != EOF &&
                     (isalnum((unsigned char)ch) || ch == '_'));

            lexeme[i] = '\0';

            if (ch != EOF) {
                ungetc(ch, input);
            }

            if (is_keyword(lexeme)) {
                print_token(lexeme, "KEYWORD");
            } else {
                print_token(lexeme, "IDENTIFIER");
            }

            continue;
        }

        /* Identify integer literals */
        if (isdigit((unsigned char)ch)) {

            char lexeme[MAX_LEXEME + 1];
            int i = 0;

            do {
                if (i >= MAX_LEXEME) {
                    fprintf(stderr, "Error: integer too long\n");
                    fclose(input);
                    return 1;
                }

                lexeme[i++] = (char)ch;
                ch = fgetc(input);

            } while (ch != EOF && isdigit((unsigned char)ch));

            lexeme[i] = '\0';

            if (ch != EOF) {
                ungetc(ch, input);
            }

            print_token(lexeme, "INTEGER");
            continue;
        }

        /* Handle comments and division */
        if (ch == '/') {

            int next = fgetc(input);

            /* Single-line comment: // */
            if (next == '/') {

                while ((ch = fgetc(input)) != EOF && ch != '\n') {
                    /* Ignore comment characters */
                }

                continue;
            }

            /* Handle multi-line comments */
            if (next == '*') {

                int previous = 0;
                int closed = 0;

                while ((ch = fgetc(input)) != EOF) {

                    if (previous == '*' && ch == '/') {
                        closed = 1;
                        break;
                    }

                    previous = ch;
                }

                if (!closed) {
                    fprintf(stderr,
                            "Error: unclosed block comment\n");
                    fclose(input);
                    return 1;
                }

                continue;
            }

            /* It is a division operator, not a comment */
            if (next != EOF) {
                ungetc(next, input);
            }

            print_token("/", "ARITHMETIC_OPERATOR");
            continue;
        }

        /* Handle two-character operators */
        char lexeme[3] = {(char)ch, '\0', '\0'};

        if (ch == '=' || ch == '!' || ch == '<' ||
            ch == '>' || ch == '&' || ch == '|') {

            int next = fgetc(input);

            /* Relational operators */
            if ((ch == '=' && next == '=') ||
                (ch == '!' && next == '=') ||
                (ch == '<' && next == '=') ||
                (ch == '>' && next == '=')) {

                lexeme[1] = (char)next;

                print_token(lexeme, "RELATIONAL_OPERATOR");
                continue;
            }

            /* Logical operators */
            if ((ch == '&' && next == '&') ||
                (ch == '|' && next == '|')) {

                lexeme[1] = (char)next;

                print_token(lexeme, "LOGICAL_OPERATOR");
                continue;
            }

            if (next != EOF) {
                ungetc(next, input);
            }
        }

        /* Handle single-character operators and delimiters */
        if (ch == '=') {
            print_token(lexeme, "ASSIGNMENT_OPERATOR");
        }

        else if (ch == '+' || ch == '-' ||
                 ch == '*' || ch == '%') {

            print_token(lexeme, "ARITHMETIC_OPERATOR");
        }

        else if (ch == '<' || ch == '>') {
            print_token(lexeme, "RELATIONAL_OPERATOR");
        }

        else if (ch == '!') {
            print_token(lexeme, "LOGICAL_OPERATOR");
        }

        else if (ch == '(' || ch == ')' ||
                 ch == '{' || ch == '}' ||
                 ch == '[' || ch == ']' ||
                 ch == ';' || ch == ',') {

            print_token(lexeme, "DELIMITER");
        }

        else {
            print_token(lexeme, "UNKNOWN");
        }
    }

    /* Check for file-reading errors */
    if (ferror(input)) {
        fprintf(stderr, "Error: failed to read input file\n");
        fclose(input);
        return 1;
    }

    fclose(input);

    return 0;
}
