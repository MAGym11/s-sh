#include "lexer_types.c"

bool is_whitespace(char character) {
    return character == ' ' || character == '\t';
}

bool is_identifier_char(char character) {
    return (character >= 'a' && character <= 'z') || (character >= 'A' && character <= 'Z') || character == '_';
}

bool is_esc_char(char character) {
    char esc_chars[] = {'0', 'n', '\\', '\'', '"'};
    for (int i = 0; i < 5; i++) {
        if (character == esc_chars[i]) return true;
    }
    return false;
}

char esc_char(char character) {
    switch (character) {
        case '0':
            return 0;
        case 'n':
            return '\n';
        default:
            return character;
    }
}

bool is_valid_char(String string) {
    if (string.length == 1 || string.length == 0) return true;
    if (string.length == 2 && string.ptr[0] == '\\' && is_esc_char(string.ptr[1])) return true;
    return false;
}

bool is_digit(char character) {
    return character >= '0' && character <= '9';
}

void to_integer_token(Token* token) {
    uint64_t int_value = 0;
    uint64_t place_value = 1;
    if (token->string_value.ptr[0] == 'b') {
        if (token->string_value.length > 64) {
            printf("Value error on line %d: integer value too large", token->line_number);
            exit(0);
        }
        for (int i = token->string_value.length-1; i > 0; i--) {
            if (token->string_value.ptr[i] == '1') int_value += place_value;
            place_value *= 2;
        }
    } else if (token->string_value.ptr[0] == 'x') {
        if (token->string_value.length > 17) {
            printf("Value error on line %d: integer value too large", token->line_number);
            exit(0);
        }
        for (int i = token->string_value.length-1; i > 0; i--) {
            if (token->string_value.ptr[i] <= '9') {
                int_value += place_value*(token->string_value.ptr[i]-48);
            } else if (token->string_value.ptr[i] <= 'F') {
                int_value += place_value*(token->string_value.ptr[i]-55);
            } else {
                int_value += place_value*(token->string_value.ptr[i]-87);
            }
            place_value *= 16;
        }
    } else {
        uint64_t prev_value;
        for (int i = token->string_value.length-1; i >= 0; i--) {
            prev_value = int_value;
            int_value += place_value*(token->string_value.ptr[i]-48);
            if (prev_value > int_value) {
                printf("Value error on line %d: integer value too large", token->line_number);
                exit(0);
            }
            place_value *= 10;
        }
    }
    token->int_value = int_value;
}

void to_real_token(Token* token) {
    uint64_t real_value = 0;
    double place_value = 1;
    if (token->string_value.ptr[0] == 'x') {
        String binary_string = {malloc(1), 1, 1};
        *binary_string.ptr = 'b';
        for (int i = 1; i < token->string_value.length; i++) {
            switch (token->string_value.ptr[i]) {
                case '0':
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    break;
                case '1':
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    break;
                case '2':
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    break;
                case '3':
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    break;
                case '4':
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    break;
                case '5':
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    break;
                case '6':
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    break;
                case '7':
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    break;
                case '8':
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    break;
                case '9':
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    break;
                case 'a':
                case 'A':
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    break;
                case 'b':
                case 'B':
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    break;
                case 'c':
                case 'C':
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '0');
                    break;
                case 'd':
                case 'D':
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    String_push(&binary_string, '1');
                    break;
                case 'e':
                case 'E':
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '0');
                    break;
                case 'f':
                case 'F':
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    String_push(&binary_string, '1');
                    break;
                case '.':
                    String_push(&binary_string, '.');
            }
        }
        free(token->string_value.ptr);
        token->string_value.ptr = binary_string.ptr;
        token->string_value.length = binary_string.length;
    }
    if (token->string_value.ptr[0] == 'b') {
        int first_1 = 0;
        int period = 0;
        for (int i = 1; i < token->string_value.length; i++) {
            if (token->string_value.ptr[i] == '1' && !first_1) {
                first_1 = i;
            } else if (token->string_value.ptr[i] == '.') {
                period = i;
            } else if (token->string_value.ptr[i] == '1') {
                if (!period) {
                    real_value |= ((uint64_t)1 << (52 - i + first_1));
                } else {
                    real_value |= ((uint64_t)1 << (53 - i + first_1));
                }
            }
        }
        uint64_t exponent = 0x3ff + period - first_1;
        if (first_1 < period) exponent--;
        exponent <<= 52;
        real_value |= exponent;

        drop_String(token->string_value);
    } else {
        sscanf(token->string_value.ptr, "%lf", &real_value); 
    }
    token->real_value = *((double*)(&real_value));
}

