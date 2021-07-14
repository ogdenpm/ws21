#include "cpp.h"
#include <stdarg.h>
sym_t *hashTable[128];



token_t *buytl(token_t *r4, token_t *r2) {

    token_t *r3 = alloc(sizeof(token_t), r2);
    r3->type = r4->type;
    r3->spc = r4->spc;
    r3->spclen = r4->spclen;
    r3->tok = r4->tok;
    r3->toklen = r4->toklen;
    return r3;
}


token_t *dodef(token_t *arg_2, token_t *arg_4, param_t *arg_6) {

    token_t *var_12;
    token_t *var_10;
    token_t *var_E;
    int var_C;
    int var_A;
    param_t *var_8;
    token_t *r4, *r2;
    token_t *r3;

    r4 = (token_t *)&var_12;
    var_C = 0;
    if (arg_2->spclen == 0 || !punct(arg_2, '('))
        r2 = arg_2;
    else {
        r2 = arg_2->next;
        if (r2->type == ID) {
            var_C = 1;
            r2 = r2->next;
        }
        while (r2 && punct(r2, ')') == 0) {
            if (punct(r2, ',') && r2->next->type == ID) {
                r2 = r2->next->next;
                var_C++;
            } else
                r2 = 0;
        }

        if (!r2) {
            wperror("bad #define arguments");
            r2 = arg_2;
        } else
            r2 = r2->next;
    }
    var_E = r2;

    while (r2->type != NL) { //3041
        if (r2->type == ID) { // 2Ea6
            r3 = arg_2->next;
            for (var_A = 0; var_A < var_C; r3 = r3->next->next, var_A++)
                if (r2->toklen == r3->toklen && cmpbuf(r2->tok, r3->tok, r3->toklen))
                    break;
        }
        if (r2->type == ID && var_A < var_C) { // 2f36
            for (var_8 = arg_6; var_A > 0 && var_8; var_8 = var_8->next, var_A--)
                ;
            if (var_8 != 0) { //3008
                for (var_10 = var_8->first; var_8->last != var_10; r4->next, var_10 = var_10->next) {
                    r4 = buytl(var_10, r2->next);
                    if (var_8->first == var_10) {
                        r4->next->spc = r2->spc;
                        r4->next->spclen = r2->next->spclen;
                    }
                }
            }
            r2 = wsfree(r2, r2->next);
        } else {
            r4 = r4->next = r2;
            r2 = r2->next;
        }
    }
    while (arg_2 != var_E)
        arg_2 = wsfree(arg_2, arg_2->next);

    r4->next = arg_4;
    return var_12;
}




token_t *doexp(token_t *arg_2) {
    token_t *var_C;
    char *var_A;
    param_t *var_8;
    token_t *r2;
    token_t *r4;
    token_t **r3;

    r3 = &arg_2;
    while (r2 = *r3) {
        if (r2->type == ID && (var_A = lookup(r2->tok, r2->toklen))) {
            r4 = stotl(var_A);
            var_8 = NULL;
            if (r4->spclen == 0 && punct(r4, '('))
                var_C = getargs(r2->next, &var_8);
            else
                var_C = r2->next;

            if ((*r3 = dodef(r4, var_C, var_8)) != var_C) {
                (*r3)->spc = r2->spc;
                (*r3)->spclen = r2->spclen;
            }
            frelst(var_8, 0);
            frelst(r2, var_C);
        } else
            r3 = (token_t **)r2;
    }
    return arg_2;
}

#if 0
void errfmt(char *r2, char *arg_4) {
    char var_10[10];
    char **r4 = &arg_4;
    char *r3;

    for (; *r2; r2++) {
        if (*r2 != '%')
            _write(errfd, r2, 1);
        else if (*++r2 == 'c')
            _write(errfd, *r4++, 1);
        else if (*r2 == 's')
            _write(errfd, &var_10, itob(&var_10, *r4++, 10));
        else {
            r3 = *r4++;
            _write(errfd, r3, *r2 == 'b' ? *r4++ : lenstr(r3));
        }
    }
}
#else
void errfmt_va(char *msg, va_list args) {

    for (; *msg; msg++) {
        if (*msg != '%')
            putc(*msg, stderr);
        else if (*++msg == 'c')
            putc(va_arg(args, int), stderr);
        else if (*msg == 's')
            fprintf(stderr, "%u", va_arg(args, int));
        else {
            char *r3 = va_arg(args, char *);
            fprintf(stderr, "%.*s", *msg == 'b' ? va_arg(args, int) : (int)strlen(r3), r3);
        }
    }
}

