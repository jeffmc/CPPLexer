#pragma once

#include "token.h"

class Lexer {
    const char* file_path;
    char* fdata;
    long fsize, pos, bol, row;
    
    bool inside_block_comment = false;

    bool equal(const char* key) const;

    inline char peek() const;
    inline char chop();
    void chop(size_t ct);
    void trim_left();
public:
    Lexer(const char* file_path);
    ~Lexer();

    bool has_next() const;
    Loc getloc() const;
    
    Token* next_token();
};