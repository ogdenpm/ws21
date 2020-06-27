#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <io.h>

#include "std.h"
#include "support.h"

typedef struct {        // valid as many_t arg
    int ntop;
    char *val[10];
} list_t;

typedef struct {
    char *str;
    int16_t code;
} code_t;

typedef  struct _token {
     struct _token *next;
     int type;
     char *spc;
     int spclen;
     char *tok;
     int toklen;
 } token_t;

typedef struct {            // needed as cpp pushes predefines ahead of the file stream
        FILE *fp;
        short _nleft;
        char *_pnext;
        char _buf[512];
} fio_t ;

typedef struct _pincl {
    struct _pincl *next;
    char *fname;
    uint16_t lineno;
//    FIO fio;
    fio_t fio;
} pincl_t;

typedef struct _sym {
    struct _sym *next;
    int len;
    char *val;
    char name[8];
} sym_t;

typedef struct _param {
    struct _param *next;
    token_t *first;
    token_t *last;
} param_t;

enum {SQSTRING=1, NL, ID, NUMBER, PUNCT, DQSTRING};
enum { P_DEFINE = 0xa, P_ELSE = 0xb, P_ENDIF = 0xc,
       P_IF = 0xd, P_IFDEF = 0xe, P_IFNDEF = 0xf,
       P_INCLUDE = 0x10, P_LINE = 0x11, P_HASH = 0x12,
       P_UNDEF = 0x13};


extern int cFlag;
extern list_t dList;
extern char *oFile;
extern int xflag;
extern int v6flag;
extern int pchar;
extern int schar;
extern char *iprefix;
extern char *_pname;
extern char **argv;
extern int argc;
extern int errfd;
extern pincl_t *pincl;
extern int nerrors;
extern int pflag;
FILE *ofd;          // added

#define lenstr  strlen

token_t *buytl(token_t *r4, token_t *r2);

token_t *dodef(token_t *arg_2, token_t *arg_4, param_t *arg_6);

int doesc(char *arg_2, char *r4, int arg_6);
token_t *doexp(token_t *arg_2);

int dopunct(token_t **arg_2);
#if 0
void errfmt(char *r2, char *arg_4);
#else
void errfmt(char *fmt, ...);
#endif
int eval(token_t *r4);
int exop(token_t **r4, int arg_4);
token_t *expr(token_t *arg_2, long *arg_4);
int expri(int r4);
token_t *extail(int arg_2, long *arg_4, int *arg_6, token_t *arg8);
token_t *exterm(token_t *arg_2, long *r4);


int firnon(char *r4, int arg_4, int arg_6);
#if NATIVE
int flaccum(char *r4, double *arg_4, int r2);
#else
int flaccum(char *r4, uint64_t *arg_4, int r2, int *skipped);
#endif
token_t *getargs(token_t *arg_2, param_t **arg_4);

token_t *getex();
char *getfnam(token_t *r4);

token_t  *getin();
char *getln(pincl_t *arg_2);
sym_t **hash(char *r4, int r2);
void install(char *arg_2, int r4, char *arg_6);

token_t *lexchar(token_t *r4);
token_t *lexfloa(token_t *arg_2);
char *lexfnxt(token_t **arg_2, char *arg_4);
token_t *lexiden(token_t *r4);
token_t *lexint(token_t *r4, int base, int skip);
token_t *lexnum(token_t *r4);
token_t *lexpunc(token_t *arg_2);
token_t *lexstri(token_t *r4);
char *lookup(char *r4, int r2);
pincl_t *nxtfile();
void pargs(char *r4, int r2);
#if 0
void ws_perror(char *arg_2, char *arg_4, char *arg_6);
#else
void wperror(char *msg, ...);
#endif
void predef(list_t *r4);
int punct(token_t *r4, int arg_4);
#if 0
void putcode(char *r2, char *arg_4 /*...*/);
#else
void putcode(char *fmt, ...);
#endif
void putls(token_t *r4);

token_t *putgr(token_t *r4, int r2);
void putns(token_t *r4);
int scntab(code_t *group, unsigned grplen, const char *token, unsigned toklen);
token_t *stotl(char *r4);
void undef(r4, arg4);



unsigned btos(char *s, unsigned n, short *pinum, short base);
void *wsfree(void *pcell, void *link);
void *frelst(void *plist, void *pstop);
FIO *finit(FIO *pfio, short fd, short mode);
short create(char *fname, short mode, unsigned rsize);
FIO *wsfclose(FIO *pfio);
//unsigned getl(FIO *pfio, char *s, unsigned n);
#ifdef NATIVE
double dtento(double d, short exp);
#else
wsDouble mkWsDouble(uint64_t matissa, short exp);
#endif
unsigned putlin(char *s, unsigned n);

//int getfile(int *pac, char ***pav, int dfd, int efd);
unsigned getl(fio_t *pfio, char *s, unsigned n);