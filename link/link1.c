#include "link.h"

int longint;
int lsfmt;
int ok;
int lenname = 15;
long maxbnd;
uint16_t binhdr;
int nlibs;
long liboff[128];

obhdr_t obhdr;

long bsiz, dsiz, tsiz;
uint16_t nsyms;
uint16_t nund;

symbol_t *stabs[32];        // 32 pointers
int afl, cfl, dfl, hfl, rfl, tfl, dround;
int xfl = 2;
list_t llist = { 10 };

long drmask;
char *endbss, *endtext, *enddata;
char *ofile = "xeq";
list_t ulist = { 10 };
long bpad = 0;
long tbias = 0;
long dbias = -1;
char *_pname = "link";
FILE *ifd;
FILE *ofd;
FILE *tfd;

iobuf_t ibuf[2];
iobuf_t obuf[4];
long iseek;

void addend(char *arg_2, int arg_4, long arg_6) {
    uint8_t var_16[16];
    int r4;
    char *r3;
    symbol_t *r2;

    if (arg_2) {
        r4 = 0;
        r3 = arg_2;
        while (r4 < 16 && *r3)
            var_16[r4++] = *r3++;
        while (r4 < 16)
            var_16[r4++] = 0;

        if (r2 = lookup(var_16))
            if (r2->flag & 4)
                remark("redefined end symbol: ", arg_2);
            else {
                r2->flag = arg_4;
                r2->val = arg_6;
            }
    }
}

void addlib(long arg_2) {

    if (nlibs >= 127)
        error("library table full", 0);

    liboff[nlibs++] = arg_2;
}


symbol_t *addsym(char *arg_2, int arg_4, long arg_6) {
    int r2, r4;
    symbol_t *r3;

    if (nsyms >= 0x800)
        error("symbol table full", 0);
    r4 = nsyms >> 6;
    r2 = nsyms++ & 0x3f;
    if (r2 == 0)
        stabs[r4] = malloc(64 * sizeof(symbol_t));

    r3 = &stabs[r4][r2];

    memcpy(r3->name, arg_2, lenname);
    r3->flag = arg_4;
    r3->val = arg_6;
    return r3;
}

void addusym(char *arg_2) {
    uint8_t var_16[16];
    char *r2;

    if (arg_2) {
        int r4 = 0;
        r2 = arg_2;
        while (r4 < 16 && *r2)
            var_16[r4++] = *r2++;

        while (r4 < 16)
            var_16[r4++] = 0;

        addsym(var_16, 0, 0L);
    }
}



// io is managed using 64 byte buffers
// all this to handle sequential reads (to check if off/siz are updated externally
// potentially could be replaced by an appropriate getc and let standard libraries handle
uint8_t getby(int arg_2) {
    long var_A;
    iobuf_t *r4;

    r4 = &ibuf[arg_2];
    if (r4->off >= r4->size) {      // next next buffer
        var_A = r4->fpos & ~0x3f;   // align start to 64 byte boundary
        if (iseek != var_A) {       // if not already there seek to right location
            fseek(ifd, var_A, 0);
            iseek = var_A;
        }
                                    // read in the buffer
        if ((r4->size = (int16_t)fread(r4->buf, 1, 64, ifd)) == 0 && ferror(ifd))
            error("read error in pass2", 0);
        iseek += r4->size;          // update for next read
        r4->fpos += r4->size;       
        r4->off -= 64;              // adjust offset
        if (r4->off > r4->size)    // not enough data
            error("early EOF in pass2", 0);       // missing arg added
    }
    return r4->buf[r4->off++];
}


FILE *gtlfile(int *pac, char ***pav, char **name) {
    FILE *fp;
    static int cnt;
    static char fname[64];

    if (pac == 0) {
        cnt = 10;
        return 0;
    }
    if (fp = getbfiles(pac, pav, stdin, stderr, 1)) {
        *name = pav[0][-1];
        if (fp != stderr || !prefix(*name, "-l"))
            return fp;
        name++;
    } else if (cnt <= llist.ntop)
        return NULL;
    else
        *name = llist.val[--cnt];

    strcpy(fname, "/lib/lib");
    if (strlen(*name) < 56)
        strcpy(fname + 8, *name);
    *name = fname;
    return (fp = fopen(fname, "rb")) ? fp : stderr;
}

static uint8_t bounds[5] = { 0, 1, 3, 7, 0 };

uint16_t gtmagic(FILE *fp) {
    uint8_t bmagic[2];
    uint16_t magic;

    if (fread(bmagic, 1, 2, fp) != 2)
        error("can't read file header", 0);

    magic = lstoi(bmagic);
    if (bmagic[0] == 0x99)
        if (bmagic[1] & 0x80)
            error("no relocation bits", 0);
        else if (binhdr == 0) {
            lenname = (bmagic[1] & 7) * 2 + 1;
            longint = bmagic[1] & 8;
            lsfmt = bmagic[1] & 0x10;
            maxbnd = bounds[(bmagic[1] & 0x60) >> 5];
            binhdr = magic;
        }
    return magic;
}



