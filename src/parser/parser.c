#include "parser_types.c"
#include "print_ast.c"

bool parse_expr_0(Expr* expr, Tokens tokens, int* pos);
bool parse_expr_2(Expr* expr, Tokens tokens, int* pos);
bool parse_exprs(Array* exprs, Tokens tokens, int* pos);
bool parse_stmt(Stmt* stmt, Tokens tokens, int* pos);
bool parse_type(Expr* expr, Tokens tokens, int* pos);

void parse_error(Tokens tokens, int temp_pos, const char* expected) {
    if (tokens.ptr[temp_pos].type != integer && tokens.ptr[temp_pos].type != real) {
        printf("Unexpected token on line %d: '", tokens.ptr[temp_pos].line_number);
        print_String(tokens.ptr[temp_pos].string_value);
        if (temp_pos) {
            if (tokens.ptr[temp_pos-1].type != integer && tokens.ptr[temp_pos-1].type != real) {
                printf("', after '");
                print_String(tokens.ptr[temp_pos-1].string_value);
            }
        }
        printf("', expected %s\n", expected);
    } else {
        printf("Unexpected token on line %d, expected %s\n", tokens.ptr[temp_pos].line_number, expected);
    }
    exit(0);
}
void semantic_error(Tokens tokens, int temp_pos, const char* message) {
    printf("Semantic error on line %d, %s\n", tokens.ptr[temp_pos].line_number, message);
    exit(0);
}

bool parse_token(Token_Type target_type, Tokens tokens, int* pos) {
    int temp_pos = *pos;

    while (tokens.ptr[temp_pos].type != target_type) {
        if (tokens.ptr[temp_pos].type != newLine) return false;
        temp_pos++;
    }
    
    *pos = temp_pos + 1;
    return true;
}

