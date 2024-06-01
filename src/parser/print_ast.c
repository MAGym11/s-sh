void print_expr(Expr expr, int indent);
void print_stmt(Stmt stmt, int indent);

void indentation(int indent) {
    for (int i = 0; i < indent; i++) printf("   ");
}

void print_exprs(Array exprs, int indent) {
    indentation(indent);
    printf("exprs:\n");
    for (int i = 0; i < exprs.length; i++) {
        print_expr(((Expr*)(exprs.ptr))[i], indent+1);
    }
}

void print_stmts(Array stmts, int indent) {
    for (int i = 0; i < stmts.length; i++) {
        print_stmt(((Stmt*)(stmts.ptr))[i], indent);
    }
}

void print_values(Array values, int indent) {
    indentation(indent);
    printf("values: ");
    for (int i = 0; i < values.length; i++) {
        printf("%d", ((uint64_t*)(values.ptr))[i]);
        if (i < values.length-1)
            printf(", ");
        else
            printf("\n");
    }
}

void print_case(Case cas, int indent) {
    indentation(indent);
    printf("case:\n");
    print_values(cas.values, indent+1);
    indentation(indent+1);
    printf("with_escape: %s\n", (cas.with_escape) ? "true" : "false");
    print_stmts(cas.stmts, indent+1);
}

void print_cases(Array cases, int indent) {
    indentation(indent);
    printf("cases:\n");
    for (int i = 0; i < cases.length; i++) {
        print_case(((Case*)(cases.ptr))[i], indent+1);
    }
}

void print_type(Type type, int indent) {
    indentation(indent);
    printf("type: ");
    switch (type.type_type) {
        case mem:
            printf("mem\n");
            break;
        case addr:
            printf("addr\n");
            break;
        case null:
            printf("null\n");
            break;
        case undefined:
            printf("undefined\n");
            break;
        case custom:
            print_String(type.value);
            printf("\n");
            break;
        default:
            printf("not defined\n");
            break;
    }
}

void print_id(Id id, int indent) {
    indentation(indent);
    printf("id: ");
    bool unknown = false;
    switch (id.id_type) {
        case func_name:
            printf("func_name\n");
            break;
        case param_name:
            printf("param_name\n");
            break;
        case var:
            printf("var\n");
            break;
        case type:
            printf("type\n");
            break;
        case label:
            printf("label\n");
            break;
        case jmp:
            printf("jump\n");
            break;
        case gnrc_name:
            printf("gnrc_name\n");
            break;
        case type_name:
            printf("type_name\n");
            break;
        case enum_value:
            printf("enum_value\n");
            break;
        case struct_value:
            printf("struct_value\n");
            break;
        case lib:
            printf("lib\n");
            break;
        case modu:
            printf("modu\n");
            break;
        default:
            printf("unknown\n");
            unknown = true;
    }
    indentation(indent+1);
    printf("name: ");
    if (unknown == false)
        print_String(id.name);
    printf("\n");
}

void print_type_expr(Expr expr, int indent) {
    indentation(indent);
    printf("type_expr: ");
    switch (expr.expr_type) {
        case none:
            printf("none\n");
            break;
        case undrscr:
            printf("underscore\n");
            print_type(expr.type, indent+1);
            break;
        case address:
            printf("address\n");
            print_type(expr.type, indent+1);
            indentation(indent+1);
            if (expr.mut)
                printf("mutable\n");
            else
                printf("immutable\n");
            print_type_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            break;
        case type_gnrc:
            printf("type_gnrc\n");
            indentation(indent+1);
            if (expr.mut)
                printf("mutable\n");
            else
                printf("immutable\n");
            print_id(expr.id_value, indent+1);
            indentation(indent+1);
            printf("gnrc:\n");
            for (int i = 0; i < expr.gnrc.length; i++) {
                print_type_expr(((Expr*)(expr.gnrc.ptr))[i], indent+2);
            }
            break;
        case type_n_gnrc:
            printf("type\n");
            indentation(indent+1);
            if (expr.mut)
                printf("mutable\n");
            else
                printf("immutable\n");
            print_id(expr.id_value, indent+1);
            break;
        default:
            printf("error\n");
    }
}

