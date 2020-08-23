#include "cp1.h"

sym_t symBF80 = { .auxb[0] = 4 };

uint16_t tBF94[] = {
        0x010, 0x011, 0x100, 0x018, 0x118, 0x018, 0x118, 0x119,
        0x119, 0x119, 0x0F9, 0x0F9, 0x100, 0x0D8, 0x0D8, 0x0F9,
        0x0F9, 0x0F9, 0x0FD, 0x0FD, 0x0FB, 0x0FB, 0x0FB, 0x0FB,
        0x0FB, 0x0FB, 0x1D8, 0x1D8, 0x1DC, 0x1DC, 0x1DA, 0x1DA,
        0x1DA, 0x1DA, 0x1DB, 0x1DB, 0x1D8, 0x1D8, 0x1D8, 0x1D8,
        0x1D8, 0x1D8, 0x1D8, 0x1D8, 0x069,    0 };

uint8_t tyops[] = {
        0x61, 0x64, 0xA9,    7, 0x66, 0x6B, 0x5A, 0x43,
        0x6C, 0x6D, 0xAA, 0xA7, 0x69, 0xA8, 0x9F, 0x88,
        0x8E, 0x8B, 0x91, 0x87, 0x8C, 0x86, 0x92, 0x8D,
        0x89, 0x8A, 0xDE, 0xD8, 0xE2, 0x85, 0x99, 0xC1,
        0x9C, 0xA3, 0x97, 0xA0, 0x95, 0x96, 0x8F, 0x90,
        0x94, 0x9B, 0x82, 0x9D, 0xAF, 0 };


void docheck(term_t *r4) {
    while (r4->code == 0xa8)
        r4 = r4->op.rhs;
    switch (r4->code) {
    case 0xaa:
        r4->code = C_EQPLUS;
        break;
    case 0xa7:
        r4->code = C_EQOR;
        break;
    case C_QMARK:
    case 0xa9:
    case 0x69:
    case C_EQUAL:
    case C_EQPLUS:
    case C_EQMINUS:
    case C_EQMUL:
    case C_EQDIV:
    case C_EQMOD:
    case C_EQAND:
    case C_EQXOR:
    case C_EQOR:
    case C_EQLSHIFT:
    case C_EQRSHIFT:
        break;
    default:
        perror("useless expression");
    }
}


int maxty(int arg_2, int arg_4, int arg_6) {

    int r4 = scnstr(typtab, arg_2);
    int r2 = scnstr(typtab, arg_4);
    int r3 = scnstr(typtab, arg_6);
    if (r3 < r4 || r3 < r2)
        perror("illegal operand type");
    else
        r3 = max(r2, r4);
    return typtab[r3];
}

term_t *ptify(term_t *arg_2, int arg_4, aux_t *arg_6) {
    if (arg_2->dataType > tunsign)
        setty((sym_t *)(arg_2 = buyop(C_CAST, arg_2, 0)), tunsign, 0);
    return (term_t *)setty((sym_t *)buyop(C_STAR, arg_2, buyterm(tint, 0, 0, bytes(dety(arg_4), arg_6), 0, 0)), tint, 0);
}

term_t *scalify(term_t *r4) {
    term_t *r2;
    if (!r4)
        return 0;
    switch (type(r4->dataType)) {
    case D_ARRAY:
        setty((sym_t *)r4, (r4->dataType & 0xfffc) | D_PTR, (intptr_t)r4->aux->next);
        break;
    case D_FUNC:
        retype((sym_t *)r4, D_PTR);
        break;
    case D_UNION: case D_STRUCT:
        perror("illegal structure reference");
        break;
    default:
        return r4;
    }

    if (r4->code == 0)
        r4->term.b15--;
    else {
        r4->op.lhs = r2 = buyop(r4->code, r4->op.lhs, r4->op.rhs);
        r4->code = C_ADDRESSOF;
        r2->op.w14 = r4->op.w14;
        r2->dataType = (D_CHAR << 2) | D_PTR;
    }
    return r4;
}

void tfn(term_t *r4) {
    term_t *r2 = r4->op.lhs;
    term_t *r3 = r4->op.rhs;
    if (r2->code == 0 && r2->dataType == 0)
        setty((sym_t *)r2, (tint << 2) | D_FUNC, 0L);
    else if (type(typify(r2)->dataType) != D_FUNC)
        perror("function required");
    else if (stype(dety(r2->dataType)))
        setty((sym_t *)r4, maxify(dety(r2->dataType)), (intptr_t)r2->aux->next);
    else
        perror("illegal return type");
    while (r3)
        if (r3->code != C_LISTNEXT) {
            reduce(scalify(typify(r3)));
            return;
        } else {
            reduce(scalify(typify(r3->op.lhs)));
            r3 = r3->op.rhs;
        }
}


