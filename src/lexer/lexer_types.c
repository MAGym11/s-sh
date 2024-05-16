typedef enum Token_Type {
    identifier, integer, real, string, character, newLine,
    backTick, tilde, exclamation, atSign, hash, percent,
    karat, ampersand, asterisk,
    oParenthesis, cParenthesis, oBracket, cBracket, oBrace, cBrace,
    minus, plus, equal, fSlash, pipe,
    colon, semicolon, lessThan, greaterThan, question,
    comma, period, underscore,
    compareEqual, inc, dec,
    plusEqual, minusEqual, multEqual, divEqual, modEqual, lShiftEqual,
    orEqual, xorEqual, lessThanEqual, greaterThanEqual,
    or, lShift,
    imply_arrow, return_arrow,
    jmp_arrow, defer_symbol,
    double_colon,
    double_period,
    colon_arrow,
    double_colon_arrow
} Token_Type;

typedef struct {
    Token_Type type;
    union {
        String string_value;
        uint64_t int_value;
        double real_value;
    };
    int line_number;
} Token;

typedef struct {
    Token* ptr;
    size_t length;
    size_t capacity;
} Tokens;

void increase_tokens_capacity(Tokens* tokens) {
    if (tokens->capacity == 0) tokens->capacity++;
    tokens->capacity *= 2;
    void* tempPtr = malloc(tokens->capacity*sizeof(Token));
    memcpy(tempPtr, tokens->ptr, tokens->length*sizeof(Token));
    free(tokens->ptr);
    tokens->ptr = tempPtr;
}

void drop_Token(Token token) {
    if (token.type != integer && token.type != real) {
        drop_String(token.string_value);
    }
}

void drop_last_Token(Tokens* tokens) {
    if (tokens->ptr[tokens->length-1].type != integer && tokens->ptr[tokens->length-1].type != real) {
        drop_String(tokens->ptr[tokens->length-1].string_value);
    }
    (tokens->length)--;
}

void add_token(Tokens* tokens, Token token) {
    if (tokens->length == tokens->capacity)
        increase_tokens_capacity(tokens);
    tokens->ptr[tokens->length] = token;
    tokens->length++;
}

void new_Tokens(Tokens* tokens) {
    tokens->length = 0;
    tokens->capacity = 1;
    tokens->ptr = malloc(sizeof(Token));
}

void drop_Tokens(Tokens tokens) {
    for (int i = 0; i < tokens.length; i++) {
        drop_last_Token(&tokens);
    }
}

Token* last_Token(Tokens tokens) {
    if (tokens.length > 0) return &(tokens.ptr[tokens.length-1]);
}
