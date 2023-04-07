#include "lexer.h"

Lexer::Lexer(const char* file_path) : file_path(file_path), pos(0), bol(0), row(0), fsize(-1), fdata(nullptr) {
    FILE* file = fopen(file_path,"r");
    fseek(file,0,SEEK_END);
    fsize = ftell(file);
    rewind(file);

    fdata = new char[fsize+1];
    size_t sz = fread(fdata, sizeof(char), fsize, file);
    if (ferror(file)) {
        fclose(file);
        fsize = -1;
        delete[] fdata;
        return;
    }
    fclose(file);
    fdata[fsize] = '\0';
}
Lexer::~Lexer() {
    if (fdata) delete[] fdata;
}

bool Lexer::equal(const char* key) const {
    return strncmp(key,fdata + pos, strlen(key)) == 0;
}
inline char Lexer::peek() const {
    return fdata[pos];
}
inline char Lexer::chop() {
    assert(has_next());
    char x = peek();
    ++pos;
    if (x == '\n') {
        bol = pos;
        ++row;
    }
    return x;
}
void Lexer::chop(size_t ct) {
    for (size_t i=0;i<ct;++i) chop();
}
void Lexer::trim_left() {
    while (has_next() && isspace(peek())) chop();
}

bool Lexer::has_next() const {
    return pos < fsize;
}

Loc Lexer::getloc() const {
    return Loc{file_path, row, pos-bol};
}

Token* Lexer::next_token() {
    trim_left();
    if (!has_next()) return nullptr;
    if (inside_block_comment) {
        Loc loc = getloc();
        long start = pos, len = 0;
        while (has_next() && peek() != '\n') {
            if (equal("*/")) {
                inside_block_comment = false;
                chop(2);
                len += 2;
                break;
            } else {
                chop();
                ++len;
            }
        }
        if (has_next() && peek() == '\n') chop();
        Token* t = new Token();
        t->srcptr = fdata + start;
        t->srclen = len;
        t->loc = loc;
        t->type = TokenType::Comment;
        return t;
    }
    if (peek() == '#') { // Preprocessor
        long start = pos, len = 0;
        Loc loc = getloc();
        while (has_next() && peek() != '\n') {
            chop();
            ++len;
        }
        if (has_next()) chop();
        Token* t = new Token();
        t->type = TokenType::PreProcessor;
        t->loc = loc;
        t->srcptr = fdata + start;
        t->srclen = len;
        return t;
    }
    if (equal("//")) {
        long start = pos, len = 0;
        Loc loc = getloc();
        while (has_next() && peek() != '\n') {
            chop();
            ++len;
        }
        if (has_next()) chop();
        Token* t = new Token();
        t->type = TokenType::Comment;
        t->loc = loc;
        t->srcptr = fdata + start;
        t->srclen = len;
        return t;

    }
    if (equal("/*")) {
        inside_block_comment = true;
        Loc loc = getloc();
        long start = pos, len = 0;
        while (has_next() && peek() != '\n') {
            if (equal("*/")) {
                inside_block_comment = false;
                chop(2);
                len += 2;
                break;
            } else {
                chop();
                ++len;
            }
        }
        if (has_next() && peek() == '\n') chop();
        Token* t = new Token();
        t->srcptr = fdata + start;
        t->srclen = len;
        t->loc = loc;
        t->type = TokenType::Comment;
        return t;
    }
    if (peek() == '\"') {
        Loc loc = getloc();
        long start = pos, len = 1;
        do {
            if (peek() == '\n') return nullptr;
            chop();
            ++len;
        } while (has_next() && peek() != '\"');

        Token* t = new Token();
        t->srcptr = fdata + start;
        t->srclen = len;
        t->loc = loc;
        t->type = TokenType::StringLiteral;
        return t;
    }

    for (size_t i=0;i<token_literals_ct;++i) { // Descending string length order 
        if (equal(token_literals[i].key)) {
            Token* t = new Token();
            t->loc = getloc();
            t->srcptr = fdata + pos;
            t->srclen = strlen(token_literals[i].key);
            t->type = token_literals[i].value;
            pos += strlen(token_literals[i].key); // DOESN'T NEED SAFETY OF CHOP()
            return t;
        }
    }

    { // Literals
        char x = peek();
        if (isalpha(x) || x == '_') { // String Literal
            long start = pos, len = 0;
            Loc loc = getloc();
            while (has_next() && (isalnum(x) || x == '_') ) { // TODO: Escape sequences
                chop();
                ++len;
                x = peek();
            }
            Token* t = new Token();
            t->type = TokenType::Identifier;
            t->srcptr = fdata + start;
            t->srclen = len;
            t->loc = loc;
            return t;
        } else if (isdigit(x)) { // Integer Literal
            Loc loc = getloc();
            long start = pos, len = 1;
            chop();
            while (isdigit(x)) {
                chop();
                ++len;
                x = peek();
            }
            Token* t = new Token();
            t->type = TokenType::IntegerLiteral;
            t->srcptr = fdata + start;
            t->srclen = len;
            t->loc = loc;
            return t;
        }
    }

    assert(false);
    return nullptr;
}