#include "cpp.h"

char escChar[] = "btnvfrBTNVFR(!)^";
char escMap[] = {'\b','\t','\n','\v','\f','\r','\b','\t','\n','\v','r', '\r', '{', '|', '}', '~'};
uint8_t opMap[] =  {C_STAR, C_DIV, C_MOD, C_PLUS, C_MINUS, C_LSHIFT, C_RSHIFT, C_LT, C_LE, C_GT, C_GE,
                       C_EQEQ, C_NE, C_AND, C_XOR, C_OR, C_ANDAND, C_OROR, C_QMARK,   0};
uint8_t opPrec[] = {0x0E,   0x0E,  0x0E,  0x0D,   0x0D,    0x0C,     0x0C,     0x0B, 0x0B, 0x0B, 0x0B,
                       0x0A,   0x0A, 9,     8,     7,    6,        5,      4};

code_t punctTab[] = {
    { "\x1" "!", C_LNOT},
    { "\x1" "%", C_MOD},
    { "\x1" "&", C_AND},
    { "\x1" "(", C_LPAREN},
    { "\x1" ")", C_RPAREN},
    { "\x1" "*", C_STAR},
    { "\x1" "+", C_PLUS},
    { "\x1" ",", C_COMMA},
    { "\x1" "-", C_MINUS},
    { "\x1" ".", C_DOT},
    { "\x1" "/", C_DIV},
    { "\x1" ":", C_COLON},
    { "\x1" ";", C_SEMI},
    { "\x1" "<", C_LT},
    { "\x1" "=", C_EQUAL},
    { "\x1" ">", C_GT},
    { "\x1" "?", C_QMARK},
    { "\x1" "[", C_LBRACKET},
    { "\x1" "]", C_RBRACKET},
    { "\x1" "^", C_XOR},
    { "\x1" "{", C_LBRACE},
    { "\x1" "|", C_OR},
    { "\x1" "}", C_RBRACE},
    { "\x1" "~", C_NOT},
    { "\x2" "!=", C_NE},
    { "\x2" "&&", C_ANDAND},
    { "\x2" "(<", C_LBRACE},
    { "\x2" "(|", C_LBRACKET},
    { "\x2" "++", C_INC},
    { "\x2" "--", C_DEC},
    { "\x2" "->", C_ARROW},
    { "\x2" "<<", C_LSHIFT},
    { "\x2" "<=", C_LE},
    { "\x2" "=%", C_EQMOD},
    { "\x2" "=&", C_EQAND},
    { "\x2" "=*", C_EQMUL},
    { "\x2" "=+", C_EQPLUS},
    { "\x2" "=-", C_EQMINUS},
    { "\x2" "=/", C_EQDIV},
    { "\x2" "==", C_EQEQ},
    { "\x2" "=^", C_EQXOR},
    { "\x2" "=|", C_EQOR},
    { "\x2" ">)", C_RBRACE},
    { "\x2" ">=", C_GE},
    { "\x2" ">>", C_RSHIFT},
    { "\x2" "\\!", C_OR},
    { "\x2" "\\(", C_LBRACE},
    { "\x2" "\\)", C_RBRACE},
    { "\x2" "\\^", C_NOT},
    { "\x2" "|)", C_RBRACKET},
    { "\x2" "||", C_OROR},
    { "\x3" "=<<", C_EQLSHIFT},
    { "\x3" "=>>", C_EQRSHIFT},
    { "\x3" "\\!!", C_OROR}
};

int doesc(char *arg_2, char *r4, int arg_6) {
    uint8_t var_9;
    int var_8; 

    char *r2 = arg_2;
    int r3;

    arg_6 = max(2, arg_6);
    for (r3 = 0; r3 < arg_6; r3++) {
        if (*r4 != '\\')
            *r2++ = *r4++;
        else if (escChar[var_8 = scanstr(escChar,*++r4)]) {
            *r2++ = escMap[var_8];
            r4++;
            r3++;
        } else if (isdigit(*r4)) {
            var_9 = 0;
            for (var_8 = 0; var_8 < 3 && isdigit(*r4); var_8++, r4++) 
                var_9 = var_9 * 8 + *r4 - '0';
            *r2++ = var_9;
            r3 += var_8;
        } else {
            *r2++ = *r4++;
            r3++;
        }
    }
    return r2 - arg_2;
}


int dopunct(token_t **arg_2) {
    token_t *r2 = *arg_2;

    int var_B;
    char var_9[3];
    int r4;
    token_t *r3;

    var_9[0] = r2->tok[0];

    for (r4 = 1, r3 = r2->next; r4 < 3 && r3->type == PUNCT && r3->spclen == 0; r4++, r3 = r3->next)
        var_9[r4] = r3->tok[0];
    while (r4 > 0 && (var_B=scntab(punctTab,0x36,var_9,r4)) == 0)
        r4--;
    if (r4 <= 0) {
         wperror("illegal character: %c",r2->tok[0]);
         *arg_2 = r2->next;
         return 0xde;   // +
    }
    while (r4 > 0) {
        r2 = r2->next;
        r4--;
    }
    *arg_2 = r2;
    return var_B;
}

int eval(token_t *r4) {
    long var_A;
    if ((r4 = expr(r4,&var_A)) == 0)
        return 0;
    if (r4->type != NL) {
        wperror("illegal #if expression");
        return 0;
    }
    return var_A != 0;
}



