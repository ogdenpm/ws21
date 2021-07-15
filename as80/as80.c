#define _CRT_NONSTDC_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stddef.h>

#include "std.h"
#include "support.h"


typedef struct _sym {
    struct _sym *next;
    uint16_t flags;
    intptr_t val;
    intptr_t fixup;
    char name[8];
} sym_t;

typedef struct _lit {
    struct _lit *next;
    uint16_t flags;
    intptr_t val;
    intptr_t fixup;
    char name[8];
    sym_t *psym;
} lit_t;

char *_pname = "as.80";

typedef struct {
    char *str;
    int16_t code;
} code_t;

#define TOKMASK 0x1ff


enum {
    T_STRING = 0x40, T_WORD,
    T_BC = 0x42, T_DE, T_HL, T_SP, T_46, T_47,
    T_BYTE = 0x48, T_ID,    // note *T_WORD maps to T_ID                      
    T_STARBC = 0x4a, T_STARDE, T_STARHL, T_STARSP, T_SUB46, T_SUB47,
    T_B = 0x50, T_C, T_D, T_E, T_H, T_L,
    T_A = 0x57, T_AF,
    T_PUBLIC = 0xAC,
    T_CNZ = 0x198, T_CZ, T_CNC, T_CC, T_CPO, T_CPE, T_CP, T_CM,
    T_JNZ, T_JZ, T_JNC, T_JC, T_JPO, T_JPE, T_JP, T_JM,
    T_CALL, T_JMP, T_CONDGROUP
};

code_t group1[] = {
    {"\x1" "a", T_A},
    {"\x1" "b", T_B},
    {"\x1" "c", T_C},
    {"\x1" "d", T_D},
    {"\x1" "e", T_E},
    {"\x1" "h", T_H},
    {"\x1" "l", T_L},
    {"\x2" "af", T_AF},
    {"\x2" "bc", T_BC},
    {"\x2" "cc", T_CC},
    {"\x2" "cm", T_CM},
    {"\x2" "cp", T_CP},
    {"\x2" "cz", T_CZ},
    {"\x2" "de", T_DE},
    {"\x2" "hl", T_HL},
    {"\x2" "jc", T_JC},
    {"\x2" "jm", T_JM},
    {"\x2" "jp", T_JP},
    {"\x2" "jz", T_JZ},
    {"\x2" "sp", T_SP},
    {"\x3" "cnc", T_CNC},
    {"\x3" "cnz", T_CNZ},
    {"\x3" "cpe", T_CPE},
    {"\x3" "cpo", T_CPO},
    {"\x3" "jmp", T_JMP},
    {"\x3" "jnc", T_JNC},
    {"\x3" "jnz", T_JNZ},
    {"\x3" "jpe", T_JPE},
    {"\x3" "jpo", T_JPO},
    {"\x4" "call", T_CALL},
    {"\x6" "public", T_PUBLIC},
    {NULL, 0}
};

enum {
    T_LPAREN = -3, T_RPAREN = -4, T_LBRACKET = -5, T_RBRACKET = -6, T_COLON = -7,
    T_STAR = 0x83,
    T_ADDCARRY = 0x108, T_RARROW, T_SUBCARRY, T_DEFINE,
    T_PUSH = 0x10D, T_SWAP, T_POP, T_EQUAL_HL, T_RARROW_HL,
    T_ROT, T_ROTCARRY, T_ROTCARRYSET, T_ROTCARRYCLEAR,
    T_EQUAL_A, T_EQUALNOT, T_COMPARE,
    T_NOT = 0x181, T_OR = 0x181, T_AND, T_PLUS, T_MINUS, T_EQUAL, T_XOR,
    T_COMMA = 0x1AD
};

code_t group2[] = {
    {"\x1" "!", T_OR},      // treat single ! as |
    {"\x1" "&", T_AND},
    {"\x1" "*", T_STAR},
    {"\x1" "+", T_PLUS},
    {"\x1" ",", T_COMMA},
    {"\x1" "-", T_MINUS},
    {"\x1" ":", T_COLON},
    {"\x1" "=", T_EQUAL},
    {"\x1" "^", T_XOR},
    {"\x1" "|", T_OR},
    {"\x2" "+^", T_ADDCARRY},
    {"\x2" "->", T_RARROW},
    {"\x2" "-^", T_SUBCARRY},
    {"\x2" "::", T_COMPARE},
    {"\x2" ":=", T_DEFINE},
    {"\x2" "<=", T_PUSH},
    {"\x2" "<>", T_SWAP},
    {"\x2" "=!", T_EQUALNOT},
    {"\x2" "=>", T_POP},
    {"\x2" "=^", T_EQUAL_HL},
    {"\x3" "->^", T_RARROW_HL},
    {"\x3" "<*>", T_ROT},
    {"\x3" "<+>", T_ROTCARRYSET},
    {"\x3" "<->", T_ROTCARRYCLEAR},
    {"\x3" "<^>", T_ROTCARRY},
    {"\x3" "=a^", T_EQUAL_A},
    {NULL, 0}
};

