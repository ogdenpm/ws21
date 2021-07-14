#include "cpp.h"
#include <stdarg.h>     // for putcode
#include <limits.h>     // for floating point

code_t cKeyword[] = {
    { "\x2" "do", C_DO},
    { "\x2" "if", C_IF},
    { "\x3" "for", C_FOR},
    { "\x3" "int", C_INT},
    { "\x4" "auto", C_AUTO},
    { "\x4" "case", C_CASE},
    { "\x4" "char", C_CHAR},
    { "\x4" "else", C_ELSE},
    { "\x4" "goto", C_GOTO},
    { "\x4" "long", C_LONG},
    { "\x5" "break", C_BREAK},
    { "\x5" "float", C_FLOAT},
    { "\x5" "short", C_SHORT},
    { "\x5" "union", C_UNION},
    { "\x5" "while", C_WHILE},
    { "\x6" "double", C_DOUBLE},
    { "\x6" "extern", C_EXTERN},
    { "\x6" "return", C_RETURN},
    { "\x6" "sizeof", C_SIZEOF},
    { "\x6" "static", C_STATIC},
    { "\x6" "struct", C_STRUCT},
    { "\x6" "switch", C_SWITCH},
    { "\x7" "default", C_DEFAULT},
    { "\x7" "typedef", C_TYPEDEF},
    { "\x8" "continue", C_CONTINUE},
    { "\x8" "register", C_REGISTER},
    { "\x8" "unsigned", C_UNSIGNED}
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
    char code;
    char str[512];
    long val;

    int r3 = doesc(str, r4->tok, r4->toklen) - 2;
    int r2 = r3 < 4 ? 1 : r3 - 3;
    for (val = 0L; r2 <= r3; r2++)
        val = (val << 8) | (str[r2] & 0xff);
    switch (r3) {
    case 0:
        code = C_INT8;                               // int8_t - no char
        break;
    case 1:
        code = str[1] >= 0 ? C_INT8 : C_INT16;      // checks sign of first char int8_t or int16_t
        break;
    case 2:
        code = str[1] >= 0 ? C_INT16 : C_UINT16;      // int16_t or uint16_t
        break;
    default:
        code = val < 0 ? C_UINT32 : C_INT32;            // uint32_t or int32_t
        break;
    }

    putcode("c4", code, &val);
    return r4->next;
}


/*
    The original Whitesmiths' code calculates the floating point constants as doubles.
    This code calculates the matissa as uint64_t, which has more precision than the 
    56 bits of the native code. It also skips adding digits when the matissa is greater
    than ULLONG_MAX / 10 + 9, as the final number is scaled back to 56 bit precision,
    the mkWsDouble routine in support.c, converts the matissa and a power of 10 exponent
    into a whitesmiths' double.
*/


token_t *lexfloa(token_t *p) {
    char *s;
    int r4;         // digits after .
    short exp;      // exponent
    int var_A;      // scaling
    int minus;      // sign
#ifdef NATIVE
    double var_14 = 0.0;
#else
    int skipped;
    uint64_t var_14 = 0;
#endif
    s = p->tok;

#ifdef NATIVE
    r2 += flaccum(r2, &var_14, arg_2->toklen);
#else
    s += flaccum(s, &var_14, p->toklen, &skipped);
#endif

    s = lexfnxt(&p, s);
    if (*s == '.') {
        p = p->next;
        s = p->tok;
#ifdef NATIVE
        r4 = flaccum(r2, &var_14, arg_2->toklen);
        var_A = -r4;
#else
        var_A = skipped;
        r4 = flaccum(s, &var_14, p->toklen, &skipped);
        var_A -= (r4 - skipped);
#endif
        s += r4;
    } else
#ifdef NATIVE
        var_A = 0;
#else
        var_A = skipped;
#endif

    s = lexfnxt(&p, s);
    if (tolower(*s) == 'e') { // 2174
        s++;
        s = lexfnxt(&p, s);
        minus = 0;
        if (*s == '+')
            s++;
        else if (*s == '-') {
            s++;
            minus = 1;
        }

        s = lexfnxt(&p, s);
        s += btos(s, p->toklen - (s - p->tok), &exp, 10);
        if (minus)
            var_A -= exp;
        else
            var_A += exp;
    }
#ifdef NATIVE
    var_14 = dtento(var_14, var_A);
    putcode("c8", 0x11, &var_14);
#else
    wsDouble wsDbl = mkWsDouble(var_14, var_A);
    putcode("c8", C_DBL, &wsDbl);
#endif
    if (p->tok != s) {
        if (p->tok + p->toklen == s)
            p = p->next;
        else {
            p = p->next;
            wperror("illegal float constant");
        }
    }
    return p;
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
    int code;
    int r2 = min(r4->toklen, 8);
    if (code = scntab(cKeyword, 0x1B, r4->tok, r2))
        putcode("c", code);
    else
        putcode("ccb", C_ID, r2, r4->tok, r2);
    return r4->next;
}

token_t *lexint(token_t *r4, int base, int skip) {
    uint16_t r3;
    int8_t code;
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
        code = r2 || lnum >= 0 ? C_INT32 : C_UINT32;
    else if ((r2 && loByte != loWord) || (!r2 && (loWord & 0xff00)))
        code = !r2 || loWord >= 0 ? C_INT16 : C_UINT16;
    else
        code = !r2 || loByte >= 0 ? C_INT8 : C_INT16;

    putcode("c4", code, &lnum);
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
    putcode("c2b", C_STRING, &var_8, var_208 + 1, var_8);
    return r4->next;
}


#if 1
void putcode(char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    for (; *fmt; ++fmt)
        if (*fmt == 'c')
            putc(va_arg(args, unsigned) & BYTMASK, ofd);
        else {
            TEXT *q = va_arg(args, TEXT *);
            COUNT i = (*fmt == 'b') ? va_arg(args, unsigned) : (*fmt == 'p') ? strlen(q) : *fmt - '0';
            while (0 <= --i)
                putc(*q++ & 0xff, ofd);
        }
    va_end(args);
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

void putcode(char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char *q;
    int c;
    char *s;

    for (; *fmt; fmt++) {
        switch (*fmt) {
        case 'c':
            putc(c = va_arg(args, int), ofd);
            break;
        case 'p':
            fprintf(ofd, "%s", s = va_arg(args, char *));
            break;
        case 'b':
            q = va_arg(args, char *);
            fwrite(q, 1, c = va_arg(args, int), ofd);
            break;
        case '2':
            putWInt(*va_arg(args, uint16_t *));
            break;
        case '4':
            putWLong(*(va_arg(args, uint32_t *)));
            break;
        case '8':
            fwrite(s = va_arg(args, uint8_t *), 1, 8 , ofd);        // write the double as an 8 byte sequences. wsDouble is in the righ format
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
                putcode("c2", C_LINENO, &pincl->lineno);
            if (pflag) {
                pflag = 0;
                if (pincl)
                    putcode("ccp", C_FILE, pincl->fname ? lenstr(pincl->fname) : 0, pincl->fname ? pincl->fname : "");
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

