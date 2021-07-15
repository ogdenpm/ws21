#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "anat.h"

int bincode[] = { 0x186, 0x10D, 0x10E, 0x184, 0x185, 0x108, 0x10A, 0x182,
				  0x187, 0x181, 0x12B, 0x112, 0x113, 0x114, 0x115, 0x1A9,
				  0x1AA, 0x110, 0x111, 0x116, 0x117, 0};
uchar crlist[] = { 0x50,0x51,0x52,0x53,0x54,0x55,0x4C,0x57,0 };
uchar irlist[] = { 0x42,0x43,0x44,0x45,0 };
literal_t *littab;
ushort ungetCnt;
ushort ungetBuf[3];
uchar L0EC4[] = { 0x4E,0x4F,0x48,0x41,0x49,0 };
uchar L0ECA[] = { 0x46,0x47,0x48,0x41,0x49,0 };


int scnstr(char *buf, int ch)
{
	char *s = buf;

	for (s = buf; *s && *s != ch; s++)
		;
	return (int)(s - buf);
}


token_t *addsym(char *name)
{
	token_t *p;
	int i;

	p = (token_t *) malloc(sizeof(token_t));
	p->link = symtab;
	symtab = p;
	p->flags = TSYM;
	p->val = 0;
	p->seg = p;
	for (i = 0; i < 8; i++) 
		if ((p->name[i] = *name++) == 0)
			break;
	while (i < 8)
		p->name[i++] = 0;
	return p;
}

void bchar(int c)
{
	ungetBuf[ungetCnt++] = c;
}

int bmatch(short arg1, token_t *tok)
{
	int type;
	type = tok->flags & TYPEMASK;
	switch (arg1) {
	case -1: return (nn(type) && tok->seg == 0 && tok->val == -1);
	case 1:	return (nn(type) && tok->seg == 0 && tok->val == 1);
	case 2:	return crlist[scnstr(crlist, type)];
	case 3:	return irlist[scnstr(irlist, type)];
	case 4:	return 1;
	case 5:	return nn(type);
	default: return (arg1 == type);
	}
}


void dosub(token_t *arg1, token_t *arg2)
{
	int type = arg2->flags & TYPEMASK;
	if (arg2->seg || nn(type) == 0) 
		error("illegal subscript");
	else if ((type = L0EC4[scnstr(L0ECA, arg1->flags & TYPEMASK)]) == 0)
		error("illegal []");
	else {
		arg1->flags = (arg1->flags & ~TYPEMASK) | type;
		arg1->val += arg2->val;
	}
}


void dounop(token_t *tok, short opcode)
{
	token_t *seg, *psym;
	int type;

	tok->flags &= 0xFDFF;
	seg = tok->seg;
	type = tok->flags & TYPEMASK;
	switch (opcode) {
	case TPUBLIC:
		for (psym = symtab; psym != 0 && strncmp(psym->name, tok->name, 8) != 0; psym = psym->link)
			;
		if (psym == 0)
			error("illegal public");
		else
			psym->flags |= TISPUBLIC;
		tok->flags |= TISPUBLIC;
		break;
	case TEQUALS:
		if (type != TSYM && nn(type) == 0 && tok->flags != TSTRING) 
			error("illegal =");
		else
			dolit(tok);
		break;
	case TCARET:
		if (seg != 0 || nn(type) == 0) 
			error("illegal !");
		else
			tok->val = ~tok->val;
		break;
	case TMINUS: 
		if(seg != 0 || nn(type) == 0) 
			error("illegal -");
		else 
			tok->val = -tok->val;
		break;
	case TPLUS:
		break;
	case TSTAR:
		if (type == TCONST)
			tok->flags = (tok->flags & ~TYPEMASK) | TSYM;
		else if (type < TADDR || type > 0x47)
			error("illegal *");
		else
			tok->flags |= 8;
		break;
	case TAND: 
		if (type != TSYM && ! nn(type))
			error("illegal &");
		else
			tok->flags = (tok->flags & ~TYPEMASK) | TADDR;
		break;
	default: 
		dobin(tok, tok, opcode);
		tok->flags |= TISPUBLIC;
	}
}