// for the following just emit the byte value of the instruction
// for in/out the next byte will also be emitted, usually a numeric value
code_t simpleInstructions[] = {
    {"\x2" "di", 0xF3},
    {"\x2" "ei", 0xFB},
    {"\x2" "in", 0xDB},
    {"\x2" "rc", 0xD8},
    {"\x2" "rm", 0xF8},
    {"\x2" "rp", 0xF0},
    {"\x2" "rz", 0xC8},
    {"\x3" "cmc", 0x3F},
    {"\x3" "daa", 0x27},
    {"\x3" "hlt", 0x76},
    {"\x3" "nop", 0x100},
    {"\x3" "out", 0xD3},
    {"\x3" "ret", 0xC9},
    {"\x3" "rnc", 0xD0},
    {"\x3" "rnz", 0xC0},
    {"\x3" "rpe", 0xE8},
    {"\x3" "rpo", 0xE0},
    {"\x3" "stc", 0x37},
    {"\x4" "rst0", 0xC7},
    {"\x4" "rst1", 0xCF},
    {"\x4" "rst2", 0xD7},
    {"\x4" "rst3", 0xDF},
    {"\x4" "rst4", 0xE7},
    {"\x4" "rst5", 0xEF},
    {"\x4" "rst6", 0xF7},
    {"\x4" "rst7", 0xFF},
    {NULL, 0}
};

int xflag;
int lno = 1;
int nerrors;
FILE *ofd = 0;
FILE *tfd[2];
FILE *infio;
//FILE *outfio[2];        // note outfio[1] is tmpfio in original, declared as array as indexed
char *ofile;
int nxtexit;
int sval;
sym_t *cobase;
sym_t *dabase;
sym_t *elc;

sym_t *psym[64];
sym_t *tptr;
char string[128];
uint8_t b6E0C[64];
char b6E4C[2] = { 0, 0 };

uint16_t bincode[] = {
            T_EQUAL, T_PUSH, T_SWAP, T_PLUS, T_MINUS, T_ADDCARRY, T_SUBCARRY, T_AND,
            T_XOR, T_OR, T_COMPARE, T_ROT, T_ROTCARRY, T_ROTCARRYSET, T_ROTCARRYCLEAR, T_JMP,
            T_CONDGROUP, T_EQUAL_HL, T_RARROW_HL, T_EQUAL_A, T_EQUALNOT, 0 };

char crlist[] = { T_B, T_C, T_D, T_E, T_H, T_L, T_STARHL, T_A, 0 };
char irlist[] = { T_BC, T_DE, T_HL, T_SP , 0 };
lit_t *littab;
int ungetCnt;
int ungetBuf[3];
int w6E92;

typedef struct {
    int16_t left, right;
    char *recipe;
} recipe_t;

recipe_t r_EQUAL[] = {
    {3, 5, "1L4,WR"},
    {T_STARBC, T_A, "2"},
    {T_A, T_STARBC, "a"},
    {T_STARDE, T_A, "12"},
    {T_A, T_STARDE, "1a"},
    {T_ID, T_HL, "22,WL"},
    {T_HL, T_ID, "2a,WR"},
    {T_ID, T_A, "32,WL"},
    {T_A, T_ID, "3a,WR"},
    {2, T_BYTE, "6L3,R"},
    {2, 2, "40L3R"},
    {T_SP, T_HL, "f9"},
    {T_BC, T_DE, "4b,42"},
    {T_BC, T_HL, "4d,44"},
    {T_DE, T_BC, "59,50"},
    {T_DE, T_HL, "5d,54"},
    {T_HL, T_BC, "69,60"},
    {T_HL, T_DE, "6b,62"},
    {0, 0, 0}
};

recipe_t r_PUSH[] = {
    {T_BC, T_SP, "c1"},
    {T_DE, T_SP, "d1"},
    {T_HL, T_SP, "e1"},
    {T_AF, T_SP, "f1"},
    {T_SP, T_BC, "c5"},
    {T_SP, T_DE, "d5"},
    {T_SP, T_HL, "e5"},
    {T_SP, T_AF, "f5"},
    {0, 0, 0}
};

recipe_t r_SWAP[] = {
    {T_HL, T_STARSP, "e3"},
    {T_STARSP, T_HL, "e3"},
    {T_HL, T_DE, "eb"},
    {T_DE, T_HL, "eb"},
    {0, 0, 0}
};

recipe_t r_PLUS[] = {
    {T_HL,    3, "9R4"},
    {   3,    1, "3L4"},
    {   2,    1, "4L3"},
    {T_A,    2, "80R"},
    {T_A, T_BYTE, "c6,R"},
    {0, 0, 0}
};

recipe_t r_MINUS[] = {
    {   3,    1, "bL4"},
    {   2,    1, "5L3"},
    {T_A,    2, "90R"},
    {T_A, T_BYTE, "d6,R"},
    {0, 0, 0}
};

recipe_t r_ADDCARRY[] = {
    {T_A,    2, "88R"},
    {T_A, T_BYTE, "ce,R"},
    {0, 0, 0}
};

recipe_t r_SUBCARRY[] = {
    {T_A,    2, "98R"},
    {T_A, T_BYTE, "de,R"},
    {0, 0, 0}
};

recipe_t r_AND[] = {
    {T_A,    2, "a0R"},
    {T_A, T_BYTE, "e6,R"},
    {0, 0, 0}
};

recipe_t r_XOR[] = {
    {T_A,    2, "a8R"},
    {T_A, T_BYTE, "ee,R"},
    {0, 0, 0}
};

