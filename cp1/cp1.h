#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <io.h>

#include "std.h"
#include "lex.h"


typedef uint64_t wsDouble;

typedef struct _aux {
    struct _aux *next;
    union {
        struct _sym *psym;
        uint8_t b[2];
        long lng;
    };
} aux_t;


typedef struct _term {
    struct _term *next;
    aux_t *aux;
    union {
        char name[8];
        wsDouble dbl;
    };
    uint16_t dataType;
    int16_t code;
    union {
        struct {
            long lng;
            uint8_t b14;
            uint8_t b15;
        } term;
        struct {
            struct _term *lhs;
            struct _term *rhs;
            struct _term *w14;
        } op;
        struct {
            char *str;
            long len;
        } lit;
        int8_t b10;
    };
} term_t;


typedef struct _case {
    struct _case *next;
    int16_t caseLabel;
    long caseValue;
} case_t;



typedef struct {
    int16_t code;
    union {
        char name[8];
        wsDouble dbl;
        long lng;
        struct {
            char *str;
            uint16_t len;
        };
    };
} tok_t;

typedef struct _sym {
    struct _sym *next;
    union {
        aux_t *aux;
        struct _sym *psym;
        uint8_t auxb[2];
        uint16_t w2;
        intptr_t intPtr;
    };
    char name[8];
    uint16_t dataType;
    int16_t code;
    union {
        struct _sym *schain;   // next structure / union
        term_t *tchain;
        uint16_t w;
        uint8_t b[2];
        long lng;
        struct {
            char *str;
            uint16_t len;
        };
    };
} sym_t;


extern int iregs;
extern int tchar;
extern int tfield;
extern int tint;
extern int tunsign;
extern int aflag;
extern int cflag;
extern int eflag;
extern int mflag;
extern int uflag;
extern int bitswd;
extern int bndef;
extern int intsize;
extern int nlen;
extern char *_pname;
extern FILE *errfd;
extern char *ofile;
extern int nerrors;
extern char *infile;
extern int lineno;
extern FILE *outfd;
extern case_t *casetab;
extern term_t *exlist;
extern sym_t *lbltab;
extern term_t *littab;
extern sym_t *mostab;
extern sym_t *strtab;
extern sym_t *symend;      // not used
extern sym_t *symtab;
extern sym_t *untab;

extern int decflag;
extern char noname[];


extern uint8_t typtab[];
extern uint8_t tyops[];

