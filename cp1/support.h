#pragma once
// used for getflags processing multiple values
// this version allows for int & char * being of different size
#include <stdint.h>

typedef struct {
	int ntop;
	union {
		int i;
		char *s;
	} val[1];
} many_t;

extern FIO wsstdout;

typedef uint64_t wsDouble;


void *alloc(unsigned nbytes, void *link);
char *getflags(int *pac, char ***pav, const char *fmt, ...);
//int16_t lstoi(uint8_t *s);
//uint8_t *itols(uint8_t *s, uint16_t val);
FILE *getfiles(int *pac, char ***pav, FILE *dfd, FILE *efd);
//int prefix(char *s1, char *s2);
//int mkexec();
char *buybuf(char *s, unsigned n);
unsigned scanstr(const uint8_t *s, uint8_t c);
int cmpbuf(char *s1, char *s2, unsigned n);
unsigned cpybuf(char *s1, char *s2, unsigned n);
unsigned btos(char *s, unsigned n, short *pinum, short base);
unsigned btol(char *s, unsigned n, long *plnum, short base);

short usage(const char *msg);