recipe_t r_NOT[] = {
    {T_A,    2, "b0R"},
    {T_A, T_BYTE, "f6,R"},
    {0, 0, 0}
};

recipe_t r_COMPARE[] = {
    {T_A,    2, "b8R"},
    {T_A, T_BYTE, "fe,R"},
    {0, 0, 0}
};

recipe_t r_ROT[] = {
    {T_A,    1, "7"},
    {T_A, -1, "f"},
    {0, 0, 0}
};

recipe_t r_ROTCARRY[] = {
    {T_A,    1, "17"},
    {T_A, -1, "1f"},
    {0, 0, 0}
};

recipe_t r_ROTCARRYSET[] = {
    {T_A,    1, "37,17"},
    {T_A, -1, "37,1f"},
    {0, 0, 0}
};

recipe_t r_ROTCARRYCLEAR[] = {
    {T_A,    1, "b7,17"},
    {T_A, -1, "b7,1f"},
    {0, 0, 0}
};

recipe_t r_JMP[] = {
    {   4, T_ID, "c3,WR"},
    {   4, T_STARHL, "e9"},
    {0, 0, 0}
};

recipe_t r_CONDGROUP[] = {
    {   4, T_ID, "c2L0,WR"},
    {0, 0, 0}
};

recipe_t r_EQUAL_HL[] = {
    {T_BC, T_HL, "71,23,70"},
    {T_DE, T_HL, "73,23,72"},
    {0, 0, 0}
};

recipe_t r_RARROW_HL[] = {
    {T_BC, T_HL, "4e,23,46"},
    {T_DE, T_HL, "5e,23,56"},
    {0, 0, 0}
};

recipe_t r_EQUAL_A[] = {
    {T_HL, T_HL, "7e,23,66,6f"},
    {0, 0, 0}
};

recipe_t r_EQUALNOT[] = {
    {T_A, T_A, "2f"},
    {0, 0, 0}
};

recipe_t *tabtab[] = {
    r_EQUAL,
    r_PUSH,
    r_SWAP,
    r_PLUS,
    r_MINUS,
    r_ADDCARRY,
    r_SUBCARRY,
    r_AND,
    r_XOR,
    r_NOT,
    r_COMPARE,
    r_ROT,
    r_ROTCARRY,
    r_ROTCARRYSET,
    r_ROTCARRYCLEAR,
    r_JMP,
    r_CONDGROUP,
    r_RARROW_HL,
    r_EQUAL_HL,
    r_EQUAL_A,
    r_EQUALNOT,
    NULL
};
uint8_t deltas[] = { 2, 0xa, 0x12, 0x1a, 0x22, 0x2a, 0x32, 0x3a, 0, 8, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0xb };

char tmpbuf[9];
int curSeg;

typedef struct {
    intptr_t psym;
    uint8_t type;
    uint8_t size;
    uint8_t buf[128];
} tmpdata_t;

tmpdata_t tmpData[2];

typedef struct {
    long fpos;
    int16_t off, size;
    uint8_t buf[512];       // original shared buffer with finit!!
} iobuf_t;

iobuf_t obuf[2];
long oseek;

void relwd(int arg2, int arg4);
void relsym(sym_t *r4);
void relseg(FILE *fp);
void relout(uint16_t tsiz, uint16_t dsiz);
void relby(int arg2, int arg4);
void putcode(int arg2, intptr_t arg4);
void drain(int arg2);
void putword(sym_t *arg2);
int putval(int arg2, sym_t *r4);
void putsp(int arg2);
void putftr();
void putbyte(intptr_t arg2);
void dolit(sym_t *arg2);
void dobin(sym_t *arg2, sym_t *arg4, int arg6);
void define(sym_t *r4, sym_t *r2);
int tolow(int arg2);
void ws_strcat(int arg2);
int scntab(code_t *group, int grplen, const char *token, int toklen);
bool nn(int arg2);
void nmerr(char *arg2, char *arg4);
bool isdig(int arg2);
bool isal(int arg2);
int gtok();
int getesc(int arg2);
int gchar();
void error(char *arg2);
void dounop(sym_t *arg2, int arg4);
void dosub(sym_t *r4, sym_t *r2);
void cherr(char *arg2, int arg4);
int bmatch(int arg2, sym_t *r4);
void bchar(int arg2);
sym_t *addsym(char *r4);
char *tname(int arg2);
void tclose(void);
char *opname(int arg2);
int main(int argc, char **argv);
sym_t **hash(char *arg2);
int gterm(sym_t *r4, char *arg4);
void gstring(sym_t *r4, char *arg4);
int gexpr(sym_t *r4, bool arg4);
int getop(int arg2);
int getal(int arg2);


int getal(int arg2) {
    char varE[8];
    int r4 = 1;
    int r2;
    sym_t *r3;

    varE[0] = arg2;

    while (isal(arg2 = gchar()) || isdig(arg2))
        if (r4 < 8)
            varE[r4++] = arg2;
    bchar(arg2);
    if (r2 = scntab(group1, 31, varE, r4))
        return r2;
    if (sval = scntab(simpleInstructions, 26, varE, r4)) {
        sval &= 0xff;
        return T_BYTE;
    }
    while (r4 < 8)
        varE[r4++] = 0;

    r3 = *hash(varE);
    while (r3 && memcmp(r3->name, varE, 8) != 0)
        r3 = r3->next;
    if (r3 == 0)
        r3 = addsym(varE);
    if ((r3->flags & TOKMASK) == T_BYTE) {
        sval = (int)r3->val;
        return T_BYTE;
    }

    tptr = r3;
    return r3->flags & TOKMASK;
}

