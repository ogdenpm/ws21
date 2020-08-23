#include "cp1.h"

term_t opBE94 = { .dataType = 4 };
uint8_t tBEAA[] = { C_EQEQ, C_NE, C_LT, C_LE, C_GT, C_GE, 0 };
uint8_t tBEB1[] = { C_NE, C_EQEQ, C_GE, C_GT, C_LE, C_LE, 0 };
// note tBEB8 may have different constant definitions as output codes
uint8_t tBEB8[] = { C_EQRSHIFT, C_GT, C_EQOR, C_EQPLUS, C_EQUAL, C_EQDIV, 0 };

int wBEBF = -1;

term_t op_BEC9;

uint8_t tBEEB[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb,  5 };


int crs() {
    return wBEBF += 2;
}


char *lblname(int r4) {
    static uint8_t lblStr[8];
    int r2;
    lblStr[0] = '>';
    for (r2 = 1; r4 && r2 < 6; r2++) {
        lblStr[r2] = (r4 & 7) + '0';
        r4 = (r4 >> 3) & 0x1ff;
    }
    while (r2 < 8)
        lblStr[r2++] = 0;
    return lblStr;
}

int paddr(char *r4, long arg_4, int arg_8) {
    int r2;

    r2 = bytes(D_PTR, 0 /*, 1 */);        // original code is wrong and has 3 args, however will return sizeof ptr
    if (*r4) {
        pcode(arg_8 + 192);
        ptname(r4);
        ptint(arg_4);
    } else
        pint(arg_4, r2);
    return r2;
}


void pauto(long arg_2) {
    pcode(0x81);
    ptint(arg_2);
}



int pcase(short r4) {
    pcode(0x82);
    ptlab(r4);
    ptlab(lineno);
    return r4;
}

void pcode(int arg_2) {
    putch(arg_2 & 0xff);
}


void pdata(char *arg_2, int arg_4) {
    pcode(0x83);
    pcode(arg_4);
    ptname(arg_2);
}

void pdef(char *arg_2) {
    pcode(0x84);
    ptname(arg_2);
}

void pend() {
    term_t *r2 = littab;
    while (r2 != 0) {
        if (r2->code != 0) {
            pcode(0xB8);
            pcode(0);
            ptname(r2->name);
            int r4 = r2->lit.len;
            pstr(r2->lit.str, r4 - 1);
        }
        wsfree(r2->lit.str, 0);
        r2 = wsfree(r2, r2->next);
    }
    littab = 0;
}


int pfloat(wsDouble *arg_2, int r2) {
    uint8_t *r4 = (uint8_t *)arg_2;
    pcode(0x85);
    pcode(r2);
    int r3 = 8;
    while (--r3 >= 0)
        pcode(*r4++);
    return r2;
}



void pfunc(char *arg_2) {
    pcode(0x86);
    ptname(arg_2);
}


int pint(long arg_2, int r4) {
    pcode(0x89);
    pcode(r4);
    ptint(arg_2);
    return r4;
}

int pjc(int arg_2, term_t *arg_4, term_t *arg_6, int r4) {
    if (r4 == 0)
        r4 = crs();
    pcode(tBEB8[scnstr(tBEAA, arg_2)]);
    ptlab(r4);
    ptexpr(arg_4);
    ptexpr(arg_6);
    return r4;
}



int pjf(term_t *r4, int r2, int r3) {
    int8_t var_7;
    if (r4 == 0)
        return r2 ? r2 : crs();
    if (r4->code == C_ANDAND) {
        r2 = pjf(r4->op.lhs, r2, 0);
        return pjf(r4->op.rhs, r2, r3);
    }
    if (r4->code == C_OROR) {
        r3 = pjt(r4->op.lhs, 0, r3);
        return pjf(r4->op.rhs, r2, r3);
    }
    if (r4->code == C_LNOT)
        return pjt(r4->op.lhs, r3, r2);

    if (tBEAA[var_7 = scnstr(tBEAA, r4->code)])
        r2 = pjc(tBEB1[var_7] & 0xff, r4->op.lhs, r4->op.rhs, r2);
    else
        r2 = pjc(0x94, r4, &opBE94, r2);
    plabel(r3);
    return r2;
}



