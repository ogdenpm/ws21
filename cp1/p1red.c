#include "cp1.h"

uint8_t tBEF7[] = { 0, 0x40, 0x40,    0, 0x41, 0x40, 0x41, 0x41,
                   0x40, 0x40, 0xC0, 0xC0, 0x40, 0xC0, 0xC0, 0xC0,
                   0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
                   0xC0, 0xC0, 0xF0, 0xD0, 0xEC, 0xC7, 0xC3, 0xEB,
                   0xF3, 0xF3, 0xD3, 0xD3, 0xC3, 0xC3, 0xC3, 0xC3,
                   0xE3, 0xE3, 0xCF, 0xD3,    0 };

bool cachk(term_t *r4) {
    return (r4->code == 0 && r4->term.b15 == 0 && r4->dataType != D_FLOAT &&
        r4->dataType != D_DOUBLE && (!aflag || r4->term.b14 == 0 || type(r4->dataType) == D_PTR));
}



bool canadd(term_t *r4, term_t *r2) {
    if (!cachk(r4) || !cachk(r2) || (r4->name[0] && r2->name[0]) || (r4->term.b14 && r2->term.b14))
        return false;
    if (!r4->name[0])
        cpynm(r4->name, r2->name);
    r4->term.lng += r2->term.lng;
    r4->term.b14 += r2->term.b14;
    if (type(r4->dataType) != D_PTR)
        if (type(r2->dataType) == D_PTR) {
            r4->dataType = r2->dataType;
            r4->aux = r2->aux;
        } else
            r4->dataType = maxty(r4->dataType, r2->dataType, D_ULONG);
        return true;
}


bool canmul(term_t *r4, term_t *r2) {
    if (iscons(r4) && iscons(r2)) {
        r4->term.lng *= r2->term.lng;
        r4->dataType = maxty(r4->dataType, r2->dataType, D_ULONG);
        return true;
    }
    return false;
}

bool cansub(term_t *r4, term_t *r2) {
    if (!cachk(r4) || !cachk(r2) || r2->name[0] && !cmpbuf(r4->name, r2->name, 8)
        || (r2->term.b14 != 0 && r4->term.b14 != r2->term.b14))
        return false;

    if (r2->name[0])
        cpynm(r4->name, noname);
    if (r2->term.b14 != 0)
        r4->term.b14 = 0;
    r4->term.lng -= r2->term.lng;
    return true;
}


term_t *cpyterm(term_t *r4, term_t *r2) {
    if (r2->code == 0)
        setad(r4, r2->name, r2->term.lng, r2->term.b14, r2->term.b15);
    else {
        r4->code = r2->code;
        r4->op.lhs = r2->op.lhs;
        r4->op.rhs = r2->op.rhs;
        r4->op.w14 = r2->op.w14;
    }
    setty((sym_t *)r4, r2->dataType, (intptr_t)r2->aux);
    return r4;
}