int getop(int arg2) {
    char var9[3];
    int r2;

    var9[0] = arg2;
    var9[1] = gchar();
    var9[2] = gchar();
    int r4 = 3;
    while (r4 > 0) {
        if (r2 = scntab(group2, 26, var9, r4))
            return r2;
        r4--;
        bchar(var9[r4]);
    }

    cherr("illegal operator", gchar());
    return 0;
}

int gexpr(sym_t *r4, bool arg4) {
    sym_t var16;
    int r3;
    int r2 = gterm(r4, arg4 ? "expression" : NULL);
    if (r4->flags)
        while ((r2 & 0x140) == 0x100) {
            if (r2 == T_COMMA) { // 5E8
                gstring(r4, "x, string");
                r3 = gterm(&var16, "string");
                gstring(&var16, "string, x");
            } else {
                r3 = gterm(&var16, "term");
                dobin(r4, &var16, r2);
            }
            r2 = r3;
        }
    return r2;
}

void gstring(sym_t *r4, char *arg4) {
    switch (r4->flags) {
    case T_BYTE:
        ws_strcat((int)r4->val);
        break;
    case T_WORD:
        ws_strcat((int)r4->val);
        ws_strcat((int)r4->val >> 8);
        break;
    case T_STRING:
        break;
    default:
        nmerr("illegal", arg4);
        break;
    }
    r4->flags = T_STRING;
    return;
}


int gterm(sym_t *r4, char *arg4) {
    sym_t var18;
    short var8;
    int r2;
    sym_t *r3;
    r4->val = 0;
    memset(r4->name, 0, 8);

    while (((r2 = gtok()) == T_BYTE || r2 == T_WORD || r2 == T_ID) && (var8 = gtok()) == T_COLON)
        define(r2 == T_BYTE ? 0 : tptr, elc);

    switch (r2) {
    case T_ID: case T_WORD: case T_BYTE:
        r3 = r2 == T_BYTE ? 0 : tptr;
#pragma warning(suppress : 6001)        // if r2 is one of these values then var8 has been initialised
        if (var8 == T_DEFINE) {
            r2 = gexpr(r4, 1);
            r4->flags |= 0x200;
            define(r3, r4);
        } else if (r2 == T_BYTE) {
            r4->flags = T_BYTE;
            r4->val = sval;
            r4->fixup = 0;
            r2 = var8;
        } else {
            r3->flags |= 0x800;
            r4->flags = r3->flags & TOKMASK;
            r4->val = r3->val;
            r4->fixup = r3->fixup;
            memcpy(r4->name, r3->name, 8);
            r2 = var8;
        }
        break;
    case T_LPAREN:
        if ((r2 = gexpr(r4, 1)) != T_RPAREN)
            error("missing )");
        else
            r2 = gtok();
        break;
    default:    //94f
        if (r2 < 0) {
            r4->flags = 0;
            if (arg4)
                nmerr("missing", arg4);
            return r2;
        } else if (r2 & 0x40) {
            r4->flags = r2;
            r2 = gtok();
        } else if (r2 & 0x80) {
            var8 = gterm(r4, "operand");
            dounop(r4, r2);
            r2 = var8;
        } else {
            error("token!");
            return r2;
        }
        break;
    }
    while (r2 == T_LBRACKET) {
        if ((r2 = gexpr(&var18, 1)) != T_RBRACKET)
            error("missing ]");
        dosub(r4, &var18);
        r2 = gtok();
    }
    return r2;
}


sym_t **hash(char *arg2) {
    char *r4 = arg2;
    int r3 = 0;
    int r2 = 8;

    while (--r2 >= 0 && *r4)
        r3 += *r4++;
    return &psym[r3 & 0x3f];
}

void signalHandler(int signal) {
    exit(1);
}


int main(int argc, char **argv) {

    char defaultName[64];
    sym_t lhs;
    int var8;
    int r2;
    char *r3;
    int r4;

    getflags(&argc, &argv, "x,o*:F <files>", &xflag, &ofile);
    if (ofile == 0 && argc > 0 && argv[0][var8 = (int)strlen(*argv) - 1] == 's' && var8 < 63) { //BE0
        ofile = defaultName;
        strcpy(defaultName, *argv);
        defaultName[var8] = 'o';
    } else if (ofile == 0)
        ofile = "xeq";

    if ((ofd = fopen(ofile, "wb+")) == 0) {
        nmerr("can't create", ofile);
        exit(0);
    }

    var8 = 2;
    while (--var8 >= 0) {
        if ((tfd[var8] = fopen(tname(var8), "wb+")) == NULL) {
            error("can't create temp file");
            exit(0);
        }
    }
    atexit(tclose);
    signal(SIGINT, signalHandler);

    cobase = addsym(".text");
    dabase = addsym(".data");
    elc = addsym(".");
    elc->fixup = (intptr_t)cobase;

    while (infio = getfiles(&argc, &argv, NULL, stderr)) { //D58
        if (infio == stderr)
            nmerr("can't read", argv[-1]);
        while ((r4 = gexpr(&lhs, 0)) != -1) {
            if (r4 != -2)
                error("syntax error");
            if ((r2 = (lhs.flags & TOKMASK)) && !(lhs.flags & 0x200)) {
                if (r2 == T_BYTE)
                    putbyte(lhs.val);
                else if (r2 == T_WORD || r2 == T_ID)
                    putword(&lhs);
                else if (r2 == T_STRING) {
                    for (r3 = string + 1; string[0]; string[0]--, r3++)
                        putbyte(*r3);
                } else
                    error("illegal constant");
            }
        }
        fclose(infio);
        putftr();
        return nerrors != 0;
    }
}

