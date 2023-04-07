#include <cassert>
#include <cctype>
#include <cstring>
#include <iostream>

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

const char* token_type_str(TokenType t) {
    #define T(E) case TokenType:: E : return #E ;
    switch (t) { T_LIST }
    #undef T
    assert(false);
    return "Bad token type str";
}

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
};

class Lexer {
    const char* file_path;
    char* fdata;
    long fsize, pos, bol, row;
    
    bool inside_block_comment = false;

    bool equal(const char* key) const {
        return strncmp(key,fdata + pos, strlen(key)) == 0;
    }

    inline char peek() const {
        return fdata[pos];
    }
    inline char chop() {
        assert(has_next());
        char x = peek();
        ++pos;
        if (x == '\n') {
            bol = pos;
            ++row;
        }
        return x;
    }
    void chop(size_t ct) {
        for (size_t i=0;i<ct;++i) chop();
    }
    void trim_left() {
        while (has_next() && isspace(peek())) chop();
    }
public:
    Lexer(const char* file_path) : file_path(file_path), pos(0), bol(0), row(0), fsize(-1), fdata(nullptr) {
        FILE* file = fopen(file_path,"r");
        fseek(file,0,SEEK_END);
        fsize = ftell(file);
        rewind(file);

        fdata = new char[fsize+1];
        size_t sz = fread(fdata, sizeof(char), fsize, file);
        if (ferror(file)) {
            fclose(file);
            printf("Couldn't read \"%s\".\n", file_path);
            exit(1);
        }
        fclose(file);
        fdata[fsize] = '\0';
    }
    ~Lexer() {
        if (fdata) delete[] fdata;
    }

    inline bool has_next() const {
        return pos < fsize;
    }
    Loc getloc() const {
        return Loc{file_path, row, pos-bol};
    }
    Token* next_token() {
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
};

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