#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "std.h"
#include "support.h"

#define SEGIN    0
#define RELIN    1

typedef struct {        // valid as many_t arg
    int ntop;
    char *val[10];
} list_t;

typedef struct {        // large object file header
    uint8_t ident, conf;    // 0
    uint32_t table;         // 2  
    uint32_t text;          // 4
    uint32_t data;          // 8
    uint32_t bss;           // 12 (0C)
    uint32_t heap;          // 16 (10)
    uint32_t textoff;       // 20 (14)
    uint32_t dataoff;       // 24 (18)
} objhdr_t;

typedef struct {        // small object file header
    uint8_t ident, conf;    // 0
    uint16_t table;         // 2  
    uint16_t text;          // 4
    uint16_t data;          // 6
    uint16_t bss;           // 8
    uint16_t heap;          // 10
    uint16_t textoff;       // 12
    uint16_t dataoff;       // 14
} objhdr16_t;


typedef struct {
    uint32_t val;
    uint8_t flag;
    uint8_t name[15];
} symbol_t;

typedef struct {
    char name[14];      // 0
    uint16_t size;      // 14
} wslib_t;

typedef struct {
    char name[8];       // 0
    char unixInfo[6];
    uint16_t size;      // 14
} v6lib_t;

typedef struct {
    char name[14];      // 0
    char unixInfo[8];   // 14   conflict in documentation - ar.h shows this to be 8 bytes doc says 6, lib uses 8, link uses 6!!!!
    uint32_t size;      // 22
} v7lib_t;


typedef struct {
    long tsiz;      // 0
    long dsiz;      // 4
    long bsiz;      // 8
    long bpad;      // 0C - 12
    long tbias;     // 10 - 16
    long dbias;     // 14 - 20
    uint16_t symSiz;    // - 22
} obhdr_t;

typedef struct {
    uint16_t tsiz;      // 0
    uint16_t dsiz;      // 2
    uint16_t bsiz;      // 4
    uint16_t bpad;      // 6
    uint16_t tbias;     // 8
    uint16_t dbias;     // 10
    uint16_t symSiz;    // 12
} obhdr16_t;

typedef struct {      // size 10 bytes
    long fpos;
    int16_t off, size;
    uint8_t buf[64];
} iobuf_t;

// shared data

extern int longint;
//extern int lsfmt;
extern int ok;
extern int lenname;
extern long maxbnd;
extern uint16_t binhdr;

extern long liboff[128];

extern obhdr_t obhdr;

extern long bsiz, dsiz, tsiz;
extern uint16_t nsyms;
extern uint16_t nund;

extern symbol_t *stabs[32];        // 32 pointers
extern int afl, cfl, dfl, hfl, rfl, tfl, dround;
extern int xfl;
extern list_t llist;

extern long drmask;
extern char *endbss, *endtext, *enddata;
extern char *ofile;
extern list_t ulist;
extern long bpad;
extern long tbias;
extern long dbias;

extern FILE *ifd;
extern FILE *ofd;
extern FILE *tfd;

extern iobuf_t ibuf[2];
extern iobuf_t obuf[4];
extern long iseek;



// function prototypes
void addend(char *arg_2, int arg_4, long arg_6);
void addlib(long arg_2);
symbol_t *addsym(char *arg_2, int arg_4, long arg_6);
void addusym(char *arg_2);
long docode(int segId, int relId, int symCnt, symbol_t **arg_8, obhdr_t *segHdr, long loadBias, long segByesLeft, long arg_14);
uint8_t getby(int arg_2);
FILE *gtlfile(int *pac, char ***pav, char **name);
uint16_t gtmagic(FILE *fp);
char *gtsyms(FILE *fp, obhdr_t *arg_4);
symbol_t *lookup(char *arg_3);
int mid1();
int mid2();
int pass1(int arg_2, char **arg_4);
int pass2(int arg_2, char **arg_4);
uint32_t rebias(int flags, long tbias, long dbias, long bbias);
void relby(int arg_2, int arg_4);
void relint(int arg_2, uint32_t arg_4);
void relsym(symbol_t *arg_2);
void relwd(int arg_2, int arg_4);
void remark(char *arg_2, char *arg_4);
uint32_t xstol(uint8_t *arg_2);
int xstos(uint8_t *arg_2);