void print_stmt(Stmt stmt, int indent) {
    indentation(indent);
    printf("stmt: ");
    switch (stmt.stmt_type) {
        case cmp_stmt:
            printf("cmp_stmt\n");
            print_stmts(stmt.stmts, indent+1);
            break;
        case loop:
            printf("loop\n");
            print_expr(((Expr*)(stmt.exprs.ptr))[0], indent+1);
            print_stmt(((Stmt*)(stmt.stmts.ptr))[0], indent+1);
            break;
        case ret:
            printf("ret\n");
            if (((Expr*)(stmt.exprs.ptr))[0].expr_type != none)
                print_expr(((Expr*)(stmt.exprs.ptr))[0], indent+1);
            break;
        case jump:
            printf("jump\n");
            print_id(stmt.id, indent+1);
            break;
        case defer:
            printf("defer\n");
            print_stmt(((Stmt*)(stmt.stmts.ptr))[0], indent+1);
            break;
        case expr:
            printf("expr\n");
            print_expr(((Expr*)(stmt.exprs.ptr))[0], indent+1);
            break;
        case import:
            printf("import\n");
            print_id(stmt.id, indent+1);
            if (stmt.stmts.length > 0)
                print_stmt(((Stmt*)(stmt.stmts.ptr))[0], indent+1);
            break;
        case module_access:
            printf("module_access\n");
            print_id(stmt.id, indent+1);
            if (stmt.stmts.length > 0)
                print_stmt(((Stmt*)(stmt.stmts.ptr))[0], indent+1);
            break;
        case nop:
            printf("nop\n");
    }
}

void print_ret_type(Expr expr) {
    if (expr.expr_type == none) return;
    indentation(1);
    printf("ret_type:\n");
    print_type_expr(expr, 2);
}

