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
T(Dot) T(Comma) T(Semicolon) T(Colon) \
T(OCurly) T(CCurly) T(OSquare) T(CSquare) T(OParen) T(CParen) \
T(BitwiseXor) T(BitwiseAnd) T(BitwiseOr) T(BitwiseLeft) T(BitwiseRight) \
T(LogicalNot) T(LogicalAnd) T(LogicalOr) T(Question) \
T(Variadic) T(PointMember) \
T(Greater) T(Lesser) T(GreaterEq) T(LesserEq) T(NotEqual) T(Equal) \
T(Assignment) T(Increment) T(Decrement) \
T(Plus) T(Minus) T(Multiply) T(Divide) T(Modulus) \
T(PlusAssn) T(MinusAssn) T(MultiplyAssn) T(DivideAssn) T(ModulusAssn) \
T(BitwiseAndAssn) T(BitwiseOrAssn) T(BitwiseXorAssn) \
T(BitwiseNot) T(BitwiseLeftAssn) T(BitwiseRightAssn) \
\
T(If) T(Else) T(For) T(While) T(Do) T(Break) T(Continue)  \
T(Switch) T(Case) T(Default) T(Return) \
T(Static) T(Const) T(Struct) T(Union) T(Enum) \
T(True) T(False) \
T(Identifier) T(StringLiteral) T(IntegerLiteral) T(CharacterLiteral) \
T(Comment) T(PreProcessor)

// TODO: Character literal

enum class TokenType {
    #define T(E) E,
    T_LIST
    #undef T
};

struct TokenLiteralPair { const char* key; TokenType value; };

#define T(K, V) TokenLiteralPair{K, TokenType:: V}
const TokenLiteralPair token_literals[] = { // descending by length
T("...", Variadic),
T("<<=", BitwiseLeftAssn),
T(">>=", BitwiseRightAssn),

T("++", Increment),
T("--", Decrement),
T("+=", PlusAssn),
T("-=", MinusAssn),
T("*=", MultiplyAssn),
T("/=", DivideAssn),
T("%=", ModulusAssn),
T("&=", BitwiseAndAssn),
T("|=", BitwiseOrAssn),
T("^=", BitwiseXorAssn),

T("&&", LogicalAnd),
T("||", LogicalOr),

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
T("/", Divide),
T("*", Multiply),
T("^", BitwiseXor),
T("&", BitwiseAnd),
T("!", LogicalNot),
T("~", BitwiseNot),
T("|", BitwiseOr),
T("?", Question),
T(">", Greater),
T("<", Lesser),
};
const TokenLiteralPair token_keywords[] = {
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
T("enum", Enum),
T("true", True),
T("for", For),
T("do", Do),
T("if", If),
};
#undef T

const size_t token_literals_ct = sizeof(token_literals) / sizeof(token_literals[0]),
             token_keywords_ct = sizeof(token_keywords) / sizeof(token_keywords[0]);

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

    static const char* str_decorator(TokenType t) {
        #define CASE(E) case TokenType:: E 
        switch (t) {
        CASE(Comment):
        CASE(PreProcessor):
            return "\x1b[30;47m"; // black fg on white bg
        CASE(Identifier):
            return "\x1b[36m"; // cyan
        
        // PRIMITIVE STRUCTURE
        CASE(Dot):
        CASE(Comma):
        CASE(Semicolon):
        CASE(Colon):
            return "\x1b[37m";
        CASE(If): CASE(Else):
        CASE(Switch): CASE(Case): CASE(Default):
        CASE(For): CASE(Do): CASE(While): CASE(Continue): CASE(Break): CASE(Return):
        CASE(OCurly): CASE(CCurly):
        CASE(Enum): CASE(Union): CASE(Struct):
        CASE(Const): CASE(Static):
            return "\x1b[35m";

        // OPERATORS;
        CASE(Variadic): CASE(PointMember): CASE(Assignment):
        CASE(OParen): CASE(CParen): CASE(OSquare): CASE(CSquare):
        
        CASE(Increment): CASE(Decrement): 
        CASE(Plus): CASE(Minus): CASE(Modulus): CASE(Divide): CASE(Multiply):
        CASE(PlusAssn): CASE(MinusAssn): CASE(MultiplyAssn): CASE(DivideAssn): CASE(ModulusAssn):

        CASE(BitwiseLeftAssn): CASE(BitwiseRightAssn):
        CASE(BitwiseXor): CASE(BitwiseAnd): CASE(BitwiseOr): CASE(BitwiseNot):
        CASE(BitwiseAndAssn): CASE(BitwiseOrAssn): CASE(BitwiseXorAssn):

        CASE(LogicalNot): CASE(LogicalAnd): CASE(LogicalOr):
        CASE(Question): CASE(Greater): CASE(Lesser): CASE(GreaterEq): CASE(LesserEq): CASE(NotEqual): CASE(Equal):
            return "\x1b[34m";

        // LITERALS
        CASE(StringLiteral): CASE(IntegerLiteral): CASE(True): CASE(False):
            return "\x1b[32m";

        default:
            return "\x1b[0m";
        }
        #undef CASE
    }
    
    void print() const {

        printf("%s:%i:%i: %s%s\x1b[0m %.*s\n", 
            loc.file, loc.row+1, loc.col+1, str_decorator(type), type_str(type),
            srclen, srcptr);
    }
};