void error(char *arg1, ...)
{
	va_list marker;

	fprintf(stderr, "%i: ", lno);
	va_start(marker, arg1);
	vfprintf(stderr, arg1, marker);
	va_end(marker);
	fprintf(stderr, "\n");
	nerrors++;
}

int gchar()
{
	int c;

	if (ungetCnt > 0) 
		return (ungetBuf[--ungetCnt]);
	c = getchar();
	if (c == '\n') {
		lno++;
		return '\n';
	}
	return c;
}



int getesc(int endch)
{
	int c;
	int i;

	switch (c = gchar()) {
	case '\\': 
		if ((c = gchar()) != EOF && c != '\n') {
			if ((i = scnstr("btnvfr", ('A' <= c && c <= 'Z') ? c + 0x20 : c)) < 6)
				return i + 8;
			if ('0' <= c && c <= '9') {
				i = c - '0';
				c = gchar();
				if (isdig(c)) {
					i = (i << 3) | (c - '0');
					c = gchar();
					if (isdig(c))
						return (i << 3) | (c - '0');
				} else {
					bchar(c);
					return i;;
				}
			}
			return c & 0xff;
		}
		/* fall through */
	 case '\n':
	 case EOF:
		error("missing %c", endch);
		bchar(c);
		return -1;
	 default: 
		if (endch != c)
			return -1;
		return c & 0xFF;
	}
}





int gtok()
{
	int c;
	int op;
	int bits;

	for (;;) {
		c = gchar();
		while (c != '\n' && c != EOF && (c <= ' ' || 0x7f < c))
			c = gchar();
		switch (c) {
		case '!': case '|': case '&': case '*': case '^':
		case '-': case ',': case '+': case '>': case '<': case '=': case ':':
			if (op = getop(c))
				return op;
			break;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			for (tval = c - '0'; isdig(c = gchar());)
				tval = tval * 10 + c - '0';
			bchar(c);
			return TCONST;
		case '0':
			c = gchar();
			if (tolow(c) == 'x') {
				bits = 4;
				c = '0';
			} else
				bits = 3;
			for (tval = 0; isdig(c) || bits == 4 && ('a' <= c && c <= 'f'
			  || 'A' <= c && c <= 'F'); c = gchar()) 
				tval = (tval << bits) + c - (c >= 'a' ? 0x57 : c >= 'A' ? 0x37 : '0');
			bchar(c);
			return TCONST;
		case '\'':
			for (tval = 0; (c = getesc('\'')) >= 0;)
				tval = (tval << 8) + c;
			return TCONST;
		case '"': 
			while ((c = getesc('"')) >= 0)
				addch(c);
			return TSTRING;
		case ']':
			return TRBRACKET;
		case '[':
			return TLBRACKET;
		case ')':
			return TRPAREN;
		case '(':
			return TLPAREN;
		case '\n':
		case ';':
			return TSEP;
		case '/': 
			while ((c = gchar()) != '\n' && c != EOF)
				;
			bchar(c);
			break;
		case EOF:
			return TEOF;
		default: 
			if (isal(c))
				return getal(c);
			else
				error("illegal character %c", c);
		}
	}
}

int isal(int c)
{
	return ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '.' || c == '_');
}

int isdig(int c)
{
	return ('0' <= c && c <= '9');
}

int nn(int c)
{
	return (c == TCONST || c == TADDR);
}


int scntab(lookup_t *table, unsigned tablen, char *tokstr, int toklen)
{
	ushort ilow, imid;
	short lendif;
	char *s;
	char *t;
	int i;

	ilow = 0;
	while (ilow < tablen) {
		imid = (tablen + ilow) >> 1;
		s = table[imid].s;
		if ((lendif = *s++ - toklen) == 0) {
			for (i = 0, t = tokstr; i < toklen; i++)
				if ((lendif  = *s++ - *t++) != 0)
					break;
		}
		if (lendif < 0)
			ilow = imid + 1;
		else if (lendif == 0)
			return table[imid].v;
		else
			tablen = imid;
	}
	return 0;
}


void addch(int c)
{
	if (string[0] < 0x7E) {
		string[0]++;
		string[string[0]] = c;
	} else {
		error("string too long");
		string[0] = 0;
	}
}

int tolow(int c)
{
	return ('A' <= c && c <= 'Z') ? c + 0x20 : c;
}


