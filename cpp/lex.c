#include "cpp.h"
#include <stdarg.h>     // for putcode
#include <limits.h>     // for floating point

code_t cKeyword[] = {
    { "\x2" "do", 0x27},
    { "\x2" "if", 0x2E},
    { "\x3" "for", 0x2C},
    { "\x3" "int", 0x2F},
    { "\x4" "auto", 0x21},
    { "\x4" "case", 0x23},
    { "\x4" "char", 0x24},
    { "\x4" "else", 0x29},
    { "\x4" "goto", 0x2D},
    { "\x4" "long", 0x30},
    { "\x5" "break", 0x22},
    { "\x5" "float", 0x2B},
    { "\x5" "short", 0x33},
    { "\x5" "union", 0x38},
    { "\x5" "while", 0x3A},
    { "\x6" "double", 0x28},
    { "\x6" "extern", 0x2A},
    { "\x6" "return", 0x32},
    { "\x6" "sizeof", 0x61},
    { "\x6" "static", 0x34},
    { "\x6" "struct", 0x35},
    { "\x6" "switch", 0x36},
    { "\x7" "default", 0x26},
    { "\x7" "typedef", 0x37},
    { "\x8" "continue", 0x25},
    { "\x8" "register", 0x31},
    { "\x8" "unsigned", 0x39}
};

int word_89F9;


int firnon(char *r4, int arg_4, int arg_6) { // str, len, base
    int r3;
    int r2;
    for (r3 = 0; r3 < arg_4; r3++, r4++) {
        if (isdigit(*r4))
            r2 = *r4 - '0';
        else if (isalpha(*r4))
            r2 = tolower(*r4) - 'a' + 10;
        else
            break;
        if (r2 >= arg_6)
            break;
    }
    return r3;
}

#if NATIVE
int flaccum(char *r4, double *arg_4, int r2) {
    int r3;
    for (r3 = 0; r3 < r2 && isdigit(*r4); r3++, r4++)
        *arg_4 = *arg_4 * 10.0 + (*r4 - '0');
    return r3;
}
#else
int flaccum(char *r4, uint64_t *arg_4, int r2, int *skipped) {
    int r3;
    *skipped = 0;
    for (r3 = 0; r3 < r2 && isdigit(*r4); r3++, r4++)
        if (*arg_4 < (ULLONG_MAX / 10 - 9))
            *arg_4 = *arg_4 * 10 + (*r4 - '0');
        else
            ++*skipped;
    return r3;
}
#endif

token_t *lexchar(token_t *r4) {
    char var_20B;
    char var_20A[512];
    long var_A;

    int r3 = doesc(var_20A, r4->tok, r4->toklen) - 2;
    int r2 = r3 < 4 ? 1 : r3 - 3;
    for (var_A = 0L; r2 <= r3; r2++)
        var_A = (var_A << 8) | (var_20A[r2] & 0xff);
    switch (r3) {
    case 0:
        var_20B = 0x18;                             // uint8_t - no char
        break;
    case 1:
        var_20B = var_20A[1] >= 0 ? 0x18 : 0x16;      // checks sign of first char uint8_t or uint16_t
        break;
    case 2:
        var_20B = var_20A[1] >= 0 ? 0x16 : 0x1A;      // uint16_t or int16_t
        break;
    default:
        var_20B = var_A < 0 ? 0x19 : 0x15;            // int32_t or uint32_t
        break;
    }

    putcode("c4", var_20B, &var_A);
    return r4->next;
}


/*
    The original Whitesmiths' code calculates the floating point constants as doubles.
    This code calculates the matissa as uint64_t, which has more precision than the 
    56 bits of the native code. It also skips adding digits when the matissa is greater
    than ULLONG_MAX / 10 + 9, as the final number is scaled back to 56 bit precision,
    the mkWsDouble routine in support.c, converts the matissa and a power of 10 exponent
    into a whitesmiths' double, with the exception that pair wise bytes are not swapped.
    The putcode function does the actual swapping when the value is emitted.
*/


