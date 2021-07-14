#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>

/*
 based on decompiled source of whitesmith's getflags function
 key changes
 1) changed to use stdarg as the original code would fail for modern processors for the following reasons
    a) size of char * and int are not guaranteed to be the same
	b) parameter passing registers is likely to break how the arguments were indexed as they were treated as an array of pointers
	c) related to above, although most pointers are likely to be same size this is not guaranteed
 2) the structure to return a list of values was modified to reflect that sizeof char * and int are not guaranteeed to be the same
 3) I/O was done using write to STDERR, modified to use putc and fputs to stderr
 4) support functions btol, replaced with strtol
 5) lenstr, replaced with strlen
 6) cmpstr replaced with strcmp(..) == 0
 7) modified code to use strchr rather than scnstr
 8) small modification to use strchr to find end of string
 7) register varaibles replaced with normal variables, letting the compiler optimise
*/
#include "std.h"
#include "support.h"

// bacause sizeof(int) might not equal sizeof(char *)
// this version requires explicit types for list
// also added support for long as now easy to add

typedef struct {
	BYTES ntop;
	union {
		TEXT *asval[1];
		BYTES aival[1];
		LONG alval[1];
	};
} multiArg_t;

typedef union {
	TEXT *sval;
	BYTES ival;
	LONG  lval;
	multiArg_t marg;
} argType_t;

enum argtype { None, String, Integer, Long };		// the different arg types

static int wputch(FILE *fp, const char *s, int col);

static void expandFlags(const char *diagnostic, const char *fmt) {
	COUNT col = usage(0);

	for (; *diagnostic; diagnostic++) {
		if (*diagnostic != 'F')
			col = wputch(stderr, diagnostic, col);
		else {
			fputs("-[", stderr);
			col += 2;
			for (; *fmt != ':'; fmt++) {
				if (fmt[0] == ',' && fmt[1] != ':')
					col = wputch(stderr, " ", col);
				else if (*fmt == '>') {
					putc('^', stderr);
					col++;
					if (fmt[1] != ':')
						col = wputch(stderr, " ", col);
				} else {
					putc(*fmt, stderr);
					col++;
				}
			}
			putc(']', stderr);
			col++;
		}
	}
	putc('\n', stderr);
	exit(1);
}
/*	collect flags from command argument list
 */
char *getflags(int *pac, char ***pav, const char *fmt, ...) {
	bool atEnd;
	char *s;		// useful pointer
	argType_t val, *p;

	va_list args;

	for (++ *pav; *pac && -- * pac; ++ * pav) {
		char *r2 = **pav;

		if (!strcmp(r2, "--")) {
			--*pac;
			++*pav;
			break;
		} else if (!strcmp(r2, "-") || (*r2 != '-' && *r2 != '+'))
			break;
		if (*r2 == '-')
			++r2;
		while (*r2) {
			const char *r4 = fmt;
			char *r3 = r2;
			enum argtype type = None;
			va_start(args, fmt);
			for (atEnd = false, p = va_arg(args, argType_t *); !atEnd; ++r4) {
				switch (*r4) {
				case '*':
					type = String;
					val.sval = *r3 ? r3 : --*pac ? *++*pav : r3;
					r3 = strchr(val.sval, '\0');
					break;
				case '?':
					type = Integer;		// boolean is numeric
					val.ival = *r3;
					if (*r3)
						++r3;
					break;
				case '#':
					if (!*r3)			// next arg if nothing left on this one
						r3 = --*pac ? *++*pav : r3;
					s = r3;
					val.lval = strtoul(r3, &r3, 0);		// let strtoul handle all of the conversion
					if (s == r3 || *r3)
						if (s = strchr(r4, ':'))
							expandFlags(s + 1, fmt);
						else
							return (r2);
					if (r4[1] == '#') {
						r4++;
						type = Long;
					} else {
						type = Integer;
						val.ival = (int)val.lval;		// convert to int
					}
					break;
				case '\0':
				case ':':
					if (type == None) {
						if (!*r4)
							return r2;
						expandFlags(++r4, fmt);
					}
				case ',':
					if (type != None) {
						switch (type) {		// save the correct format
						case String:    p->sval = val.sval; break;
						case Integer:   p->ival = val.ival; break;
						case Long:      p->lval = val.lval; break;
						}
						r2 = r3;
						atEnd = true;
					} else {
						r3 = r2;
						p = va_arg(args, argType_t *);
					}
					break;
				case '>':
					if (type != None) {
						if (p->marg.ntop > 0) {
							p->marg.ntop--;
							switch (type) {	// save the correct list arg format
							case String:    p->marg.asval[p->marg.ntop] = val.sval; break;
							case Integer:   p->marg.aival[p->marg.ntop] = val.ival; break;
							case Long:      p->marg.alval[p->marg.ntop] = val.lval; break;
							}
							r2 = r3;
							atEnd = YES;
						} else if (s = strchr(r4, ':'))
							expandFlags(s + 1, fmt);
						else
							return (r2);
					} else {
						r3 = r2;
						p = va_arg(args, argType_t *);
					}
					break;
				default:
					if (*r4 == *r3) {
						type = Integer;
						val.ival = YES;
						++r3;
					} else {
						type = None;
						while (*r4 != ',' && *r4 != '>' && *r4 != ':' && *r4)
							++r4;
						--r4;
					}
					break;
				}


			}
			va_end(args);
		}
	}
	return (NULL);
}



static int wputch(FILE *fp, const char *s, int col)
{
	/* break on space of tab after column 60 */
	if (col <= 60 || (*s != ' ' && *s != '\t')) {
		putc(*s, fp);
		return col + 1;
	} else {
		fputs("\n\t", fp);
		return 4;
	}
}