bool parse_module(Stmt* stmt, Tokens tokens, int* pos) {
    Stmt temp_stmt1;
    temp_stmt1.stmts.length = 0; temp_stmt1.stmts.capacity = 0; temp_stmt1.stmts.ptr = 0;
    Stmt temp_stmt2;

    int temp_pos = *pos;
    
    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    temp_stmt1.id.id_type = modu;
    temp_stmt1.id.name = tokens.ptr[temp_pos-1].string_value;
    if (!parse_token(colon_arrow, tokens, &temp_pos)) goto parse_import_ret;
    if (!parse_module(&temp_stmt2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "identifier");
    Stmts_push(&temp_stmt1.stmts, temp_stmt2);

parse_import_ret:
    temp_stmt1.stmt_type = module_access;

    *stmt = temp_stmt1;

    *pos = temp_pos;
    return true;
}

bool parse_import(Stmt* stmt, Tokens tokens, int* pos) {
    Stmt temp_stmt1;
    temp_stmt1.stmts.length = 0; temp_stmt1.stmts.capacity = 0; temp_stmt1.stmts.ptr = 0;
    Stmt temp_stmt2;

    int temp_pos = *pos;

    if (!parse_token(atSign, tokens, &temp_pos)) return false;
    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    temp_stmt1.id.id_type = lib;
    temp_stmt1.id.name = tokens.ptr[temp_pos-1].string_value;
    if (!parse_token(double_colon_arrow, tokens, &temp_pos)) goto parse_import_ret;
    if (!parse_module(&temp_stmt2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "identifier");
    Stmts_push(&temp_stmt1.stmts, temp_stmt2);

parse_import_ret:
    temp_stmt1.stmt_type = import;

    *stmt = temp_stmt1;

    *pos = temp_pos;
    return true;
}

bool parse_gnrc(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr;

    int temp_pos = *pos;

    if (!parse_token(lessThan, tokens, &temp_pos)) return false;
    if (!parse_type(&temp_expr, tokens, &temp_pos)) return false;
    if (!parse_token(greaterThan, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'>'");

    *expr = temp_expr;
    
    *pos = temp_pos;
    return true;
}

bool parse_cust_type(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    temp_expr1.exprs.ptr = 0; temp_expr1.exprs.capacity = 0; temp_expr1.exprs.length = 0;
    Expr temp_expr2;

    int temp_pos = *pos;

    if (parse_token(underscore, tokens, &temp_pos)) {
        temp_expr1.expr_type = undrscr;
        temp_expr1.type.type_type = null;

        *expr = temp_expr1;

        *pos = temp_pos;
        return true;
    }

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    temp_expr1.id_value.id_type = type_name;
    temp_expr1.id_value.name = tokens.ptr[temp_pos-1].string_value;

    if (parse_gnrc(&temp_expr2, tokens, &temp_pos)) {
        Exprs_push(&temp_expr1.exprs, temp_expr2);
        temp_expr1.expr_type = type_gnrc;
    } else
        temp_expr1.expr_type = type_n_gnrc;

    temp_expr1.type.type_type = custom;
    temp_expr1.type.value = temp_expr1.id_value.name;

    *expr = temp_expr1;
    
    *pos = temp_pos;
    return true;
}

bool parse_type(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr;
    Expr temp_expr1;
    bool temp_mut;
    temp_expr.exprs.capacity = 0; temp_expr.exprs.length = 0; temp_expr.exprs.ptr = 0;
    temp_expr.type.type_type = undefined;

    int temp_pos = *pos;

    if (parse_token(exclamation, tokens, &temp_pos))
        temp_mut = true;
    else
        temp_mut = false;

    if (parse_token(ampersand, tokens, &temp_pos)) {
        temp_expr.expr_type = address;
        temp_expr.type.type_type = addr;
        if (parse_type(&temp_expr1, tokens, &temp_pos)) {
            if (temp_expr1.type.type_type == null) semantic_error(tokens, temp_pos, "invalid type: address of null type");
        } else {
            temp_expr1.expr_type = none;
            temp_expr1.type.type_type = undefined;
            if (parse_token(exclamation, tokens, &temp_pos))
                temp_expr1.mut = true;
            else
                temp_expr1.mut = false;
        }
        Exprs_push(&temp_expr.exprs, temp_expr1);
    } else {
        if (!parse_cust_type(&temp_expr, tokens, &temp_pos)) return false;
        if (temp_expr.type.type_type == null && temp_mut) semantic_error(tokens, temp_pos, "null type cannot be mutable");
    }

    temp_expr.mut = temp_mut;

    *expr = temp_expr;

    *pos = temp_pos;
    return true;
}

bool parse_ret_type(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr;

    int temp_pos = *pos;

    if (!parse_type(&temp_expr, tokens, &temp_pos)) return false;

    *expr = temp_expr;
    
    *pos = temp_pos;
    return true;
}

bool parse_else(Array* stmts, Tokens tokens, int* pos) {
    Stmt temp_stmt;

    int temp_pos = *pos;

    if (!parse_token(underscore, tokens, &temp_pos)) return false;
    if (!parse_token(imply_arrow, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'=>'");
    if (!parse_stmt(&temp_stmt, tokens, &temp_pos)) parse_error(tokens, temp_pos, "statement");
    Stmts_push(stmts, temp_stmt);

    *pos = temp_pos;
    return true;
}

bool parse_defer(Stmt* stmt, Tokens tokens, int* pos) {
    Stmt temp_stmt1;
    temp_stmt1.stmts.capacity = 0; temp_stmt1.stmts.length = 0; temp_stmt1.stmts.ptr = 0;
    Stmt temp_stmt2;

    int temp_pos = *pos;

    if (!parse_token(defer_symbol, tokens, &temp_pos)) return false;
    if (!parse_stmt(&temp_stmt2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "statement");
    Stmts_push(&temp_stmt1.stmts, temp_stmt2);

    temp_stmt1.stmt_type = defer;

    *stmt = temp_stmt1;

    *pos = temp_pos;
    return true;
}

bool parse_jump(Stmt* stmt, Tokens tokens, int* pos) {
    Stmt temp_stmt;

    int temp_pos = *pos;

    if (!parse_token(jmp_arrow, tokens, &temp_pos)) return false;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    temp_stmt.id.id_type = jmp;
    temp_stmt.id.name = tokens.ptr[temp_pos-1].string_value;
    temp_stmt.stmt_type = jump;

    *stmt = temp_stmt;

    *pos = temp_pos;
    return true;
}

bool parse_ret(Stmt* stmt, Tokens tokens, int* pos) {
    Stmt temp_stmt;
    Expr temp_expr;

    int temp_pos = *pos;

    if (!parse_token(return_arrow, tokens, &temp_pos)) return false;
    temp_expr.expr_type = none;
    if (!parse_expr_2(&temp_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
    Exprs_push(&temp_stmt.exprs, temp_expr);

    temp_stmt.stmt_type = ret;

    *stmt = temp_stmt;
    
    *pos = temp_pos;
    return true;
}

bool parse_loop(Stmt* stmt, Tokens tokens, int* pos) {
    Stmt temp_stmt1;
    Expr temp_expr;
    Stmt temp_stmt2;
    temp_stmt1.exprs.ptr = 0; temp_stmt1.exprs.length = 0; temp_stmt1.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_token(question, tokens, &temp_pos)) return false;
    if (!parse_expr_2(&temp_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
    Exprs_push(&temp_stmt1.exprs, temp_expr);
    if (!parse_stmt(&temp_stmt2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "statement");
    Stmts_push(&temp_stmt1.stmts, temp_stmt2);

    temp_stmt1.stmt_type = loop;

    *stmt = temp_stmt1;

    *pos = temp_pos;
    return true;
}

bool parse_stmts(Array* stmts, Tokens tokens, int* pos) {
    Stmt temp_stmt;

    int temp_pos = *pos;

    if (!parse_stmt(&temp_stmt, tokens, &temp_pos)) return false;
    Stmts_push(stmts, temp_stmt);

    if (!parse_token(newLine, tokens, &temp_pos) && !parse_token(semicolon, tokens, &temp_pos)) {
        goto parse_stmts_ret;
    }
    parse_stmts(stmts, tokens, &temp_pos);

parse_stmts_ret:
    *pos = temp_pos;
    return true;
}

bool parse_cmp_stmt(Stmt* stmt, Tokens tokens, int* pos) {
    Stmt temp_stmt;
    temp_stmt.stmts.ptr = 0; temp_stmt.stmts.length = 0; temp_stmt.stmts.capacity = 0; 

    int temp_pos = *pos;

    if (!parse_token(oBrace, tokens, &temp_pos)) return false;
    parse_stmts(&temp_stmt.stmts, tokens, &temp_pos);
    if (!parse_token(cBrace, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'}' or '\\n' or ';'");

    temp_stmt.stmt_type = cmp_stmt;
    *stmt = temp_stmt;

    *pos = temp_pos;
    return true;
}

bool parse_stmt(Stmt* stmt, Tokens tokens, int* pos) {
    Stmt temp_stmt;
    Expr temp_expr;
    temp_stmt.exprs.ptr = 0; temp_stmt.exprs.length = 0; temp_stmt.exprs.capacity = 0; 

    int temp_pos = *pos;

    if (parse_cmp_stmt(&temp_stmt, tokens, &temp_pos)) goto parse_stmt_ret;
    if (parse_loop(&temp_stmt, tokens, &temp_pos)) goto parse_stmt_ret;
    if (parse_ret(&temp_stmt, tokens, &temp_pos)) goto parse_stmt_ret;
    if (parse_jump(&temp_stmt, tokens, &temp_pos)) goto parse_stmt_ret;
    if (parse_defer(&temp_stmt, tokens, &temp_pos)) goto parse_stmt_ret;
    if (parse_expr_0(&temp_expr, tokens, &temp_pos)) {
        temp_stmt.stmt_type = expr;
        Exprs_push(&temp_stmt.exprs, temp_expr);
        goto parse_stmt_ret;
    }
    if (parse_import(&temp_stmt, tokens, &temp_pos)) goto parse_stmt_ret;
    if (parse_token(semicolon, tokens, &temp_pos)) {
        temp_stmt.stmt_type = nop;
        goto parse_stmt_ret;
    }

    return false;

parse_stmt_ret:
    *stmt = temp_stmt;

    *pos = temp_pos;
    return true;
}

bool parse_label(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr;

    int temp_pos = *pos;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    temp_expr.id_value.id_type = label;
    temp_expr.id_value.name = tokens.ptr[temp_pos-1].string_value;

    if (!parse_token(colon, tokens, &temp_pos)) return false;

    temp_expr.expr_type = label_expr;
    temp_expr.type.type_type = undefined;

    *expr = temp_expr;

    *pos = temp_pos;
    return true;
}

bool parse_structure_n_type(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr;
    temp_expr.exprs.ptr = 0; temp_expr.exprs.length = 0; temp_expr.exprs.capacity = 0; 

    int temp_pos = *pos;

    if (!parse_token(oBrace, tokens, &temp_pos)) return false;
    if (!parse_exprs(&temp_expr.exprs, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expressions");
    if (!parse_token(cBrace, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'}'");

    temp_expr.expr_type = structure_n_type;
    temp_expr.type.type_type = mem;

    *expr = temp_expr;

    *pos = temp_pos;
    return true;
}

bool parse_structure(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    temp_expr1.exprs.ptr = 0; temp_expr1.exprs.length = 0; temp_expr1.exprs.capacity = 0; 
    temp_expr1.structure_type.ptr = 0; temp_expr1.structure_type.length = 0; temp_expr1.structure_type.capacity = 0; 
    Expr temp_expr2;

    int temp_pos = *pos;

    if (!parse_cust_type(&temp_expr2, tokens, &temp_pos)) return false;
    if (!parse_token(oBrace, tokens, &temp_pos)) return false;
    if (!parse_exprs(&temp_expr1.exprs, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expressions");
    if (!parse_token(cBrace, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'}'");

    temp_expr1.expr_type = structure;
    temp_expr1.type.type_type = undefined;
    Exprs_push(&temp_expr1.structure_type, temp_expr2);

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_func_call(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr;
    temp_expr.exprs.ptr = 0; temp_expr.exprs.length = 0; temp_expr.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    temp_expr.id_value.id_type = func_name;
    temp_expr.id_value.name = tokens.ptr[temp_pos-1].string_value;

    if (!parse_token(oParenthesis, tokens, &temp_pos)) return false;
    parse_exprs(&temp_expr.exprs, tokens, &temp_pos);
    if (!parse_token(cParenthesis, tokens, &temp_pos)) parse_error(tokens, temp_pos, "')'");

    temp_expr.expr_type = func_call;
    temp_expr.type.type_type = undefined;

    *expr = temp_expr;

    *pos = temp_pos;
    return true;
}

bool parse_deref(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    temp_expr2.exprs.ptr = 0; temp_expr2.exprs.length = 0; temp_expr2.exprs.capacity = 0; 

    int temp_pos = *pos;

    if (!parse_token(oBracket, tokens, &temp_pos)) return false;
    if (!parse_expr_2(&temp_expr1, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
    if (!parse_token(cBracket, tokens, &temp_pos)) parse_error(tokens, temp_pos, "']'");

    temp_expr2.expr_type = deref;
    Exprs_push(&temp_expr2.exprs, temp_expr1);
    temp_expr2.type.type_type = undefined;

    *expr = temp_expr2;

    *pos = temp_pos;
    return true;
}

bool parse_parentheses(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    temp_expr1.exprs.ptr = 0; temp_expr1.exprs.length = 0; temp_expr1.exprs.capacity = 0; 
    Expr temp_expr2;

    int temp_pos = *pos;

    if (!parse_token(oParenthesis, tokens, &temp_pos)) return false;
    if (!parse_expr_2(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
    if (!parse_token(cParenthesis, tokens, &temp_pos)) parse_error(tokens, temp_pos, "')'");
    Exprs_push(&temp_expr1.exprs, temp_expr2);
    temp_expr1.expr_type = paren;

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_g(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr;
    
    int temp_pos = *pos;

    if (parse_parentheses(&temp_expr, tokens, &temp_pos)) goto parse_expr_g_ret;
    if (parse_deref(&temp_expr, tokens, &temp_pos)) {
        goto parse_expr_g_ret;
    }
    if (parse_func_call(&temp_expr, tokens, &temp_pos)) goto parse_expr_g_ret;
    if (parse_structure(&temp_expr, tokens, &temp_pos)) goto parse_expr_g_ret;
    if (parse_structure_n_type(&temp_expr, tokens, &temp_pos)) goto parse_expr_g_ret;
    if (parse_token(identifier, tokens, &temp_pos)) {
        temp_expr.expr_type = id;
        temp_expr.id_value.id_type = var;
        temp_expr.id_value.name = tokens.ptr[temp_pos-1].string_value;
        temp_expr.type.type_type = undefined;
        goto parse_expr_g_ret;
    }
    if (parse_token(integer, tokens, &temp_pos)) {
        temp_expr.expr_type = in;
        temp_expr.int_value = tokens.ptr[temp_pos-1].int_value;
        if (tokens.ptr[temp_pos-1].int_value <= 0x7fffffff) {
            temp_expr.type.type_type = custom;
            to_String("i32", &temp_expr.type.value);
        } else if (tokens.ptr[temp_pos-1].int_value <= 0x7fffffffffffffff) {
            temp_expr.type.type_type = custom;
            to_String("i64", &temp_expr.type.value);
        } else if (tokens.ptr[temp_pos-1].int_value <= 0xffffffffffffffff) {
            temp_expr.type.type_type = custom;
            to_String("u64", &temp_expr.type.value);
        }
        goto parse_expr_g_ret;
    }
    if (parse_token(real, tokens, &temp_pos)) {
        temp_expr.expr_type = re;
        temp_expr.real_value = tokens.ptr[temp_pos-1].real_value;
        temp_expr.type.type_type = custom;
        to_String("f64", &temp_expr.type.value);
        goto parse_expr_g_ret;
    }
    if (parse_token(string, tokens, &temp_pos)) {
        temp_expr.expr_type = str;
        temp_expr.str_value = tokens.ptr[temp_pos-1].string_value;
        temp_expr.type.type_type = mem;
        goto parse_expr_g_ret;
    }
    if (parse_token(character, tokens, &temp_pos)) {
        temp_expr.expr_type = cha;
        temp_expr.int_value = tokens.ptr[temp_pos-1].int_value;
        temp_expr.type.type_type = custom;
        to_String("u8", &temp_expr.type.value);
        goto parse_expr_g_ret;
    }
    if (parse_token(underscore, tokens, &temp_pos)) {
        temp_expr.expr_type = undrscr;
        temp_expr.str_value = tokens.ptr[temp_pos-1].string_value;
        temp_expr.type.type_type = null;
        goto parse_expr_g_ret;
    }
    return false;

parse_expr_g_ret:
    *expr = temp_expr;

    *pos = temp_pos;
    return true;
}

bool parse_expr_f(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_g(&temp_expr1, tokens, &temp_pos)) return false;
    if (!parse_token(colon, tokens, &temp_pos)) goto parse_expr_f_ret;
    if (!parse_type(&temp_expr2, tokens, &temp_pos)) return false;

    Exprs_push(&temp_expr3.exprs, temp_expr1);
    Exprs_push(&temp_expr3.exprs, temp_expr2);
    temp_expr1 = temp_expr3;

    temp_expr1.expr_type = cast;
    temp_expr1.type.type_type = undefined;

parse_expr_f_ret:
    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_gnrc_dec(Id* generic, bool* has_generic, Tokens tokens, int* pos) {
    *has_generic = false;

    Id temp_generic;

    int temp_pos = *pos;

    if (!parse_token(lessThan, tokens, &temp_pos)) return false;
    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    temp_generic.id_type = gnrc_name;
    temp_generic.name = tokens.ptr[temp_pos-1].string_value;
    if (!parse_token(greaterThan, tokens, &temp_pos)) return false;

    *generic = temp_generic;
    *has_generic = true;

    *pos = temp_pos;
    return true;
}

bool parse_dot_op(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_f(&temp_expr1, tokens, &temp_pos)) return false;

    for (;;) {
        if (!parse_token(period, tokens, &temp_pos)) goto parse_dot_op_loop_end;
        temp_expr3.expr_type = dot_op;

        if (!parse_func_call(&temp_expr2, tokens, &temp_pos)) {
            if (!parse_token(identifier, tokens, &temp_pos)) parse_error(tokens, temp_pos, "function call or identifier");
            temp_expr2.expr_type = id;
            temp_expr2.type.type_type = undefined;
            temp_expr2.id_value.id_type = enum_value;
            temp_expr2.id_value.name = tokens.ptr[temp_pos-1].string_value;
        }
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr1.type.type_type = undefined;

        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
    }
parse_dot_op_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_double_colon_op(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    temp_expr1.exprs.ptr = 0; temp_expr1.exprs.length = 0; temp_expr1.exprs.capacity = 0;
    Expr temp_expr2;

    int temp_pos = *pos;

    if (!parse_cust_type(&temp_expr2, tokens, &temp_pos)) return false;
    Exprs_push(&temp_expr1.exprs, temp_expr2);

    if (!parse_token(double_colon, tokens, &temp_pos)) return false;

    if (!parse_func_call(&temp_expr2, tokens, &temp_pos)) {
        if (!parse_token(identifier, tokens, &temp_pos)) parse_error(tokens, temp_pos, "function call or identifier");
        temp_expr2.expr_type = id;
        temp_expr2.type.type_type = undefined;
        temp_expr2.id_value.id_type = enum_value;
        temp_expr2.id_value.name = tokens.ptr[temp_pos-1].string_value;
    }
    Exprs_push(&temp_expr1.exprs, temp_expr2);
    
    temp_expr1.expr_type = double_colon_op;
    temp_expr1.type.type_type = undefined;

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_e(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;

    int temp_pos = *pos;

    if (parse_double_colon_op(&temp_expr1, tokens, &temp_pos)) goto parse_expr_e_ret;
    if (parse_dot_op(&temp_expr1, tokens, &temp_pos)) goto parse_expr_e_ret;
    return false;

parse_expr_e_ret:
    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_d(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    temp_expr1.exprs.ptr = 0; temp_expr1.exprs.length = 0; temp_expr1.exprs.capacity = 0;

    int temp_pos = *pos;

    if (parse_token(tilde, tokens, &temp_pos)) {
        temp_expr1.expr_type = bNot;
    } else if (parse_token(exclamation, tokens, &temp_pos)) {
        temp_expr1.expr_type = not;
    } else if (parse_token(ampersand, tokens, &temp_pos)) {
        temp_expr1.expr_type = address;
    } else if (parse_token(minus, tokens, &temp_pos)) {
        temp_expr1.expr_type = negate;
    } else if (parse_token(inc, tokens, &temp_pos)) {
        temp_expr1.expr_type = preInc;
    } else if (parse_token(dec, tokens, &temp_pos)) {
        temp_expr1.expr_type = preDec;
    } else {
        goto parse_expr_d;
    }
    if (!parse_expr_e(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
    goto parse_expr_d_ret;
parse_expr_d:
    if (!parse_expr_e(&temp_expr2, tokens, &temp_pos)) return false;
    if (parse_token(inc, tokens, &temp_pos)) {
        temp_expr1.expr_type = postInc;
    } else if (parse_token(dec, tokens, &temp_pos)) {
        temp_expr1.expr_type = postDec;
    } else {
        temp_expr1 = temp_expr2;
        goto parse_expr_d_expr_e;
    }
parse_expr_d_ret:
    Exprs_push(&temp_expr1.exprs, temp_expr2);

    temp_expr1.type.type_type = undefined;
parse_expr_d_expr_e:
        
    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_c(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_d(&temp_expr1, tokens, &temp_pos)) return false;

    for (;;) {
        if (parse_token(percent, tokens, &temp_pos)) {
            temp_expr3.expr_type = mod;
        } else if (parse_token(asterisk, tokens, &temp_pos)) {
            temp_expr3.expr_type = mult;
        } else if (parse_token(fSlash, tokens, &temp_pos)) {
            temp_expr3.expr_type = divi;
        } else {
            goto parse_expr_c_loop_end;
        }
        if (!parse_expr_d(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = undefined;
    }
parse_expr_c_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_b(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_c(&temp_expr1, tokens, &temp_pos)) return false;
    
    for (;;) {
        if (parse_token(plus, tokens, &temp_pos)) {
            temp_expr3.expr_type = add;
        } else if (parse_token(minus, tokens, &temp_pos)) {
            temp_expr3.expr_type = sub;
        } else {
            goto parse_expr_b_loop_end;
        }
        if (!parse_expr_c(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = undefined;
    }
parse_expr_b_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_a(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_b(&temp_expr1, tokens, &temp_pos)) return false;
    
    for (;;) {
        if (parse_token(lShift, tokens, &temp_pos)) {
            temp_expr3.expr_type = bLShift;
        } else if (parse_token(greaterThan, tokens, &temp_pos)) {
            if (!parse_token(greaterThan, tokens, &temp_pos)) {
                temp_pos--;
                goto parse_expr_a_loop_end;
            }
            temp_expr3.expr_type = bRShift;
        } else {
            goto parse_expr_a_loop_end;
        }
        if (!parse_expr_b(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = undefined;
    }
parse_expr_a_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_9(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_a(&temp_expr1, tokens, &temp_pos)) return false;
    
    for (;;) {
        if (parse_token(lessThan, tokens, &temp_pos)) {
            temp_expr3.expr_type = lThan;
        } else if (parse_token(greaterThan, tokens, &temp_pos)) {
            if (parse_token(greaterThan, tokens, &temp_pos) || parse_token(greaterThanEqual, tokens, &temp_pos)) {
                temp_pos -= 2;
                goto parse_expr_9_loop_end;
            }
            temp_expr3.expr_type = gThan;
        } else if (parse_token(lessThanEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = lThanEqual;
        } else if (parse_token(greaterThanEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = gThanEqual;
        } else {
            goto parse_expr_9_loop_end;
        }
        if (!parse_expr_a(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = undefined;
    }
parse_expr_9_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_8(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_9(&temp_expr1, tokens, &temp_pos)) return false;
    
    for (;;) {
        if (parse_token(compareEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = cmpEqual;
        } else if (parse_token(exclamation, tokens, &temp_pos)) {
            if (!parse_token(equal, tokens, &temp_pos)) {
                temp_pos--;
                goto parse_expr_8_loop_end;
            }
            temp_expr3.expr_type = notEquals;
        } else {
            goto parse_expr_8_loop_end;
        }
        if (!parse_expr_9(&temp_expr2, tokens, &temp_pos)) {
            if (temp_expr3.expr_type == notEquals)
                parse_error(tokens, temp_pos, "expression");
            else
                return false;
        }
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = undefined;
    }
parse_expr_8_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_7(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_8(&temp_expr1, tokens, &temp_pos)) return false;
    
    for (;;) {
        if (!parse_token(ampersand, tokens, &temp_pos)) goto parse_expr_7_loop_end;
        if (parse_token(ampersand, tokens, &temp_pos)) {
            temp_pos -= 2;
            goto parse_expr_7_loop_end;
        }
        temp_expr3.expr_type = bAnd;
        if (!parse_expr_8(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = undefined;
    }
parse_expr_7_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_6(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_7(&temp_expr1, tokens, &temp_pos)) return false;
    
    for (;;) {
        if (!parse_token(karat, tokens, &temp_pos)) goto parse_expr_6_loop_end;
        temp_expr3.expr_type = bXor;
        if (!parse_expr_7(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = undefined;
    }
parse_expr_6_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_5(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_6(&temp_expr1, tokens, &temp_pos)) return false;
    
    for (;;) {
        if (!parse_token(pipe, tokens, &temp_pos)) goto parse_expr_5_loop_end;
        temp_expr3.expr_type = bOr;
        if (!parse_expr_6(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = undefined;
    }
parse_expr_5_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_4(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_5(&temp_expr1, tokens, &temp_pos)) return false;
    
    for (;;) {
        if (!parse_token(ampersand, tokens, &temp_pos)) goto parse_expr_4_loop_end;
        if (!parse_token(ampersand, tokens, &temp_pos)) goto parse_expr_4_loop_end;
        temp_expr3.expr_type = andCmp;
        if (!parse_expr_5(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = undefined;
    }
parse_expr_4_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_3(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_4(&temp_expr1, tokens, &temp_pos)) return false;
    
    for (;;) {
        if (!parse_token(or, tokens, &temp_pos)) goto parse_expr_3_loop_end;
        temp_expr3.expr_type = orCmp;
        if (!parse_expr_4(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = undefined;
    }
parse_expr_3_loop_end:

    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_expr_2(Expr* expr, Tokens tokens, int* pos) {
    Array exprs = {0, 0, 0};
    Expr temp_expr;

    int temp_pos = *pos;

    if (!parse_expr_3(&temp_expr, tokens, &temp_pos)) return false;

    if (!parse_token(imply_arrow, tokens, &temp_pos)) {
        *expr = temp_expr;
        *pos = temp_pos;
        return true;
    }
    Exprs_push(&exprs, temp_expr);

    if (!parse_expr_g(&temp_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
    Exprs_push(&exprs, temp_expr);

    if (!parse_token(underscore, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'_'");
    if (!parse_token(imply_arrow, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'=>'");
    
    if (!parse_expr_g(&temp_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
    Exprs_push(&exprs, temp_expr);

    expr->expr_type = ternary;
    expr->type.type_type = undefined;
    expr->exprs = exprs;
    
    *pos = temp_pos;
    return true;
}

bool parse_expr_1(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr1;
    Expr temp_expr2;
    Expr temp_expr3;
    temp_expr3.exprs.ptr = 0; temp_expr3.exprs.length = 0; temp_expr3.exprs.capacity = 0;

    int temp_pos = *pos;

    if (!parse_expr_2(&temp_expr1, tokens, &temp_pos)) return false;
    
    for (;;) {
        if (parse_token(equal, tokens, &temp_pos)) {
            temp_expr3.expr_type = equals;
        } else if (parse_token(plusEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = plusEquals;
        } else if (parse_token(minusEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = minusEquals;
        } else if (parse_token(multEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = multEquals;
        } else if (parse_token(divEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = divEquals;
        } else if (parse_token(modEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = modEquals;
        } else if (parse_token(lShiftEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = lShiftEquals;
        } else if (parse_token(greaterThan, tokens, &temp_pos)) {
            if (!parse_token(greaterThanEqual, tokens, &temp_pos)) {
                temp_pos--;
                goto parse_expr_1_loop_end;
            }
            temp_expr3.expr_type = rShiftEquals;
        } else if (parse_token(ampersand, tokens, &temp_pos)) {
            if (!parse_token(equal, tokens, &temp_pos)) {
                temp_pos--;
                goto parse_expr_1_loop_end;
            }
            temp_expr3.expr_type = andEquals;
        } else if (parse_token(orEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = orEquals;
        } else if (parse_token(xorEqual, tokens, &temp_pos)) {
            temp_expr3.expr_type = xorEquals;
        } else {
            goto parse_expr_1_loop_end;
        }
        if (!parse_expr_2(&temp_expr2, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
        Exprs_push(&temp_expr3.exprs, temp_expr1);
        Exprs_push(&temp_expr3.exprs, temp_expr2);
        temp_expr1 = temp_expr3;
        temp_expr3.exprs.ptr = 0; 
        temp_expr3.exprs.length = 0;
        temp_expr3.exprs.capacity = 0;
        temp_expr1.type.type_type = null;
    }

parse_expr_1_loop_end:
    *expr = temp_expr1;

    *pos = temp_pos;
    return true;
}

bool parse_case_value(uint64_t* value, Tokens tokens, int* pos) {
    int temp_pos = *pos;

    if (!parse_token(integer, tokens, &temp_pos))
        if (!parse_token(character, tokens, &temp_pos)) return false;
    *value = tokens.ptr[temp_pos-1].int_value;

    *pos = temp_pos;
    return true;
}

bool check_case_imply(Tokens tokens, int pos) {
    int temp_pos = pos;

    uint64_t temp_value;
    if (!parse_case_value(&temp_value, tokens, &temp_pos)) return false;
    
    while (!parse_token(imply_arrow, tokens, &temp_pos)) {
        if (!parse_token(comma, tokens, &temp_pos)) parse_error(tokens, temp_pos, "',' or '=>'");
        if (!parse_case_value(&temp_value, tokens, &temp_pos)) parse_error(tokens, temp_pos, "integer or character");
    }

    return true;
}

bool parse_case_stmts(Array* stmts, bool* with_escape, Tokens tokens, int* pos) {
    Stmt temp_stmt;
    Array temp_stmts = {0, 0, 0};

    int temp_pos = *pos;

    if (!parse_cmp_stmt(&temp_stmt, tokens, &temp_pos)) {
        if (!parse_stmt(&temp_stmt, tokens, &temp_pos)) parse_error(tokens, temp_pos, "statement");
        Stmts_push(&temp_stmts, temp_stmt);
        if (!parse_token(newLine, tokens, &temp_pos)) {
            if (!parse_token(semicolon, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'\\n' or ';'");
        }
        for (;;) {
            if (check_case_imply(tokens, temp_pos)) break;
            if (!parse_stmt(&temp_stmt, tokens, &temp_pos)) break;
            if (!parse_token(newLine, tokens, &temp_pos)) {
                if (!parse_token(semicolon, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'\\n' or ';'");
            }
            Stmts_push(&temp_stmts, temp_stmt);
        }
parse_case_stmts_no_escape_ret:
        *with_escape = false;
        *stmts = temp_stmts;

        *pos = temp_pos;
        return true;
    }
    *with_escape = true;
    Stmts_push(&temp_stmts, temp_stmt);
    *stmts = temp_stmts;
    
    *pos = temp_pos;
    return true;
}

bool parse_case(Case* cas, Tokens tokens, int* pos) {
    Array values = {0, 0, 0};
    bool with_escape;
    Array stmts = {0, 0, 0};
    uint64_t temp_value;

    int temp_pos = *pos;
    
    if (!parse_case_value(&temp_value, tokens, &temp_pos)) return false;
    uint64_t_push(&values, temp_value);
    
    while (!parse_token(imply_arrow, tokens, &temp_pos)) {
        if (!parse_token(comma, tokens, &temp_pos)) parse_error(tokens, temp_pos, "',' or '=>'");
        if (!parse_case_value(&temp_value, tokens, &temp_pos)) parse_error(tokens, temp_pos, "integer or character");
        uint64_t_push(&values, temp_value);
    }

    if (!parse_case_stmts(&stmts, &with_escape, tokens, &temp_pos)) return false;
    
    cas->values = values;
    cas->with_escape = with_escape;
    cas->stmts = stmts;

    *pos = temp_pos;
    return true;
}

bool parse_match(Expr* expr, Tokens tokens, int* pos) {
    Array exprs = {0, 0, 0};
    Array cases = {0, 0, 0};
    Expr temp_expr;
    Case temp_case;

    int temp_pos = *pos;
    
    if (!parse_expr_9(&temp_expr, tokens, &temp_pos)) return false;
    Exprs_push(&exprs, temp_expr);

    if (!parse_token(compareEqual, tokens, &temp_pos)) return false;
    if (!parse_token(oBrace, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'{'");

    while (!parse_token(cBrace, tokens, &temp_pos)) {
        if (!parse_case(&temp_case, tokens, &temp_pos)) parse_error(tokens, temp_pos, "case or '}'");
        Cases_push(&cases, temp_case);
    }

    expr->expr_type = match;
    expr->type.type_type = null;
    expr->exprs = exprs;
    expr->cases = cases;

    *pos = temp_pos;
    return true;
}

bool parse_imply(Expr* expr, Tokens tokens, int* pos) {
    Array exprs = {0, 0, 0};
    Array stmts = {0, 0, 0};
    Expr temp_expr;
    Stmt temp_stmt;

    int temp_pos = *pos;

    if (!parse_expr_3(&temp_expr, tokens, &temp_pos)) return false;
    Exprs_push(&exprs, temp_expr);

    if (!parse_token(imply_arrow, tokens, &temp_pos)) return false;

    if (!parse_stmt(&temp_stmt, tokens, &temp_pos)) parse_error(tokens, temp_pos, "statement");
    Stmts_push(&stmts, temp_stmt);

    if (!parse_else(&stmts, tokens, &temp_pos))
        expr->expr_type = imply;
    else {
        expr->expr_type = imply_else;
    }

    expr->type.type_type = null;
    expr->exprs = exprs;
    expr->stmts = stmts;
    
    *pos = temp_pos;
    return true;
}

bool parse_exprs(Array* exprs, Tokens tokens, int* pos) {
    Expr temp_expr;

    int temp_pos = *pos;

    if (!parse_expr_2(&temp_expr, tokens, &temp_pos)) return false;
    Exprs_push(exprs, temp_expr);

    if (!parse_token(comma, tokens, &temp_pos)) goto parse_exprs_ret;

    if (!parse_exprs(exprs, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");

parse_exprs_ret:
    *pos = temp_pos;
    return true;
}

bool parse_infer_type_assgn(Expr* expr, Tokens tokens, int* pos) {
    Array exprs = {0, 0, 0};
    Id id_value;
    Expr temp_expr;

    int temp_pos = *pos;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    id_value.id_type = var;
    id_value.name = tokens.ptr[temp_pos-1].string_value;

    if (!parse_token(colon, tokens, &temp_pos)) return false;

    if (!parse_token(equal, tokens, &temp_pos)) return false;

    if (!parse_expr_2(&temp_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
    Exprs_push(&exprs, temp_expr);
    
    expr->expr_type = infer_type_assgn;
    expr->type.type_type = null;
    expr->id_value = id_value;
    expr->exprs = exprs;
    expr->type.type_type = null;

    *pos = temp_pos;
    return true;
}

bool parse_assgn(Expr* expr, Tokens tokens, int* pos) {
    Array exprs = {0, 0, 0};
    Id id_value;
    Expr temp_expr;

    int temp_pos = *pos;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    id_value.id_type = var;
    id_value.name = tokens.ptr[temp_pos-1].string_value;

    if (!parse_token(colon, tokens, &temp_pos)) return false;

    if (!parse_type(&temp_expr, tokens, &temp_pos)) return false;
    Exprs_push(&exprs, temp_expr);

    if (!parse_token(equal, tokens, &temp_pos)) return false;

    if (!parse_expr_2(&temp_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "expression");
    Exprs_push(&exprs, temp_expr);
    
    expr->expr_type = assgn;
    expr->type.type_type = null;
    expr->id_value = id_value;
    expr->exprs = exprs;

    *pos = temp_pos;
    return true;
}

bool parse_expr_0(Expr* expr, Tokens tokens, int* pos) {
    Expr temp_expr;

    int temp_pos = *pos;

    if (parse_infer_type_assgn(&temp_expr, tokens, &temp_pos)) goto parse_expr_0_ret;
    if (parse_assgn(&temp_expr, tokens, &temp_pos)) goto parse_expr_0_ret;
    if (parse_imply(&temp_expr, tokens, &temp_pos)) goto parse_expr_0_ret;
    if (parse_match(&temp_expr, tokens, &temp_pos)) goto parse_expr_0_ret;
    if (parse_expr_1(&temp_expr, tokens, &temp_pos)) goto parse_expr_0_ret;
    if (parse_label(&temp_expr, tokens, &temp_pos)) goto parse_expr_0_ret;

    return false;

parse_expr_0_ret:
    *expr = temp_expr;

    *pos = temp_pos;
    return true;
}

bool parse_param(Param* param, Tokens tokens, int* pos) {
    Param temp_param;

    int temp_pos = *pos;

    if (parse_token(ampersand, tokens, &temp_pos)) {
        if (!parse_token(identifier, tokens, &temp_pos)) return false;
        temp_param.id.id_type = param_name;
        temp_param.id.name = tokens.ptr[temp_pos-1].string_value;

        if (!parse_token(colon, tokens, &temp_pos)) parse_error(tokens, temp_pos, "':'");
        if (!parse_token(ampersand, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'&'");
        if (!parse_type(&temp_param.type_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "type");
        if (temp_param.type_expr.type.type_type == null) semantic_error(tokens, temp_pos, "invalid type: address of null");
        temp_param.is_reference = true;
    } else {
        if (!parse_token(identifier, tokens, &temp_pos)) return false;
        temp_param.id.id_type = param_name;
        temp_param.id.name = tokens.ptr[temp_pos-1].string_value;
        if (!parse_token(colon, tokens, &temp_pos)) parse_error(tokens, temp_pos, "':'");
        if (!parse_type(&temp_param.type_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "type");
        if (temp_param.type_expr.type.type_type == null) semantic_error(tokens, temp_pos, "parameter cannot have type null");
        temp_param.is_reference = false;
    }

    *param = temp_param;

    *pos = temp_pos;
    return true;
}

bool parse_params(Array* params, Tokens tokens, int* pos) {
    Param temp_param;

    int temp_pos = *pos;

    if (!parse_param(&temp_param, tokens, &temp_pos)) return false;
    Params_push(params, temp_param);

    if (!parse_token(comma, tokens, &temp_pos)) goto parse_params_ret;

    if (!parse_params(params, tokens, &temp_pos)) parse_error(tokens, temp_pos, "parameter");

parse_params_ret:
    *pos = temp_pos;
    return true;
}

bool parse_func(Func* func, Tokens tokens, int* pos) {
    Id id;
    Array params = {0, 0, 0};
    Expr ret_type;
    Stmt stmt;

    int temp_pos = *pos;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    id.id_type = func_name;
    id.name = tokens.ptr[temp_pos-1].string_value;

    if (!parse_token(oParenthesis, tokens, &temp_pos)) return false;

    parse_params(&params, tokens, &temp_pos);

    if (!parse_token(cParenthesis, tokens, &temp_pos)) return false;

    ret_type.expr_type = none;
    parse_ret_type(&ret_type, tokens, &temp_pos);

    if (!parse_stmt(&stmt, tokens, &temp_pos)) parse_error(tokens, temp_pos, "type or statement");

    func->id = id;
    func->params = params;
    func->ret_type = ret_type;
    func->stmt = stmt;

    *pos = temp_pos;
    return true;
}

bool parse_enum_val(Param* val, Tokens tokens, int* pos) {
    Param temp_val;

    int temp_pos = *pos;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    temp_val.id.id_type = enum_value;
    temp_val.id.name = tokens.ptr[temp_pos-1].string_value;

    if (!parse_token(colon, tokens, &temp_pos)) {
        temp_val.type_expr.expr_type = none;
        temp_val.type_expr.type.type_type = null;
        goto parse_enum_val_ret;
    }

    if (!parse_type(&temp_val.type_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "type");

parse_enum_val_ret:

    *val = temp_val;

    *pos = temp_pos;
    return true;
}

bool parse_enum_vals(Array* vals, Tokens tokens, int* pos) {
    Param temp_val;

    int temp_pos = *pos;

    if (!parse_enum_val(&temp_val, tokens, &temp_pos)) return false;
    Params_push(vals, temp_val);

    if (!parse_token(comma, tokens, &temp_pos)) goto parse_enum_vals_ret;

    if (!parse_enum_vals(vals, tokens, &temp_pos)) {
        if (!parse_token(cBrace, tokens, &temp_pos))
            parse_error(tokens, temp_pos, "enum value");
        temp_pos--;
    }

parse_enum_vals_ret:
    *pos = temp_pos;
    return true;
}

bool parse_enum(Cust_Type* en, Tokens tokens, int* pos) {
    Id id;
    bool has_generic;
    Id generic;
    Array vals = {0, 0, 0};

    int temp_pos = *pos;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    id.id_type = type_name;
    id.name = tokens.ptr[temp_pos-1].string_value;

    parse_gnrc_dec(&generic, &has_generic, tokens, &temp_pos);

    if (!parse_token(colon, tokens, &temp_pos)) return false;

    if (!parse_token(oBrace, tokens, &temp_pos)) return false;

    if (!parse_enum_vals(&vals, tokens, &temp_pos)) return false;

    if (!parse_token(cBrace, tokens, &temp_pos)) return false;

    en->id = id;
    en->cust_type_type = enu;
    en->has_generic = has_generic;
    en->generic = generic;
    en->vals = vals;

    *pos = temp_pos;
    return true;
}

bool parse_struct_val(Param* val, Tokens tokens, int* pos) {
    Param temp_val;

    int temp_pos = *pos;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    temp_val.id.id_type = struct_value;
    temp_val.id.name = tokens.ptr[temp_pos-1].string_value;

    if (!parse_token(colon, tokens, &temp_pos)) parse_error(tokens, temp_pos, "':'");

    if (!parse_type(&temp_val.type_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "type");

    *val = temp_val;

    *pos = temp_pos;
    return true;
}

bool parse_struct_vals(Array* vals, Tokens tokens, int* pos) {
    Param temp_val;

    int temp_pos = *pos;

    if (!parse_struct_val(&temp_val, tokens, &temp_pos)) return false;
    Params_push(vals, temp_val);

    if (!parse_token(semicolon, tokens, &temp_pos)) goto parse_struct_vals_ret;

    if (!parse_struct_vals(vals, tokens, &temp_pos)) {
        if (!parse_token(cBrace, tokens, &temp_pos))
            parse_error(tokens, temp_pos, "struct value");
        temp_pos--;
    }

parse_struct_vals_ret:
    *pos = temp_pos;
    return true;
}

bool parse_struct(Cust_Type* struc, Tokens tokens, int* pos) {
    Id id;
    bool has_generic;
    Id generic;
    Array vals = {0, 0, 0};

    int temp_pos = *pos;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    id.id_type = type_name;
    id.name = tokens.ptr[temp_pos-1].string_value;

    parse_gnrc_dec(&generic, &has_generic, tokens, &temp_pos);

    if (!parse_token(colon, tokens, &temp_pos)) return false;
    if (!parse_token(oBrace, tokens, &temp_pos)) return false;
    if (!parse_struct_vals(&vals, tokens, &temp_pos)) return false;
    if (!parse_token(cBrace, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'}'");

    struc->id = id;
    struc->cust_type_type = stru;
    struc->has_generic = has_generic;
    struc->generic = generic;
    struc->vals = vals;

    *pos = temp_pos;
    return true;
}

bool parse_self_param(Param* self_param, bool* has_self_param, Tokens tokens, int* pos) {
    Param temp_param;
    temp_param.type_expr.type.type_type = undefined;
    temp_param.type_expr.expr_type = none;

    int temp_pos = *pos;

    if (parse_token(ampersand, tokens, &temp_pos)) {
        if (!parse_token(identifier, tokens, &temp_pos)) return false;
        temp_param.id.id_type = param_name;
        temp_param.id.name = tokens.ptr[temp_pos-1].string_value;

        temp_param.is_reference = true;
    } else {
        if (!parse_token(identifier, tokens, &temp_pos)) return false;
        temp_param.id.id_type = param_name;
        temp_param.id.name = tokens.ptr[temp_pos-1].string_value;

        temp_param.is_reference = false;
    }

    *has_self_param = true;
    *self_param = temp_param;

    *pos = temp_pos;
    return true;
}

bool parse_m_param(Param* param, Tokens tokens, int* pos) {
    Param temp_param;

    int temp_pos = *pos;

    if (parse_token(ampersand, tokens, &temp_pos)) {
        if (!parse_token(identifier, tokens, &temp_pos)) parse_error(tokens, temp_pos, "identifier");
        temp_param.id.id_type = param_name;
        temp_param.id.name = tokens.ptr[temp_pos-1].string_value;

        if (!parse_token(colon, tokens, &temp_pos)) return false;
        if (!parse_token(ampersand, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'&'");
        if (!parse_type(&temp_param.type_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "type");
        if (temp_param.type_expr.type.type_type == null) semantic_error(tokens, temp_pos, "invalid type: address of null");
        temp_param.is_reference = true;
    } else {
        if (!parse_token(identifier, tokens, &temp_pos)) return false;
        temp_param.id.id_type = param_name;
        temp_param.id.name = tokens.ptr[temp_pos-1].string_value;
        if (!parse_token(colon, tokens, &temp_pos)) return false;
        if (!parse_type(&temp_param.type_expr, tokens, &temp_pos)) parse_error(tokens, temp_pos, "type");
        if (temp_param.type_expr.type.type_type == null) semantic_error(tokens, temp_pos, "parameter cannot have type null");
        temp_param.is_reference = false;
    }

    *param = temp_param;

    *pos = temp_pos;
    return true;
}

bool parse_m_params(bool* has_self_param, Param* self_param, Array* params, Tokens tokens, int* pos) {
    Param temp_param;
    *has_self_param = false;

    int temp_pos = *pos;

    if (!parse_m_param(&temp_param, tokens, &temp_pos)) {
        if (!parse_self_param(self_param, has_self_param, tokens, &temp_pos)) return false;
        if (!parse_token(comma, tokens, &temp_pos)) goto parse_m_params_ret;
    } else {
        Params_push(params, temp_param);
        if (!parse_token(comma, tokens, &temp_pos)) goto parse_m_params_ret;
    }

    if (!parse_param(&temp_param, tokens, &temp_pos)) parse_error(tokens, temp_pos, "parameter");
    Params_push(params, temp_param);

    if (!parse_token(comma, tokens, &temp_pos)) goto parse_m_params_ret;

    if (!parse_params(params, tokens, &temp_pos)) parse_error(tokens, temp_pos, "parameter");

parse_m_params_ret:
    *pos = temp_pos;
    return true;
}

bool parse_method(Method* method, Tokens tokens, int* pos) {
    Id id;
    bool has_self_param;
    Param self_param;
    Array params = {0, 0, 0};
    Expr ret_type;
    Stmt stmt;

    int temp_pos = *pos;

    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    id.id_type = func_name;
    id.name = tokens.ptr[temp_pos-1].string_value;

    if (!parse_token(oParenthesis, tokens, &temp_pos)) return false;

    parse_m_params(&has_self_param, &self_param, &params, tokens, &temp_pos);

    if (!parse_token(cParenthesis, tokens, &temp_pos)) parse_error(tokens, temp_pos, "')'");

    ret_type.expr_type = none;
    parse_ret_type(&ret_type, tokens, &temp_pos);

    if (!parse_stmt(&stmt, tokens, &temp_pos)) return false;

    method->id = id;
    method->has_self_param = has_self_param;
    method->self_param = self_param;
    method->params = params;
    method->ret_type = ret_type;
    method->stmt = stmt;

    *pos = temp_pos;
    return true;
}

bool parse_methods(Array* methods, Tokens tokens, int* pos) {
    Method temp_method;

    int temp_pos = *pos;

    while (temp_pos < tokens.length) {
        if (parse_method(&temp_method, tokens, &temp_pos))
            Methods_push(methods, temp_method);
        else {
            if (methods->length >= 1) goto parse_methods_ret;
            return false;
        }
    }

parse_methods_ret:
    *pos = temp_pos;
    return true;
}

bool parse_imple(Imple* imple, Tokens tokens, int* pos) {
    Id cust_type_name;
    bool has_generic;
    Id generic;
    Array methods = {0, 0, 0};

    int temp_pos = *pos;
    
    if (!parse_token(identifier, tokens, &temp_pos)) return false;
    cust_type_name.id_type = type_name;
    cust_type_name.name = tokens.ptr[temp_pos-1].string_value;

    parse_gnrc_dec(&generic, &has_generic, tokens, &temp_pos);

    if (!parse_token(double_colon, tokens, &temp_pos)) return false;
    if (!parse_token(oBrace, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'{'");
    if (!parse_methods(&methods, tokens, &temp_pos)) parse_error(tokens, temp_pos, "method");
    if (!parse_token(cBrace, tokens, &temp_pos)) parse_error(tokens, temp_pos, "'}'");
    
    imple->cust_type_name = cust_type_name;
    imple->has_generic = has_generic;
    imple->generic = generic;
    imple->methods = methods;

    *pos = temp_pos;
    return true;
}

void parse(Tokens tokens) {
    int pos = 0;

    Array funcs = {0, 0, 0};
    Array cust_types = {0, 0, 0};
    Array imples = {0, 0, 0};
    Array stmts = {0, 0, 0};
    Func temp_func;
    Cust_Type temp_cust_type;
    Imple temp_imple;
    Stmt temp_import;

    while (pos < tokens.length) {
        if (parse_func(&temp_func, tokens, &pos))
            Funcs_push(&funcs, temp_func);
        else if (parse_enum(&temp_cust_type, tokens, &pos))
            Cust_Types_push(&cust_types, temp_cust_type);
        else if (parse_struct(&temp_cust_type, tokens, &pos))
            Cust_Types_push(&cust_types, temp_cust_type);
        else if (parse_imple(&temp_imple, tokens, &pos))
            Imples_push(&imples, temp_imple);
        else if (parse_import(&temp_import, tokens, &pos))
            Stmts_push(&stmts, temp_import);
        else
            break;
    }

    for (int i = 0; i < cust_types.length; i++) {
        print_cust_type(((Cust_Type*)(cust_types.ptr))[i]);
    }

    for (int i = 0; i < imples.length; i++) {
        print_imple(((Imple*)(imples.ptr))[i]);
    }

    for (int i = 0; i < funcs.length; i++) {
        print_func(((Func*)(funcs.ptr))[i]);
    }

    for (int i = 0; i < stmts.length; i++) {
        print_stmt(((Stmt*)(stmts.ptr))[i], 0);
    }
}
