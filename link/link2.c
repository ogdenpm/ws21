#include "link.h"

int nxtexit;


long docode(int segId, int relId, int symCnt, symbol_t **symtab, obhdr_t *segHdr, long loadBias, long segBytesLeft, long outstandingSkip) {
    int var_8;
    long cnt;
    long totalSkipCnt = 0;
    long bias;
    uint8_t var_18[4];
    int r2;
    symbol_t *r3 = NULL;        // avoid false positive 4703
    uint16_t r4;

    while (r2 = getby(RELIN)) { // get the next reloc code
        if (r2 < 32) {          // simple cnt
            cnt = r2;
            totalSkipCnt += cnt;
        } else if (r2 < 64) {   // big cnt
            cnt = (r2 - 32) * 256 + getby(RELIN) + 32;
            totalSkipCnt += cnt;
        } else {
            for (cnt = 0L; cnt < totalSkipCnt; cnt++)    // copy skipped segment data over
                relby(segId, getby(SEGIN));

            totalSkipCnt += outstandingSkip;                // add any outstanding cnt from previous module
            outstandingSkip = 0;

            while (totalSkipCnt > 0x201f) {                 // emit max cnt relocation information codes
                relby(relId, 0x3f);                         // until outstanding cnt is small enough
                relby(relId, 0xff);
                totalSkipCnt -= 0x201F;
            }

            r4 = (uint16_t)totalSkipCnt;                    // emit remaining cnt info
            totalSkipCnt = 0L;                              
            if (r4 >= 32) {                                 // use big format
                r4 -= 32;
                relby(relId, (r4 /256) | 32);
                relby(relId, r4);
            } else if (r4 != 0)                             // use simple format
                relby(relId, r4);
#pragma warning(suppress : 26451)   // arithmetic is ok here
            if ((r4 = (r2 - 64) >> 2) == 47 && (r4 += getby(RELIN)) >= 175)     // check for extended code
                r4 = (r4 - 0xaf) * 256 + getby(RELIN) + 175;

            switch (r4) {
            case 0:
                bias = 0L;
                break;
            case 1:
                bias = tsiz - segHdr->tbias;
                break;
            case 2:
                bias = dsiz - segHdr->dbias;
                break;
            case 3:
                bias = bsiz - (segHdr->dsiz + segHdr->dbias);
                break;
            default:                // symbol lookup
                r4 -= 4;            // convert code to symbol
                if (r4 >= symCnt || (r3 = symtab[r4]) == NULL)
                    error("bad symbol number", 0);
                if (r3->flag & 4) {         // not external ref
                    bias = r3->val;         // get the symbol offset
                    r4 = r3->flag & 3;      // and fixup flags
                } else {
                    bias = 0L;
                    r4 = r3->val + 4;       // convert to new global index
                }
                break;
            }

            if (r4 < 47)                    // simple reloc entry
                relby(relId, ((r4 << 2) + 64) | (r2 & 3));      // low 2 bits of r2 bit0 = 1 => subtract, bit1 = 1 => long
            else if (r4 < 175) {
                relby(relId, (r2 & 3) + (63 << 2));
                relby(relId, r4 - 47);
            } else {
                r4 -= 175;
                relby(relId, (r2 & 3) + (63 << 2));
                relby(relId, (r4 / 256) + 128);
                relby(relId, r4);
            }

            if (r2 & 1)
                bias -= loadBias;

            if (r2 & 1) {
                for (var_8 = 0; var_8 < 4; var_8++)
                    var_18[var_8] = getby(SEGIN);

                bias += xstol(var_18);
                relint(segId, bias);
                cnt = 4;
                if (!longint)
                    remark("bad relocation item", "");
            } else {
                var_18[0] = getby(SEGIN);
                var_18[1] = getby(SEGIN);
                bias += xstos(var_18);
                r4 = (uint16_t)bias;
                relwd(segId, r4);
                cnt = 2;
                if (!longint && r4 != bias)
                    remark("range error", "");
            }
        }
        segBytesLeft -= cnt;

    }
    totalSkipCnt += segBytesLeft;
    for (cnt = 0L; cnt < totalSkipCnt; cnt++)
        relby(segId, getby(SEGIN));
    return totalSkipCnt + outstandingSkip;
}



