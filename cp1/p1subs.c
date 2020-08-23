#include "cp1.h"

int16_t bndtab[] = { 0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 1, -1 };
uint16_t bytab[] = { 1, 1, 2, 2, 2, 4, 4, 4, 4, 8, 2, 0 };      // type sizes in bytes
uint8_t typtab[] = { D_CHAR, D_UCHAR, D_SHORT, D_FSHORT, D_USHORT, D_LONG, D_FLONG, D_ULONG,
                     D_FLOAT, D_DOUBLE, D_PTR, 0 };

long bndify(int arg_2, aux_t *r4, long arg_6) {
    uint16_t var_8;

    var_8 = (1 << bound(arg_2, r4)) - 1;
    arg_6 += var_8;
    arg_6 &= ~var_8;
    return arg_6;
}



int bound(int r4, aux_t *r2) {
    int var_8;
    sym_t *r3;
    if ((var_8 = bndtab[scnstr(typtab, type(r4))]) != -1)
        return min(var_8, bndef);
    if (type(r4) == D_ARRAY)
        return bound(dety(r4), r2->next);
    if (r4 == D_STRUCT || r4 == D_UNION) {
        var_8 = 0;
        r3 = r2->psym->schain;
        while (r3) {
            var_8 = max(var_8, bound(r3->dataType, r3->aux));
            r3 = r3->next;
        }
        return var_8;
    }
    return 4;
}



aux_t *buymat(long arg_2, aux_t *arg_6) {
    aux_t *r4 = alloc(sizeof(aux_t), arg_6);    // sizeof => 6
    r4->lng = arg_2;
    return r4;
}

term_t *buyop(int arg_2, term_t *arg_4, term_t *arg_6) {
    term_t *r4 = alloc(sizeof(term_t), exlist);    // sizeof => 0x16
    exlist = r4;
    r4->dataType = 0;
    r4->aux = 0;
    r4->code = arg_2;
    r4->op.lhs = arg_4;
    r4->op.rhs = arg_6;
    r4->op.w14 = 0;
    return r4;
}

aux_t *buysat(sym_t *r4, aux_t *arg_4) {
    aux_t *r2 = alloc(sizeof(aux_t), arg_4);        // sizeof => 6
    r2->psym = r4;
    return r2;
}


sym_t *buysym(int arg_2, aux_t *arg_4, char *r4, int arg_8) {
    sym_t *r2 = alloc(sizeof(sym_t), 0);             // sizeof => 0x14
    setty(r2, arg_2, (intptr_t)arg_4);
    cpynm(r2->name, r4 ? r4 : noname);
    r2->code = arg_8;
    r2->lng = 0L;
    return r2;
}

term_t *buyterm(int arg_2, aux_t *arg_4, char *arg_6, long arg_8, uint8_t arg_C, uint8_t arg_E) {
    term_t *r4 = alloc(sizeof(term_t), exlist);    // sizeof => 0x16
    exlist = r4;
    r4->dataType = arg_2;
    r4->aux = arg_4;
    r4->code = 0;
    cpynm(r4->name, arg_6 ? arg_6 : noname);
    r4->term.lng = arg_8;
    r4->term.b14 = arg_C;
    r4->term.b15 = arg_E;
    return r4;
}

long bytes(int r4, aux_t *r2) {

    long var_10;
    long var_C;
    int var_8;
    term_t *r3;

    if ((var_8 = bytab[scnstr(typtab, type(r4))]) != 0)
        return (long)var_8;
    switch (type(r4)) {
    case D_ARRAY:
        if (r2->lng == 0)
            wsperror("array size unknown");
        return bytes(dety(r4), r2->next) * r2->lng;
    case D_STRUCT:
        if ((r3 = r2->psym->tchain) == 0) {
            wsperror("structure size unknown");
            return 0L;
        }
        while (r3->next)
            r3 = r3->next;
        return bndify(r4, r2, r3->term.lng + bytes(r3->dataType, r3->aux));
    case D_UNION:
        if ((r3 = r2->psym->tchain) == 0) {
            wsperror("union size unknown");
            return 0L;
        }
        var_C = 0L;
        while (r3) {
            var_10 = bytes(r3->dataType, r3->aux);
            var_C = max(var_C, var_10);
            r3 = r3->next;
        }
        return bndify(r4, r2, var_C);
    default:
        wsperror("function size undefined");
        return 0L;
    }
}


