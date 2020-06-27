#include "cpp.h"

char escChar[] = "btnvfrBTNVFR(!)^";
char escMap[] = {'\b','\t','\n','\v','\f','\r','\b','\t','\n','\v','r', '\r', '{', '|', '}', '~'};
uint8_t byte_888E[] = {0xE2, 0x85, 0x99, 0xDE, 0xD8, 0x97, 0xA0, 0x95, 0x96, 0x8F, 0x90, 0x94, 0x9B,0xC1,0xA3, 0x9C, 0x82, 0x9D, 0x9F,   0};
uint8_t byte_88A2[] = {0x0E, 0x0E, 0x0E, 0x0D, 0x0D, 0x0C, 0x0C, 0x0B, 0x0B, 0x0B, 0x0B, 0x0A, 0x0A,   9,   8,    7,    6,    5,    4};

code_t punctTab[] = {
    { "\x1" "!", 0x5A},
    { "\x1" "%", 0x99},
    { "\x1" "&", 0xC1},
    { "\x1" "(", 0x6},
    { "\x1" ")", 0xA},
    { "\x1" "*", 0xE2},
    { "\x1" "+", 0xDE},
    { "\x1" ",", 0x2},
    { "\x1" "-", 0xD8},
    { "\x1" ".", 0x3},
    { "\x1" "/", 0x85},
    { "\x1" ":", 0x1},
    { "\x1" ";", 0x0B},
    { "\x1" "<", 0x95},
    { "\x1" "=", 0x88},
    { "\x1" ">", 0x8F},
    { "\x1" "?", 0x9F},
    { "\x1" "[", 0x4},
    { "\x1" "]", 0x8},
    { "\x1" "^", 0xA3},
    { "\x1" "{", 0x5},
    { "\x1" "|", 0x9C},
    { "\x1" "}", 0x9},
    { "\x1" "~", 0x43},
    { "\x2" "!=", 0x9B},
    { "\x2" "&&", 0x82},
    { "\x2" "(<", 0x5},
    { "\x2" "(|", 0x4},
    { "\x2" "++", 0x53},
    { "\x2" "--", 0x44},
    { "\x2" "->", 0x7},
    { "\x2" "<<", 0x97},
    { "\x2" "<=", 0x96},
    { "\x2" "=%", 0x8C},
    { "\x2" "=&", 0x86},
    { "\x2" "=*", 0x91},
    { "\x2" "=+", 0x8E},
    { "\x2" "=-", 0x8B},
    { "\x2" "=/", 0x87},
    { "\x2" "==", 0x94},
    { "\x2" "=^", 0x92},
    { "\x2" "=|", 0x8D},
    { "\x2" ">)", 0x9},
    { "\x2" ">=", 0x90},
    { "\x2" ">>", 0xA0},
    { "\x2" "\\!", 0x9C},
    { "\x2" "\\(", 0x5},
    { "\x2" "\\)", 0x9},
    { "\x2" "\\^", 0x43},
    { "\x2" "|)", 0x8},
    { "\x2" "||", 0x9D},
    { "\x3" "=<<", 0x89},
    { "\x3" "=>>", 0x8A},
    { "\x3" "\\!!", 0x9D}
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
    if (r4 == 0 || byte_888E[(r2 = scanstr(byte_888E, r4))] == 0)
        return 0;
    else
        return byte_88A2[r2];
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
        case 0xde: // +
            var_A+=var_12;
            break;
        case 0xd8:  // -
            var_A = var_E;
            break;
        case 0xe2:  // *
            var_A *= var_12;
            break;
        case 0x85: // /     -- if var_12 == 0 then mimic ws 8080 long behaviour i.e. 1 for -ve, else -1
            if (var_12)
                var_A /= var_12;
            else {
                var_A = var_A < 0 ? 1 : -1;
                wperror("divide by zero in #if");
            }
            break;
         case 0x99: // %    -- if var_12 == 0 then mimic ws 8080 long behaviour i.e. var_A unchanged
             if (var_12)
                 var_A %= var_12;
             else
                 wperror("mod by zero in #if");
            break;
        case 0xc1:  // &
            var_A &= var_12;
            break;
        case 0x9c:  // |
            var_A |= var_12;
            break;
        case 0xa3:  // ^
            var_A ^= var_12;
            break;
        case 0x97: // <<
            var_A <<= var_12;
            break;
        case 0xa0:  // >>
            var_A >>= var_12;
            break;
        case 0x95:  // <
            var_A = var_E < 0;
            break;
        case 0x94:  // ==
            var_A = var_E == 0;
            break;
        case 0x8f:  // >
            var_A = var_E > 0;
            break;
        case 0x96:  // <=
            var_A = var_E <= 0;
            break;
        case 0x9b:  // !=
            var_A = var_E != 0;
            break;
        case 0x90:  // >=
            var_A = var_E >= 0;
        case 0x82:  // &&
            var_A = var_A && var_12;
            break;
        case 0x9d:  // ||
            var_A = var_A || var_12;
        case 0x9f:  // ?
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

        if (btol(r3,r2,r4,var_8) != r2) {
            wperror("illegal number in #if");
            *r4 = 0;
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
    } else {
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