void drcopy() {
    long cnt = obuf[3].fpos;
    char *r3;
    int r2, r4;

    if (cnt) {
        fseek(tfd, 0L, 0);
        while (cnt) {
            r3 = obuf[0].buf;
            if (fread(r3, 1, r2 = cnt < 64 ? cnt : 64, tfd) != r2)
                error("can't read temp file", 0);
            for (r4 = r2; --r4 >= 0; r3++)
                relby(2, *r3);
            cnt -= r2;
        }
    }

}



typedef int (*ifunc)();

int endeup(void) {
    fclose(tfd);
    remove(uname());
    return nxtexit;
}

int link1(FILE *fp, int arg_4) {
    obhdr_t hdr;
    int symEntrySiz;
    int symCnt;
    int intSize;
    uint32_t var_2A;
    char *symTab;
    int r4;
    char *r3;
    symbol_t *r2;

    symTab = gtsyms(fp, &hdr);
    intSize = longint ? 4 : 2;
    symEntrySiz = lenname + intSize + 1;
    symCnt = hdr.symSiz / symEntrySiz;

    if (arg_4 == 0) {
        r3 = symTab + intSize;
        for (r4 = 0; r4 < symCnt; r3 += symEntrySiz, r4++) {
            if ((*r3 & 8) && (*r3 & 4) && (r2 = lookup(r3 + 1)) && r2->flag == 0) {
                arg_4 = 1;
                break;
            }
        }
    }
    if (arg_4) { // BDD
        r3 = symTab + intSize;
        r4 = 0;
        for (r4 = 0; r4 < symCnt; r3 += symEntrySiz, r4++) {
            var_2A = (longint ? xstol(r3 - 2) : xstos(r3 - 2)) + rebias(*r3, hdr.tbias, hdr.dbias, hdr.dbias + hdr.dsiz);
            if (*r3 & 8) {
                if ((r2 = lookup(r3 + 1)) == NULL)
                    addsym(r3 + 1, *r3 & 4 ? *r3 & 0xf : 0, var_2A);
                else if ((*r3 & 4) && r2->flag)
                    remark("multiply defined: ", r3 + 1);
                else if (*r3 & 4) {
                    r2->flag = *r3 & 0xf;
                    r2->val = var_2A;
                } else if (r2->flag == 0)
                    r2->val = var_2A < r2->val ? r2->val : var_2A;
            }
        }
        bsiz += hdr.bsiz;
        if (xfl & 2)
            tsiz += hdr.tsiz;
        else
            dsiz += hdr.tsiz;
        if (xfl & 1)
            tsiz += hdr.dsiz;
        else
            dsiz += hdr.dsiz;
    }
    free(symTab);
    return arg_4;
}



void link2(long arg_2) {
    char *symTab;
    symbol_t **var_28;      // symbol table memory is used to create index table
    int intSize;
    int symCnt;
    int symEntrySize;
    obhdr_t hdr;
    symbol_t *r2;
    char *r3;
    int r4;
    static long long_624B, long_624F;

//  dumpsyms();


    symTab = gtsyms(ifd, &hdr);
    intSize = longint ? 4 : 2;
    symEntrySize = intSize + 1 + lenname;
    symCnt = hdr.symSiz / symEntrySize;
    var_28 = (symbol_t **)symTab;
    r3 = symTab + intSize;
    for (r4 = 0; r4 < symCnt; r4++) {
        if ((*r3 & 0xc) != 8)
            r2 = 0;
        else if ((r2 = lookup(r3 + 1)) == 0)
                error("phase error", 0);
        *var_28++ = r2;                    // var_28 ++ increments by 2
        r3 += symEntrySize;
    }

    arg_2 += longint ? 0x1c : 0x10;
    fseek(ifd, arg_2, 0);
    iseek = arg_2;
    ibuf[0].fpos = arg_2 & ~0x3f;
    ibuf[0].size = arg_2 & 0x3f;
    ibuf[0].off = ibuf[0].size + 64;
    arg_2 += hdr.tsiz + hdr.dsiz + (uint16_t)hdr.symSiz;
    ibuf[1].fpos = arg_2 & ~0x3f;
    ibuf[1].size = arg_2 & 0x3f;
    ibuf[1].off = ibuf[1].size + 64;
    long_624B = docode((xfl & 2) ? 0 : 1, (xfl & 2) ? 2 : 3, symCnt, (symbol_t **)symTab, &hdr,
        tsiz - hdr.tbias, hdr.tsiz, long_624B);   // last 3 args are longs
    long_624F = docode((xfl & 1) ? 0 : 1, (xfl & 1) ? 2 : 3, symCnt, (symbol_t **)symTab, &hdr,
        dsiz - hdr.dbias, hdr.dsiz, long_624F);   // last 3 args are longs
    free(symTab);
    bsiz += hdr.bsiz;
    dsiz += hdr.dsiz;
    tsiz += hdr.tsiz;
}

