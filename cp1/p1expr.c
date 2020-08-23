#include "cp1.h"

uint8_t binaryOps[] = { C_STAR, C_DIV, C_MOD, C_PLUS, C_MINUS, C_LSHIFT, C_RSHIFT, C_LT,
                   C_LE, C_GT, C_GE, C_EQEQ, C_NE, C_AND, C_XOR, C_OR,
                   C_ANDAND, C_OROR, C_QMARK, C_EQUAL, C_EQMUL, C_EQDIV, C_EQMOD, C_EQPLUS,
                   C_EQMINUS, C_EQLSHIFT, C_EQRSHIFT, C_EQAND, C_EQXOR, C_EQOR, 0 };
uint8_t tBC5B[] = { C_EQMUL, C_EQDIV, C_EQMOD, C_EQPLUS, C_EQMINUS, C_EQLSHIFT, C_EQRSHIFT, 0,
                   0,0,0,0,0, C_EQAND, C_EQXOR, C_EQOR,
                   0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

uint8_t binaryPrecedence[] = { 0xe, 0xe, 0xe, 0xd, 0xd, 0xc, 0xc, 0xb,
                        0xb, 0xb, 0xb, 0xa, 0xa, 9, 8, 7,
                          6, 5, 3, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1 };
uint8_t precedence[] = { 0xe, 0xe, 0xe, 0xd, 0xd, 0xc, 0xc, 0xb,
                    0xb, 0xb, 0xb, 0xa, 0xa, 9, 8, 7,
                      6, 5, 4, 2, 2, 2, 2, 2,
                      2, 2, 2, 2, 2, 2 };
uint8_t tUnary[] = { C_STAR, C_AND, C_PLUS, C_MINUS, C_LNOT, C_NOT, 0 };
uint8_t unaryMap[] = { C_DEREF, C_ADDRESSOF, C_UPLUS, C_UMINUS, C_LNOT, C_NOT };
uint8_t bBCCE[] = { C_INT8, C_UINT8, C_INT16, C_UINT16,  C_INT32, C_UINT32 };
uint8_t bBCD4[] = { D_CHAR, D_UCHAR, D_SHORT, D_USHORT, D_LONG, D_ULONG };


long _const(int arg_2) {
    term_t *r4;
    if ((r4 = gexpr(arg_2)) == 0)
        return 0L;
    if (iscons(r4) == 0) {
        wsperror("constant required");
        return 0L;
    }
    return r4->term.lng;
}

term_t *gelist(bool arg_2) {
    exlist = frelst(exlist, 0);
    return reduce(scalify(typify(melist(arg_2))));
}



term_t *gexpr(bool arg_2) {
    exlist = frelst(exlist, 0);
    return reduce(scalify(typify(mexpr(arg_2))));
}


term_t *gtest(bool arg_2) {
    need(C_LPAREN);
    term_t *r4 = gelist(arg_2);
    need(C_RPAREN);
    return r4;
}


term_t *mbin() {
    tok_t var_10;
    int r2;
    int r3;

    if ((r3 = alt(binaryOps)) == 0)
        return NULL;;
    if (r3 == C_QMARK) {
        term_t *r4 = buyop(C_QMARK, 0, 0);
        r4->op.w14 = mexpr(1);
        need(C_COLON);
        return r4;
    }
    if (gettok(&var_10)->code != C_EQUAL)
        baktok(&var_10);
    else if ((r2 = tBC5B[scnstr(binaryOps, r3)]) != 0)
        r3 = r2 & 0xff;
    else
        baktok(&var_10);
    return buyop(r3, 0, 0);
}



term_t *mcast() {
    sym_t var_1A;
    if (gscty(&var_1A, 0) == 0)
        return NULL;
    sym_t *r4 = gdecl(&var_1A, 1);
    need(C_RPAREN);
    return wsfree(r4, setty((sym_t *)buyop(C_CAST, 0, 0), r4->dataType, r4->intPtr));
}

term_t *melist(bool arg_2) {
    term_t *r4;

    if ((r4 = mexpr(arg_2)) == 0)
        return 0;
    while (eat(C_COMMA))
        r4 = buyop(C_NEXTEXPR, r4, mexpr(1));
    return r4;
}




term_t *mexpr(bool arg_2) {
    term_t *var_8;
    term_t *r4;

    if ((r4 = mterm(arg_2)) == 0)
        return 0;
    if ((var_8 = mbin()) != 0)
        return mtail(0, r4, &var_8);
    return r4;
}


term_t *mident(char *arg_2) {
    sym_t *r4;
    term_t *r2;

    if ((r4 = lookup(arg_2, symtab, 0)) == NULL) {
        r4 = buysym(0, 0, arg_2, C_EXTERN);
        r4->next = symtab;
        symtab = r4;
    }
    r2 = buyterm(r4->dataType, r4->aux, 0, 0L, 0, 1);
    switch (r4->code) {
    case C_STATICINIT: case C_STATIC: case C_EXTERNINIT: case C_EXTERN:
        cpynm(r2->name, r4->name);
        r4->b[0] = 1;
        break;
    case C_LABEL:
        cpynm(r2->name, lblname(r4->w));
        break;
    case 0x3b:
        r2->term.lng = r4->lng;
        r2->term.b14 = 0xa0;
        break;
    case C_AUTO:
        r2->term.lng = r4->lng;
        r2->term.b14 = 0x20;
        break;
    case C_REGISTER:
        r2->term.b14 = r4->b[0];
        r2->term.b15 = 0;
        break;
    default:
        nmerr("illegal use of typedef", r4->name);
        setty((sym_t *)r2, tint, 0);
        break;
    }
    return r2;
}



term_t *mtail(int arg_2, term_t *r4, term_t **arg_6) {
    term_t *var_A = *arg_6;
    int binPrec;
    int r3;

    while (var_A && arg_2 < precedence[r3 = scnstr(binaryOps, var_A->code)]) { //w0E
        binPrec = binaryPrecedence[r3];
        term_t *r2 = mterm(1);
        if ((var_A = mbin()) && binPrec < precedence[scnstr(binaryOps, var_A->code)])
            r2 = mtail(binPrec, r2, &var_A);

        (*arg_6)->op.lhs = r4;
        r4 = *arg_6;
        (*arg_6)->op.rhs = r2;
        *arg_6 = var_A;
    }
    return r4;
}


term_t *mterm(bool arg_2) {
    tok_t var_10;
    term_t *r4;
    term_t *r2;

    switch (gettok(&var_10)->code) {
    case C_ID:
        r4 = mident(var_10.name);
        break;
    case C_UINT16:
        if (tint > D_SHORT)
            var_10.code = C_INT32;
    case C_UINT32: case C_INT32: case C_INT16: case C_UINT8: case C_INT8:
        r4 = buyterm(bBCD4[scnstr(bBCCE, var_10.code)], 0, 0, var_10.lng, 0, 0);
        break;
    case C_STRING:
        littab = r2 = alloc(sizeof(term_t), littab);
        r2->lit.str = var_10.str;
        r2->lit.len = var_10.len + 1;
        setty((sym_t *)r2, (tchar << 2) | D_ARRAY, (intptr_t)r2->lit.str);
        r2->code = C_LABEL;
        cpynm(r2->name, lblname(crs()));
        r4 = buyterm(r2->dataType, r2->aux, r2->name, 0L, 0, 1);
        break;
    case C_DBL:
        r4 = buyterm(D_DOUBLE, 0, (char *)&var_10.dbl, 0L, 0, 0);
        break;
    case C_NOT: case C_LNOT: case C_MINUS: case C_PLUS: case C_AND: case C_STAR:
        return buyop(unaryMap[scnstr(tUnary, var_10.code)] & 0xff, mterm(1), 0);
    case C_DEC: case C_INC:
        return buyop(var_10.code == C_INC ? C_EQPLUS : C_EQMINUS, mterm(1), buyterm(tint, 0, 0, 1L, 0, 0));
    case C_LPAREN:
        if (r4 = mcast())
            r4->op.lhs = mterm(1);
        else {
            r4 = melist(1);
            need(C_RPAREN);
        }
        break;
    case C_SIZEOF:
        if (gettok(&var_10)->code != C_LPAREN && (r4 = mcast())) {
            setad(r4, 0, bytes(r4->dataType, r4->aux), 0, 0);
            setty((sym_t *)r4, tint, 0);
        } else {
            baktok(&var_10);
            r4 = buyop(C_SIZEOF, mterm(1), 0);
        }
        break;
    default:
        baktok(&var_10);
        if (arg_2) {
            wsperror("missing expression");
            r4 = buyterm(tint, 0, 0, 0L, 0, 0);
        } else
            return 0;
        break;
    }
    return mtrail(r4);
}

term_t *mtrail(term_t *r4) {
    tok_t var_10;
    term_t *r2;

    while (1) {
        switch (gettok(&var_10)->code) {
        case C_LPAREN:
            r4 = buyop(C_STARTLIST, r4, mexpr(0));
            if (r4->op.rhs) {
                for (r2 = r4; eat(C_COMMA); r2 = r2->op.rhs)
                r2->op.rhs = buyop(C_LISTNEXT, r2->op.rhs, mexpr(1));
            } else
                r4->code = C_ENDLIST;
            need(C_RPAREN);
            break;
        case C_LBRACKET:
            r4 = buyop(C_DEREF, buyop(C_PLUS, r4, melist(1)), 0);
                need(C_RBRACKET);
            break;
        case C_DOT: case C_ARROW:
            r4 = buyop(C_ARROW, var_10.code == C_ARROW ? r4 : buyop(C_ADDRESSOF, r4, 0), 0);
            if (ident(&var_10) == 0) {
                wsperror("missing member name");
                var_10.name[0] = 0;
            }
            r4->op.rhs = buyterm(tint, 0, var_10.name, 0L, 0, 0);
            break;

        case C_INC: case C_DEC:
            r4 = buyop(var_10.code == C_INC ? C_POSTINC : C_POSTDEC, r4, buyterm(tint, 0, 0, 1L, 0, 0));
            break;
        default:
            baktok(&var_10);
            return r4;
        }
    }
}