void underscore_check(Token* token) {
    if (token->string_value.length == 1 && token->string_value.ptr[0] == '_')
        token->type = underscore;
}

bool handle_character(char curr_char, char* prev_char, int* line_number, Tokens* tokens, char* in_string, bool* in_esc, char* in_comment) {
    if (curr_char == EOF) return false;

    if (*in_comment) {
        if (*in_comment == 1 && curr_char == '\n') {
            *in_comment = 0;
        } else if (curr_char == '\n') {
            (*line_number)++;
        }
        if (*in_comment == 2 && curr_char == '*') {
            *in_comment = 3;
            return true;
        } else if (*in_comment == 3 && curr_char == '/') {
            *in_comment = 0;
        } else if (*in_comment == 3) {
            *in_comment = 2;
            return true;
        } else {
            return true;
        }
    }

    if (*in_string) {
        if (*in_esc) {
            if (is_esc_char(curr_char)) {
                *in_esc = false;
                if (*in_string == 2)
                    String_push(&(last_Token(*tokens)->string_value), esc_char(curr_char));
                else if (*in_string == 1)
                    last_Token(*tokens)->int_value = esc_char(curr_char);
                *prev_char = curr_char;
            } else {
                printf("Invalid escape character: %c\n", curr_char);
                return false;
            }
        } else if (curr_char == '\\') {
            *in_esc = true;
        } else if ((curr_char == '\'' && *in_string == 1) || (curr_char == '"' && *in_string == 2)) {
            *in_string = 0;
            *prev_char = curr_char;
        } else if (*in_string == 2) {
            String_push(&(last_Token(*tokens)->string_value), curr_char);
            *prev_char = curr_char;
        } else if (*in_string == 1) {
            last_Token(*tokens)->int_value = curr_char;
        }
        if (*in_string == 1 && !is_valid_char(tokens->ptr[tokens->length-1].string_value)) {
            printf("Character must be of length 1\n");
            exit(0);
        }
        return true;
    }
    
    if (is_whitespace(curr_char)) {
        *prev_char = curr_char;
        return true;
    }

    Token temp_token;
    temp_token.line_number = *line_number;

    String temp_value = {malloc(1), 1, 1};
    *temp_value.ptr = curr_char;
    temp_token.string_value = temp_value;

    if (is_identifier_char(curr_char)) {
        if ((curr_char == 'b' || curr_char == 'x') && tokens->ptr[tokens->length-1].type == integer && tokens->ptr[tokens->length-1].string_value.length >= 1) {
            if (tokens->ptr[tokens->length-1].string_value.ptr[0] == '0') {
                tokens->ptr[tokens->length-1].string_value.ptr[0] = curr_char;
                *prev_char = curr_char;
                return true;
            }
        }
        if ((curr_char >= 'a' && curr_char <= 'f') || (curr_char >= 'A' && curr_char <= 'F')) {
            if ((tokens->ptr[tokens->length-1].type == integer || tokens->ptr[tokens->length-1].type == real) && tokens->ptr[tokens->length-1].string_value.ptr[0] == 'x' && !is_whitespace(*prev_char)) {
                goto IS_DIGIT;
            }
        }
        if (is_identifier_char(*prev_char)) {
            String_push(&(last_Token(*tokens)->string_value), curr_char);
        } else {
            temp_token.type = identifier;
            add_token(tokens, temp_token);
        }
        *prev_char = curr_char;
        return true;
    }

    if (is_digit(curr_char)) {
        IS_DIGIT:
        if (is_digit(*prev_char) || is_identifier_char(*prev_char)) {
            String_push(&(last_Token(*tokens)->string_value), curr_char);
        } else if (!is_whitespace(*prev_char) && last_Token(*tokens)->type == real) {
            String_push(&(last_Token(*tokens)->string_value), curr_char);
            tokens->ptr[tokens->length-1].type = real;
        } else {
            temp_token.type = integer;
            add_token(tokens, temp_token);
        }
        if (tokens->ptr[tokens->length-1].string_value.ptr[0] == 'b' && curr_char > '1') {
            printf("Invalid binary number: '%c' on line %d\n", curr_char, *line_number);
        }
        *prev_char = curr_char;
        return true;
    }

    switch (curr_char) {
        case '`':
            temp_token.type = backTick;
            break;
        case '~':
            temp_token.type = tilde;
            break;
        case '!':
            temp_token.type = exclamation;
            break;
        case '@':
            temp_token.type = atSign;
            break;
        case '#':
            temp_token.type = hash;
            break;
        case '%':
            temp_token.type = percent;
            break;
        case '^':
            temp_token.type = karat;
            break;
        case '&':
            temp_token.type = ampersand;
            break;
        case '*':
            if (*prev_char == '/') {
                *in_comment = 2;
                if (last_Token(*tokens)->type == fSlash) {
                    drop_Token(*last_Token(*tokens));
                    tokens->length--;
                }
                return true;
            }
            temp_token.type = asterisk;
            break;
        case '(':
            temp_token.type = oParenthesis;
            break;
        case ')':
            temp_token.type = cParenthesis;
            break;
        case '[':
            temp_token.type = oBracket;
            break;
        case ']':
            temp_token.type = cBracket;
            break;
        case '{':
            temp_token.type = oBrace;
            break;
        case '}':
            temp_token.type = cBrace;
            break;
        case '-':
            if (last_Token(*tokens)->type == minus) {
                last_Token(*tokens)->type = dec;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            } else if (last_Token(*tokens)->type == lessThan) {
                last_Token(*tokens)->type = return_arrow;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            }
            temp_token.type = minus;
            break;
        case '+':
            if (last_Token(*tokens)->type == plus) {
                last_Token(*tokens)->type = inc;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            }
            temp_token.type = plus;
            break;
        case '=':
            switch (last_Token(*tokens)->type) {
                case equal:
                    last_Token(*tokens)->type = compareEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
                case plus:
                    last_Token(*tokens)->type = plusEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
                case minus:
                    last_Token(*tokens)->type = minusEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
                case asterisk:
                    last_Token(*tokens)->type = multEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
                case fSlash:
                    last_Token(*tokens)->type = divEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
                case percent:
                    last_Token(*tokens)->type = modEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
                case lShift:
                    last_Token(*tokens)->type = lShiftEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
                case lessThan:
                    last_Token(*tokens)->type = lessThanEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
                case greaterThan:
                    last_Token(*tokens)->type = greaterThanEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
                case pipe:
                    last_Token(*tokens)->type = orEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
                case karat:
                    last_Token(*tokens)->type = xorEqual;
                    String_push(&(last_Token(*tokens)->string_value), curr_char);
                    *prev_char = curr_char;
                    return true;
            }
            temp_token.type = equal;
            break;
        case '/':
            if (*prev_char == '/') {
                *in_comment = 1;
                if (last_Token(*tokens)->type == fSlash) {
                    drop_Token(*last_Token(*tokens));
                    tokens->length--;
                }
                return true;
            }
            temp_token.type = fSlash;
            break;
        case '|':
            if (last_Token(*tokens)->type == pipe) {
                last_Token(*tokens)->type = or;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            }
            temp_token.type = pipe;
            break;
        case ':':
            if (last_Token(*tokens)->type == colon) {
                last_Token(*tokens)->type = double_colon;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            }
            temp_token.type = colon;
            break;
        case ';':
            temp_token.type = semicolon;
            break;
        case '<':
            if (last_Token(*tokens)->type == lessThan) {
                last_Token(*tokens)->type = lShift;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            }
            temp_token.type = lessThan;
            break;
        case '>':
            if (last_Token(*tokens)->type == equal) {
                last_Token(*tokens)->type = imply_arrow;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            } else if (last_Token(*tokens)->type == minus) {
                last_Token(*tokens)->type = jmp_arrow;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            } else if (last_Token(*tokens)->type == lessThan) {
                last_Token(*tokens)->type = defer_symbol;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            } else if (last_Token(*tokens)->type == colon) {
                last_Token(*tokens)->type = colon_arrow;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            } else if (last_Token(*tokens)->type == double_colon) {
                last_Token(*tokens)->type = double_colon_arrow;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            }
            temp_token.type = greaterThan;
            break;
        case '?':
            temp_token.type = question;
            break;
        case ',':
            temp_token.type = comma;
            break;
        case '.':
            if ((is_digit(*prev_char) || (*prev_char >= 'a' && *prev_char <= 'f') || (*prev_char >= 'A' && *prev_char <= 'F')) && last_Token(*tokens)->type == integer) {
                last_Token(*tokens)->type = real;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                drop_Token(temp_token);
            } else if (last_Token(*tokens)->type == period) {
                last_Token(*tokens)->type = double_period;
                String_push(&(last_Token(*tokens)->string_value), curr_char);
                *prev_char = curr_char;
                return true;
            } else {
                temp_token.type = period;
                add_token(tokens, temp_token);
            }
            *prev_char = curr_char;
            return true;
        case '\'':
            temp_token.type = character;
            *in_string = 1;
            temp_token.string_value.length = 0;
            add_token(tokens, temp_token);
            *prev_char = curr_char;
            return true;
        case '"':
            temp_token.type = string;
            *in_string = 2;
            temp_token.string_value.length = 0;
            add_token(tokens, temp_token);
            *prev_char = curr_char;
            return true;
        case '\n':
            temp_token.type = newLine;
            (*line_number)++;
            break;
        default:
            printf("Unknown character: %c\n", curr_char);
            return false;

    }
    add_token(tokens, temp_token);
    *prev_char = curr_char;
    return true;
}

