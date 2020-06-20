#define _CRT_SECURE_NO_WARNINGS
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "std.h"


extern char *_pname;

void error(char *s1, char *s2) {
    fprintf(stderr, "%s: %s%s\n", _pname, s1 ? s1 : "", s2 ? s2 : "");
    exit(0);
}

int16_t lstoi(uint8_t *s) {
    return s[0] + s[1] * 256;
}

uint8_t *itols(uint8_t *s, uint16_t val) {
    s[0] = val % 256;
    s[1] = val / 256;
    return s;
}

short usage(const char *msg) {
    short cnt = printf("usage: %s %s", _pname, msg ? msg : "");
    if (msg && *msg && strchr(msg, '\0')[-1] == '\n')
        exit(0);
    return cnt;
}

char *uname() {
    return "ctempc.";
}


FILE *getbfiles(int *pac, char ***pav, FILE *dfd, FILE *efd, int rsize) {
    FILE *r3;
    if (*pac < 0)
        r3 = NULL;
    else if (*pac == 0 || strcmp(**pav, "-") == 0)
        r3 = dfd;
    else if ((r3 = fopen(**pav, rsize ? "rb" : "rt")) == NULL)
        r3 = efd;
    ++*pav;
    if (--*pac <= 0)
        *pac = -1;
    return r3;
}

int prefix(char *s1, char *s2) {
    return 1;
}

int mkexec() {
    return 1;
}