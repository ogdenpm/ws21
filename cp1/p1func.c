#include "cp1.h"

int regset = 0x1c;
long autoff;
int deflbl;
term_t rterm;
uint8_t bBD10[] = { 1, 1, 1, 1, 1, 3, 3, 3, 0x40, 0x40, 1 };
int wBD23;
int wBD25;
term_t op_BD27;       // might be alternative 16 byte structure
tok_t tok_BD3D;





void autinit(sym_t *r4) {
    term_t var_1E;
    int var_8;
    term_t *r2;

    eat(C_EQUAL);
    var_8 = eat(C_LBRACE);
    r2 = gexpr(0);
    if (r2) {
        pregs(regset);
        pauto(autoff);
        if (r4->code == C_AUTO)
            setad((term_t *)setty((sym_t *)&var_1E, r4->code, r4->intPtr), 0, r4->lng, 0x20, 1);
        else
            setad((term_t *)setty((sym_t *)&var_1E, r4->code, r4->intPtr), 0, 0L, r4->b[0], 0);
        pmove(&var_1E, r2);
    }
    if (var_8)
        need(C_RBRACE);
}


void doblock(int arg_2, int arg_4) {
    tok_t var_2E;
    sym_t *var_24;
    sym_t var_22;
    long var_E;
    int var_A;
    bool var_8;

    var_A = regset;
    var_E = autoff;
    var_24 = symtab;
    sym_t *r4;
    int r3;
    sym_t *r2;

    while (gscty(&var_22, C_AUTO, C_REGISTER, C_EXTERN, C_STATIC, C_TYPEDEF, 0) != 0) {
        while (r4 = gdecl(&var_22, 0)) { // 321A
            if ((var_8 = type(r4->dataType) == D_FUNC && r4->code != C_TYPEDEF) && r4->code != C_STATIC)
                r4->code = C_EXTERN;
            if (r2 = lookup(r4->name, symtab, var_24))
                nmerr("redeclared local", r4->name);
            else if ((r4->code != C_EXTERN && !var_8 ) || (r2 = lookex(r4->name, var_24)) == NULL) {
                r4->next = symtab;
                symtab = r4;    // 3282
            } else if (r2->dataType == 0) //3308
                setty(r2, r4->dataType, r4->intPtr);
            else if (cmptype(r4, r2) == 0)
                nmerr("redeclared", r4->name);
            if (r2)
                r4 = wsfree(r4, r2);

            if (r4->code != C_TYPEDEF && var_8 == 0) {
                if (r4->code == C_STATIC) {
                    r4->code = C_LABEL;
                    r4->w = crs();
                    datinit(r4);
                } else if (r4->code == C_REGISTER || r4->code == C_AUTO) {
                    if (r4->code == C_REGISTER && (r3 = rbuy(r4->dataType, &regset))) {
                        r4->b[0] = r3;
                        r4->dataType = maxify(r4->dataType);
                    } else {
                        r4->code = C_AUTO;
                        autoff -= bytes(r4->dataType, r4->aux);
                        autoff &= ~((1 << bound(r4->dataType, r4->aux)) - 1L);
                        r4->lng = autoff;
                    }
                    autinit(r4);
                }
            }
            if (!eat(C_COMMA))
                break;
        }
        need(C_SEMI);
    }
    if (var_A != regset)
        pregs(regset);

    if (autoff != var_E)
        pauto(autoff);

    while (gettok(&var_2E)->code != C_RBRACE) {
        baktok(&var_2E);
        if (var_2E.code == EOF) {
            wsperror("unexpected EOF");
            break;
        }
        dostat(arg_2, arg_4);
    }

    if (var_A != regset) {
        regset = var_A;
        pregs(regset);
    }

    if (autoff != var_E) {
        autoff = var_E;
        pauto(autoff);
    }

    perc(var_24);
}