void print_Token(Token token) {
    printf("{ type:%d, value:'", token.type);
    if (token.type != integer && token.type != real)
        print_String(token.string_value);
    else if (token.type == integer)
        printf("%d", token.int_value);
    else
        printf("%lf", token.real_value);

    printf("', line_num: %d }\n", token.line_number);
}

void tokenise(Tokens* tokens, FILE* file) {
    int line_number = 1;
    char curr_char;
    char prev_char = EOF;
    char in_string = 0;
    bool in_esc = false;
    char in_comment = 0;

    new_Tokens(tokens);

    do {
        curr_char = fgetc(file);
    } while (handle_character(curr_char, &prev_char, &line_number, tokens, &in_string, &in_esc, &in_comment));

    for (int i = 0; i < tokens->length; i++) {
        if (tokens->ptr[i].type == integer) {
            to_integer_token(&(tokens->ptr[i]));
        }
        if (tokens->ptr[i].type == real) {
            to_real_token(&(tokens->ptr[i]));
        }
        
        if (tokens->ptr[i].type == identifier) {
            underscore_check(&(tokens->ptr[i]));
        }
    }

    /*
    for (int i = 0; i < tokens->length; i++) {
        printf("%d: ", i);
        print_Token(tokens->ptr[i]);
    }
    //*/
}
