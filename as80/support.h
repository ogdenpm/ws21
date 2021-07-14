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


char *getflags(int *pac, char ***pav, const char *fmt, ...);
int16_t lstoi(uint8_t *s);
uint8_t *itols(uint8_t *s, uint16_t val);
char *uname();
short usage(const char *msg);
FILE *getfiles(int *pac, char ***pav, FILE *dfd, FILE *efd);
int prefix(char *s1, char *s2);
int mkexec();
char *buybuf(char *s, int size);
int scanstr(char *s, int c);