void errfmt(char *msg, ...) {
    va_list args;
    va_start(args, msg);
    errfmt_va(msg, args);
    va_end(args);
}

#endif

token_t *getargs(token_t *arg_2, param_t **arg_4) {
    int var_8;
    token_t *r3 = arg_2;
    param_t *r2;

    if (!punct(r3, '('))
        return r3;

    param_t **r4 = arg_4;
    r3 = r3->next;

    while (r3 && !punct(r3, ')')) { // 33EB
        *r4 = r2 = alloc(sizeof(param_t), 0);
        r2->first = r3;
        var_8 = 0;
        while (r3 && (var_8 > 0 || (!punct(r3, ',') && !punct(r3, ')')))) {
            if (punct(r3, '('))
                var_8++;
            else if (punct(r3, ')'))
                var_8--;
            r3 = r3->next;
        }
        if (r3) {
            r2->last = r3;
            if (punct(r3, ','))
                r3 = r3->next;
        }
        r4 = (param_t **)r2;
    }
    if (r3)
        return r3->next;
    wperror("bad macro arguments");
    *arg_4 = 0;
    return arg_2;
}


char *getfnam(token_t *r4) {
    int var_8;
    int var_A;
    char *r2;
    char *r3;

    if (r4->type == ID) {
        r2 = buybuf(r4->tok, r4->toklen + 1);
        r2[r4->toklen] = 0;     // convert to c string
    } else if (r4->type == DQSTRING) { // 3564
        r2 = buybuf(r4->tok + 1, r4->toklen - 1);
        r2[r4->toklen - 2] = 0;
    } else if (punct(r4, '<')) {
        r3 = r4->next->tok;
        while (*r3 != '>' && !iswhite(*r3))
            r3++;
        var_8 = r3 - r4->next->tok;
        r2 = alloc(lenstr(iprefix) + var_8, 0);
        r3 = iprefix;
        while (*r3) {
            cpybuf(r2, r3, var_A = scanstr(r3, '|'));
            cpybuf(r2 + var_A, r4->next->tok, var_8);
            r2[var_A + var_8] = 0;
            if (access(r2, 4) == 0)     // check can read
                break;
            r3 += r3[var_A] ? var_A + 1 : var_A;
        }
    } else
        r2 = 0;
    return r2;
}

sym_t **hash(char *r4, int r2) {
    int r3;
    for (r3 = 0; r2 != 0; r2--)
        r3 += *r4++;
    return &hashTable[r3 % 128];
}



void install(char *arg_2, int r4, char *arg_6) {
    r4 = r4 < 8 ? r4 : 8;
    sym_t **r2 = hash(arg_2, r4);

    sym_t *r3 = alloc(sizeof(sym_t), *r2);
    *r2 = r3;
    cpybuf(r3->name, arg_2, r4);
    r3->len = r4;
    r3->val = arg_6;
}


char *lookup(char *r4, int r2) {  // tok, toklen
    r2 = r2 < 8 ? r2 : 8;
    sym_t *r3;
    for (r3 = *hash(r4, r2); r3; r3 = r3->next) {
        if (r3->len == r2 && cmpbuf(r4, r3->name, r2))
            return r3->val;
    }
    return 0;
}

pincl_t *nxtfile() {
    pincl_t *r2;
    FILE *r4;

    while ((r4 = getfiles(&argc, &argv, stdin, stderr)) == stderr)
        wperror("can't open %p", argv[-1]);

    if (r4 == NULL)
        return 0;

    r2 = alloc(sizeof(pincl_t), 0);
    if (r4 == stdin)
        r2->fname = 0;
    else
        r2->fname = buybuf(argv[-1], lenstr(argv[-1]) + 1);
    pflag = 1;
    r2->lineno = 0;
//    finit(&r2->fio, r4, 0);
    r2->fio._nleft = 0;
    r2->fio.fp = r4;
    return r2;
}


void pargs(char *r4, int r2) {   // r2 is length
    fio_t *r3 = &pincl->fio;

    while (r2 > 0 && r3->_nleft < 511) {
        if ((r3->_buf[r3->_nleft] = *r4++) == '\n')
            pincl->lineno--;
        r2--;
        r3->_nleft++;
    }
    if (r2 > 0 && r3->_nleft < 512) {
        r3->_buf[r3->_nleft++] = '\n';
        wperror("too many -d arguments");
    }
    r3->_pnext = r3->_buf;      // point to start of buffer
}