void clrsym(sym_t *r4) {
    r4->next = 0;
    r4->dataType = 0;
    r4->aux = 0;
    int r2;
    for (r2 = 0; r2 < 8; r2++)
        r4->name[r2] = 0;
    r4->code = 0;
    r4->lng = 0L;
}

void cpynm(char *r4, char *r2) {
    int r3 = 8;
    while (--r3 >= 0)
        *r4++ = *r2++;
}


tok_t *cpytok(tok_t *r4, tok_t *r2) {
    r4->code = r2->code;
    cpynm(r4->name, r2->name);
    return r4;
}

int dety(int arg_2) {
    return (arg_2 >> 2) & 0x3fff;
}

int dlit(term_t *r4) {
    return r4->dataType == D_DOUBLE && r4->code == 0 && r4->term.b15 == 0;
}


int dtype(int arg_2) {
    return arg_2 == 0x28 || arg_2 == 0x24;
}

void exchk(sym_t *r4) {

    if (nlen >= 8 && cflag == 0)
        return;
    sym_t *r2 = r4;

    while (r2 = r2->next)
        if (exmatch(r4, r2)) {
            nmerr("external name conflict", r4->name);
            break;
        }
}

bool exmatch(sym_t *arg_2, sym_t *arg_4) {
    if (arg_4->code != C_EXTERN && arg_4->code != C_EXTERNINIT
        && arg_4->code != C_STATIC && arg_4->code != C_STATICINIT)
        return 0;
    int r4 = nlen < 8 ? nlen : 8;
    char *r2 = arg_2->name;
    char *r3 = arg_4->name;

    while (--r4 >= 0 && (cflag && tolower(*r2) == tolower(*r3)) || (!cflag && *r2 == *r3)) {
        r2++;
        r3++;
    }
    return r4 < 0;
}


void fixlint() {
    int r4 = scnstr(typtab, D_PTR);
    bitswd = 32;
    intsize = 4;
    tfield = D_FLONG;
    tint = D_LONG;
    tunsign = D_ULONG;
    bndtab[r4] = 2;     // treat as long
    bytab[r4] = 4;      // pointer 4 bytes
}



int iscons(term_t *r4) {
    return r4->code == 0 && r4->name[0] == 0 && r4->term.b14 == 0 && r4->term.b15 == 0 && r4->dataType != C_DOUBLE;
}


int itype(arg_2) {
    return (scnstr(typtab, type(arg_2)) < 7);
}



uint16_t lname(char *r4) {
    int r2 = 8;
    while (--r2 >= 0) {
        if (r4[r2])
            return r2 + 1;
    }
    return 0;
}



sym_t *lookup(char *arg_2, sym_t *r4, sym_t *arg_6) {
    int var_8;
    char *r2, *r3;
    while (r4 != arg_6) {
        r2 = r4->name;
        r3 = arg_2;
        for (var_8 = 8; var_8 > 0 && *r2++ == *r3++; var_8--)
            ;
        if (var_8 == 0)
            return r4;
        r4 = r4->next;
    }
    return 0;
}


int maxify(int arg_2) {
    switch (arg_2) {
    case 0x1c: case 0x10:
        return tunsign;
    case 0x14: case 0xc: case 0x8: case 0x4:
        if (arg_2 < tint)
            return tint;
    default:
        return arg_2;
    case 0x24:
        return 0x28;
    }
}



int ptype(int arg_2) {
    return (arg_2 & 3) == 3;
}


sym_t *retype(sym_t *r4, int r2) {
    int r3;
    if (r4 == 0)
        wsperror("incomplete declaration");
    else {
        for (r3 = 0; r4->dataType & (3 << (r3 * 2)); r3++)
            ;
        if (r3 > 5)
            wsperror("declaration too complex");
        else
            r4->dataType = (r4->dataType << 2) | r2;
    }
    return r4;
}


term_t *setad(term_t *r4, char *r2, long arg_6, uint8_t arg_A, uint8_t arg_C) {
    r4->code = 0;
    cpynm(r4->name, r2 ? r2 : noname);
    r4->term.lng = arg_6;
    r4->term.b14 = arg_A;
    r4->term.b15 = arg_C;
    return r4;
}

sym_t *setty(sym_t *r4, int arg_4, intptr_t arg_6) {    // temporary hack until I better understand the 3rd arg

    r4->dataType = arg_4;
    r4->intPtr = arg_6;
    return r4;
}

int stype(int arg_2) {
    return typtab[scnstr(typtab, type(arg_2))] != 0;
}

int type(int r4) {
    if (r4 & 3)
        return r4 & 3;
    return r4;
}