int exop(token_t **r4, int arg_4) {
    token_t *var_8;
    int r2;

    var_8 = *r4;
    if (var_8->type == PUNCT) {//128C
        r2 = dopunct(&var_8);
        if (r2 & arg_4) {
            *r4 = var_8;
            return r2;
        }
    }
    return 0;
}


token_t *expr(token_t *arg_2, long *arg_4) {
    int var_8;
 if ((arg_2=exterm(arg_2,arg_4)) == 0)
     return 0;
 if ((var_8 = exop(&arg_2,0x80)) != 0)
     return extail(0,arg_4,&var_8,arg_2);
 return arg_2;
}


int expri(int r4) {
    int r2;
    if (r4 == 0 || opMap[(r2 = scanstr(opMap, r4))] == 0)
        return 0;
    else
        return opPrec[r2];
}


token_t *extail(int arg_2, long *arg_4, int *arg_6, token_t *arg_8) {

    int var_14;
    long var_12;
    long var_E;
    long var_A;

    int r2;
    int r4;
    var_14 = *arg_6;

    while (arg_2 < (r4 = expri(var_14))) {
        if (arg_8=exterm(arg_8,&var_12)) {
            if (r4 < expri(var_14 = exop(&arg_8,0x80)))
                 arg_8 = extail(r4,&var_12,&var_14,arg_8);
        }
        if (arg_8 == 0)
            return 0;

        r2 = *arg_6;
        var_A = *arg_4;
        var_E = var_A - var_12;
        switch(r2) {
        case C_PLUS:
            var_A+=var_12;
            break;
        case C_MINUS:
            var_A = var_E;
            break;
        case C_STAR:
            var_A *= var_12;
            break;
        case C_DIV: // if var_12 == 0 then mimic ws 8080 long behaviour i.e. 1 for -ve, else -1
            if (var_12)
                var_A /= var_12;
            else {
                var_A = var_A < 0 ? 1 : -1;
                wperror("divide by zero in #if");
            }
            break;
         case C_MOD: // if var_12 == 0 then mimic ws 8080 long behaviour i.e. var_A unchanged
             if (var_12)
                 var_A %= var_12;
             else
                 wperror("mod by zero in #if");
            break;
        case C_AND:
            var_A &= var_12;
            break;
        case C_OR:
            var_A |= var_12;
            break;
        case C_XOR:
            var_A ^= var_12;
            break;
        case C_LSHIFT:
            var_A <<= var_12;
            break;
        case C_RSHIFT:
            var_A >>= var_12;
            break;
        case C_LT:
            var_A = var_E < 0;
            break;
        case C_EQEQ:
            var_A = var_E == 0;
            break;
        case C_GT:
            var_A = var_E > 0;
            break;
        case C_LE:
            var_A = var_E <= 0;
            break;
        case C_NE:
            var_A = var_E != 0;
            break;
        case C_GE:
            var_A = var_E >= 0;
        case C_ANDAND:
            var_A = var_A && var_12;
            break;
        case C_OROR:
            var_A = var_A || var_12;
        case C_QMARK:
            if (!punct(arg_8,':') || (arg_8 = expr(arg_8->next, &var_E)) == 0) {
                wperror("illegal ? : in #if");
                return 0;
            }
            var_A = var_A ? var_12 : var_E;
            break;
        default:
            wperror("illegal operator in #if");
            return 0;
        }
        *arg_4 = var_A;
        *arg_6 = var_14;
    }
    return arg_8;
}


token_t *exterm(token_t *arg_2, long *r4) {
    int var_20A = 0;        // suppress false positive warning
    char var_208[512];
    int var_8;  // base
    char *r3;
    int r2;

    if (arg_2->type == NUMBER) {
        r3 = arg_2->tok;
        r2 = arg_2->toklen;
        if (*r3 != '0')
            var_8 = 10;
        else if (r2 > 1 && tolower(r3[1]) == 'x') {
            var_8 = 16;
            r3 += 2;
            r2 = 2;
        } else
            var_8 = 8;

        if (btol(r3, r2, r4, var_8) != r2) {
            wperror("illegal number in #if");
            *r4 = 0;
        }
        return arg_2->next;
    } else if (arg_2->type == SQSTRING) { 
        r3 = &var_208[1];
        r2 = doesc(var_208,arg_2->tok,arg_2->toklen) - 2;
        for (*r4=0L; r2; r3++, r2--)
            *r4 = (*r4 << 8) | (*r3 & 0xff);
    } else if (punct(arg_2, '(')) {
        if ((arg_2=expr(arg_2->next,r4)) == 0)
            return 0;
        if (!punct(arg_2,')')) {
            wperror("missing ) in #if");
            return 0;
        }
        return arg_2->next;
    } else if ((var_20A = exop(&arg_2,0x40)) == 0) {
        wperror("illegal #if syntax");
        return 0;
    } else if ((arg_2=exterm(arg_2,r4)) == 0)
        return 0;
    else {
        if (var_20A == 0xD8) // -
            *r4 = -*r4;
        else if (var_20A == 0x5A) // !
            *r4 = ! *r4;
        else if (var_20A == 0x43) // ~
            *r4 = ~*r4;
        else if (var_20A != 0xde) // +
            wperror("illegal unary op in #if");
        return arg_2;
    }

    return arg_2->next;
}