#if 0
void ws_perror(char *arg_2, char *arg_4, char *arg_6) {
    if (!pincl)
        errfmt("EOF: ");
    else if (pincl->fname == 0)
        errfmt("%s: ", pincl->lineno);
    else
        errfmt("%p %s: ", pincl->fname, pincl->lineno);        // w2  -> char *, w4  -> short
    errfmt(arg_2, arg_4, arg_6);
    errfmt("\n");
    nerrors++;
}
#else
void wperror(char *msg, ...) {
    if (!pincl)
        errfmt("EOF: ");
    else if (pincl->fname == 0)
        errfmt("%s: ", pincl->lineno);
    else
        errfmt("%p %s: ", pincl->fname, pincl->lineno);        // w2  -> char *, w4  -> short
    va_list args;
    va_start(args, msg);
    errfmt_va(msg, args);
    va_end(args);
    errfmt("\n");
    nerrors++;
}
#endif

void predef(list_t *r4) {
    int r2;
    char *r3;
    char var_9 = pchar;
    int var_8;

    for (r2 = 0; r2 < 10 - r4->ntop; r2++) {
        r3 = r4->val[9 - r2];
        pargs(&var_9, 1);
        pargs("define ", 7);
        var_8 = scanstr(r3, '=');
        pargs(r3, var_8);
        pargs(" ", 1);
        if (r3[var_8] == 0)
            pargs("1", 1);
        else {
            r3 += var_8 + 1;
            pargs(r3, (int)lenstr(r3));
        }
        pargs("\n", 1);
    }
}



int punct(token_t *r4, int arg_4) {
    return r4->toklen == 1 && r4->tok[0] == arg_4;  // r4->w8 (uint8_t *)
}

// use simple binary search to check for match
int scntab(code_t *group, unsigned grplen, const char *token, unsigned toklen) {
    int cmp;
    unsigned low = 0;
    unsigned mid;;
    char *r3;
    unsigned r4;
    const char *r2;
    while (low < grplen) {
        mid = (grplen + low) >> 1;
        r3 = group[mid].str;
        if ((cmp = *r3++ - toklen) == 0)
#pragma warning(suppress : 6011)
            for (r4 = 0, r2 = token; r4 < toklen && (cmp = *r3++ - *r2++) == 0; r4++)
                ;
        if (cmp < 0)
            low = mid + 1;
        else if (cmp == 0)
            return group[mid].code;
        else
            grplen = mid;
    }
    return 0;
}



token_t *stotl(char *r4) {
    token_t *r2;
    token_t *head;
    token_t **r3 = &head;

    do {
        r2 = alloc(sizeof(token_t), 0);
        r2->spc = r4;

        while (*r4 <= ' ' && *r4 <= 0x7f && *r4 != '\n')
            r4++;

        r2->spclen = r4 - r2->spc;
        r2->tok = r4;
        if (*r4 == '\n') {
            r4++;
            r2->type = NL;
        } else if (isalpha(*r4)  || *r4 == '_') {
            while (isalpha(*r4) || *r4 == '_' || isdigit(*r4))
                r4++;
            r2->type = ID;
        } else if (isdigit(*r4)) {
            while (isalpha(*r4) || isdigit(*r4))
                r4++;
            r2->type = NUMBER;
        } else if (*r4 == '"' || *r4 == '\'') {
            while (*++r4 != *r2->tok && *r4 != '\n') {
                if (*r4 == '\\' && r4[1] != '\n')
                    r4++;
                if (*r4 == '\\')
                    break;
            }
            if (*r4 == *r2->tok)
                r4++;
            else
                wperror("unbalanced %s", *r2->tok);
            r2->type = *r2->tok == '"' ? DQSTRING : SQSTRING;
            if (r4 - r2->tok >= 512) {
                wperror("string too long");
                r2->tok = r4 - 512;
            }
        } else {
            r4++;
            r2->type = PUNCT;
        }
        r2->toklen = r4 - r2->tok;
        *r3 = r2;
        r3 = (token_t **)r2;
    } while (r2->type != NL);
    return head;
}

void undef(char *r4, int arg4) {
    arg4 = arg4 < 8 ? arg4 : 8;
    sym_t *r2 = (sym_t *)hash(r4, arg4);
    sym_t *r3;

    while (r3 = r2->next) {
        if (r3->len == arg4 && cmpbuf(r4, r3->name, arg4))
            break;
        r2 = r3;
    }
    if (r3) {
        wsfree(r3->val, 0);
        r2->next = wsfree(r3, r3->next);
    }
}