char *gtsyms(FILE *fp, obhdr_t *arg_4) {
    uint8_t hdr[26];      // capture header
    obhdr_t *r4 = arg_4;
    char *r2;

    int hdrSize = longint ? 0x1a : 0xe;
    if (fread(hdr, 1, hdrSize, fp) != hdrSize)
        error("cant read binary header", 0);

    r4->symSiz = xstos(hdr);
    if (longint) {
        r4->tsiz = xstol(hdr + 2);
        r4->dsiz = xstol(hdr + 6);
        r4->bsiz = xstol(hdr + 10);
        r4->tbias = xstol(hdr + 18);
        r4->dbias = xstol(hdr + 22);
    } else {
        r4->tsiz = xstos(hdr + 2);
        r4->dsiz = xstos(hdr + 4);
        r4->bsiz = xstos(hdr + 6);
        r4->tbias = xstos(hdr + 10);
        r4->dbias = xstos(hdr + 12);
    }
    fseek(fp, r4->tsiz + r4->dsiz, 1);
    r2 = malloc(r4->symSiz);
    if (fread(r2, 1, r4->symSiz, fp) != r4->symSiz)
        error("can't read symbol table", 0);
    return r2;
}

// vc++ errors complaining r2 by be uninitialised, however when r4 == 0 then it will be hence the warning is a false positive
#pragma warning(disable : 4703)
symbol_t *lookup(char *arg_2) {
    int r4;
    symbol_t *r2;

    for (r4 = 0; r4 < nsyms; r4++, r2++) {
        if ((r4 & 0x3f) == 0)
            r2 = stabs[r4 >> 6];
        if (memcmp(arg_2, r2->name, lenname) == 0)
            return r2;
    }
    return 0;
}
#pragma warning(default : 4703)

uint32_t rebias(int flags, long tbias, long dbias, long bbias) {
    switch (flags & 3) {
    default: return 0;
    case 1: return tsiz - tbias;
    case 2: return dsiz - dbias;
    case 3: return bsiz - bbias;
    }
}

void relby(int arg_2, int arg_4) {
    register int r4 = arg_2;
    int var_8;
    iobuf_t *r3;
    static oseek[2];
    int r2;
//    printf("relby(%d %02X)\n", arg_2, arg_4);
    r3 = &obuf[r4 & 3];
    var_8 = r4 & 4;
    if ((afl || rfl) && (r4 & 0xa) == 2)
        return;

    r4 &= 3;
    if (r3->size >= 64 || var_8) { // 2E1D
        if (oseek[r4 == 3] != r3->fpos && fseek(r4 == 3 ? tfd : ofd, r3->fpos, 0) != 0)
            goto fail;
        if ((r2 = r3->size - r3->off) && fwrite(&r3->buf[r3->off], 1, r2, r4 == 3 ? tfd : ofd) != r2) //2DB0
            goto fail;
        r3->fpos += r2;
        r3->off = 0;
        r3->size = 0;
        oseek[r4 == 3] = r3->fpos;
    }
    if (var_8 == 0)
        r3->buf[r3->size++] = arg_4;
    return;
fail:
    error("can't write ", r4 == 3 ? "temp file" : "output file");
}



void relint(int arg_2, uint32_t arg_4) {
    uint8_t var_A[4];
    int r2;
    int r4;

    if (longint == 0)
        relwd(arg_2, r2 = arg_4);
    else if (lsfmt) {
        r4 = 4;
        while (--r4 > 0) {
            relby(arg_2, r2 = arg_4);
            arg_4 >>= 8;
        }
    } else {
        r4 = 4;
        while (--r4 > 0) {
            var_A[r4] = arg_4;
            arg_4 >>= 8;
        }
        for (r4 = 0; r4 < 4; r4++)
            relby(arg_2, var_A[r4]);
    }
}





void relsym(symbol_t *arg_2) {
    symbol_t *r4 = arg_2;
    int r2;
    relint(10, r4->val);
    relby(10, afl || cfl ? 12 : r4->flag);
    for (r2 = 0; r2 < lenname; r2++)
        relby(10, r4->name[r2]);
}


void relwd(int arg_2, int arg_4) {
    if (lsfmt) {
        relby(arg_2, arg_4);
        relby(arg_2, arg_4 >> 8);
    } else {
        relby(arg_2, arg_4 >> 8);
        relby(arg_2, arg_4);
    }
}


void remark(char *arg_2, char *arg_4) {
    printf("%s%.*s\n", arg_2, lenname, arg_4);
    ok = 0;
}

uint32_t xstol(uint8_t *arg_2) {
    register uint8_t *r4 = arg_2;
    register int r2;
    uint32_t var_A = 0;

    if (lsfmt) {
        r2 = 4;
        while (--r2 > 0)
            var_A = (var_A << 8) | r4[r2];
    } else
        for (r2 = 0; r2 < 4; r2++)
            var_A = (var_A << 8) | r4[r2];
    return var_A;
}


int xstos(uint8_t *arg_2) {
    register uint8_t *r4 = arg_2;
    int16_t r2;
    if (lsfmt)
        r2 = (r4[1] << 8) | r4[0];
    else
        r2 = (r4[0] << 8) | r4[1];
    return r2;
}

