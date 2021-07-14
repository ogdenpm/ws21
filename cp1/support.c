#include "cp1.h"

extern char *_pname;



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

FILE *getfiles(int *pac, char ***pav, FILE *dfd, FILE *efd) {
    FILE *r3;
    if (*pac < 0)
        r3 = NULL;
    else if (*pac == 0 || strcmp(**pav, "-") == 0)
        r3 = dfd;
    else if ((r3 = fopen(**pav, "rt")) == NULL)
        r3 = efd;
    ++ *pav;
    if (-- * pac <= 0)
        *pac = -1;
    return r3;
}

int prefix(char *s1, char *s2) {
    return 1;
}

int mkexec() {
    return 1;
}

unsigned scnstr(const uint8_t *s, uint16_t c) {
    const uint8_t *t;
    for (t = s; *t && *t != c; t++)
        ;
    return (unsigned)(t - s);
}

unsigned inbuf(char *p, unsigned n, char *s) {
    for (unsigned i = 0; i < n; i++) {
        if (p[i] == *s)
            return i;
        if (*s) {
            for (char *t = s + 1; *t; t++)
                if (p[i] == *t)
                    return i;
        }
    }
    return n;
}


char *buybuf(char *s, unsigned len) {
    char *buf = malloc(len);
    memcpy(buf, s, len);
    return buf;
}

typedef struct _link {
    struct _link *next;
} link_t;