term_t *reduce(term_t *r4) {
    int var_8;
    if (r4 == 0)
        return 0;
    term_t *r2 = r4->op.lhs;
    term_t *r3 = r4->op.rhs;
    var_8 = tBEF7[scnstr(tyops, r4->code)];
    if (var_8 & 0x80)
        reduce(r3);
    if (var_8 & 0x40)
        reduce(r2);
    if ((var_8 & 0x20) && r2->code == 0 && (r3->code != 0 || iscons(r2))) {
        r4->op.lhs = r3;
        r4->op.rhs = r2;
        r2 = r3;
        r3 = r4->op.rhs;
    }
    if ((var_8 & 0x10) && iscons(r3) && r3->term.lng == 0L)
        return cpyterm(r4, r2);
    if ((var_8 & 8) && iscons(r3) && r3->term.lng == 0L)
        return cpyterm(r4, r3);
    if ((var_8 & 4) && iscons(r3) && r3->term.lng == 1L)
        return cpyterm(r4, r2);
    if ((var_8 & 2) && !iscons(r3))
        return r4;
    if ((var_8 & 1) && !iscons(r2))
        return r4;
    switch(r4->code) {
    case C_ADDRESSOF:
        if (r2->code == 0) {
            r2->term.b15--;
            break;
        }
        if (r2->code == C_DEREF) {
            cpyterm(r4, r2->op.lhs);
            return r4;
        }
        return r4;
    case C_CAST:
        if (r2->code == 0 && (type(r4->dataType) == type(r2->dataType) || (!dtype(r4->dataType) && !dtype(r2->dataType))))
            break;
        return r4;
    case C_DEREF:
        if (r2->code == 0) {
            r2->term.b15++;
            break;
        }
        if (r2->code == C_ADDRESSOF) {
            cpyterm(r4, r2->op.lhs);
            return r4;
        }
        return r4;
    case C_LNOT:
        r2->term.lng = !r2->term.lng;
        break;

    case C_NOT:
        r2->term.lng = ~r2->term.lng;
        break;
    case C_UMINUS:
        if (dlit(r2)) { // 6205
            if (r2->dbl != 0) {                         // leave 0 as is, otherwise toggle sign bit
                uint8_t *bp = (uint8_t *) &r2->dbl;       // msb of 2nd byte is the sign
                bp[1] ^= 0x80;
            }
            break;
        }
        if (iscons(r2)) {
            r2->term.lng = -r2->term.lng;
            break;
        } else
            return r4;
    case C_UPLUS:
        if (dlit(r2) || iscons(r2))
            break;
        return r4;
    case C_QMARK:
        reduce(r4->op.w14);
        if (iscons(r2))
            cpyterm(r4, r2->term.lng != 0 ? r4->op.w14 : r3);
        return r4;
    case C_PLUS:
            if (canadd(r2, r3))
                break;
            if (r2->code == C_PLUS && dtype(r4->dataType) == 0 && !canadd(r2->op.rhs, r3)) { // 6350
                r2->dataType = r4->dataType;
                r2->aux = r4->aux;
                return cpyterm(r4, r2);
            }
            return r4;
        case C_MINUS:
            if (cansub(r2, r3))
                break;
            if (iscons(r3)) {
                r4->code = C_PLUS;
                r3->term.lng = -r3->term.lng;
            }
            return r4;
        case C_STAR:
            if (canmul(r2, r3))
                break;
            if (r2->code == C_PLUS && dtype(r4->dataType) && canmul(r2->op.rhs, r3)) {
                r2->code = C_STAR;
                r4->code = C_PLUS;
                r4->op.rhs = r2->op.rhs;
                r2->op.rhs = r3;
                return r4;
            }
            return r4;
        case C_DIV:
            r2->term.lng /= r3->term.lng;
            break;
        case C_MOD:
            r2->term.lng %= r3->term.lng;
            break;
        case C_AND:
            r2->term.lng &= r3->term.lng;
            break;
        case C_OR:
            r2->term.lng |= r3->term.lng;
            break;
        case C_XOR:
            r2->term.lng ^= r3->term.lng;
            break;
        case C_LSHIFT:
            r2->term.lng <<= (short)r3->term.lng;
            break;
        case C_RSHIFT:
            r2->term.lng >>= (short)r3->term.lng;
            break;
        case C_LT:
            r2->term.lng = r2->term.lng < r3->term.lng;
            break;
        case C_LE:
            r2->term.lng = r2->term.lng <= r3->term.lng;
            break;
        case C_GT:
            r2->term.lng = r2->term.lng > r3->term.lng;
            break;
        case C_GE:
            r2->term.lng = r2->term.lng >= r3->term.lng;
            break;
        case C_EQEQ:
            r2->term.lng = r2->term.lng == r3->term.lng;
            break;
        case C_NE:
            r2->term.lng = r2->term.lng != r3->term.lng;
            break;
        default:
            return r4;
    }
    setad(r4, r2->name, r2->term.lng, r2->term.b14, r2->term.b15);
    return r4;
}

