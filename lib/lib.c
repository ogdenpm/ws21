#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_WARNINGS

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#include "std.h"
#include "support.h"

int hsize = 16;
uint16_t magic = 0xff75;
int nsize = 14;
int roundAlign;
long msize;
int cfl, dfl, pfl, rfl, tfl, vfl, v6fl, v7fl, xfl;
char *_pname = "lib";
int vers;
int nxtexit;

#ifdef _MSC_VER
int endup(void);
#else
void endup(int n, void *p);
#endif
int gthdr(FILE *fp, uint8_t *arg_4, int arg_6);
char *inlist(int arg_2, char **arg_4, char *arg_6);
FILE *lcreate(char *arg_2);
FILE *lopen(char *arg_2, int arg_4);
void pthdr(FILE *fp, uint8_t *arg_4);
void putback(char *arg_2, long *arg_4, int cnt);
int repl(int arg_2, char **arg_4, char *arg_6, int arg_8);
void report(char *s1, char *s2);
void skip(FILE *fp, long arg_4, int arg_8);
int tab(int arg_2, char **arg_4, char *arg_6, int arg_8);
void to11l(uint8_t *arg_2, uint32_t arg_4);
uint32_t x11tol(uint8_t *r4);
int xtract(int arg_2, char **arg_4, char *arg_6, int arg_8);

long copy(FILE *fpin, FILE *fpout, long arg_6) {
    uint8_t var_20A[512];
    long var_A = 0;
    int r4;
    int r2;

    while (arg_6 == 0 || var_A < arg_6) {
        if (arg_6 != 0 && var_A + 512 >= arg_6)
            r4 = (int)(arg_6 - var_A);
        else
            r4 = 512;

        r2 = (int)fread(var_20A, 1, r4, fpin);
        if (r2 == 0 && arg_6 != 0L)
            error("read error", 0);
        else if (r2 == 0)
            break;
        else if (fwrite(var_20A, 1, r2, fpout) != r2)
            error("write error", 0);
        var_A += r2;   // 1F4
    }
    if (arg_6 == 0L && (var_A & roundAlign) && putc(0, fpout) == EOF)
        error("padding error", 0);
    return (long)var_A;
}

int del(int arg_2, char **arg_4, char *arg_6, int arg_8) {
    FILE *fpout, *fpin;
    int var_8 = 1;
    char hdr[26];
    char *var_24;

    if (arg_2 == 0)
        error("no delete files", 0);

    fpin = lopen(arg_6, 1);     // modified last argument to 1 so error if library file doesn't exist
    fpout = lcreate(0);

    while (gthdr(fpin, hdr, 1)) {
        if ((var_24 = inlist(arg_2, arg_4, hdr)) == 0) {
            pthdr(fpout, hdr);
            copy(fpin, fpout, msize + (msize & roundAlign));
            if (vfl)
                report("c ", hdr);
        } else {
            skip(fpin, msize, roundAlign);
            if (vfl)
                report("d ", var_24);
        }
    }
    fclose(fpin);
    fclose(fpout);
    putback(arg_6, 0, 0);
    return var_8;
}