// original allocated 64 byte buffers on stack
// this version declares the buffers as part of the iobuf_t  type;

int main(int argc, char **argv) {
    int r4;

    getflags(&argc, &argv, "a,b##,c,db##,dr#,d,eb*,ed*,et*,h,l*>o*,r,tb##,t,u*>x#:F <files>",
        &afl, &bpad, &cfl, &dbias, &dround, &dfl, &endbss, &enddata, &endtext, &hfl, &llist, &ofile, &rfl, &tbias, &tfl, &ulist, &xfl);
    (long)drmask = (1L << (dround & 0xf)) + ~0;

    for (r4 = ulist.ntop; r4 < 10; r4++)
        addusym(ulist.val[r4]);
    ok = 1;
    if (pass1(argc, argv) && mid1() && mid2() && pass2(argc, argv))
        mkexec(ofile);
    return 0;
}


int mid1() {
    symbol_t *r2 = NULL;        // to supress 4703 error
    int r4;

    tsiz = (tsiz + maxbnd) & ~maxbnd;
    dsiz = (dsiz + maxbnd) & ~maxbnd;
    bsiz = (bsiz + maxbnd) & ~maxbnd;
    addend(endtext, 13, tsiz);
    addend(enddata, 14, dsiz);
    addend(endbss, 15, bsiz);
    obhdr.tsiz = tsiz;
    obhdr.dsiz = dsiz;
    obhdr.bsiz = bsiz;
    obhdr.bpad = bpad;
    obhdr.tbias = tbias;
    obhdr.dbias = dbias == -1L ? (obhdr.tbias + obhdr.tsiz + drmask) & ~drmask : dbias;
    obhdr.symSiz = ((tfl ? 0 : longint ? 5 : 3) + lenname) * nsyms;
    bsiz = obhdr.dbias + dsiz;
    dsiz = obhdr.dbias;
    tsiz = obhdr.tbias;

    for (r4 = 0; r4 < nsyms; r4++, r2++) { // fpin => struct size 20
        if ((r4 & 0x3f) == 0)
            r2 = stabs[r4 >> 6];    // stabs array of pointers
        if (r2->flag) {
            r2->flag |= 8;
            r2->val += rebias(r2->flag, 0L, 0L, 0L);
        } else if (r2->val && !dfl) {
            r2->flag = 0xf;
            r2->val = (r2->val + maxbnd) & ~maxbnd;
            obhdr.bsiz += r2->val;
            r2->val = obhdr.bsiz - r2->val + bsiz;
        } else if (!dfl)
            remark("undefined: ", r2->name);
        else
            nund++;
    }
    return ok;
}

void signalHandler(int signal) {
    exit(0);
}