void dostat(int arg_2, int arg_4) {
    int var_8;
    int var_A;
    int var_C;
    sym_t *r2;
    term_t *r3;
    case_t *r4;

    wBD23 = 0;

    while (wBD23 == 0) {
        switch (gettok(&tok_BD3D)->code) {
        case C_CASE:
            casetab = alloc(sizeof(case_t), casetab);
            casetab->caseValue = _const(1);
            need(C_COLON);
            for (r4 = casetab->next; r4 && r4 != (case_t *)&casetab && casetab->caseValue == r4->caseValue; r4 = r4->next)
                ;
            if (r4 != (case_t *)&casetab)
                wsperror("illegal case");
            casetab->caseLabel = pcase(crs());
            break;

        case C_DEFAULT:
            need(C_COLON);
            if (casetab == NULL || deflbl)
                wsperror("illegal default");
            deflbl = pcase(crs());
            break;
        default:
            if (tok_BD3D.code == C_ID || eat(C_COLON) == 0)
                wBD23 = 1;
            else {
                if ((r2 = lookup(tok_BD3D.name, lbltab, 0)) == NULL) {
                    r2 = buysym(0, 0, tok_BD3D.name, 0);
                    r2->next = lbltab;
                    lbltab = r2;
                }
                if (r2->code)
                    nmerr("redefined label", r2->name);
                r2->code = C_LABEL;
                if (r2->w == 0)
                    r2->w = crs();
                plabel(r2->w);
                break;
            }
        }
        switch (tok_BD3D.code) {
        case C_LBRACE:
            doblock(arg_2, arg_4);
            break;
        case C_SEMI:
            break;
        case C_IF:
            var_A = pjf(gtest(0), crs(), 0);
            dostat(arg_2, arg_4);
            if (eat(C_ELSE)) {
                var_A = pjump(0, var_A);
                dostat(arg_2, arg_4);
            }
            plabel(var_A);
            break;
        case C_WHILE:
            var_8 = plabel(crs());
            var_A = pjf(gtest(0), crs(), 0);
            dostat(var_A, var_8);
            pjump(var_8, var_A);
            break;
        case C_DO:
            var_8 = plabel(crs());
            var_C = crs();                  // split to force order
            dostat(var_A = crs(), var_C);
            plabel(var_C);
            need(C_WHILE);
            pjt(gtest(0), var_A, var_8);
            need(C_SEMI);
            break;
        case C_FOR:
            need(C_LPAREN);
            if (r3 = gelist(0))
                pvoid(r3);
            need(C_SEMI);
            var_8 = plabel(crs());
            var_A = pjf(gelist(0), crs(), 0);
            need(C_SEMI);
            if (r3 = gelist(0)) {
                var_C = pjump(0, wBD25 = crs());
                pvoid(r3);
                pjump(var_8, var_C);
                var_8 = wBD25;
            }
            need(C_RPAREN);
            dostat(var_A, var_8);
            pjump(var_8, var_A);
            break;
        case C_BREAK:
            if (arg_2)
                pjump(arg_2, 0);
            else
                wsperror("illegal break");
            need(C_SEMI);
            break;
        case C_CONTINUE:
            if (arg_4)
                pjump(arg_4, 0);
            else
                wsperror("illegal continue");
            need(C_SEMI);
            break;
        case C_RETURN:
            if (r3 = gelist(0))
                pmove(&rterm, r3);
            pret();
            need(C_SEMI);
            break;
        case C_GOTO:
            if (ident(&tok_BD3D) == 0)
                wsperror("missing goto label");
            else if (r2 = lookup(tok_BD3D.str, lbltab, 0))
                pjump(r2->w, 0);
            else {
                r2 = buysym(0, 0, tok_BD3D.str, 0);
                r2->next = lbltab;
                lbltab = r2;
                r2->w = pjump(0, 0);
            }
            need(C_SEMI);
            break;
        case C_SWITCH:
            op_BD27.dataType = tint;
            op_BD27.term.b14 = 1;
            pmove(&op_BD27, gtest(1));
            r4 = casetab;
            casetab = (case_t *)&casetab;
            var_8 = deflbl;
            deflbl = 0;
            pswitch(var_C = crs());
            dostat(var_A = crs(), arg_4);
            pswtab(casetab, deflbl ? deflbl : var_A, var_C);
            pcase(var_A);
            deflbl = var_8;
            casetab = r4;
            break;
        default:
            baktok(&tok_BD3D);
            if (r3 = gelist(0)) {
                pvoid(r3);
                need(C_SEMI);
            } else
                recover("illegal statement");
            break;
        }
    }
}


