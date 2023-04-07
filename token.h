#pragma once

#include <cstdio>
#include <cassert>
#include <cctype>
#include <cstring>

struct Loc {
    const char* file;
    long row, col;
};

#define T_LIST \
T(Dot) T(Comma) \
T(Semicolon) T(Colon) \
T(OCurly) T(CCurly) \
T(OSquare) T(CSquare) \
T(OParen) T(CParen) \
T(Assignment) T(Plus) T(Minus) T(FSlash) T(Star) \
T(Carat) T(Ampersand) T(Exclaim) T(Bar) T(Question) \
T(Greater) T(Lesser) T(Modulus) \
\
T(BitwiseLeft) T(BitwiseRight) \
T(Variadic) T(And) T(Or) \
T(GreaterEq) T(LesserEq) T(NotEqual) T(Equal) T(PointMember) \
T(Increment) T(Decrement) T(IncrementAssn) T(DecrementAssn) T(StarAssn) T(FSlashAssn) T(ModulusAssn) \
T(BitwiseAndAssn) T(BitwiseOrAssn) T(BitwiseXorAssn) \
T(BitwiseNot) T(BitwiseLeftAssn) T(BitwiseRightAssn) \
\
T(Return) T(Break) T(Continue) T(If) T(Else) \
T(For) T(While) T(Do) T(Switch) T(Case) T(Default) \
T(Static) T(Const) T(Struct) T(Union) T(Enum) T(True) T(False) \
\
T(Identifier) T(StringLiteral) T(IntegerLiteral) T(CharacterLiteral) \
T(Comment) T(PreProcessor)

// TODO: Character literal

enum class TokenType {
    #define T(E) E,
    T_LIST
    #undef T
};

struct TokenLiteralPair { const char* key; TokenType value; };
const TokenLiteralPair token_literals[] = { // descending by length
#define T(K, V) TokenLiteralPair{K, TokenType:: V}

T("continue", Continue),

T("default", Default),

T("static", Static),
T("return", Return),
T("switch", Switch),

T("union", Union),
T("const", Const),
T("break", Break),
T("while", While),
T("false", False),

T("case", Case),
T("else", Else),
T("true", True),

T("...", Variadic),
T("for", For),
T("<<=", BitwiseLeftAssn),
T(">>=", BitwiseRightAssn),

T("do", Do),
T("if", If),
T("++", Increment),
T("--", Decrement),
T("+=", IncrementAssn),
T("-=", DecrementAssn),
T("*=", StarAssn),
T("/=", FSlashAssn),
T("%=", ModulusAssn),
T("&=", BitwiseAndAssn),
T("|=", BitwiseOrAssn),
T("^=", BitwiseXorAssn),

T("&&", And),
T("||", Or),

T(">=", GreaterEq),
T("<=", LesserEq),
T("!=", NotEqual),
T("==", Equal),
T("->", PointMember),

T(".", Dot),
T(",", Comma),
T(";", Semicolon),
T(":", Colon),
T("(", OParen),
T(")", CParen),
T("{", OCurly),
T("}", CCurly),
T("[", OSquare),
T("]", CSquare),
T("=", Assignment),
T("+", Plus),
T("-", Minus),
T("%", Modulus),
T("/", FSlash),
T("*", Star),
T("^", Carat),
T("&", Ampersand),
T("!", Exclaim),
T("~", BitwiseNot),
T("|", Bar),
T("?", Question),
T(">", Greater),
T("<", Lesser),

#undef T
};

const size_t token_literals_ct = sizeof(token_literals) / sizeof(token_literals[0]);

struct Token {
    union {
        char* v_cstr;
        int v_int;
        long v_long;
        bool v_bool;
    };
    bool cstr_ownership = false;

    const char* srcptr;
    size_t srclen;
    
    TokenType type;
    Loc loc;
    ~Token() {
        if (cstr_ownership && v_cstr != nullptr) delete[] v_cstr;
    }

    static const char* type_str(TokenType t) {
        #define T(E) case TokenType:: E : return #E ;
        switch (t) { T_LIST }
        #undef T
        assert(false);
        return "Bad token type str";
    }
    
    void print() const {
        printf("%s:%i:%i: %s %.*s\n", 
            loc.file, loc.row+1, loc.col+1, type_str(type),
            srclen, srcptr);
    }
};