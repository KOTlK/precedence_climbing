/*
    source: https://eli.thegreenplace.net/2012/08/02/parsing-expressions-by-precedence-climbing

    I couldn't read the python code, because it is nearly impossible task for me,
    so I tried to implement it myself in a little more reasonable language.
*/
#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <cstring>

#define u32 uint32_t
#define s32 int32_t
#define MIN_TOKENS_COUNT 128
#define MAX_STRING_LENGTH 512

#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

#define Malloc(type, size) (type*)malloc(size)

enum Token_Type {
    PLUS         = '+',
    MINUS        = '-',
    MUL          = '*',
    DIV          = '/',
    EXP          = '^',
    MOD          = '%',
    NONE         = 256,
    NUMBER       = 257,
    OPENPARENTHESES  = 258,
    CLOSEPARENTHESES = 259
};

enum Associativity {
    LEFT  = 0,
    RIGHT = 1
};

struct Token {
    Token_Type type;
    s32        value;
    bool       is_binary;
};

struct Tokenizer {
    Token *tokens;
    u32    count;
    u32    capacity;
    u32    current_token;
};

// Length based string
struct Thestring {
    u32   length;
    char *text;

    char &operator[](u32 i)        { return text[i]; }
    inline void set(u32 i, char c) { text[i] = c; }
};

Thestring    *make_string(char *txt);
Thestring    *eat_spaces(char *text);
const char   *token_to_str(Token token);
Tokenizer    *tokenize(Thestring *text);
Token         get_next_token(Tokenizer *tokenizer);
Token         get_current_token(Tokenizer *tokenizer);
Token         get_current_token(Tokenizer *tokenizer);
void          push_token(Tokenizer *tokenizer, Token token);
s32           parse_expression(char *expr);
s32           parse_subexpression(Tokenizer *tokenizer, s32 precedence);
s32           is_number(char c);
s32           get_precedence(Token token);
s32           compute_operator(Token token, s32 val, s32 sub_val);
Associativity get_associativity(Token token);
void          print_error();
bool          is_operator_or_parentheses(Token token);

s32 parse_expression(char *expr) {
    // convert expression into normal string and remove all the spaces
    Thestring *text = eat_spaces(expr);

    Tokenizer *tokenizer = tokenize(text);

    s32 result = parse_subexpression(tokenizer, -9999);

    free(text);
    free(tokenizer->tokens);
    free(tokenizer);

    return result;
}

s32 parse_subexpression(Tokenizer *tokenizer, s32 precedence) {
    Token token = get_next_token(tokenizer);

    s32 result;
    s32 sign = 1;

    if(token.type == MINUS && token.is_binary == false) {
        sign = -1;
        token = get_next_token(tokenizer);
    }

    if(token.type == OPENPARENTHESES) {
        result = parse_subexpression(tokenizer, -9999);
        get_next_token(tokenizer);
    } else {
        result = token.value;
    }

    // if token was unary minus, invert the result of subexpression
    result *= sign;

    while(true) {
        Token op        = get_current_token(tokenizer);
        s32   next_prec = get_precedence(op);

        if(op.type == NONE || op.type == CLOSEPARENTHESES) break;
        if(op.is_binary == false) break;
        if(next_prec <= precedence) break;

        Associativity assoc = get_associativity(op);

        if(assoc == LEFT) next_prec++;

        get_next_token(tokenizer);
        s32 value = parse_subexpression(tokenizer, next_prec);

        result = compute_operator(op, result, value);
    }

    return result;
}

// Transform text into tokens
Tokenizer *tokenize(Thestring *txt) {
    Thestring text = *txt;
    Tokenizer *tokenizer = Malloc(Tokenizer, sizeof(Tokenizer));

    tokenizer->capacity      = MIN_TOKENS_COUNT;
    tokenizer->tokens        = Malloc(Token, sizeof(Token) * tokenizer->capacity);
    tokenizer->count         = 0;
    tokenizer->current_token = 0;

    for(u32 i = 0; i < txt->length; ++i) {
        switch(text[i]) {
            case ' '  : break;
            case '\r' : break;
            case '\0' : break;
            case '\n' : break;
            case '(' : {
                Token token;
                token.type       = OPENPARENTHESES;
                token.is_binary  = false;

                push_token(tokenizer, token);
            } break;
            case ')' : {
                Token token;
                token.type       = CLOSEPARENTHESES;
                token.is_binary  = false;

                push_token(tokenizer, token);
            } break;
            case '+' : {
                Token token;
                token.type       = PLUS;
                token.is_binary  = true;

                push_token(tokenizer, token);
            } break;
            case '-' : {
                Token token;
                token.type       = MINUS;

                if(i == 0) {
                    token.is_binary = false;
                } else if (is_operator_or_parentheses(tokenizer->tokens[tokenizer->count - 1])) {
                    token.is_binary = false;
                }else {
                    token.is_binary = true;
                }

                push_token(tokenizer, token);
            } break;
            case '*' : {
                Token token;
                token.type       = MUL;
                token.is_binary  = true;

                push_token(tokenizer, token);
            } break;
            case '/' : {
                Token token;
                token.type       = DIV;
                token.is_binary  = true;

                push_token(tokenizer, token);
            } break;
            case '^' : {
                Token token;
                token.type       = EXP;
                token.is_binary  = true;

                push_token(tokenizer, token);
            } break;
            case '%' : {
                Token token;
                token.type       = MOD;
                token.is_binary  = true;

                push_token(tokenizer, token);
            } break;
            default : {
                if(!is_number(text[i])) {
                    print_error();
                    printf("index: %i, unexpected symbol '%c' while parsing the expression\n", i, text[i]);
                    return tokenizer;
                }

                const s32 buf_len = 10;
                char      buf[10];
                s32       c = 0;

                for(s32 j = 0; j < buf_len; ++j) {
                    buf[j] = 0;
                }

                Token token;
                token.type      = NUMBER;
                token.is_binary = false;

                while(is_number(text[i + c]) && c < 10) {
                    buf[c] = text[i + c];
                    c++;
                }

                token.value = atoi(buf);

                push_token(tokenizer, token);
                i += c - 1;
            } break;
        }
    }

    return tokenizer;
}