bool fninit(sym_t *r4) {

    int8_t var_3F;
    term_t var_3E;
    term_t *var_28;
    sym_t *var_26;
    sym_t *var_24;
    sym_t var_22;
    sym_t *var_E;
    long var_C;
    bool var_8;
    sym_t *r2 = NULL;       //TO CHECK - potentially the while loop fails on first pass and r2 is uninitialised
    sym_t *r3;

    var_8 = 0;
    var_E = r4->aux->psym;
    var_26 = symtab;

    while (gscty(&var_22, C_PARAM, C_REGISTER, 0)) {
        var_8 = 1;
        while (r2 = gdecl(&var_22, 0)) {
            if ((r3 = lookup(r2->name, var_E, 0)) == NULL) // 3E49
                nmerr("not an argument", r2->name);
            else if (r3->code != 0)
                nmerr("redeclared argument", r2->name);
            else
                setty(r3, r2->dataType, r2->intPtr)->code = r2->code;
            if (eat(C_COMMA))
                wsfree(r2, 0);
            else
                break;
        }
        need(C_SEMI);
    }
    if (var_8)
        need(C_LBRACE);
    else if (eat(C_LBRACE))
        var_8 = 1;

    if (var_8) {
        if (r4->code == C_EXTERNINIT || r4->code == C_STATICINIT)
            nmerr("function redefined", r2->name);      // to check whether this should be r4
        else {
            r4->code = r4->code == C_EXTERN ? C_EXTERNINIT : C_STATICINIT;
            pfunc(r4->name);
        }
    }
    if (var_8) {
        setty((sym_t *)&rterm, maxify(dety(r4->dataType)), (intptr_t)r4->aux->next);
        if (stype(rterm.dataType) == 0) {
            wsperror("illegal return type");
            rterm.dataType = tint;
        }
        rterm.term.b14 = bBD10[scnstr(typtab, type(rterm.dataType))];
        regset = iregs;
        autoff = 0L;
        var_C = 0L;
        var_24 = (sym_t *)&var_E;
        while (r3 = var_24->next) {
            if (r3->code == 0)
                setty(r3, tint, C_PARAM);
            else if ((r3->dataType & 3) == D_FUNC)
                retype(r3, D_PTR);
            else if ((r3->dataType & 3) == D_ARRAY)
                setty(r3, (r3->dataType & 0xfffc) | D_PTR, (intptr_t)r3->aux->next);
            else
                r3->dataType = maxify(r3->dataType);

            r3->lng = bndify(r3->dataType, r3->aux, var_C);
            var_C = bytes(r3->dataType, r3->aux + r3->lng);
            if (r3->code == C_REGISTER && (var_3F = rbuy(r3->dataType, &regset))) { // 42BB 
                var_28 = buyterm(r3->dataType, r3->aux, 0, r3->lng, 0xa0, 1);
                setad((term_t *)setty((sym_t *)&var_3E, r3->dataType, r3->intPtr), 0, 0L, var_3F, 0);
                pregs(regset);
                pauto(autoff);
                pmove(&var_3E, var_28);
                r3->b[0] = var_3F;
            } else
                r3->code = 0x3B;
            var_24 = r3;
        }
        var_24->next = symtab;
        symtab = var_E;
        lbltab = 0;
        doblock(0, 0);
        pret();
        r3 = lbltab;
        while (r3) {
            if (r3->code == 0)
                nmerr("missing label", r3->name);
            r3 = wsfree(r3, r3->next);
        }
        perc(var_26);
        pend();
    }
    return var_8;
}


sym_t *lookex(char *arg_2, sym_t *r4) {
    while (r4 = lookup(arg_2, r4, 0)) {
        if (r4->code == C_EXTERN || r4->code == C_EXTERNINIT ||
            r4->code == C_STATIC || r4->code == C_STATICINIT)
            return r4;
        r4 = r4->next;
    }
    return 0;
}


void perc(sym_t *r4) {
    sym_t *r3 = symtab;
    symtab = r4;
    while (r4 != r3) {
        if (r3->code == C_EXTERN || r3->code == C_STATIC) {
            sym_t *r2 = r3->next;
            r3->next = symtab;
            symtab = r3;
            r3 = r2;
        } else
            r3 = wsfree(r3, r3->next);
    }
}

