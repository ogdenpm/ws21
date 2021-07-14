#include "cp1.h"
#include <fcntl.h>

int iregs = 0x1C;
int tchar = D_CHAR;
int tfield = D_FSHORT;
int tint = D_SHORT;
int tunsign = D_USHORT;
int aflag = 0;
int cflag = 0;
int eflag = 0;
int mflag = 0;
int uflag = 0;
int bitswd = 16;
int bndef = 1;
int intsize = 2;
int nlen = 7;
char *_pname = "p1";
FILE *errfd;
char *ofile;
int nerrors = 0;
char *infile;
short lineno = 0;
FILE *outfd;
case_t *casetab;
term_t *exlist;
sym_t *lbltab;
term_t *littab;
sym_t *mostab;
sym_t *strtab;      // structure table
sym_t *symend;      // not used
sym_t *symtab;
sym_t *untab;

uint8_t tBBE0[] = { C_LPAREN, C_LBRACKET, 0 };
uint16_t unsignedTypes[] = { D_UCHAR,  D_USHORT,  D_ULONG,    0,    0,    0,    0,    0 };
uint16_t signedTypes[] = { D_CHAR, D_SHORT, D_LONG, D_FLOAT, D_DOUBLE, D_STRUCT, D_UNION, 0 };
uint8_t tBC03[] = { C_CHAR, C_SHORT, C_LONG, C_FLOAT, C_DOUBLE, C_STRUCT, C_UNION, 0 };
uint8_t tBC0B[] = { C_INT, C_FLOAT, C_DOUBLE, 0 };
uint8_t tBC0F[] = { C_CHAR, C_SHORT, C_LONG, 0 };
uint8_t tBC13[] = { C_EXTERN, C_STATIC, C_AUTO, C_REGISTER, C_TYPEDEF, 0 };
uint8_t tBC19[] = { C_STRUCT, C_UNION, 0 };
uint16_t wBC1C[] = { 0, D_CHAR, D_SHORT, 0x1c };

int nRegVar = 3;


bool cmptype(sym_t *arg_2, sym_t *arg_4) {
    uint16_t r3 = arg_2->dataType;
    uint16_t var_8 = arg_4->dataType;

    aux_t *r4 = arg_2->aux;
    aux_t *r2 = arg_4->aux;
    while ((r3 & 3) && (r3 & 3) == (var_8 & 3)) { // 1CA
        if ((r3 & 3) != D_PTR) {
            if ((r3 & 3) == D_FUNC) {
                r4 = r4->next;
                r2 = r2->next;
            } else {
                if (r4->lng != 0L && r2->lng != 0L && r4->lng != r2->lng)
                    return 0;
                r4 = r4->next;
                r2 = r2->next;
            }
        }
        r3 = dety(r3);
        var_8 = dety(var_8);
    }
    if (var_8 != r3)
        return false;
    if (r3 != C_STRUCT && var_8 != C_LONG)
        return true;
    return r4->psym == r2->psym;
}

aux_t *decsu(int arg_2) {
    tok_t var_34;
    sym_t *var_2A;
    sym_t *var_28;
    sym_t *var_26;
    sym_t var_24;
    long var_10;
    long var_C;
    int var_8;

    sym_t *r3;

    var_2A = 0;
    var_28 = (sym_t *)&var_2A;
    var_10 = 0L;
    var_C = 0L;
    int r4 = 0;
    int r2;
    ident(&var_34);

    if (eat(C_LBRACE)) {
        while (!eat(C_RBRACE) && gscty(&var_24, 0)) {
            while ((r3 = gdecl(&var_24, 0)) || peek(C_COLON)) {
                if (r3 == 0) {
                    var_28->next = r3 = buysym(tint, 0, 0, 0);
                    var_28 = r3;
                } else if ((var_26 = lookup(r3->name, var_2A, 0)) == 0) {
                    var_28->next = r3;
                    var_28 = r3;
                } else {
                    nmerr("member redefined", r3->name);
                    r3 = wsfree(r3, var_26);
                }

                if (!eat(C_COLON)) {
                    r4 = 0;
                    var_8 = 1;
                } else {
                    if (arg_2 != C_STRUCT)
                        wsperror("illegal field");

                    r2 = _const(1);
                    if (r3->dataType != tint || r3->dataType != tunsign)
                        wsperror("illegal bitfield");

                    r3->dataType = tfield;
                    if (r2 < 0 || r2 > bitswd) {
                        wsperror("bad field width");
                        r2 = bitswd;
                    }
                    r3->aux = buysat(0, 0);
                    r3->aux->b[1] = r2;
                    if (r2 == 0) {
                        r4 = 0;
                        var_8 = 0;
                    } else if (r4 == 0 || r2 + r4 > bitswd) {
                        r3->aux->b[0] = 0;
                        r4 = r2;
                        var_8 = 1;
                    } else {
                        r3->aux->b[0] = r4;
                        r4 += r2;
                        var_8 = 0;
                    }
                }
                if (arg_2 == C_UNION)
                    r3->lng = 0L;
                else if (var_8 == 0)
                    r3->lng = var_C;
                else {
                    r3->lng = bndify(r3->dataType, r3->aux, var_10);
                    var_10 = bytes(r3->dataType, r3->aux) + r3->lng;
                    var_C = r3->lng;
                }
                if (!eat(C_COMMA))
                    break;
            }
            need(C_SEMI);
        }
    }

    var_28->next = 0;
    if (!mflag) {
        for (r3 = var_2A; r3; r3 = r3->next) { // 800 
            if ((var_26 = lookup(r3->name, mostab, 0)) == NULL) {
                var_26 = buysym(r3->dataType, r3->aux, r3->name, 0);
                var_26->lng = r3->lng;
                var_26->next = mostab;
                mostab = var_26;
            } else if (cmptype(r3, var_26) == 0 || var_26->lng != r3->lng)
                nmerr("member conflict", r3->name);
        }
    }

    if (var_34.code == 0) {
        r3 = buysym(0, 0, 0, 0);
        r3->schain = var_2A;
        if (var_2A == 0)
            wsperror("no structure definition");
    } else if ((r3 = lookup(var_34.name, arg_2 == C_STRUCT ? strtab : untab, 0)) == 0) {
        r3 = buysym(0, 0, var_34.name, 0);
        if (arg_2 == C_STRUCT) {
            r3->next = strtab;
            strtab = r3;
        } else {
            r3->next = untab;
            untab = r3;
        }
        r3->schain = var_2A;
    } else if (var_2A == 0 && r3->schain != 0)
        var_2A = r3->schain;
    else if (var_2A != 0 && r3->schain != 0)
        nmerr("redefined tag", var_34.name);
    return buysat(r3, 0);
}