const char *token_to_str(Token token) {
    switch(token.type) {
        case PLUS  : return "+";
        case MINUS : return "-";
        case MUL   : return "*";
        case DIV   : return "/";
        case EXP   : return "^";
        case MOD   : return "%";
        case NUMBER       : {
            char *buf = (char*)malloc(10); // Leak. don't care.

            sprintf(buf, "%i", token.value);

            return buf;
        }
        case OPENPARENTHESES  : return "OPENPARENTHESES";
        case CLOSEPARENTHESES : return "CLOSEPARENTHESES";
        default : {
            return "UNKNOWN";
        }
    }
}

void push_token(Tokenizer *tokenizer, Token token) {
    tokenizer->tokens[tokenizer->count] = token;
    tokenizer->count++;

    if(tokenizer->count == tokenizer->capacity) {
        tokenizer->capacity = tokenizer->capacity << 1;
        tokenizer->tokens   = (Token*)realloc(tokenizer->tokens, tokenizer->capacity);
    }
}

// In ascii symbols between 47 and 58 are numbers
s32 is_number(char c) {
    return c > 47 && c < 58;
}

s32 get_precedence(Token token) {
    switch(token.type) {
        case PLUS         : return 10;
        case MINUS        : return 10;
        case MUL          : return 20;
        case DIV          : return 20;
        case MOD          : return 20;
        case EXP          : return 30;
        default : return 0;
    }
}

s32 compute_operator(Token token, s32 lhs, s32 rhs) {
    switch(token.type) {
        case PLUS  : return lhs + rhs;
        case MINUS : return lhs - rhs;
        case MUL   : return lhs * rhs;
        case DIV   : return lhs / rhs;
        case MOD   : return lhs % rhs;
        case EXP   : return lhs ^ rhs;
        default    : return lhs;
    }
}

Token get_next_token(Tokenizer *tokenizer) {
    if(tokenizer->current_token == tokenizer->count) {
        Token token;
        token.type = NONE;

        return token;
    }

    return tokenizer->tokens[tokenizer->current_token++];
}

Token get_current_token(Tokenizer *tokenizer) {
    if(tokenizer->current_token == tokenizer->count) {
        Token token;
        token.type = NONE;

        return token;
    }

    return tokenizer->tokens[tokenizer->current_token];
}

Thestring *make_string(char *txt, u32 len) {
    char *data = (char*)malloc(sizeof(Thestring) + len);
    Thestring *string = (Thestring*)data;
    char *text = (char*)(data + sizeof(Thestring));

    for(u32 i = 0; i < len; ++i) {
        text[i] = txt[i];
    }

    string->text   = text;
    string->length = len;

    return string;
}

Thestring *eat_spaces(char *txt) {
    char buf[MAX_STRING_LENGTH];

    u32 i = 0;
    u32 j = 0;

    while(txt[i] != '\0') {
        if(txt[i] == ' ')  { i++; continue; }
        if(txt[i] == '\r') { i++; continue; }
        if(txt[i] == '\n') { i++; continue; }
        if(txt[i] == '\t') { i++; continue; }

        buf[j++] = txt[i++];
    }

    Thestring *string = make_string(buf, j);

    return string;
}

void print_error() {
    printf("%serror: %s", COLOR_RED, COLOR_RESET);
}

Associativity get_associativity(Token token) {
    switch(token.type) {
        case PLUS  : return LEFT;
        case MINUS : return LEFT;
        case MUL   : return LEFT;
        case DIV   : return LEFT;
        case EXP   : return RIGHT;
        case MOD   : return LEFT;
        default : return LEFT;
    }
}

bool is_operator_or_parentheses(Token token) {
    switch(token.type) {
        case NUMBER : return false;
        case CLOSEPARENTHESES : return false;
        default : return true;
    }
}

#endif // PARSRE_H
