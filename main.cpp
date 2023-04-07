#include <iostream>

#include "lexer.h"

template<typename T>
void arr_push(T val, T*& arr, size_t& ct, size_t& capacity) {
    ++ct;
    if (ct >= capacity) {
        capacity *= 2;
        arr = (T*) realloc(arr, sizeof(T) * capacity);
    }
    arr[ct-1] = val;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Must specify file! EX: \"lexer src.c\"\n");
        return 1;
    }
    Lexer lexer = Lexer(argv[1]);

    size_t ct = 0, capacity = 16;
    Token** tkns = (Token**) malloc(sizeof(Token*) * capacity);

    while (lexer.has_next()) {
        Token* t = lexer.next_token();
        if (t != nullptr) {
            printf("%s:%i:%i: %s %.*s\n", 
                t->loc.file, t->loc.row+1, t->loc.col+1, token_type_str(t->type),
                t->srclen, t->srcptr);
            arr_push(t,tkns,ct,capacity);
        }
    }

    free(tkns);
    
    return 0;
}