sym_t *dterm(sym_t *r4, bool arg_4) {
    tok_t var_16;
    sym_t *var_C;
    int var_A;
    sym_t *var_8;
    sym_t *r3;
    aux_t *r2;

    if (eat(C_STAR))
        return retype(dterm(r4, arg_4), D_PTR);
    if (!(var_A = eat(C_LPAREN)) || (arg_4 && (var_A = eat(C_RPAREN)))) { // afd
        if (var_A == C_RPAREN) {
            var_16.code = C_RPAREN;
            baktok(&var_16);
            var_16.code = C_LPAREN;
            baktok(&var_16);
            r3 = buysym(r4->dataType, r4->aux, r4->name, r4->code);
        } else {
            ident(&var_16);
            if (var_16.code == 0 && !arg_4)
                return 0;
            if (var_16.code && arg_4)
                nmerr("identifier not allowed", var_16.name);
            r3 = buysym(r4->dataType, r4->aux, r4->name, r4->code);
            cpynm(r3->name, var_16.name);
        }
    } else {
        if ((r3 = dterm(r4, arg_4)) == NULL) {
            wsperror("bad (declaration)");
            r3 = buysym(r4->dataType, r4->aux, r4->name, r4->code);
        }
        need(C_RPAREN);
    }

    for (r2 = (aux_t *)&r3->aux; r2->next; r2 = r2->next)
        ;
    while (var_A = alt(tBBE0)) { // CA7
        if (var_A == C_LPAREN) { // CAF
            var_C = 0;
            if (ident(&var_16)) { // CFD
                var_C = var_8 = buysym(0, 0, var_16.name, 0);

                while (eat(C_COMMA)) { // cfd
                    if (ident(&var_16) == 0)
                        wsperror("missing argument");
                    var_8 = var_8->next = buysym(0, 0, var_16.name, 0);
                }
            }
            r2->next = buysat(var_C, 0);
            need(C_RPAREN);
            retype(r3, 1);
        } else {
            r2->next = buymat(_const(0), 0);
            need(C_RBRACKET);
            retype(r3, D_ARRAY);
        }
        r2 = r2->next;
    }
    return r3;
}


sym_t *gdecl(sym_t *arg_2, bool arg_4) {
    uint16_t var_A;
    aux_t *var_8;
    sym_t *r3;
    aux_t *r4;
    int r2;

    var_8 = arg_2->aux;
    arg_2->aux = 0;
    r3 = dterm(arg_2, arg_4);
    arg_2->aux = var_8;
    if (r3 == 0)
        return 0;
    r4 = (aux_t *)&r3->aux;
    while (r4->next)
        r4 = r4->next;
    r4->next = var_8;
    for (r2 = 0, var_A = r3->dataType; arg_2->dataType != var_A; r2++, var_A >>= 2)
        ;

    while (r2 > 0) { // EC4
        var_A = (var_A << 2) | (r3->dataType & 3);
        r2--;
        r3->dataType >>= 2;
    }
    r3->dataType = var_A;
    return r3;
}

