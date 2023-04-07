#pragma once

#include <cstdarg>
#include "lexer.h"

class Parser {
    const Token** toks;
    const size_t tokcount, tokcap;
    size_t pos;

    bool has_next() const {
        return pos < tokcount;
    }

    const Token* peek_token() const {
        assert(has_next());
        return toks[pos];
    }
 
    const Token* expect_token(size_t ct, ...) {
        va_list vl;
        va_start(vl, ct);
        printf("Expecting: ");
        for (int i=0;i<ct;++i) {
            TokenType t = va_arg(vl, TokenType);
            printf("%s ", Token::type_str(t));
        }
        printf("\n");
        va_end(vl);

        assert(has_next());
        return toks[pos++];
    }
public:
    Parser(const Token** toks, size_t tokcount, size_t tokcap) : toks(toks), tokcount(tokcount), tokcap(tokcap), pos(0) {

    }
    ~Parser() {

    }
    void parse() {
        while (has_next()) {
            const Token* t = expect_token(2, TokenType::PreProcessor, TokenType::Comment);
            switch (t->type) {
                case TokenType::PreProcessor:
                case TokenType::Comment:
                    continue;
                default:
                    break;
            }

        }
    }
};