void tpoints(term_t *r4, term_t *r2, term_t *arg_6) {

    sym_t *var8;
    var8 = mostab;
    sym_t *r3;

    if (mflag || (r4->dataType != (D_STRUCT << 2) + D_PTR && r4->dataType != (D_UNION << 2) + D_PTR)) {
        if (mflag)
            perror("illegal member");
        else if (itype(r4->dataType))
            arg_6->op.lhs = r4 = (term_t *)setty((sym_t *)buyop(C_CAST, r4, 0), (D_CHAR << 2) | D_PTR, 0);
        else if (!ptype(r4->dataType))
            perror("illegal selection");
    } else
        var8 = r4->aux->psym->schain;
    if (r2->name[0] == 0)
        r3 = &symBF80;
    else if ((r3 = lookup(r2->name, var8, 0)) == NULL) {
        nmerr("unknown member", r2->name);
        r3 = &symBF80;
    }
    arg_6->op.lhs = (term_t *)setty((sym_t *)buyop(C_PLUS, r4, r2), (D_CHAR << 2) | D_PTR, 0);
    arg_6->code = C_DEREF;
    setty((sym_t *)arg_6, r3->dataType, (intptr_t)r3->aux);
    setad(r2, 0, r3->lng, 0, 0);
}



void tquery(term_t *r4) {
    term_t *r3 = r4->op.rhs;
    term_t *r2 = scalify(typify(r4->op.w14));
    if (ptype(r2->dataType) && itype(r3->dataType))
        setty((sym_t *)r4, r2->dataType, (intptr_t)r2->aux);    // 7de0
    else if (itype(r2->dataType) && ptype(r3->dataType))
        setty((sym_t *)r4, r3->dataType, (intptr_t)r3->aux);
    else if (ptype(r2->dataType) && ptype(r3->dataType))
        setty((sym_t *)r4, r2->dataType, (intptr_t)r2->aux);    // 7de0
    else
        r4->dataType = maxty(r2->dataType, r3->dataType, D_DOUBLE);

    r4->dataType = maxify(r4->dataType);
    if (r2->dataType != r4->dataType)
        r4->op.w14 = (term_t *)setty((sym_t *)buyop(C_CAST, r2, 0), r4->dataType, (intptr_t)r4->aux);

    if (r3->dataType != r4->dataType)
        r4->op.rhs = (term_t *)setty((sym_t *)buyop(C_CAST, r3, 0), r4->dataType, (intptr_t)r4->aux);
}