// modified to use stdarg
bool gscty(sym_t *r4, ...) {
    va_list arg4;
    va_start(arg4, r4);
    tok_t var_18;
    sym_t *var_E;
    bool isUnsigned;
    int chShLg;
    int var_8;
    int firstCode;
    int r2, r3;

    clrsym(r4);
    r2 = alt(tBC13);
    isUnsigned = eat(C_UNSIGNED);
    chShLg = alt(tBC0F);      // check if char, short or long
    r3 = alt(tBC0B);            // int, float or double
    if (r3 == 0) {
        if (r3 = alt(tBC19))  // struct or union
            r4->aux = decsu(r3);
        else if (ident(&var_18) && (var_E = lookup(var_18.name, symtab, 0)) && var_E->code == C_TYPEDEF) {
            setty(r4, var_E->dataType, (intptr_t)var_E->aux);
            r3 = C_STRUCT;
        } else if (var_18.code)
            baktok(&var_18);
    }
    for (firstCode = var_8 = va_arg(arg4, int); var_8 && var_8 != r2; var_8 = va_arg(arg4, int))
        ;
    va_end(arg4);
    if (var_8 != r2)
        wsperror("illegal storage class");
    r4->code = r2 ? r2 : firstCode;
    if (r4->dataType && !chShLg && !isUnsigned)
        return true;
    if (r3 == 0 || r3 == C_INT) {
        if (chShLg)
            r3 = chShLg;
    } else if (chShLg == C_LONG && r3 == C_FLOAT && !isUnsigned)
        r3 = C_DOUBLE;
    else if (!chShLg && isUnsigned)
        wsperror("illegal type modifier");
    if (!r3 || r3 == C_INT)
        r4->dataType = isUnsigned ? tunsign : tint;
    else if ((r4->dataType = (isUnsigned ? unsignedTypes : signedTypes)[scnstr(tBC03, r3)]) == 0)
        r4->dataType = tint;
    return r2 || r3 || isUnsigned || chShLg;
}



int main(int argc, char **argv) {

    sym_t var_1C;
    bool var_8;
    sym_t *r3;
    sym_t *r2;

    infile = buybuf("", 1);
    getflags(&argc, &argv, "a,c,e,l,m,n#,o*,b#,r#,u:F <file>", &aflag, &cflag, &eflag, &intsize,
        &mflag, &nlen, &ofile, &bndef, &nRegVar, &uflag);
    if (intsize != 2)
        fixlint();
    bndef &= 3;
    if (uflag)
        tchar = D_UCHAR;

    iregs = wBC1C[nRegVar & 3];
    if (ofile) {
        if ((outfd = fopen(ofile, "wb")) == NULL) {
            wsperror("bad output file");
            exit(0);
        } else
            errfd = stdout;
    } else {
        outfd = stdout;
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
        errfd = stderr;
    }
    if (argc != 0 && freopen(*argv, "rb", stdin) == NULL) {
        wsperror("bad input file!");
        exit(0);
    }

    nerrors = 0;
    while (!eat(EOF)) { //1484
        var_8 = gscty(&var_1C, C_EXTERN, C_STATIC, C_TYPEDEF, 0);
        while (r2 = gdecl(&var_1C, 0)) { // 14BF
            var_8 = 1;
            if ((r3 = lookup(r2->name, symtab, 0)) == NULL) {
                r2->next = symtab;
                symtab = r2; // 1521
            } else if (r2->code == C_TYPEDEF || r3->code == C_TYPEDEF) {
                nmerr("redeclared typedef", r2->name);
                r3 = 0;
            } else if (r3->dataType && cmptype(r2, r3) == 0) {
                nmerr("redeclared external", r2->name);
                r3 = 0;
            } else if (r2->code == C_STATIC && r3->code == C_EXTERN)
                r3->code = C_STATIC;
            else if (r2->code == C_STATIC && r3->code == C_EXTERNINIT)
                r3->code = C_STATICINIT;

            if (r3) {
                if (r3->dataType == 0)
                    setty(r3, r2->dataType, (intptr_t)r2->aux);
                else if (type(r3->dataType) == D_ARRAY) {
                    if (r3->aux->lng == 0)
                        r3->aux->lng = r2->aux->lng;
                } else if (type(r3->dataType) == D_FUNC)
                    r3->aux->psym = r2->aux->psym;
                r2 = wsfree(r2, r3);
            }
            if (r2->code != C_TYPEDEF)
                if (type(r2->dataType) != D_FUNC)
                    datinit(r2);
                else if (fninit(r2))
                    break;
            if (!eat(C_COMMA))
                break;
        }
        if (!eat(C_SEMI) && !var_8)
            recover("bad external syntax");
    }
    for (r3 = symtab; r3; r3 = r3->next) {
        if (r3->code == C_EXTERN) {
            if (eflag == 0 || r3->b[0] == 0)
                pref(r3->name);
            exchk(r3);
        } else if (r3->code == C_EXTERNINIT) {
            pdef(r3->name);
            exchk(r3);
        } else if (r3->code == C_STATIC)
            nmerr("undefined static", r3->name);
    }
    //putch(EOF);
    fclose(outfd);
    return (nerrors != 0);
}