token_t *lexfloa(token_t *arg_2) {
    char *r2;
    int r4;         // digits after .
    short var_C;      // exponent
    int var_A;      // scaling
    int var_8;      // sign
#ifdef NATIVE
    double var_14 = 0.0;
#else
    int skipped;
    uint64_t var_14 = 0;
#endif
    r2 = arg_2->tok;

#ifdef NATIVE
    r2 += flaccum(r2, &var_14, arg_2->toklen);
#else
    r2 += flaccum(r2, &var_14, arg_2->toklen, &skipped);
#endif

    r2 = lexfnxt(&arg_2, r2);
    if (*r2 == '.') {
        arg_2 = arg_2->next;
        r2 = arg_2->tok;
#ifdef NATIVE
        r4 = flaccum(r2, &var_14, arg_2->toklen);
        var_A = -r4;
#else
        var_A = skipped;
        r4 = flaccum(r2, &var_14, arg_2->toklen, &skipped);
        var_A -= (r4 - skipped);
#endif
        r2 += r4;
    } else
#ifdef NATIVE
        var_A = 0;
#else
        var_A = skipped;
#endif

    r2 = lexfnxt(&arg_2, r2);
    if (tolower(*r2) == 'e') { // 2174
        r2++;
        r2 = lexfnxt(&arg_2, r2);
        var_8 = 0;
        if (*r2 == '+')
            r2++;
        else if (*r2 == '-') {
            r2++;
            var_8 = 1;
        }

        r2 = lexfnxt(&arg_2, r2);
        r2 += btos(r2, arg_2->toklen - (r2 - arg_2->tok), &var_C, 10);
        if (var_8)
            var_A -= var_C;
        else
            var_A += var_C;
    }
#ifdef NATIVE
    var_14 = dtento(var_14, var_A);
    putcode("c8", 0x11, &var_14);
#else
    wsDouble wsDbl = mkWsDouble(var_14, var_A);
    putcode("c8", 0x11, &wsDbl);
#endif
    if (arg_2->tok != r2) {
        if (arg_2->tok + arg_2->toklen == r2)
            arg_2 = arg_2->next;
        else {
            arg_2 = arg_2->next;
            wperror("illegal float constant");
        }
    }
    return arg_2;
}


char *lexfnxt(token_t **arg_2, char *arg_4) {
    token_t *r4 = *arg_2;
    if (arg_4 < r4->tok + r4->toklen)
        return arg_4;
    r4 = r4->next;
    *arg_2 = r4;
    return r4->tok;
}

token_t *lexiden(token_t *r4) {
    int r3;
    int r2 = min(r4->toklen, 8);
    if (r3 = scntab(cKeyword, 0x1B, r4->tok, r2))
        putcode("c", r3);
    else
        putcode("ccb", 0x12, r2, r4->tok, r2);
    return r4->next;
}

token_t *lexint(token_t *r4, int base, int skip) {
    uint16_t r3;
    int8_t var_10;
    int8_t loByte;
    long lnum;
    int16_t hiWord;
    int16_t loWord;

    int r2 = base == 10;
    r3 = r4->toklen - skip;
    if (btol(r4->tok + skip, r3, &lnum, base) < r3)
        wperror("illegal constant %b", r4->tok, r4->toklen);

    hiWord = ((uint32_t)lnum) >> 16;
    loWord = (int16_t)lnum;
    loByte = (int8_t)loWord;
    if (tolower(r4->tok[r4->toklen - 1]) == 'l' || (r2 && loWord != lnum) || (!r2 && hiWord != 0))
        var_10 = r2 || lnum >= 0 ? 0x15 : 0x19;
    else if ((r2 && loByte != loWord) || (!r2 && (loWord & 0xff00)))
        var_10 = !r2 || loWord >= 0 ? 0x16 : 0x1A;
    else
        var_10 = !r2 || loByte >= 0 ? 0x18 : 0x16;

    putcode("c4", var_10, &lnum);
    return r4->next;
}


