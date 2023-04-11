#include <iostream>

#include "parser.h"

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
        printf("\x1b[31mMust specify file!\x1b[0m EX: \"lexer src.c\"\n");
        return 1;
    }
    Lexer lexer = Lexer(argv[1]);

    size_t ct = 0, capacity = 16;
    Token** tkns = (Token**) malloc(sizeof(Token*) * capacity);

    printf("LEXING: \n");

    while (lexer.has_next()) {
        Token* t = lexer.next_token();
        assert(t != nullptr);
        t->print();
        arr_push<Token*>(t,tkns,ct,capacity);
    }

    printf("Found %u tokens: \n");

    printf("\nPARSING: \n");
    
    Parser p((const Token**)tkns, ct, capacity);
    p.parse();

    free(tkns);
    
    return 0;
}