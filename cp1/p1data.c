#include "cp1.h"

int decflag;



long arinit(int arg_2, aux_t *arg_4, term_t *r4, bool arg_8) {
    long var_A;
    sym_t *r3;
    short r2;

    if ((arg_2 == D_CHAR || arg_2 == D_UCHAR) && r4 && (r3 = lookup(r4->name, (sym_t *)littab, 0))) { //1889
        r2 = r3->len;
        pstr(r3->str, r2 - 1);
        r3->code = 0;
        if (arg_4->lng == 0)
            arg_4->lng = r2;
        else if (arg_4->lng < r2)
            wsperror("string initializer too long");
        var_A = r2;
    } else {
        var_A = 0L;
        r2 = 0;
        while (arg_4->lng == 0 || r2 < arg_4->lng) {
            if (r4 != 0 || eat(C_LBRACE) != 0 || (r4 = gexpr(false)) != 0)
                break;
            var_A += dinit(arg_2, arg_4->next, r4, 10);
            if (r4 == 0)
                need(C_RBRACE);
            if (arg_8)
                eat(C_COMMA);
            r2++;
            r4 = 0;
        }
        if (arg_4->lng == 0)
            arg_4->lng = r2;
    }
    return var_A;
}


void datinit(sym_t *r4) {
    uint16_t var_8;
    bool r2;
    term_t *r3;

    decflag = 1;
    eat(C_EQUAL);
    var_8 = bound(r4->dataType, r4->aux);
    if ((r3 = gexpr(0)) || (r2 = eat(C_LBRACE)) || r4->code == C_LABEL) { // 1C03
        if (r4->code == C_EXTERNINIT || r4->code == C_STATICINIT)
            nmerr("redefined", r4->name);
        else if (r4->code == C_EXTERN) {
            r4->code = C_EXTERNINIT;
            pdata(r4->name, var_8);
        } else if (r4->code == C_STATIC) { //1BCC
            r4->code = C_STATICINIT;
            pdata(r4->name, var_8);
        } else if (r4->code == C_LABEL)
            pdata(lblname(r4->w), var_8);

        dinit(r4->dataType, r4->aux, r3 ? r3 : gexpr(0), r3 == 0);
        if (r3 == 0 && r2 != 0 && !eat(C_RBRACE))
            recover("missing }");
        pend();
    }
    decflag = 0;
}



long dinit(int arg_2, aux_t *arg_4, term_t *r4, bool arg_8) {
    long var_16 = 0L;
    long var_12;
    wsDouble var_E;
    short r2;

    if (type(arg_2) == D_ARRAY)
        var_16 = arinit(dety(arg_2), arg_4, r4, arg_8);
    else if (type(arg_2) == D_STRUCT)
        var_16 = stinit(arg_4->psym->psym->schain, r4, arg_8);
    else if (r4 || (r4 = gexpr(0))) {
        switch (type(arg_2)) {
        case D_UNION:
            if (arg_4->psym->schain)
                var_16 = dinit(arg_4->psym->schain->dataType, arg_4->psym->schain->aux, r4, arg_8);
            break;
        case 0x20: case 0x18: case 0x14: case 0xc: case 0x8: case 0x4:
            if (iscons(r4) == 0)
                wsperror("illegal integer initializer");
            else {
                r2 = (short)bytes(arg_2, arg_4);
                var_16 = pint(r4->term.lng, r2);
            }
            break;
        case D_PTR:
            if (r4->code != 0 || r4->term.b15 > 1 || r4->term.b14 != 0)
                wsperror("illegal pointer initializer");
            else
                var_16 = paddr(r4->name, r4->term.lng, bound(dety(arg_2), arg_4));
            break;
        case D_DOUBLE: case D_FLOAT:
            r2 = (short)bytes(arg_2, arg_4);
            if (dlit(r4) != 0)
                var_16 = pfloat(&r4->dbl , r2);
            else if (iscons(r4) != 0) { // 2030
                var_E = long2WsDouble(r4->term.lng);      // portable version to get ws double format
                var_16 = pfloat(&var_E, r2);
            } else
                wsperror("illegal double initializer");
            break;
        default:
            wsperror("cannot initialize");
            break;
        }
        if (arg_8)
            eat(C_COMMA);
    }
    var_12 = bytes(arg_2, arg_4);
    pspace(var_12 - var_16);
    return var_12;
}

long stinit(sym_t *r4, term_t *r2, bool arg_6) {

    long var_E = 0L;
    long var_A;
    term_t *r3;

    while (r4 && (r2 != 0 || eat(C_LBRACE) || (r2 = gexpr(false)))) {
        while (r4 && r4->name[0] == 0)
            r4 = r4->next;
        if (!r4)
            break;
        pspace(r4->lng - var_E);
        if (r4->dataType != tfield)
            var_E = dinit(r4->dataType, r4->aux, r2, arg_6) + r4->lng;
        else {
            for (var_A = 0, r3 = r2; r3 || (r3 = gexpr(0)); r4 = r4->next) {
                if (r4->name[0]) {
                    if (!iscons(r3))
                        wsperror("illegal field initializer");
                    else
                        var_A |= (((1L << r4->aux->b[1]) - 1) & r3->term.lng) << r4->aux->b[0];
                    if (arg_6 == 0)
                        eat(C_COMMA);
                    r3 = 0;
                }
                if (!r4 || r4->next->dataType != tfield || r4->next->name[0] == 0)
                    break;
            }
            var_E = pint(var_A, bytes(tfield, 0)) + r4->lng;
        }
        if (r2 == 0)
            need(C_RBRACE);
        if (arg_6 == 0)
            eat(C_COMMA);
        r2 = 0;
        r4 = r4->next;
    }
    return var_E;
}