char *opname(int arg2) {
    int r4 = arg2;
    code_t *r2 = group2;

    for (r2 = group2; r2->str && r2->code != r4; r2++)
        ;
    if (r2 == NULL)
        for (r2 = group1; r2->str && r2->code != r4; r2++)
            ;
    return r2 ? r2->str + 1 : "!?";
}


void tclose(void) {
    int r4 = 2;

    while (--r4 >= 0) {
        fclose(tfd[r4]);
        remove(tname(r4));
    }
}


char *tname(int arg2) {
    b6E4C[0] = arg2 + '0';
    strcpy(b6E0C, uname());
    strcat(b6E0C, b6E4C);
    return b6E0C;
}


sym_t *addsym(char *r4) {
    int r2;
    sym_t **var8 = hash(r4);
    sym_t *r3 = malloc(sizeof(sym_t));
    r3->next = *var8;
    *var8 = r3;
    r3->flags = T_ID;
    r3->val = 0;
    r3->fixup = (intptr_t)r3;
    r2 = 0;
    for (r2 = 0; r2 < 8 && (r3->name[r2] = *r4++); r2++)
        ;
    while (r2 < 8)
        r3->name[r2++] = 0;
    return r3;
}

void bchar(int arg2) {
    ungetBuf[ungetCnt++] = arg2;
    if (arg2 == '\n')
        lno--;
}

int bmatch(int arg2, sym_t *r4) {
    int r2 = r4->flags & TOKMASK;
    switch (arg2) {
    case -1:
        return nn(r2) && r4->fixup == 0 && r4->val == -1;
    case 1:
        return nn(r2) && r4->fixup == 0 && r4->val == 1;
    case 2:
        return strchr(crlist, r2) ? r2 : 0;
    case 3:
        return strchr(irlist, r2) ? r2 : 0;
    case 4:
        return true;
    case 5:
        return nn(r2);
    default:
        return arg2 == r2;
    }
}


void cherr(char *arg2, int arg4) {
    static char w6E92[2] = { 0, 0 };

    w6E92[0] = arg4;
    nmerr(arg2, w6E92);
}

// map subscript token values
void dosub(sym_t *base, sym_t *subscript) {
    static uint8_t map[2][6] = { {T_46, T_47, T_BYTE, T_WORD, T_ID, 0}, {T_SUB46, T_SUB47, T_BYTE, T_WORD, T_ID } };

    int r3 = subscript->flags & TOKMASK;
    if (subscript->fixup || nn(r3) == 0)
        error("illegal subscript");
    else {
        r3 = map[1][scanstr(map[0], base->flags & TOKMASK)];
        if (r3 == 0)
            error("illegal []");
        else {
            base->flags = (base->flags & ~TOKMASK) | r3;        // set the mapped flag
            base->val += subscript->val;                             // add the offset
        }
    }
}


void dounop(sym_t *arg2, int arg4) {
    arg2->flags &= 0xfdff;
    int r4 = (int)arg2->fixup;
    sym_t *r3;
    int r2 = arg2->flags & TOKMASK;
    switch (arg4) {
    case T_AND:
        if (r2 != T_ID && nn(r2) == 0)
            error("illegal &");
        else
            arg2->flags = (arg2->flags & ~TOKMASK) | T_WORD;
        break;
    case T_STAR:
        if (r2 == T_BYTE)
            arg2->flags = (arg2->flags & ~TOKMASK) | T_ID;
        else if (r2 < T_WORD || r2 > 0x47)
            error("illegal *");
        else
            arg2->flags |= 8;
        break;
    case T_PLUS:
        break;
    case T_MINUS:
        if (r4 == 0 && nn(r2) == 0)
            error("illegal -");
        else
            arg2->val = -arg2->val;
        break;
    case T_XOR:
        if (r4 == 0 && nn(r2) == 0)
            error("illegal !");
        else
            arg2->val = ~arg2->val;
        break;
    case T_EQUAL:
        if (r2 != T_ID && nn(r2) == 0 && arg2->flags != T_STRING)
            error("illegal =");
        else
            dolit(arg2);
        break;
    case T_PUBLIC:
        for (r3 = *hash(arg2->name); r3 && memcmp(r3->name, arg2->name, 8) != 0; r3 = r3->next)
            ;
        if (r3 == NULL)
            error("illegal public");
        else
            r3->flags |= 0x200;
        arg2->flags |= 0x200;
        break;
    default:
        dobin(arg2, arg2, arg4);
        arg2->flags |= 0x200;
        break;
    }
}

void error(char *arg2) {
    nmerr(arg2, "");
}

int gchar() {
    int r4 = ungetCnt > 0 ? ungetBuf[--ungetCnt] : getc(infio);
    if (r4 == '\n')
        lno++;
    return r4;
}