#ifdef _MSC_VER
int endup(void) {
#else
void endup(int n, void *p) {
#endif
    remove(uname());
#ifdef _MSC_VER
    return 0;
#endif
}


int gthdr(FILE *fp, uint8_t *arg_4, int arg_6) {
    if (fread(arg_4, 1, hsize, fp) != hsize || *arg_4 == 0)
        return 0;
    if (vers != 7)
        msize = lstoi(arg_4 + 14);
    else
        msize = x11tol(arg_4 + 22);

    arg_4[nsize] = 0;
    return arg_6 == 0 || msize != 0xff ? 1 : 0;
}



char *inlist(int arg_2, char **arg_4, char *arg_6) {
    if (arg_2 == 0)
        return 0;
    for (; arg_2 != 0; arg_2--, arg_4++) {
        if (strcmp(*arg_4, arg_6) == 0)
            return *arg_4;
    }
    return 0;
}

void signalHandler(int signal) {
    exit(0);
}


FILE *lcreate(char *arg_2) {
    FILE *fp;
    uint8_t var_8[2];

    itols(var_8, magic);
        if (arg_2 == 0) {
            arg_2 = uname();
            if (nxtexit++ == 0)
#ifdef _MSC_VER
                onexit(endup);
#else
                on_exit(endup, NULL);
#endif
            signal(SIGINT, signalHandler);
        }
    if ((fp = fopen(arg_2, "wb")) == 0)
        error("can't create ", arg_2);
    else if (fwrite(var_8, 1, 2, fp) != 2)
        error("can't write to ", arg_2);
    return fp;
}


FILE *lopen(char *arg_2, int arg_4) {
    FILE *fp;
    if ((fp = fopen(arg_2, "rb")) == 0 && arg_4 != 0)
        error("can't read library ", arg_2);
    else if (fp == 0)
        magic = vers == 7 ? 0xff65 : vers == 6 ? 0xff6d : 0xff75;
    else if (fread((char *)&magic, 1, 2, fp) != 2)
        error("can't read header for ", arg_2);
    else
        magic = lstoi((uint8_t *)&magic);

    switch (magic) {
    case 0xff75:
        hsize = 16;
        nsize = 14;
        roundAlign = 0;
        vers = 0;
        break;
    case 0xff6d:
        hsize = 16;
        nsize = 8;
        roundAlign = 1;
        vers = 6;
        break;
    case 0xff65:
        hsize = 26;
        nsize = 14;
        roundAlign = 1;
        vers = 7;
        break;
    default:
        error("not library format: ", arg_2);
        break;
    }
    return fp;
}



int main(int argc, char **argv) {

    int r2;
    char **r3 = ++argv;
    int (*r4)(int, char **, char *, int) = tab;        // value to suppress compiler 

    if (--argc < 1)
        usage("expects <lfile>");

    getflags(&argc, &argv, "c,d,p,r,t,v6,v7,v,x:<lfile> F <files>", &cfl, &dfl, &pfl,
        &rfl, &tfl, &v6fl, &v7fl, &vfl, &xfl);
    r2 = 0;
    vers = v6fl ? 6 : v7fl ? 7 : 0;
    if (cfl + dfl + pfl + rfl + tfl + xfl > 1)
        usage("takes only one of -[c d p r t x]");
    else if (xfl)
        r4 = xtract;
    else if (tfl)
        r4 = tab;
    else if (rfl)
        r4 = repl;
    else if (pfl) {
        r4 = xtract;
        r2 = 1;
    } else if (dfl)
        r4 = del;
    else if (cfl) {
        r4 = repl;
        r2 = 1;
    }

    return r4(argc, argv, *r3, r2);
}


void putback(char *arg_2, long *arg_4, int cnt) {

    uint8_t hdr[26];
    FILE *fpin, *fpout;

    fpin = lopen(uname(), 1);
    fpout = lcreate(arg_2);

    while (gthdr(fpin, hdr, 0)) {
        if (msize == -1)
            if (--cnt >= 0)
                msize = *arg_4++;
            else
                error("replacement botch", 0);
        pthdr(fpout, hdr);
        copy(fpin, fpout, msize + (msize & roundAlign));
    }
    fclose(fpout);
    fclose(fpin);
    return;
}

void pthdr(FILE *fp, uint8_t *arg_4) {
    int r4;
    for (r4 = 0; r4 < 14 && arg_4[r4]; r4++)
        ;

    while (r4 < 22)
        arg_4[r4++] = 0;

    if (vers != 7)
        itols(arg_4 + 14, (short)msize);
    else
        to11l(arg_4 + 22, msize);

    if (fwrite(arg_4, 1, hsize, fp) != hsize)
        error("can't write header", 0);

    return;
}

int repl(int arg_2, char **arg_4, char *arg_6, int arg_8) {
    char *var_28;
    long *var_26;
    long *var_24;
    uint8_t hdr[26];
    int var_8;
    FILE *fpin, *fpout, *fpmod;


    if (arg_2 == 0)
        error("no replacement files", 0);

    if (arg_8 != 0)
        remove(arg_6);

    fpin = lopen(arg_6, 0);
    fpout = lcreate(0);
    var_26 = var_24 = malloc(arg_2 * sizeof(long));
    if (fpin) {
        while (gthdr(fpin, hdr, 1)) {
            if ((var_28 = inlist(arg_2, arg_4, hdr)) == 0) {
                pthdr(fpout, hdr);
                copy(fpin, fpout, msize + (msize & roundAlign));
                if (vfl)
                    report("c ", hdr);
            } else {
                fseek(fpin, msize + (msize & roundAlign), 1);
                if ((fpmod = fopen(hdr, "rb")) == 0)         // get file name from hdr
                    error("can't read ", hdr);

                msize = -1;
                pthdr(fpout, hdr);
                *var_26++ = copy(fpmod, fpout, 0L);  // var_26 (long *)
                fclose(fpmod);
                if (vfl)
                    report("r ", var_28);
                *var_28 = 0;
            }
        }

        fclose(fpin);
    }

    for (; arg_2; arg_2--, arg_4++) {
        if (*arg_4) {
            for (var_8 = 0; var_8 < 14; var_8++)
                if ((hdr[var_8] = (*arg_4)[var_8]) == 0)
                    break;
            msize = -1;
            if ((fpmod = fopen(hdr, "rb")) == 0)
                error("can't read ", hdr);

            pthdr(fpout, hdr);
            *var_26++ = copy(fpmod, fpout, 0L);
            fclose(fpmod);
            if (vfl)
                report("a ", *arg_4);
        }
    }

    fclose(fpout);
    putback(arg_6, var_24, (int)(var_26 - var_24));   // pointers scaled by 4
    return 1;
}

void report(char *s1, char *s2) {       // replaced putstr
    printf("%s%s\n", s1, s2 ? s2 : "");
}

void skip(FILE *fp, long arg_4, int arg_8) {
    if (arg_8)
        arg_4 += (arg_4 & 1);
    if (arg_4 != 0)
        fseek(fp, arg_4, 1);
}


int tab(int arg_2, char **arg_4, char *arg_6, int arg_8) {
    char vstr[10];
    uint8_t hdr[26];
    int r4 = arg_2 ? 0 : 1;
    vstr[0] = ':';
    vstr[1] = ' ';
    FILE *fpin = lopen(arg_6, 1);
    if (vfl) {
        printf("Library %s", arg_6);
        switch (vers) {
        case 0:
            printf(" Whitesmiths standard format.\n");
            break;
        case 6:
            printf(" Unix v.6 format.\n");
            break;
        case 7:
            printf(" Unix v.7 format.\n");
            break;
        default:
            break;
        }
    }
    while (gthdr(fpin, hdr, 1)) {
        if (inlist(arg_2, arg_4, hdr) == 0 || r4)
            if (vfl) {
                sprintf(vstr + 2, "%ld", msize);
                report(hdr, vstr);
            } else
                report(hdr, 0);
            skip(fpin, msize, roundAlign);
    }
    fclose(fpin);
    return 1;
}


void to11l(uint8_t *arg_2, uint32_t arg_4) {
    itols(arg_2, arg_4 >> 16);
    itols(arg_2 + 2, arg_4);
}

uint32_t x11tol(uint8_t *r4) {
    return (lstoi(r4) << 16) + (lstoi(r4 + 2) & 0xffff);
}

int xtract(int arg_2, char **arg_4, char *arg_6, int arg_8) {
    uint8_t hdr[26];
    FILE *fpin, *fpout;
    int r4 = arg_2 ? 0 : 1;

    fpin = lopen(arg_6, 1);

    while (gthdr(fpin, hdr, 1)) {
        if (inlist(arg_2, arg_4, hdr) == 0 && r4 == 0)
            skip(fpin, msize, roundAlign);
        else {
            if (arg_8 == 0) {
                if ((fpout = fopen(hdr, "wb")) == 0)
                    error("can't create ", hdr);
            } else
                fpout = stdout;

            copy(fpin, fpout, msize);
            skip(fpin, msize & roundAlign, 0);
            if (arg_8 == 0)
                fclose(fpout);
            if (vfl)
                report("x ", hdr);
        }
    }
    fclose(fpin);
    return 1;
}