void print_expr(Expr expr, int indent) {
    indentation(indent);
    printf("expr: ");
    switch (expr.expr_type) {
        case none:
            printf("none\n");
            print_type(expr.type, indent+1);
            break;
        case assgn:
            printf("assgn\n");
            print_type(expr.type, indent+1);
            print_id(expr.id_value, indent+1);
            indentation(indent+1);
            printf("type:\n");
            print_type_expr(((Expr*)(expr.exprs.ptr))[0], indent+2);
            indentation(indent+1);
            printf("value:\n");
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+2);
            break;
        case infer_type_assgn:
            printf("infer_type_assgn\n");
            print_type(expr.type, indent+1);
            print_id(expr.id_value, indent+1);
            if (expr.mut) {
                indentation(indent+1);
                printf("mutable\n");
            }
            indentation(indent+1);
            printf("value:\n");
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+2);
            break;
        case label_expr:
            printf("label\n");
            print_type(expr.type, indent+1);
            print_id(expr.id_value, indent+1);
            break;
        case imply:
            printf("imply\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_stmt(((Stmt*)(expr.stmts.ptr))[0], indent+1);
            break;
        case imply_else:
            printf("imply\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_stmt(((Stmt*)(expr.stmts.ptr))[0], indent+1);
            indentation(indent);
            printf("expr: else\n");
            print_stmt(((Stmt*)(expr.stmts.ptr))[1], indent+1);
            break;
        case match:
            printf("match\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_cases(expr.cases, indent+1);
            break;
        case equals:
            printf("equals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case plusEquals:
            printf("plusEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case minusEquals:
            printf("minusEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case multEquals:
            printf("multEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case divEquals:
            printf("divEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case modEquals:
            printf("modEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case lShiftEquals:
            printf("lShiftEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case rShiftEquals:
            printf("rShiftEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case andEquals:
            printf("andEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case orEquals:
            printf("orEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case xorEquals:
            printf("xorEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case ternary:
            printf("ternary\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[2], indent+1);
            break;
        case orCmp:
            printf("orCmp\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case andCmp:
            printf("andCmp\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case bOr:
            printf("bOr\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case bXor:
            printf("bXor\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case bAnd:
            printf("bAnd\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case cmpEqual:
            printf("cmpEqual\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case notEquals:
            printf("notEquals\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case bLShift:
            printf("bLShift\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case bRShift:
            printf("bRShift\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case lThan:
            printf("lThan\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case gThan:
            printf("gThan\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case lThanEqual:
            printf("lThanEqual\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case gThanEqual:
            printf("gThanEqual\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case sub:
            printf("sub\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case add:
            printf("add\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case mod:
            printf("mod\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case mult:
            printf("mult\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case divi:
            printf("divi\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case not:
            printf("not\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            break;
        case bNot:
            printf("bNot\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            break;
        case address:
            printf("address\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            break;
        case negate:
            printf("negate\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            break;
        case preInc:
            printf("preInc\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            break;
        case preDec:
            printf("preDec\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            break;
        case postInc:
            printf("postInc\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            break;
        case postDec:
            printf("postDec\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            break;
        case dot_op:
            printf("dot_op\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case double_colon_op:
            printf("double_colon_op\n");
            print_type(expr.type, indent+1);
            print_type_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case cast:
            printf("cast\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            print_type_expr(((Expr*)(expr.exprs.ptr))[1], indent+1);
            break;
        case paren:
            printf("paren\n");
            print_type(expr.type, indent+1);
            print_exprs(expr.exprs, indent+1);
            break;
        case deref:
            printf("deref\n");
            print_type(expr.type, indent+1);
            print_expr(((Expr*)(expr.exprs.ptr))[0], indent+1);
            break;
        case structure:
            printf("structure\n");
            print_type(expr.type, indent+1);
            print_type_expr(((Expr*)(expr.structure_type.ptr))[0], indent+1);
            print_exprs(expr.exprs, indent+1);
            break;
        case structure_n_type:
            printf("structure_n_type\n");
            print_type(expr.type, indent+1);
            print_exprs(expr.exprs, indent+1);
            break;
        case func_call:
            printf("func_call\n");
            print_type(expr.type, indent+1);
            print_id(expr.id_value, indent+1);
            if (expr.exprs.length > 0)
                print_exprs(expr.exprs, indent+1);
            break;
        case func_call_gnrc:
            printf("func_call_gnrc\n");
            print_type(expr.type, indent+1);
            print_id(expr.id_value, indent+1);
            indentation(indent+1);
            printf("generic:\n");
            print_exprs(expr.gnrc, indent+2);
            print_exprs(expr.exprs, indent+1);
            break;
        case id:
            printf("identifier\n");
            print_type(expr.type, indent+1);
            print_id(expr.id_value, indent+1);
            break;
        case in:
            printf("int\n");
            print_type(expr.type, indent+1);
            indentation(indent+1);
            printf("int: %llu\n", expr.int_value);
            break;
        case re:
            printf("real\n");
            print_type(expr.type, indent+1);
            indentation(indent+1);
            printf("real: %lf\n", expr.real_value);
            break;
        case str:
            printf("string\n");
            print_type(expr.type, indent+1);
            indentation(indent+1);
            printf("string: \"");
            print_String(expr.str_value);
            printf("\"\n");
            break;
        case cha:
            printf("cha\n");
            print_type(expr.type, indent+1);
            indentation(indent+1);
            printf("char: '%c'\n", expr.int_value);
            break;
        case undrscr:
            printf("underscore\n");
            print_type(expr.type, indent+1);
            break;
        case type_gnrc:
            printf("type_gnrc\n");
            indentation(indent+1);
            if (expr.mut)
                printf("mutable\n");
            else
                printf("immutable\n");
            print_id(expr.id_value, indent+1);
            indentation(indent+1);
            printf("gnrc:\n");
            for (int i = 0; i < expr.gnrc.length; i++) {
                print_type_expr(((Expr*)(expr.gnrc.ptr))[i], indent+2);
            }
            break;
        case type_n_gnrc:
            printf("type\n");
            indentation(indent+1);
            if (expr.mut)
                printf("mutable\n");
            else
                printf("immutable\n");
            print_id(expr.id_value, indent+1);
            break;
        default:
            printf("error\n");
    }
}

void print_param(Param param, int indent) {
    indentation(indent);
    printf("param:\n");
    print_id(param.id, indent+1);
    if (param.is_reference) {
        indentation(indent+1);
        printf("reference\n");
    }
    indentation(indent+1);
    printf("type:\n");
    print_type_expr(param.type_expr, indent+2);
}

void print_params(Array params, int indent) {
    if (params.length <= 0)
        return;
    indentation(indent);
    printf("params:\n");
    for (int i = 0; i < params.length; i++) {
        print_param(((Param*)(params.ptr))[i], indent+1);
    }
}

void print_ids(Array ids, int indent) {
    indentation(indent);
    printf("ids:\n");
    for (int i = 0; i < ids.length; i++) {
        print_id(((Id*)(ids.ptr))[i], indent+1);
    }
}

void print_func(Func func) {
    printf("func: ");
    print_String(func.id.name);
    printf("\n");
    if (func.has_generic) {
        indentation(1);
        printf("generic:\n");
        print_ids(func.generic, 2);
    }
    print_params(func.params, 2);
    print_ret_type(func.ret_type);
    print_stmt(func.stmt, 1);
}

void print_enum_val(Param val) {
    indentation(3);
    printf("enum_val:\n");
    print_id(val.id, 4);
    indentation(4);
    printf("type:\n");
    print_type_expr(val.type_expr, 5);
}

void print_enum_vals(Array vals) {
    if (vals.length <= 0)
        return;
    indentation(2);
    printf("enum_vals:\n");
    for (int i = 0; i < vals.length; i++) {
        print_enum_val(((Param*)(vals.ptr))[i]);
    }
}

void print_enum(Cust_Type en) {
    indentation(1);
    printf("enum: ");
    print_String(en.id.name);
    printf("\n");
    if (en.has_generic) {
        indentation(2);
        printf("generic_name:\n");
        print_ids(en.generic, 3);
    }
    print_enum_vals(en.vals);
}

void print_struct_val(Param val) {
    indentation(3);
    printf("struct_val:\n");
    print_id(val.id, 4);
    indentation(4);
    printf("type:\n");
    print_type_expr(val.type_expr, 5);
}

void print_struct_vals(Array vals) {
    if (vals.length <= 0)
        return;
    indentation(2);
    printf("struct_vals:\n");
    for (int i = 0; i < vals.length; i++) {
        print_struct_val(((Param*)(vals.ptr))[i]);
    }
}

void print_struct(Cust_Type struc) {
    indentation(1);
    printf("struct: ");
    print_String(struc.id.name);
    printf("\n");
    if (struc.has_generic) {
        indentation(2);
        printf("generic_name:\n");
        print_ids(struc.generic, 3);
    }
    print_struct_vals(struc.vals);
}

void print_cust_type(Cust_Type cust_type) {
    printf("cust_type: ");
    if (cust_type.cust_type_type == stru) {
        printf("struct\n");
        print_struct(cust_type);
    } else if (cust_type.cust_type_type == enu) {
        printf("enum\n");
        print_enum(cust_type);
    }
}

void print_self_param(Param param) {
    indentation(3);
    printf("self_param:\n");
    print_id(param.id, 4);
    if (param.is_reference) {
        indentation(4);
        printf("reference\n");
    }
        indentation(4);
    if (param.is_mutable) printf("mutable\n");
    else printf("immutable\n");
    indentation(4);
    printf("type:\n");
    print_type_expr(param.type_expr, 5);
}

void print_method(Method method) {
    indentation(2);
    printf("method: ");
    print_String(method.id.name);
    printf("\n");
    if (method.has_generic) {
        indentation(3);
        printf("generic:\n");
        print_ids(method.generic, 4);
    }
    if (method.has_self_param)
        print_self_param(method.self_param);
    print_params(method.params, 3);
    print_ret_type(method.ret_type);
    print_stmt(method.stmt, 3);
}

void print_methods(Array methods) {
    if (methods.length <= 0)
        return;
    indentation(1);
    printf("methods:\n");
    for (int i = 0; i < methods.length; i++) {
        print_method(((Method*)(methods.ptr))[i]);
    }
}

void print_imple(Imple imple) {
    printf("imple: ");
    print_String(imple.cust_type_name.name);
    printf("\n");
    if (imple.has_generic) {
        indentation(1);
        printf("generic_name:\n");
        print_ids(imple.generic, 2);
    }
    print_methods(imple.methods);
}
