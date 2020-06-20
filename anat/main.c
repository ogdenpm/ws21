#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "anat.h"

lookup_t L0D4F[] = {
	{"\001a", 0x57},	{"\001b", 0x50},		{"\001c", 0x51},		{"\001d", 0x52},
	{"\001e", 0x53},	{"\001h", 0x54},		{"\001l", 0x55},		{"\002af", 0x58},
	{"\002bc", 0x42},	{"\002cc", 0x19B},		{"\002cm", 0x19F},		{"\002cp", 0x19E},
	{"\002cz", 0x199},	{"\002de", 0x43},		{"\002hl", 0x44},		{"\002jc", 0x1A3},
	{"\002jm", 0x1A7},	{"\002jp", 0x1A6},		{"\002jz", 0x1A1},		{"\002sp", 0x45},
	{"\003cnc", 0x19A},	{"\003cnz", 0x198},		{"\003cpe", 0x19D},		{"\003cpo", 0x19C},
	{"\003jmp", 0x1A9},	{"\003jnc", 0x1A2},		{"\003jnz", 0x1A0},		{"\003jpe", 0x1A5},
	{"\003jpo", 0x1A4},	{"\004call", 0x1A8},	{"\006public", 0xAC},	{0, 0}
};

lookup_t L0DCF[] = {
	{"\001!", 0x181},	{"\001&", 0x182},	{"\001*", 0x83},	{"\001+", 0x184},
	{"\001,", 0x1AD},	{"\001-", 0x185},	{"\001:", TCOLON},	{"\001=", 0x186},
	{"\001^", 0x187},	{"\001|", 0x181},	{"\002+^", 0x108},	{"\002->", 0x109},
	{"\002-^", 0x10A},	{"\002::", 0x12B},	{"\002:=", 0x10B},	{"\002<=", 0x10D},
	{"\002<>", 0x10E},	{"\002=!", 0x117},	{"\002=>", 0x10F},	{"\002=^", 0x110},
	{"\003->^", 0x111},	{"\003<*>", 0x112},	{"\003<+>", 0x114},	{"\003<->", 0x115},
	{"\003<^>", 0x113},	{"\003=a^", 0x116},	{0, 0}
};

lookup_t L0E3B[] = {
	{"\002di", 0xF3},	{"\002ei", 0xFB},	{"\002in", 0xDB},	{"\002rc", 0xD8},
	{"\002rm", 0xF8},	{"\002rp", 0xF0},	{"\002rz", 0xC8},	{"\003cmc", 0x3F},
	{"\003daa", 0x27},	{"\003hlt", 0x76},	{"\003nop", 0x100},	{"\003out", 0xD3},
	{"\003ret", 0xC9},	{"\003rnc", 0xD0},	{"\003rnz", 0xC0},	{"\003rpe", 0xE8},
	{"\003rpo", 0xE0},	{"\003stc", 0x37},	{"\004rst0", 0xC7},	{"\004rst1", 0xCF},
	{"\004rst2", 0xD7},	{"\004rst3", 0xDF},	{"\004rst4", 0xE7},	{"\004rst5", 0xEF},
	{"\004rst6", 0xF7},	{"\004rst7", 0xFF},	{0, 0}
};

int iflag;
int lno = 1;
int nerrors;
char *L10B5;
char *sname;
int tval;
token_t *code, *data, *elc;
token_t *symtab;
token_t *tptr;
char string[128];




int getal(int c)
{
	char tokstr[8];
	int type, toklen;
	token_t *p;

	tokstr[0] = c;
	toklen = 1;
	while (isal(c = gchar()) || isdig(c)) {
		if (toklen < 8) 
			tokstr[toklen++] = c;
	}
	bchar(c);
	if ((type = scntab(L0D4F, 31, tokstr, toklen)) != 0)
		return type;

	if ((tval = scntab(L0E3B, 26, tokstr, toklen)) != 0) {
		tval &= TYPEMASK;
		return TCONST;
	}
	while (toklen < 8)
		tokstr[toklen++] = 0;
	for (p = symtab; p && strncmp(p->name, tokstr, 8) != 0; p = p->link)
		;
	if (!p)
		p = addsym(tokstr);
	if ((p->flags & TYPEMASK) == TCONST) {
		tval = p->val;
		return TCONST;
	}
	tptr = p;
	return p->flags & TYPEMASK;
}

int getop(int c)
{
	char tokstr[3];
	int i, type;

	tokstr[0] = c;
	tokstr[1] = gchar();
	tokstr[2] = gchar();

	for (i = 3; i > 0;) {
		if ((type = scntab(L0DCF, 26, tokstr, i)) != 0)
			return type;
#pragma warning(suppress : 6385)		// suppress false positive warning
		bchar(tokstr[--i]);
	}
	error("illegal operator %c", gchar());
	return 0;
}




