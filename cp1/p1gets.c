#include "cp1.h"
#if _DEBUG
void dumptok(tok_t *tok);
#endif
extern bool needNL;

char noname[8];
int tokSP;
tok_t tokStack[3];           // tok size is 10 bytes;
int inCnt;
char inBuf[128];
char *inPtr;

int outCnt;
char outBuf[128];

int alt(char *r4) {     // accept one of the alternatives, 0 if none
    tok_t var_10;
    int r2;

    gettok(&var_10);
    if ((r2 = r4[scnstr(r4, var_10.code)]) == 0)
        baktok(&var_10);
    return r2 & 0xff;
}


void baktok(tok_t *r4) {
    if (tokSP < 3)
        cpytok(&tokStack[tokSP++], r4);
    else {
        wsperror("!TOKEN OVERFLOW");
        exit(0);
    }
}

int eat(int r4) {
    tok_t var_10;
    if (r4 != gettok(&var_10)->code) {
        baktok(&var_10);
        return 0;
    }
    return r4;
}

int getch() {
    return getc(stdin);
    //if (inCnt == 0)
    //    inCnt = fread(inPtr = inBuf, 1, 128, stdin);
    //if (inCnt <= 0)
    //    return inCnt = EOF;
    //inCnt--;
    //return *inPtr++ & 0xff;
}

void getstr(char *r4, int r2) {
    while (r2 > 0) {
        *r4++ = needc();
        r2--;
    }
}

tok_t *gettok(tok_t *r4) {
    if (tokSP > 0)
        return cpytok(r4, &tokStack[--tokSP]);
    return gtok(r4);
}


tok_t *gtok(tok_t *r4) {
    char *var_8;
    int r2, r3;

    while ((r4->code = getch()) == C_LINENO || r4->code == C_FILE) {
        if (r4->code == C_LINENO) {
            getstr((char *)&lineno, 2);
        } else {
            r2 = needc();
            var_8 = alloc(r2 + 1, 0);
            for (r3 = 0; r3 < r2; r3++)
                var_8[r3] = needc();
            var_8[r2] = 0;
            infile = wsfree(infile, var_8);
        }
    }
    if (r4->code == EOF || r4->code == 0)
        r4->code = EOF;
    else if ((r4->code & 0xf0) == 0x10)
        switch (r4->code) {
        case C_DBL:
            getstr((char *)&r4->dbl, 8);    // read double in raw format
            break;
        case C_ID:
            if ((r2 = needc()) > 8)
                r2 = 8;
            for (r3 = 0; r3 < r2; r3++)
                r4->name[r3] = needc();
            while (r2 < 8)
                r4->name[r2++] = 0;
            break;
        case C_UINT32: case C_INT32: case C_UINT16: case C_INT16: case C_UINT8: case C_INT8:
            getstr((char *)&r4->lng, 4);        // read long in raw
            break;
        case C_STRING:
            getstr((char *)&r4->len, 2);
            r4->str = alloc(r4->len, 0);
            for (r3 = 0; r3 < r4->len; r3++)
                r4->str[r3] = needc();
            break;
        default:
            wsperror("!BAD CHAR");
            exit(0);
            break;
        }
#if _DEBUG
    dumptok(r4);
#endif
    return r4;
}


tok_t *ident(tok_t *r4) {
    if (gettok(r4)->code != C_ID) {
        baktok(r4);
        r4->code = 0;
        return 0;
    }
    return r4;
}

uint8_t bBDF7[] = { C_LPAREN, C_RPAREN, C_SEMI, C_COLON, C_LBRACE, C_RBRACE,
                        C_WHILE, C_RBRACKET, 0 };
char *off_BE00[] = { "(", ")", ";", ":", "{", "}", "while", "]", "???" };

int need(int r4) {
    if (eat(r4) == 0) {
        nmerr("missing", off_BE00[scnstr(bBDF7, r4)]);
        return 0;
    }
    return r4;
}


int needc() {
    int r4 = getch();
    if (r4 == EOF) {
        wsperror("!EOF");
        exit(0);
    }
    return r4;
}


void nmerr(char *arg_2, char *arg_4) {      // modified to use modern C I/O
    fprintf(errfd, "%s%s%d: %s %.*s\n", infile, *infile ? " " : "", lineno, arg_2, inbuf(arg_4, 8, "\0"), arg_4);
    nerrors++;
}




int peek(int r4) {
    tok_t var_10;
    baktok(gettok(&var_10));
    if (var_10.code == r4)
        return r4;
    else
        return 0;
}

void wsperror(char *arg_2) {        // modified to use modern C I/O
    fprintf(errfd, "%s%s%d: %s\n", infile, *infile ? " " : "", lineno, arg_2);
    nerrors++;
}


void putch(int arg_2) {
    putc(arg_2, outfd);
#ifdef _DEBUG
    if (needNL)
        putchar(' ');
    printf("%02X", arg_2 & 0xff);
    needNL = true;
    //if (outCnt == 128 || (arg_2 < 0 && outCnt != 0)) {
    //    if (fwrite(outBuf, 1, outCnt, outfd) != outCnt) {
    //        wsperror("can't write!");
    //        exit(0);
    //    }
    //    outCnt = 0;
    //}
    //outBuf[outCnt++] = arg_2;
#endif
}

void recover(char *arg_2) {
    tok_t var_10;

    wsperror(arg_2);
    while (gettok(&var_10)->code != -1 && var_10.code != C_SEMI) {
        if (var_10.code == C_RBRACE)
            return;
    }
    baktok(&var_10);
}