int getesc(int arg2) {
    int r2;
    int r4;
    switch (r2 = gchar()) {
    case '\n': case EOF:
        break;
    case '\\':
        if ((r2 = gchar()) == EOF || r2 == '\n')
            break;
        if ((r4 = scanstr("btnvfr", ('A' <= r2 && r2 <= 'Z') ? r2 + 0x20 : r2)) < 6)
            return r4 + 8;
        if ('0' <= r2 && r2 <= '9') {
            r4 = r2 - '0';
            if (isdig(r2 = gchar())) {
                r4 = r4 << 3 | (r2 - '0');
                if (isdig(r2 = gchar()))
                    return r4 << 3 | (r2 - '0');
            }
        }
        bchar(r2);
        return r2 & 0xff;
    default:
        return arg2 == r2 ? -1 : r2 & 0xff;
    }
    cherr("missing", arg2);
    bchar(r2);
    return -1;
}


int gtok() {
    int r3;
    int r4;
    int r2;
    while (1) {
        r3 = gchar();
        while (r3 != '\n' && r3 != EOF && (r3 <= ' ' || r3 >= 0x7f))
            r3 = gchar();
        switch (r3) {
        case EOF: return EOF;
        case '/':
            while ((r3 = gchar()) != '\n' && r3 != EOF)
                ;
            bchar(r3);
            break;
        case';': case '\n': return -2;
        case '(': return T_LPAREN;
        case ')': return T_RPAREN;
        case '[': return T_LBRACKET;
        case ']': return T_RBRACKET;
        case '"':
            while ((r3 = getesc('"')) >= 0)
                ws_strcat(r3);
            return T_STRING;
        case '\'':
            sval = 0;
            while ((r3 = getesc('\'')) >= 0)
                sval = (sval << 8) + r3;
            return T_BYTE;

        case '0':
            if (tolow(r3 = gchar()) == 'x') {
                r4 = 4;
                r3 = '0';
            } else
                r4 = 3;
            sval = 0;
            while (isdig(r3) || (r4 == 4 && (('a' <= r3 && r3 <= 'f') || 'A' <= r3 && r3 <= 'F'))) {
                sval = (sval << r4) + r3 - (r3 >= 'a' ? 'a' - 10 : r3 >= 'A' ? 'A' - 10 : '0');
                r3 = gchar();
            }
            bchar(r3);
            return T_BYTE;
        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
            sval = r3 - '0';

            while (isdig(r3 = gchar()))
                sval = sval * 10 + r3 - '0';
            bchar(r3);
            return T_BYTE;
        case ':': case '=': case '<': case '>': case '+':
        case ',': case '-': case '^': case '*': case '&':
        case '|': case '!':
            r2 = getop(r3);
            if (r2)
                return r2;
            break;
        default:
            if (isal(r3))
                return getal(r3);
            cherr("illegal character", r3);
            break;
        }
    }
}


bool isal(int arg2) {
    int r4 = arg2;
    return 'a' <= r4 && r4 <= 'z' ||
        'A' <= r4 && r4 <= 'Z' ||
        r4 == '.' || r4 == '_';
}


bool isdig(int arg2) {
    int r4 = arg2;
    return ('0' <= r4 && r4 <= '9');
}

void nmerr(char *arg2, char *arg4) {
    if (arg4)
        printf("%d: %s %s\n", lno, arg2, arg4);
    else
        printf("%d: %s\n", lno, arg2);
    nerrors++;
}

bool nn(int arg2) {
    return arg2 == T_BYTE || arg2 == T_WORD;
}