int gexpr(token_t *tok, int arg2)
{
	token_t tmptok;
	int n, m;
	
	n = gterm(tok, arg2 != 0 ? "expression" : 0);
	if (tok->flags) { 
		while ((n & 0x140) == TBINOP) {
			if (n == TCOMMA) {
				gstring(tok, "x, string");
				m = gterm(&tmptok, "string");
				gstring(&tmptok, "string, x");
			} else {
				m = gterm(&tmptok, "term");
				dobin(tok, &tmptok, n);
			}
			n = m;
		}
	}
	return n;
}



void gstring(token_t *tok, char *s)
{
	switch (tok->flags) {
	case TCONST:
		addch(tok->val);
		break;
	case TADDR:
		addch(tok->val);
		addch(tok->val >> 8);
		break;
	case TSTRING:
		break;
	default:
		error("illegal %s", s);
	}
	tok->flags = TSTRING;
	return;
}



int gterm(token_t *tok, char *s)
{
	token_t tmptok;
	int follow;
	int type;
	token_t *p;

	tok->val = 0;
	memset(tok->name, 0, 8);
	while (((type = gtok()) == TCONST || type == TADDR || type == TSYM) && (follow = gtok()) == TCOLON)
		define(type == TCONST ? 0 : tptr, elc);

	switch (type) {
	case TLPAREN:
		type = gexpr(tok, 1);
		if (type != TRPAREN)
			error("missing )");
		else
			type = gtok();
		break;
	case TSYM: case TADDR: case TCONST:
		p = type == TCONST ? 0 : tptr;
#pragma warning(suppress : 6001)		// suppress false positive warning
		if (follow == TCOLONEQ) {
			type = gexpr(tok, 1);
			define(p, tok);
			tok->flags |= TISPUBLIC;
		} else if (type == TCONST) {
			tok->flags = TCONST;
			tok->val = tval;
			tok->seg = 0;
			type = follow;
		} else {
			p->flags |= TISUSED;
			tok->flags = p->flags & TYPEMASK;
			tok->val = p->val;
			tok->seg = p->seg;
			memcpy(tok->name, p->name, 8);
			type = follow;
		}
		break;
	default:
		if (type < 0) {
			tok->flags = 0;
			if (s != 0)
				error("missing %s", s);
			return type;
		} else if (type & 0x40) {
			tok->flags = type;
			type = gtok();
		} else if (type & 0x80) {
			follow = gterm(tok, "operand");
			dounop(tok, type);
			type = follow;
		} else {
			error("token! %o", type);
			return type;
		}
	}
	while (type == TLBRACKET) {
		type = gexpr(&tmptok, 1);
		if (type != TRBRACKET)
			error("missing ]");
		else {
			dosub(tok, &tmptok);
			type = gtok();
		}
	}
	return type;
}

int main(int argc, char **argv)
{
	char *s;
	int expr;
	int type;
	int n;
	char name[16];
	token_t tmptok;

	argc--, argv++;
	while (argc > 1 && (argv)[0][0] == '-') {
		switch (argv[0][1]) {
		case 'i':
				iflag = 1;
				break;
		case 'o':
			if (--argc > 1)
				L10B5 = *++argv;
			else {
				fprintf(stderr, "missing file name to -o flag\n");
				exit(0);
			}
			break;
		case 's':
			if (--argc > 1)
				sname = *++argv;
			else {
				fprintf(stderr, "missing start name to -s flag\n");
				exit(0);
			}
			break;
		default:
			fprintf(stderr, "bad flag %s\n", *argv);
			exit(0);
		}
		argc--, argv++;
	}


	if (!L10B5 && argc > 0 && argv[0][n = (int)(strlen(argv[0]) - 1)] == '8' && n < 15) {
		L10B5 = name;
		strcpy(name, argv[0]);
		name[n] = 'm';
	}

	if (L10B5) {
		if (freopen(L10B5, "w", stdout) == 0) {
			error("can't create %s", L10B5);
			exit(0);
		}
	}
	puthdr(L10B5 ? L10B5 : argc > 0 ? argv[0] : 0);
	code = addsym(".text");
	data = addsym(".data");
	elc = addsym(".");
	elc->seg = code;
	
	while (argc-- > 0) {
		if (freopen(*argv++, "r", stdin) == 0) 
			error("can't read %s", argv[-1]);
		else {
			while ((expr = gexpr(&tmptok, 0)) != -1) {
				if (expr != -2)
					error("syntax error");
				if ((type = tmptok.flags & TYPEMASK) != 0 && (tmptok.flags & TISPUBLIC) == 0) {
					if (type == TCONST)
						putbyte(tmptok.val);
					else if (type == TADDR || type == TSYM)
						putword(&tmptok);
					else if (type == TSTRING)
						for (s = string + 1; string[0] != 0; string[0]--, s++)
							putbyte(*s);
					else
						error("illegal constant");
				}
			}
			fclose(stdin);
		}
	}
	putftr(symtab);
	return (nerrors == 0);
}


char *opname(int type)
{
	lookup_t *p;

	for (p = L0DCF; p->s && p->v != type; p++) 
		;
	if (p == 0)
		for (p = L0D4F; p->s && p->v != type; p++)
			;
	return (p->s ? p->s : "!?");
}