int mid2() {
    symbol_t *r3 = NULL;    // to suppress false positive 4703 warning
    int r2;
    int r4;

    if ((ofd = fopen(ofile, "wb")) == NULL)
        error("can't create ", ofile);

    if (!afl && !rfl) {
        if ((tfd = fopen(uname(), "w+b")) == NULL)
            error("can't create tmp file", 0);
        _onexit(endeup);
        signal(SIGINT, signalHandler);
    }
    if (cfl) {
        obhdr.tsiz = 0;
        obhdr.dsiz = 0;
        obhdr.bsiz = 0;
        obhdr.bpad = 0;
        obhdr.tbias = 0;
        obhdr.dbias = 0;
    }

    obuf[2].fpos = obhdr.tsiz + obhdr.dsiz + (hfl ? 0 : longint ? 0x1c : 0x10);
    obuf[2].off = obuf[2].fpos & 0x3f;
    obuf[2].size = obuf[2].off;
    obuf[1].fpos = (hfl ? 0 : longint ? 0x1C : 0x10) + obhdr.tsiz;
    obuf[1].off = obuf[1].fpos & 0x3f;
    obuf[1].size = obuf[1].off;
    if (!hfl) {
        relby(0, 0x99);
        relby(0, (rfl ? 0x80 : 0) | ((binhdr >> 8) & 0x7f));
        relwd(0, obhdr.symSiz);
        relint(0, obhdr.tsiz);
        relint(0, obhdr.dsiz);
        relint(0, obhdr.bsiz);
        relint(0, obhdr.bpad);
        relint(0, obhdr.tbias);
        relint(0, obhdr.dbias);
    }

    if (!tfl) {
        for (r4 = 0, r2 = 0; r2 < nund && r4 < nsyms; r4++, r3++) {
            if ((r4 & 0x3f) == 0)
                r3 = stabs[r4 >> 6];
            if ((r3->flag & 4) == 0) {
                r3->flag |= 8;
                relsym(r3);
                r3->val = r2++;    // fpin is short
            }
        }

        for (r4 = 0; r4 < nsyms; r4++, r3++) {    // r3 => struct size 20
            if ((r4 & 0x3f) == 0)
                r3 = stabs[r4 >> 6];
#pragma warning(disable: 4703)  // false positive, r3 initialised when rr == 0
            if (r3->flag & 4)
                relsym(r3);
#pragma warning(default: 4703)
        }
    }
    return ok;
}



int pass1(int arg_2, char **arg_4) {
    char hdr[26];                   /// fix to read 26 byte header to align with lib & ar.h
    char *var_E;
    long var_C;
    int magic;
    uint16_t r4;
    FILE *fpin;

    gtlfile(0, 0, 0);       // dummy args added

    while (fpin = gtlfile(&arg_2, &arg_4, &var_E)) {
        if (fpin == stdin)
            error("can't link from STDIN", 0);
        else if (fpin == stderr)
            error("can't open: ", var_E);
        else if ((magic = gtmagic(fpin)) == binhdr)
            ok = link1(fpin, 1);
        else if (magic != 0xff75 && magic != 0xff6d && magic != 0xff65)
            error("bad file format: ", var_E);
        else {
            r4 = magic == 0xff65 ? 26 : 16;         // fixed to load 26 bytes for v7 header
            var_C = 2;

            while (fread(hdr, 1, r4, fpin) == r4 && *hdr) {
                if (binhdr != gtmagic(fpin))
                    error("bad library file in ", var_E);
                else if (link1(fpin, 0))
                    addlib(var_C + r4);
                // bug fix offsets here reflected v7 ar format but hdr assumed to be 24 vs. 26 bytes
                // also lstoi returns integer so low word with high bit set converts to -ve number
                var_C += (magic != 0xff65 ? lstoi(hdr + 14) : (long)(lstoi(hdr + 22) << 16) + (lstoi(hdr + 24) & 0xffff)) + r4;
                if (magic != 0xff75) // 1d91
                    var_C += (var_C & 1);
                fseek(fpin, var_C, 0);
            }
            addlib(0L);
        }
        fclose(fpin);
    }
    if (binhdr == 0)
        error("no object files", 0);
    return ok;
}

int pass2(int argc, char **argv) {
    uint16_t magic;
    long var_C;
    long *var_E;
    char *var_10;

    if (!cfl) {
        gtlfile(0, 0, 0);       // dummy args added to match declaration
        var_E = liboff;

        while ((ifd = gtlfile(&argc, &argv, &var_10)) != NULL) {
            if ((magic = gtmagic(ifd)) == binhdr)
                link2(0L);
            else
                while (var_C = *var_E++) {
                    fseek(ifd, var_C + 2, 0);
                    link2(var_C);
                }
            fclose(ifd);
        }
    }
    relby(4, 0);
    relby(5, 0);
    afl = 0;
    relby(2, 0);
    relby(0xf, 0);
    drcopy();
    relby(2, 0);
    relby(0xe, 0);
    return ok;
}


