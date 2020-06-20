#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

/*
 based on decompiled source of whitesmith's getflags function
 key changes
 1) changed to use stdarg as the original code would fail for modern processors for the following reasons
    a) size of char * and int are not guaranteed to be the same
	b) paarameter passing registers is likely to break how the arguments were indexed as they were treated as an array of pointers
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
	int ntop;
	int val[1];
} ilist1_t;

typedef struct {
	int ntop;
	long val[1];
} llist1_t;

typedef struct {
	int ntop;
	char *val[1];
} slist1_t;


static int wputch(FILE *fp, const char *arg2, int arg3);

static void expandFlags(const char *diagnostic, const char *fmt)
{
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
	exit(0);
}

enum { String, Integer, Long};		// basic data type

char *getflags(int *pac, char ***pav, const char *fmt, ...)
{
	va_list args = NULL;

	int match;
	int atEnd;
	int base;
	int numOffset;
	long nval = 0;
	char *sval = NULL;
	char *s;			// useful pointer

	va_start(args, fmt);
	for (++*pav; *pac && --*pac; ++*pav) {

		char *r2 = **pav;
		if (strcmp(r2, "--") == 0) {
			--*pac;
			++*pav;
			break;
		}
		if (strcmp(r2, "-") == 0 || (*r2 != '-' && *r2 != '+'))
			break;
		if (*r2 == '-')
			++r2;
		while (*r2) {
			const char *r4 = fmt;
			atEnd = match = 0;
			TEXT *r3 = r2;

			int type;
			for (; atEnd == 0; r4++) {
				switch (*r4) {
				case '*':
					type = String;
					match = 1;
					sval = *r3 != 0 ? r3 : --*pac != 0 ? *++* pav : r3;
					r3 = strchr(sval, '\0');
					break;
				case '?':				// boolean is numeric
					type = Integer;
					match = 1;
					nval = *r3;
					if (*r3)
						r3++;
					break;
				case '#':
					if (*r3 == 0)			// next arg if nothing left on this one
						r3 = (-- * pac) ? *++ * pav : r3;

					// original sets numOffset to skip past any +/- but always checks on first char
					// this means that for [+-]num, num is always treated as decimal
					// this has some logic to it, altough it would be simple to use strtol sign and auto base handling
					match = (numOffset = *r3 == '-' || *r3 == '+') || (isdigit(*r3));
					if (*r3 != '0')
						base = 10;
					else if (tolower(r3[1]) == 'x')
						base = 16;
					else
						base = 8;

					nval = strtol(r3, &s, base);
					if (s <= r3 + numOffset)		// failed to parse a number (might have had sign only)
						match = 0;
					if (tolower(*s) == 'l')			// btol absorbs training l/L
						s++;
					if (match == 0 || *(r3 = s)) {
						if (s = strchr(r4, ':'))
							expandFlags(s + 1, fmt);		// user diagnostic exit
						else
							return r2;					// return rest of arg
					}
					if (r4[1] == '#') {					// ## now available for list
						type = Long;
						r4++;
					} else
						type = Integer;
					break;

				case 0:
				case ':':
					if (!match) {
						if (*r4 == 0)					// return problem flag
							return r2;
   						expandFlags(++r4, fmt);			// diagnostic message
					}									// fall through

				case ',':
#pragma warning(disable : 6269)		// disable warning due to va_arg used to skip stack parameter
					if (match) {
						switch (type) {
						case String: *va_arg(args, char **) = sval; break;
						case Integer: *va_arg(args, int *) = nval; break;
						case Long: *va_arg(args, long *) = nval; break;
						}
						r2 = r3;
						atEnd = 1;
					}  else {
						switch (type) {
						case String: va_arg(args, char **); break;
						case Integer: va_arg(args, int *); break;
						case Long: va_arg(args, long *); break;
						}
					}
					r3 = r2;
					break;

				case '>':
					if (match) {
						slist1_t *pl = va_arg(args, slist1_t *);			// will fail it pointers to slist1_t, ilist1_t and llist_t are not same length
						if (pl->ntop != 0) {								// if space then save arg
							switch (type) {
							case String: pl->val[--pl->ntop] = sval; break;
							case Integer: ((ilist1_t *)pl)->val[--pl->ntop] = nval; break;
							case Long: ((llist1_t *)pl)->val[--pl->ntop] = nval; break;
							}
							r2 = r3;
							atEnd = 1;
						} else if (s = strchr(r4, ':')) {	// user diagnostic
							expandFlags(s + 1, fmt);
						} else
							return r2;									// return where we ended
					} else {
						va_arg(args, slist1_t *);			// skip the arg
#pragma warning(default : 6269)
						r3 = r2;
					}
					break;
				default:
					type = Integer;
					if (*r4 == *r3) {									// char match (set true flag)
						match = 1;
						nval = 1;
						r3++;
					} else {
						match = 0;										// skip to next option

						while (*r4 != ',' && *r4 != '>' && *r4 != ':' && *r4 != '\0') {
							if (*r4 == '*')
								type = String;
							else if (*r4 == '#' && r4[1] == '#') {
								r4++;
								type = Long;
							}
							++r4;
						}
						--r4;											// backup so for loop picks up the break char 
					}
					break;
				}
			}
			va_end(args);			// reset args
			va_start(args, fmt);
		}

	}
	va_end(args);
	return 0;
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
