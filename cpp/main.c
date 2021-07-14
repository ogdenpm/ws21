#include "cpp.h"
#include <fcntl.h>

int cFlag;
list_t dList = { 10 };
char *oFile;
int xflag;
int v6flag;
int pchar = '#';
int schar = '@';
char *iprefix = "|";
char *_pname = "pp";
char **argv;
int argc;
FILE *ofd;              // added
pincl_t *pincl;

int nerrors;
int pflag;
code_t cppKeyword[] = {
    {"\x2" "IF", P_IF},
    {"\x2" "if", P_IF},
    {"\x4" "ELSE", P_ELSE},
    {"\x4" "LINE", P_LINE},
    {"\x4" "else", P_ELSE},
    {"\x4" "line", P_LINE},
    {"\x5" "ENDIF", P_ENDIF},
    {"\x5" "IFDEF", P_IFDEF},
    {"\x5" "UNDEF", P_UNDEF},
    {"\x5" "endif", P_ENDIF},
    {"\x5" "ifdef", P_IFDEF},
    {"\x5" "undef", P_UNDEF},
    {"\x6" "DEFINE", P_DEFINE},
    {"\x6" "IFNDEF", P_IFNDEF},
    {"\x6" "define", P_DEFINE},
    {"\x6" "ifndef", P_IFNDEF},
    {"\x7" "INCLUDE", P_INCLUDE},
    {"\x7" "include", P_INCLUDE}
};
uint8_t lineBuf[512];

token_t *getex() {
    int r2;
    token_t *r4 = getin();

    if (r4) { // 212
        if (punct(r4, pchar) || punct(r4, schar)) {
            if (r2 = scntab(cppKeyword, 18, r4->next->tok, r4->next->toklen)) {
                r4 = wsfree(r4, r4->next);
                r4->type = r2;
            } else
                r4->type = P_HASH;
        }
        switch (r4->type) {
        case P_DEFINE: case P_UNDEF: case P_INCLUDE: case P_HASH:
            break;
        case P_IFNDEF: case P_IFDEF:
            if (r4->next->type != ID)
                wperror("bad #%b", r4->tok, r4->toklen);
            else if (lookup(r4->next->tok, r4->next->toklen) == 0)
                r4->next = frelst(r4->next, 0);
            break;
        default:
            r4 = doexp(r4);
            break;
        }
    }
    return r4;
}


token_t *getin() {
    char *r4;
    while (pincl || (pincl = nxtfile())) {
        if (r4 = getln(pincl))
            return stotl(r4);
        fclose(pincl->fio.fp);
        wsfree(pincl->fname, 0);
        pincl = wsfree(pincl, pincl->next);
        pflag = 1;
    }
    return 0;
}


char *getln(pincl_t *arg_2) {
    char var_B;
    int var_A;
    int var_8;
    uint8_t *r3;
    int r4;
    fio_t *r2;

    var_B = 0;
    r3 = lineBuf;
    r2 = &arg_2->fio;
    r4 = getl(r2, lineBuf, 512);
    while (r4 > 0) {
        if (cFlag == 0 && *r3 == '\\' && r4 > 1 && r3[1] == '\n') {
            arg_2->lineno++;
            r4 = getl(r2, r3, 512 - (r3 - lineBuf));
        } else if (cFlag == 0 && *r3 == '\\') {
            r3 += 2;
            r4 -= 2;
        } else if (cFlag == 0 && var_B == 0 && *r3 == '/' && r4 > 1 && r3[1] == '*') { // 558
            for (var_8 = 2; var_8 < r4 - 1; var_8++)
                if (r3[var_8] == '*' && r3[var_8 + 1] == '/')
                    break;

            if (var_8 < r4 - 1) { // 625
                *r3++ = ' ';
                r4 -= (var_8 + 2);
                for (var_A = 0; var_A < r4; var_A++)
                    r3[var_A] = r3[var_A + var_8 + 1];      // remove the comment
            } else if ((r4 = getl(r2, r3 + 2, 512 - (r3 + 2 - lineBuf))) <= 0) {
                wperror("missing */");
                break;
            } else {
                arg_2->lineno++;
                r4 += 2;
            }
        } else if (*r3 == '\n') {
            arg_2->lineno++;
            return lineBuf;
        } else {
            if (cFlag == 0 && (*r3 == '"' || *r3 == '\'')) {
                if (var_B == *r3)
                    var_B = 0;
                else
                    var_B = *r3;        // bug, unescaped ' or " inside string will cause problems
            }
            r3++;
            r4--;
        }

    }
    if (lineBuf == r3)
        return 0;
    wperror("truncated line");
    lineBuf[511] = '\n';
    return lineBuf;
}