int type(int r4);
int stype(int arg_2);
sym_t *setty(sym_t *r4, int arg_4, intptr_t arg_6);
term_t *setad(term_t *r4, char *r2, long arg_6, uint8_t arg_A, uint8_t arg_C);
sym_t *retype(sym_t *r4, int r2);
int ptype(int arg_2);
int maxify(int arg_2);
sym_t *lookup(char *arg_2, sym_t *r4, sym_t *arg_6);
uint16_t lname(char *r4);
int itype(arg_2);
int iscons(term_t *r4);
void fixlint();
bool exmatch(sym_t *arg_2, sym_t *arg_4);
void exchk(sym_t *r4);
int dtype(int arg_2);
int dlit(term_t *r4);
int dety(int arg_2);
tok_t *cpytok(tok_t *r4, tok_t *r2);
void cpynm(char *r4, char *r2);
void clrsym(sym_t *r4);
long bytes(int r4, aux_t *r2);
term_t *buyterm(int arg_2, aux_t *arg_4, char *arg_6, long arg_8, uint8_t arg_C, uint8_t arg_E);
sym_t *buysym(int arg_2, aux_t *arg_4, char *r4, int arg_8);
aux_t *buysat(sym_t *r4, aux_t *arg_4);
term_t *buyop(int arg_2, term_t *arg_4, term_t *arg_6);
aux_t *buymat(long arg_2, aux_t *arg_6);
int bound(int r4, aux_t *r2);
long bndify(int arg_2, aux_t *r4, long arg_6);
term_t *reduce(term_t *r4);
term_t *cpyterm(term_t *r4, term_t *r2);
bool cansub(term_t *r4, term_t *r2);
bool canmul(term_t *r4, term_t *r2);
bool canadd(term_t *r4, term_t *r2);
bool cachk(term_t *r4);
int rbuy(int r4, int *r2);
void pvoid(arg_2);
void ptname(char *r4);
void ptlab(short arg_2);
void ptint(long arg_2);
void ptexpr(term_t *r4);
void pswtab(case_t *r4, int arg_4, int arg_6);
void pswitch(short arg_2);
void pstr(char *r4, int r2);
void pspace(long arg_2);
void pret();
void pregs(int arg_2);
void pref(char *arg_2);
void pmove(term_t *arg_2, term_t *arg_4);
int plabel(int r4);
int pjump(int r4, int r2);
int pjt(term_t *r4, int r2, int r3);
int pjf(term_t *r4, int r2, int r3);
int pjc(int arg_2, term_t *arg_4, term_t *arg_6, int r4);
int pint(long arg_2, int r4);
void pfunc(char *arg_2);
int pfloat(wsDouble *arg_2, int r2);
void pend();
void pdef(char *arg_2);
void pdata(char *arg_2, int arg_4);
void pcode(int arg_2);
int pcase(short r4);
void pauto(long arg2);
int paddr(char *r4, long arg_4, int arg_8);
char *lblname(int r4);
int crs();
int main(int argc, char **argv);
bool gscty(sym_t *r4, ...);     // need to fix with stdarg
sym_t *gdecl(sym_t *arg_2, bool arg_4);
sym_t *dterm(sym_t *r4, bool arg_4);
aux_t *decsu(int arg_2);
bool cmptype(sym_t *arg_2, sym_t *arg_4);
void recover(char *arg_2);
void putch(int arg_2);
void wsperror(char *arg_2);
int peek(int r4);
void nmerr(char *arg_2, char *arg_4);
int needc();
int need(int r4);
tok_t *ident(tok_t *r4);
tok_t *gtok(tok_t *r4);
tok_t *gettok(tok_t *r4);
void getstr(char *r4, int r2);
int getch();
int eat(int r4);
void baktok(tok_t *r4);
int alt(char *r4);     // accept one of the alternatives, 0 if none
void perc(sym_t *r4);
sym_t *lookex(char *arg_2, sym_t *r4);
bool fninit(sym_t *r4);
void dostat(int arg_2, int arg_4);
void doblock(int arg_2, int arg_4);
void autinit(sym_t *r4);
term_t *mtrail(term_t *r4);
term_t *mterm(bool arg_2);
term_t *mtail(int arg_2, term_t *r4, term_t **arg_6);
term_t *mident(char *arg_2);
term_t *mexpr(bool arg_2);
term_t *melist(bool arg_2);
term_t *mcast();
term_t *mbin();
term_t *gtest(bool arg_2);
term_t *gexpr(bool arg_2);
term_t *gelist(bool arg_2);

long _const(int arg_2);
long stinit(sym_t *r4, term_t *r2, bool arg_6);
long dinit(int arg_2, aux_t *arg_4, term_t *r4, bool arg_8);
void datinit(sym_t *r4);
long arinit(int arg_2, aux_t *arg_4, term_t *r4, bool arg_8);

void untest(term_t *r4, term_t *r2);
term_t *typify(term_t *r4);
void tquery(term_t *r4);
void tpoints(term_t *r4, term_t *r2, term_t *arg_6);
void tfn(term_t *r4);
term_t *scalify(term_t *r4);
term_t *ptify(term_t *arg_2, int arg_4, aux_t *aux);
int maxty(int arg_2, int arg_4, int arg_6);
void docheck(term_t *r4);


// support functions

void *alloc(unsigned, void *);
unsigned scnstr(const uint8_t *, uint16_t );        // 2nd arg is actuall uint8_t, this hack prevents lots of warnings.
void *wsfree(void *, void *);
unsigned inbuf(char *, unsigned, char *);
unsigned itob(char *s, int, short);
void wsperror(char *);
int cmpbuf(char *, char *, unsigned);
char *buybuf(char *, unsigned);
char *getflags(int *pac, char ***pav, const char *fmt, ...);
void *frelst(void *, void *);
wsDouble long2WsDouble(long matissa);