term_t *typify(term_t *r4) {
    if (!r4)
        return NULL;
    term_t *r2 = r4->op.lhs;
    term_t *r3 = r4->op.rhs;
    int var_8 = tBF94[scnstr(tyops, r4->code)];
    if (var_8 & 0x80)
        typify(r3);
    if (var_8 & 0x40)
        scalify(r3);
    if (var_8 & 0x10)
        typify(r2);
    if (var_8 & 0x8)
        scalify(r2);

    if ((var_8 & 0x20) && r2->code != C_DEREF && r2->code != C_ARROW) {
        if (r2->code != 0 || (r2->term.b15 == 0 && (r2->term.b14 == 0 || (r2->term.b14 & 0x20))))
            perror("lvalue required");
    }
    if (var_8 & 4)
        r4->dataType = maxty(r2->dataType, r3->dataType, D_DOUBLE);
    if (var_8 & 2)
        r4->dataType = maxty(r2->dataType, r3->dataType, D_ULONG);
    if (var_8 & 1)
        setty((sym_t *)r4, r2->dataType, (intptr_t)r2->aux);

    switch (r4->code) {
    case 0x0:
        if (r4->dataType == 0 && decflag == 0)
            nmerr("undeclared", r4->name);
        break;
    case 0x61:
        setad(r4, 0, bytes(r2->dataType, r2->aux), 0, 0);
        break;
    case C_ADDRESSOF:
        if (type(r2->dataType) == tfield || (r2->code != C_DEREF && r2->code != C_ARROW &&
            (r2->code || r2->term.b15 == 0)))
            perror("illegal &");
        retype((sym_t *)r4, D_PTR);
        break;
    case C_ENDLIST: case C_STARTLIST:
        tfn(r4);
        break;
    case C_ARROW:
        tpoints(r2, r3, r4);
        break;
    case C_CAST:
        if (!stype(r4->dataType)) {
            perror("illegal cast");
            setty((sym_t *)r4, tint, 0);
        }
        break;
    case C_DEREF:
        if (type(r2->dataType) != D_PTR)
            perror("illegal indirection");
        else
            setty((sym_t *)r4, dety(r2->dataType), (intptr_t)r2->aux);
        break;
    case C_NOT:
        if (itype(r4->dataType) == 0)
            perror("integer type required");
        break;
    case C_UPLUS: case C_UMINUS:
        if (!itype(r4->dataType) && !dtype(r4->dataType))
            perror("arithmetic type required");
        break;
    case C_NEXTEXPR:
        docheck(r2);
        setty((sym_t *)r4, r3->dataType, (intptr_t)r3->aux);
        break;
    case C_QMARK:
        tquery(r4);
        break;
    case 0xaf:
        r4->code = C_EQUAL;
        // fall through
    case C_EQUAL:
        if (ptype(r2->dataType) == 0 || dtype(r3->dataType) == 0)
            if (dtype(r2->dataType) == 0 && ptype(r3->dataType) != 0)
                perror("illegal assignment");
        break;
    case 0xa7: case 0xaa:
        if (dtype(r4->dataType) != 0)
            perror("illegal postop");
        // fall through
    case C_EQMINUS: case C_EQPLUS:
        if (ptype(r2->dataType) != 0 && itype(r3->dataType) != 0)
            r4->op.rhs = ptify(r3, r2->dataType, r2->aux);
        else if (ptype(r2->dataType) != 0 || ptype(r3->dataType) != 0)
            perror("illegal =+");
        break;
    case C_PLUS:
        if (ptype(r2->dataType) && itype(r3->dataType)) { //857c
            r4->op.rhs = ptify(r3, r2->dataType, r2->aux);
            setty((sym_t *)r4, r2->dataType, (intptr_t)r2->aux);
        } else if (itype(r2->dataType) && ptype(r3->dataType)) {
            r4->op.lhs = ptify(r2, r3->dataType, r3->aux);
            setty((sym_t *)r4, r3->dataType, (intptr_t)r3->aux);
        } else
            r4->dataType = maxty(r2->dataType, r3->dataType, D_DOUBLE);
        break;
    case C_MINUS:
        if (ptype(r2->dataType) && itype(r3->dataType)) {
            r4->op.rhs = ptify(r3, r2->dataType, r2->aux);
            setty((sym_t *)r4, r2->dataType, (intptr_t)r2->aux);
        } else if (ptype(r2->dataType) && ptype(r3->dataType) &&
            bytes(dety(r2->dataType), r2->aux) == bytes(dety(r3->dataType), r3->aux)) {
            r4->op.lhs = (term_t *)setty((sym_t *)buyop(C_MINUS, r2, r3), r2->dataType, (intptr_t)r2->aux);
            r4->op.rhs = buyterm(tint, 0, 0, bytes(dety(r2->dataType), r2->aux), 0, 0);
            r4->code = C_DIV;
        } else
                r4->dataType = maxty(r2->dataType, r3->dataType, D_DOUBLE);
        break;
    case C_GT: case C_LE: case C_GE: case C_LT:
        if (r4->code == C_LT || r4->code == C_GE)
            untest(r2, r3);
        else
            untest(r3, r2);

    case C_NE: case C_EQEQ:
        if (ptype(r2->dataType) != 0 && itype(r3->dataType) != 0)
            break;
        if (itype(r2->dataType) != 0 && ptype(r3->dataType) != 0)
            break;
        if (ptype(r2->dataType) != 0 && ptype(r3->dataType) != 0)
            break;
        if (ptype(r2->dataType) != 0 || ptype(r3->dataType) != 0)
            perror("illegal comparison");
        break;
    }   // default falls through
    if (r4->dataType == 0)
        r4->dataType = tint;
    else if (var_8 & 0x100)
        r4->dataType = maxify(r4->dataType);
    return r4;
}



void untest(term_t *r4, term_t *r2) {
    if (iscons(r2) && r2->term.lng == 0L) {
        if (ptype(r4->dataType) == 0 &&
            (r4->dataType == D_UCHAR || r4->dataType == D_USHORT || r4->dataType == D_ULONG))
            wsperror("illegal unsigned compare");
    }
}