token_t *lexnum(token_t *r4) {
    int r2 = firnon(r4->tok, r4->toklen, 10);
    if (punct(r4, '.') || (r2 < r4->toklen && tolower(r4->tok[r2]) == 'e') || punct(r4->next, '.'))
        return lexfloa(r4);
    if (r4->tok[0] != '0')
        return lexint(r4, 10, 0);
    if (r4->toklen > 1 && tolower(r4->tok[1]) == 'x')
        return lexint(r4, 16, 2);
    return lexint(r4, 8, 1);
}

token_t *lexpunc(token_t *arg_2) {
    putcode("c", dopunct(&arg_2));
    return arg_2;
}


token_t *lexstri(token_t *r4) {
    char var_208[512];
    uint16_t var_8;

    var_8 = doesc(var_208, r4->tok, r4->toklen) - 2;
    putcode("c2b", 0x17, &var_8, var_208 + 1, var_8);
    return r4->next;
}


#if 0
void putcode(char *r2, char *arg_4 /*...*/) {        // to fix variable args
    int var_8;
    char **r4;
    char *r3;
    for (r4 = &arg_4; *r2; r2++) {
        if (*r2 == 'c')
            putch((int)*r4++ & 0xff);
        else {
            r3 = *r4++;
            var_8 = *r2 == 'b' ? *r4++ : *r2 == 'p' ? lenstr(r3) : *r2 - '0';
            while (--var_8 >= 0)
                putch(*r3++ & 0xff);
            r2++;
        }
    }
}
#else
void putWInt(uint16_t val) {        // little edian order 
    putc(val, ofd);
    putc(val / 256, ofd);
}

void putWLong(uint32_t val) {       // pdp 11 order, high word, low word both in little edian order
    putWInt(val / 65536);
    putWInt(val);
}

void putWDouble(wsDouble val) {     // swapped pairs of bytes
    putWInt((uint16_t)(val / 0x1000000000000));
    putWInt((uint16_t)(val / 0x100000000));
    putWInt((uint16_t)(val / 0x10000));
    putWInt((uint16_t)val);
}

void putcode(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *r3;

    for (; *fmt; fmt++) {
        switch (*fmt) {
        case 'c':
            putc(va_arg(args, int), ofd);
            break;
        case 'p':
            fprintf(ofd, "%s", va_arg(args, char *));
            break;
        case 'b':
            r3 = va_arg(args, char *);
            fprintf(ofd, "%.*s", va_arg(args, int), r3);
            break;
        case '2':
            putWInt(*va_arg(args, uint16_t *));
            break;
        case '4':
            putWLong(*(va_arg(args, uint32_t *)));
            break;
        case '8':
            putWDouble(*va_arg(args, wsDouble *));
            break;
        default:
            fprintf(stderr, "invalid putcode %c type\n", *fmt);
            break;
        }
    }
    va_end(args);
}
#endif


void putls(token_t *r4) {
    token_t *r2;
    if (xflag) {
        if (r4->type != NL) {
            if (pincl)
                putcode("c2", 0x14, &pincl->lineno);
            if (pflag) {
                pflag = 0;
                if (pincl)
                    putcode("ccp", 0x13, pincl->fname ? lenstr(pincl->fname) : 0, pincl->fname ? pincl->fname : "");
            }
        }
        while (r4->type != NL)
            if (r4->type == ID)
                r4 = lexiden(r4);
            else if (r4->type == DQSTRING)
                r4 = lexstri(r4);
            else if (r4->type == SQSTRING)
                r4 = lexchar(r4);
            else if (r4->type == NUMBER || (punct(r4, '.') && r4->next->type == NUMBER))
                r4 = lexnum(r4);
            else
                r4 = lexpunc(r4);

    } else {
        if (v6flag) {
            if (pincl->fname)
                putc('\x1', ofd);
            else
                while (++word_89F9 < pincl->lineno) // 2A98
                    putc('\n', ofd);
        }
        r2 = r4; // 2a5e
        while (1) {
            fwrite(r2->spc, 1, r2->spclen, ofd);
            if (!r2->next)
                break;
            fwrite(r2->tok, 1, r2->toklen, ofd);
            r2 = r2->next;
        }
        putc('\n', ofd);
    }
}