void *alloc(unsigned nbytes, void *link) {
    void *p = malloc(nbytes < sizeof(void *) ? sizeof(void *) : nbytes);
    if (!p) {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    ((link_t *)p)->next = link;
    return p;
}

void *wsfree(void *pcell, void *link) {
    free(pcell);
    return link;
}

void *frelst(void *plist, void *pstop) {
    while (plist && plist != pstop)
        plist = wsfree(plist, ((link_t *)plist)->next);
    return plist;
}

int cmpbuf(char *s1, char *s2, unsigned n) {
    return memcmp(s1, s2, n) == 0;
}

unsigned cpybuf(char *s1, char *s2, unsigned n) {
    memcpy(s1, s2, n);
    return n;
}



unsigned btol(char *s, unsigned n, long *plnum, short base) {
    char *start = s;
    bool minus = false;
    int digit;

    long  val = 0L;

    while (n != 0 && iswhite(*s)) {
        n--;
        s++;
    }

    if (n != 0) {
        if (*s == '-') {
            minus = true;
            s++;
            n--;
        } else if (*s == '+') {
            s++;
            n--;
        }
    }

    if (base == 16 && n >= 2 && *s == '0' && tolower(s[1]) == 'x') {
        s += 2;
        n -= 2;
    }

    while (n != 0) {
        if (isdigit(*s))
            digit = *s - '0';           // ws does not check digits for >= base. simple to fix
        else if (isalpha(*s)) {
            digit = tolower(*s) - 'a' + 10;
            if (digit >= base)
                break;
        } else
            break;
        val = val * base + digit;
        n--;
        s++;
    }
    *plnum = minus ? -val : val;
    if (n != 0 && tolower(*s) == 'l')
        s++;
    return s - start;
}

unsigned btos(char *s, unsigned n, short *pinum, short base) {
    long num;
    unsigned cnt = btol(s, n, &num, base);
    *pinum = (short)num;
    return cnt;
}


#ifdef NATIVE

double dtens[] = { 1.0, 1.0E1, 1.0E2, 1.0E4, 1.0E8, 1.0E16, 1.0E32 };
short ntens = 7;


double dtento(double dnum, short exp) {
    short r2;
    short r3;
    if (exp < 0) { // 69B1
        r2 = -exp;
        r3 = 1;
        while (r2 && r3 < ntens) { // 6A31
            if (r2 & 1)
                dnum /= dtens[r3];
            r2 >>= 1;
            r3++;
        }
        while (r2) { // 6A9b
            dnum /= dtens[ntens - 1];
            dnum /= dtens[ntens - 1];
            r2--;
        }
    } else if (exp > 0) { // 6A9B
        r2 = exp;
        r3 = 1;
        while (r2 && r3 < ntens) { // 6a1c
            if (r2 & 1)
                dnum *= dtens[r3];
            r2 >>= 1;
            r3++;
        }
        while (r2) {
            dnum *= dtens[ntens - 1];
            dnum *= dtens[ntens - 1];
            r2--;
        }
    }
    return dnum;
}

#else
/*
    ws floating point format
    s eeeeeeee mm... (55 bits)
    s = sign bit 0 -> +ve, 1 -> -ve     // not used here as cpp only emits +ve doubles
    e = exponent + 127 - matissa is multiplied by 2 ^ (e - 127)
    m.. = matissa, the there is an additional implied 56th high bit of 1, execept for case of exponent = 0 in which case 0
    the matissa is in range [1/2-1)

    Note this function is not an exact match the whitesmith's 8080 code and may deviate in the least signficant bits in some cases
    most likely due to different internal rounding strategies for intermediate values. The biggest difference I have seen is +/- 2
    to the least significant byte of the matissa;
    Some examples of differences
    1) 1.0E-1  this version rounds the least siginficant bit the Whitesmith's code doesn't
    2) 1.0E32  this version correctly rounds the least significant bit, the Whitesmiths' code has an incorrect rounding as verified
       by calculating to 128bit precision. This invalid value is used in dtento, for scaling to a power of 10.
    3) On numbers with a large number of digits, running whitesmiths' code in an emulation environment crashes, this doesn't
    4) Due to the crashing I was unable to verify what the whitesmiths' code does with very large / small numbers.
       This code sets to 0 for undeflow and to max double for overflow

    Note I could not use the IEEE double format since it trades precision for exponent size.

*/
uint64_t mkWsDouble(uint64_t matissa, short exp) {
    if (matissa == 0)       // zero result quick return 
        return 0;
    while (exp > 0 && matissa < UINT64_MAX / 10) {  // maximise bit usage for +ve exponents
        exp--;
        matissa *= 10;
    }


    short wExp = 184;					// this would be marginally faster if the non portable bit functions were used
    // normalise to 64 bits
    if (matissa < (1ull << 32)) {
        wExp -= 32;
        matissa <<= 32;
    }
    if (matissa < (1ull << 48)) {
        wExp -= 16;
        matissa <<= 16;
    }
    if (matissa < (1ull << 56)) {
        wExp -= 8;
        matissa <<= 8;
    }

    while (matissa < (1ull << 63)) {      // finish normailsation to 64bits
        wExp--;
        matissa <<= 1;
    }

    if (exp < 0) {
        while (exp < 0) {
            while (matissa < (1ull << 63)) {	// scale matissa back to max possible
                wExp--;
                matissa <<= 1;
            }
            if (++exp == 0)					// minor optimisation use / 100 if exp allows
                matissa /= 10;
            else {
                matissa /= 100;
                exp++;
            }

        }
    } else {
        while (exp > 0) {						// adjust matissa so we can * 10
            while (matissa > UINT64_MAX / 10) {
                wExp++;
                matissa >>= 1;
            }
            matissa *= 10;						// do the x10
            exp--;
        }
    }
#ifdef ROUND
    while (matissa >= 0x200000000000000ull) {
        wExp++;
        matissa >>= 1;
    }
    matissa++;  // round
#endif
    while (matissa >= 0x100000000000000ull) {	// scale the matissa to allow for the exponent
        wExp++;
        matissa >>= 1;
    }
    //    printf("%d %x - %llu %llx\n", wExp, wExp, matissa, matissa);
    if (wExp <= 0)
        return 0;
    if (wExp > 255)
        return ~(1ull << 63);
    return (matissa & ~(1ull << 63)) | ((uint64_t)wExp << 55);    // merge in the exponent and, removing matissa high bit
}

#endif