int main(int _argc, char **_argv) {
    token_t *r4;
    
    argv = _argv;
    argc = _argc;


    getflags(&argc, &argv, "c,d*>i*,o*,p?,s?,x,6:F <files>", &cFlag, &dList, &iprefix, &oFile, &pchar, &schar, &xflag, &v6flag);
    if (pincl = nxtfile())
        predef(&dList);
    if (oFile) {
        if ((ofd = fopen(oFile, xflag != 0 ? "wb" : "wt")) == NULL)
            wperror("bad output file");
    } else {
        ofd = stdout;
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
    }

    while (r4 = putgr(getex(), 0)) {
        wperror("misplaced #%b", r4->tok, r4->toklen);
        frelst(r4, 0);
    }
    if (ofd != stdout)
        fclose(ofd);
 //   wsfclose(&wsstdout);
    return nerrors == 0;
}


token_t *putgr(token_t *r4, int r2) {
    int r3;

    while (r4 && r4->type != P_ELSE && r4->type != P_ENDIF) {
        if (r4->type != P_IF && r4->type != P_IFDEF && r4->type != P_IFNDEF) {
            if (r2)
                frelst(r4, 0);
            else
                putns(r4);                
            r4 = getex();
        } else {
            if (r4->type == P_IF)
                r3 = r2 ? 0 : eval(r4->next);
            else if (r4->type == P_IFDEF)
                r3 = r4->next != 0;
            else
                r3 = r4->next == 0;

            frelst(r4, 0);
            if ((r4 = putgr(getex(), r2 || !r3)) && r4->type == P_ELSE) {
                frelst(r4, 0);
                r4 = putgr(getex(), r2 || r3);
            }

            if (r4 && r4->type == P_ENDIF) {
                frelst(r4, 0);
                r4 = getex();
            } else
                wperror("missing #endif");
        }
    }
    return r4;
}


void putns(token_t *r4) {
    short var_A;
    FILE *fp;
    token_t *r3;
    char *r2;

    switch (r4->type) {
    case P_DEFINE:
        if (r4->next->type != ID)
            wperror("bad #define");
        else {
            for (r3 = r4->next; r3->next; r3 = r3->next)
                ;
            install(r4->next->tok, r4->next->toklen,
                buybuf(r4->next->next->spc, &r3->tok[r3->toklen] - r4->next->next->spc));
        }
        break;
    case P_UNDEF:
        if (r4->next->type != ID)
            wperror("bad #undef");
        else
            undef(r4->next->tok, r4->next->toklen);
        break;
    case P_INCLUDE:      // #include
        if ((r2 = getfnam(r4->next)) == 0)
            wperror("bad #include");
        else if ((fp = fopen(r2, "rt")) == NULL) {
            wperror("can't #include %p", r2);
            wsfree(r2, 0);
        } else {
            pincl = alloc(sizeof(pincl_t), pincl);
            pincl->fname = r2;
            pincl->lineno = 0;
            //finit(&pincl->fio, fp, 0);
            pincl->fio._nleft = 0;
            pincl->fio.fp = fp;
            pflag = 1;
        }
        break;
    case P_LINE:      // #line
        if (r4->next->type != NUMBER)
            wperror("bad #line");
        else {
            btos(r4->next->tok, r4->next->toklen, &var_A, 10);
            pincl->lineno = var_A;
            if (r2 = getfnam(r4->next->next)) {
                pflag = 1;
                if (pincl->fname)
                    wsfree(pincl->fname, 0);
                pincl->fname = r2;
            }
        }
        break;
    case P_HASH:      // #
        if (r4->next->type != NL)
            wperror("bad #xxx");
        break;
    default:
        putls(r4);
        break;
    }
    frelst(r4, 0);
}