int pjt(term_t *r4, int r2, int r3) {
    if (r4 == 0)
        return pjump(r3, r2);

    if (r4->code == C_ANDAND) {
        r2 = pjf(r4->op.lhs, r2, 0);
        return pjt(r4->op.rhs, r2, r3);
    }
    if (r4->code == C_OROR) {
        r3 = pjt(r4->op.lhs, 0, r3);
        return pjt(r4->op.rhs, r2, r3);
    }

    if (r4->code == C_LNOT)
        return pjf(r4->op.lhs, r3, r2);

    if (tBEAA[scnstr(tBEAA, r4->code)])
        r3 = pjc(r4->code, r4->op.lhs, r4->op.rhs, r3);
    else
        r3 = pjc(C_NE, r4, &opBE94, r3);

    plabel(r2);
    return r3;
}



int pjump(int r4, int r2) {
    if (r4 == 0)
        r4 = crs();
    pcode(0x8b);
    ptlab(r4);
    plabel(r2);
    return r4;
}


int plabel(int r4) {
    if (r4) {
        pcode(0x8C);
        ptlab(r4);
        ptlab(lineno);
    }
    return r4;
}


void pmove(term_t *arg_2, term_t *arg_4) {
    op_BEC9.code = 0xaf;
    op_BEC9.aux = 0;
    op_BEC9.op.lhs = arg_2;
    op_BEC9.op.rhs = arg_4;
    pvoid(typify(&op_BEC9));
}



void pref(char *arg_2) {
    pcode(0x90);
    ptname(arg_2);
}


void pregs(int arg_2) {
    pcode(0x91);
    pcode(arg_2);
}

void pret() {
    pcode(0x92);
}


void pspace(long arg_2) {
    if (arg_2 > 0) {
        pcode(0x93);
        ptint(arg_2);
    }
}

void pstr(char *r4, int r2) {
    pcode(0x94);
    pcode(r2);
    while (r2 > 0) {
        pcode(*r4++);
        r2--;
    }
}


void pswitch(short arg_2) {
    pcode(0x95);
    ptlab(arg_2);
}

void pswtab(case_t *r4, int arg_4, int arg_6) {

    pcode(0xB9);
    pcode(bound(tunsign, 0));
    ptname(lblname(arg_6));
    case_t *r2 = r4->next;
    while (r2 != r4) { // 554A
        paddr(lblname(r2->caseLabel), 0L, 4);
        pint(r2->caseValue, intsize);
        r2 = wsfree(r2, r2->next);
    }
    pint(0L, intsize);
    paddr(lblname(arg_4), 0L, 4);
    pcode(0x97);
}


void ptexpr(term_t *r4) {
    pcode(r4->code);
    int r2 = tBEEB[scnstr(typtab, type(r4->dataType))];
    if (r2 != 0xb)
        pcode(r2);
    else
        pcode(bound(dety(r4->dataType), r4->aux) * 16 | 0xb);

    if (r4->dataType == tfield) {
        pcode(r4->aux->b[0]);
        pcode(r4->aux->b[1]);
    }
    if (r4->code) {
        ptexpr(r4->op.lhs);
        if (r4->code == C_QMARK)
            ptexpr(r4->op.w14);
        if (r4->code & 0x80)
            ptexpr(r4->op.rhs);
        return;
    }
    if (r4->dataType == D_DOUBLE && r4->term.b15 == 0 && r4->term.b14 == 0) { // 5745
        pcode(8);
        r2 = 8;
        char *r3 = r4->name;
        while (--r2 > 0)
            pcode(*r3++);
    } else
        ptname(r4->name);

    ptint(r4->term.lng);
    pcode(r4->term.b14);
    pcode(r4->term.b15);
}




void ptint(long arg_2) { 
    pcode((arg_2 >> 16) & 0xff);
    pcode(arg_2 >> 24);
    pcode(arg_2 & 0xff);
    pcode((arg_2 >> 8) & 0xff);
}


void ptlab(short arg_2) {
    pcode(arg_2 & 0xff);
    pcode((arg_2 >> 8) & 0xff);
}


void ptname(char *r4) {
    int r2 = lname(r4);
    if (r2 > nlen)
        r2 = nlen;
    pcode(r2);
    while (--r2 >= 0) {
        pcode(cflag ? tolower(*r4) : *r4);
        r4++;
    }
}


void pvoid(term_t *arg_2) {

    docheck(arg_2);
    pcode(0x96);
    ptexpr(arg_2);
}

int rbuy(int r4, int *r2) {
    if (r2 == 0)
        return 0;
    switch (type(r4)) {
    case D_CHAR: break;
    case D_UCHAR: break;
    case D_SHORT: break;
    case D_USHORT: break;
    case D_PTR: break;
    case D_ULONG: case D_LONG:
        if (r4 <= tunsign)
            break;
    default:
        return 0;
    }
    int r3 = *r2;
    *r2 = r3 & (r3 - 1);
    return r3 ^ *r2;
}