// use simple binary search to check for match
int scntab(code_t *group, int grplen, const char *token, int toklen) {
    int cmp;
    int low = 0;
    int mid;;
    char *r3;
    int r4;
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

void ws_strcat(int arg2) {
    if (string[0] < 0x7e)
        string[++string[0]] = arg2;
    else {
        error("string too long");
        string[0] = 0;
    }
}

int tolow(int arg2) {
    int r4 = arg2;
    return (r4 >= 'A' && r4 <= 'Z') ? r4 - 0x20 : r4;
}


void define(sym_t *r4, sym_t *r2) {

    char *var9;     // bug in original as var9 declare as a char
    int var8;
    sym_t *r3;

    if (r4 == 0)
        error("constant redefined");
    else {
        var9 = 0;
        r3 = (sym_t *)r2->fixup;
        var8 = r2->flags & TOKMASK;
        if (r4 == elc) {
            if (r3 != cobase && r3 != dabase)
                error("illegal .:=");
            else if (r4->fixup != (intptr_t)r3) {
                r4->val = r3->val;
                r4->fixup = (intptr_t)r3;
                putcode(3, 0);      // summy 2nd arg
            }
            putsp((int)(r2->val - r4->val));
        } else if (r3->val && r2 != elc)
            var9 = "illegal definition of";
        else if ((r4->flags & 0x400) || ((r4->flags & 0x800) && var8 != T_ID))
            var9 = "redefinition of";
        else {
            r4->flags = (r4->flags & ~TOKMASK) | var8;
            r4->val = r2->val;
            r4->fixup = r2->fixup;
        }
        if (var9) {
            memcpy(tmpbuf, r4->name, 8);
            nmerr(var9, tmpbuf);
        }
    }
}


void dobin(sym_t *arg2, sym_t *arg4, int arg6) {

    int var1E;
    sym_t *var1C;
    sym_t var1A;
    int varA;
    int var8;
    recipe_t *r4;
    char *r3;
    int r2;

    varA = arg6;
    if (varA >= T_CNZ && varA <= T_JMP) {
        var1A.flags = varA;
        if (varA != T_JMP)
            varA = T_CONDGROUP;
        arg2 = &var1A;
    }

    arg2->flags |= 0x200;
    if (varA == T_RARROW || varA == T_POP) {   // swap left & right
        var1C = arg2;
        arg2 = arg4;
        arg4 = var1C;
        varA = varA == T_RARROW ? T_EQUAL : T_PUSH;
    }

    // find the index of the bincode into the recipe array
    for (r2 = 0; bincode[r2] != 0 && bincode[r2] != varA; r2++)
        ;
    if (bincode[r2] == 0) {
        error("unknown binop!");
        return;
    }
    for (r4 = tabtab[r2]; r4->left && (bmatch(r4->left, arg2) == 0 || bmatch(r4->right, arg4) == 0); r4++) // 2486
        ;
    if (r4->left == 0) {
        nmerr("illegal operands for", opname(arg6));
        return;
    }

    for (r2 = 0, r3 = r4->recipe; *r3; r3++) {
        switch (*r3) {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            r2 = r2 * 16 + *r3 - '0';
            break;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
            r2 = r2 * 16 + *r3 - 'a' + 10;
            break;
        case 'L':
            var8 = *++r3 - '0';
            r2 += putval(r4->left, arg2) << var8;
            break;
        case 'R':
            var1E = putval(r4->right, arg4);
            r2 += (var1E <<= r3[1] ? *++r3 - '0' : 0);
            break;
        case ',':
            putbyte(r2);
            r2 = 0;
            break;
        default:
            putword(*++r3 == 'L' ? arg2 : arg4);
            r2 = -1;
            break;
        }
    }
    if (r2 >= 0)
        putbyte(r2);
}

void dolit(sym_t *r4) {
    sym_t *r3 = (sym_t *)r4->fixup;
    lit_t *r2 = littab;

    while (r2) { // 26d4
        if (r2->flags != T_STRING && r2->fixup == (intptr_t)r3 && r4->val == r2->val &&
            (r3 == 0 || memcmp(r4->name, r2->name, 8) == 0))
            break;
        r2 = r2->next;
    }
    if (r2 == NULL) {
        r2 = malloc(sizeof(lit_t));
        r2->next = littab;
        littab = r2;
        r2->flags = r4->flags;
        r2->val = (r4->flags != T_STRING) ? r4->val : (intptr_t)buybuf(string, string[0]);
        r2->fixup = (intptr_t)r3;
        memcpy(r2->name, r4->name, 8);
        r2->psym = addsym("<LIT>");
    }
    r4->flags = r4->flags & ~TOKMASK | T_ID;
    r4->val = 0;
    r4->fixup = (intptr_t)r2->psym;     // used for shared lit values
    if (r2->flags == T_STRING)
        string[0] = 0;
}

void putbyte(intptr_t arg2) {
    putcode(0, arg2);
    ((sym_t *)elc->fixup)->val = ++elc->val;
}

void putftr() {
    uint8_t var19;
    //    uint16_t var_16 = 0;
    //    uint16_t var14 = 0;
    //    uint16_t var12 = (uint16_t)cobase;

    lit_t *r4;
    char *r2;

    if (littab) {
        define(elc, cobase);

        for (r4 = littab; r4; r4 = r4->next) {
            define(r4->psym, elc);
            if (r4->flags != T_STRING)
                putword((sym_t *)r4);
            else
                for (r2 = (char *)r4->val, var19 = *r2++; var19; var19--, r2++)
                    putbyte(*r2);
        }
    }
    putcode(1, 0);      // dummy 2nd arg
    relout((uint16_t)cobase->val, (uint16_t)dabase->val);
}

void putsp(int arg2) {
    if (arg2 < 0)
        error("can't backup .");
    else
        while (arg2 > 0) {
            putbyte(0);
            arg2--;
        }
}

int putval(int arg2, sym_t *r4) {
    int r3 = r4->flags & TOKMASK;
    switch (arg2) {
    case 2:
        return scanstr(crlist, r3);
    case 3:
        return scanstr(irlist, r3);
    case 4:
#pragma warning(suppress : 6385)
        return deltas[r3 - T_CNZ];
    case T_BYTE:
        return r4->val & 0xff;
    default:
        error("putval!");
        return 0;
    }
}


void putword(sym_t *arg2) {
    if (arg2->fixup)
        putcode(2, arg2->fixup);
    putbyte(arg2->val);
    putbyte(arg2->val >> 8);
}

void drain(int arg2) {
    tmpdata_t *r4 = &tmpData[arg2];
    if (r4->size != 0 || r4->type == 2) {
        fwrite(r4, 1, r4->size + offsetof(tmpdata_t, buf), tfd[arg2]);
        r4->size = 0;
        r4->psym = 0;
    }
}

void putcode(int arg2, intptr_t arg4) {
    int r4;
    switch (arg2) {
    case 0:
        if (tmpData[curSeg].size == 0x7f)
            drain(curSeg);
        tmpData[curSeg].buf[tmpData[curSeg].size++] = (uint8_t)arg4;
        break;
    case 1:
        r4 = 2;
        while (--r4 >= 0) {
            drain(r4);
            tmpData[r4].type = 2;
            drain(r4);
            putc(-1, tfd[r4]);
        }
        break;
    case 2:
        drain(curSeg);
        tmpData[curSeg].psym = arg4;
        break;
    case 3:
        curSeg = !curSeg;
        break;
    default:
        error("putcode!");
        break;
    }
}

void relby(int arg2, int arg4) {
    iobuf_t *r2 = &obuf[arg2 & 1];
    int r4;
    if (r2->size >= 0x200 || arg2 > 1) {
        if (r2->fpos != oseek && fseek(ofd, r2->fpos, 0) != 0 ||
            (r4 = r2->size - r2->off) && fwrite(r2->buf + r2->off, 1, r4, ofd) != r4) {
               error("can't write object file");
            exit(1);
        } else {
            r2->fpos += r4;
            r2->off = 0;
            r2->size = 0;
            oseek = r2->fpos;
        }
    }
    if (arg2 <= 1)
        r2->buf[r2->size++] = arg4;
}

void relout(uint16_t tsiz, uint16_t dsiz) {
    uint16_t u;

    cobase->val = 0;
    dabase->val = 0;
    int size = 0;
    u = 3;
    int k = 64;
    sym_t *p;

    while (--k >= 0) {
        p = psym[k];
        while (p) {
            if (p->fixup != 0) {
                if (p->fixup == (intptr_t)cobase)
                    p->fixup = 1;
                else if (p->fixup == (intptr_t)dabase) {
                    p->val += tsiz;
                    p->fixup = 2;
                } else {
                    p->flags |= 0x200;
                    p->fixup = ++u;
                }
            }
            if (p->flags & 0x200)
                size++;
            p = p->next;
        }
    }

    if (size >= 170)
        error("too many symbols");
    obuf[1].fpos = tsiz + 16L + dsiz;
    obuf[1].off = obuf[1].fpos & TOKMASK;
    obuf[1].size = obuf[1].fpos;
    relwd(0, 0x1499);  // magic number
    relwd(0, size * 12); // symtab size
    relwd(0, tsiz);    // tsiz
    relwd(0, dsiz);    // dsiz
    relwd(0, 0);
    relwd(0, 0);
    relwd(0, 0);
    relwd(0, tsiz);    // dbias
    k = 64;

    while (--k >= 0) {
        p = psym[k];

        while (p && size > 0) {
            if (p->fixup >= 4) {
                relsym(p);
                size--;
            }
            p = p->next;
        }
    }
    k = 64;
    while (--k >= 0) {
        p = psym[k];
        while (p && size > 0) {
            if ((p->flags & 0x200) && p->fixup < 4) {
                relsym(p);
                size--;
            }
            p = p->next;
        }
    }

    relseg(tfd[0]);
    relseg(tfd[1]);
    relwd(2, 0);
    relwd(3, 0);
}

void relseg(FILE *fp) {
    int nabs;
    int size;
    int i;
    int b;

    rewind(fp);

    nabs = 0;

    while (1) {
        fread(tmpData, 1, offsetof(tmpdata_t, buf), fp);
        if (tmpData[0].type == 2) {
            relby(1, 0);
            return;
        }
        size = tmpData[0].size;
        fread(tmpData[0].buf, 1, size, fp);
        if (tmpData[0].psym == 0) {
            nabs += size;
            i = 0;
            while (i < size)
                relby(0, tmpData[0].buf[i++]);
        } else {
            i = (int)((sym_t *)tmpData[0].psym)->val + (tmpData[0].buf[0] & 0xff) + (tmpData[0].buf[1] << 8);
            b = (int)((sym_t *)tmpData[0].psym)->fixup;
            tmpData[0].buf[0] = i;
            tmpData[0].buf[1] = i >> 8;
            i = 0;
            while (i < size)
                relby(0, tmpData[0].buf[i++]);
            if (b == 0)
                nabs += size;
            else {
                while (nabs >= 0x201f) {
                    relby(1, 0x3f);
                    relby(1, 0xff);
                    nabs -= 0x201f;
                }
                if (nabs >= 32) {
                    nabs -= 32;
                    relby(1, (nabs >> 8) | 0x20);
                    relby(1, nabs);
                } else if (nabs)
                    relby(1, nabs);

                nabs = size - 2;
                if (b < 0x2f)
                    relby(1, (b + 0x10) << 2);
                else if (b < 0xaf) {
                    relby(1, 0xfc);
                    relby(1, b - 0x2f);
                } else {
                    b -= 0xaf;
                    relby(1, 0xfc);
                    relby(1, (b >> 8) | 0x80);
                    relby(1, b);
                }
            }
        }
    }
}


void relsym(sym_t *r4) {
    int r2;
    relwd(1, (int)r4->val);
    r2 = r4->fixup < 4 ? (int)r4->fixup | 4 : 0;
    if (r4->flags & 0x200)
        r2 |= 8;
    relby(1, r2);
    r2 = 0;

    while (r2 < 8)
        relby(1, r4->name[r2++]);

    while (r2 < 9) {
        relby(1, 0);
        r2++;
    }
}


void relwd(int arg2, int arg4) {
    relby(arg2, arg4);
    relby(arg2, arg4 >> 8);
}
// tread replaced by fread