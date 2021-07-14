#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "anat.h"



short outmode;
char L0DDC[] = {"\tcseg\n"};
char L0DDE[] = {"\tdseg\n"};
char *reg8[] = {"b", "c", "d", "e", "h", "l", "m", "a", "?"};
char *reg16[] = {"b", "d", "h", "sp", "??"};
char *calljmp[] = {"cnz", "cz", "cnc", "cc", "cpo", "cpe", "cp", "cm",
				   "jnz", "jz", "jnc", "jc", "jpo", "jpe", "jp", "jm",
				   "call", "jmp", "j??"};

short litcnt;


token_t *elc, *code, *data;


int instr(char *s, char *cset)
{
	char *t;
	char *tset;

	for (t = s; *t; t++)
		for (tset = cset; *tset; tset++)
			if (*t == *tset)
				return (int)(t - s);
	return (int)(t - s);
}

void define(token_t *lhs, token_t *rhs)
{
	int flags;
	if (lhs == 0)
		error("constant redefined");
	else {
		flags = rhs->flags & TYPEMASK;
		if (elc == lhs) {
			if (code != rhs->seg &&  data != rhs->seg) 
				error("illegal .:=");
			else if (lhs->seg != rhs->seg) {
				lhs->seg = rhs->seg;
				printf(code == rhs->seg ? "\tcseg\n" : "\tdseg\n");
			}
			putsp(rhs->val - lhs->val);
		} else if (rhs->seg != 0 && elc != rhs)
			error("illegal definition of %.8s", lhs->name);
		else if ((lhs->flags & TISDEFINED) || ((lhs->flags & TISUSED) && flags != TSYM))
				error("redefinition of %.8s", lhs->name);
		else {
			lhs->flags = (lhs->flags & ~TYPEMASK) | TISDEFINED | flags;
			lhs->val = rhs->val;
			lhs->seg = rhs->seg;
			if (elc == rhs) {
				putval(TADDR, lhs);
				printf(":\n");
			}
		}
	}
}


void dobin(token_t *lhs, token_t *rhs, short opcode)
{
	short lopcode;
	token_t tmpToken;
	int i;
	tabent_t *p;
	char *s;
	
	lopcode = opcode;
	if (lopcode >= TCNZ && lopcode <= TJMP) {	// calls & jumps
		tmpToken.flags = lopcode;
		if (lopcode != TJMP)		// treat all but jmp in the same way
			lopcode = TJGROUP;		// by mapping to op 0x1AA
		lhs = &tmpToken;
	}
	lhs->flags |= TISPUBLIC;
	if (lopcode == TRPTR || lopcode == TRDPTR) {	 // map -> to = and => to <= by swapping arguments
		token_t *tmp;
		tmp = lhs;
		lhs = rhs;
		rhs = tmp;
		lopcode = lopcode == TRPTR ? TEQUALS : TLDPTR;
	}
	for (i = 0; bincode[i] && bincode[i] != lopcode; i++)
		;
	if (bincode[i] == 0) {
		error("unknown binop!");
		return;
	}
	for (p = tabtab[i]; p->n ; p++)
		if (bmatch(p->n, lhs) && bmatch(p->m, rhs))
			break;
	if (p->n == 0) {
		error("illegal operands for %s", opname(opcode));
		return;
	}

	putchar('\t');
	for (s = p->s; *s; s++) {
		switch(*s) {
		case ' ':
			putchar('\t');
			break;
		case ';':
			putchar('\n');
			putchar('\t');
			break;
		case 'R':
			putval(p->m, rhs);
			break;
		case 'L':
			putval(p->n, lhs);
			break;
		default:
			putchar(*s);
		}
	}
	putchar('\n');
	return;
}

void dolit(token_t *tok)
{
	literal_t *plit;


	for (plit = littab; plit; plit = plit->link) {
		if (plit->flags != TSTRING && plit->seg == tok->seg && tok->val == plit->val &&
		  (tok->seg == 0 || strncmp(tok->name, plit->name, 8) == 0))
			break;
	}
	if (plit == 0) {
		plit = (literal_t *)malloc(sizeof(literal_t));
		plit->link = littab;
		littab = plit;
		plit->flags = tok->flags;
		if (tok->flags != TSTRING) {
			plit->val = tok->val;
		} else {
			plit->lit = (char *)malloc(string[0]);
			memcpy(plit->lit, string, string[0]);
		}
		plit->seg = tok->seg;
		memcpy(plit->name, tok->name, 8);
		plit->id = ++litcnt;
	}
	tok->flags = (tok->flags & ~TYPEMASK) | TSYM;
	tok->val = 0;
	tok->seg = code;
	litlbl(tok, plit->id);
	if (plit->flags == TSTRING)
		string[0] = 0;
}

token_t *litlbl(token_t *tok, short n)
{
	int i;
	tok->name[0] = '_';
	tok->name[1] = '_';
	for (i = 2; i <= 4; n >>=4, i++)
		tok->name[i] = (n & 0xf) + 'a';
	return tok;
}

void putbyte(int n)
{
	printf("\tdb %i\n", n & 0xFF);
}

void putftr(token_t *tok)
{
	token_t tmpToken;
	unsigned char i;
	unsigned char *s;
	literal_t *plit;

	tmpToken.flags = 0;
	tmpToken.val = 0;
	tmpToken.seg = code;
	if (littab) {
		printf("\tcseg\n");
		plit = littab;
		while (plit) {
			putval(TSYM, litlbl(&tmpToken, plit->id));
			printf("; ");
			if (plit->flags != TSTRING)
				putword((token_t *)plit);
			else {
				s = plit->lit;
				for (i = *s++; i; i--, s++)
					putbyte(*s);
			}
			plit = plit->link;
		}
	}

	while (tok) {
		if (tok->seg && tok->seg != code && tok->seg != data) {
			printf("\textrn\t");
			tok->flags |= TISPUBLIC;
		} else if (tok->flags & TISPUBLIC)
			printf("\tpublic\t");
		if (tok->flags & TISPUBLIC) {
			putval(TADDR, tok);
			putchar('\n');
		}
		tok = tok->link;
	}
	if (sname)
		printf("\tend %s\n", sname);
	else
		printf("\tend\n");
}

void puthdr(char *s)
{
	int i;

	addsym("stack")->seg = code;
	addsym("memory")->seg = code;
	if (s) {
		while (s[i = instr(s, "/:]")])
#pragma warning(suppress : 26451)	// suppress intelliSense warning
			s += i + 1;
		printf(iflag ? "\tname\t%.*s@\n" : "\ttitle\t%.*s\n", instr(s, "."), s);
	}
	printf("\tcseg\n");
}


void putsp(int n)
{
	int i, j;

	if (n & 1)
		printf("\tdb\t0\n");
	for (i = n >> 1; i > 0; i -= 8) {
		printf("\tdw\t0");
		for (j = ((i < 8) ? i : 8) -1 ; j > 0; j--)
			printf(",0");
		putchar('\n');
	}
}

void putval(int code, token_t *tok)
{
	int i;
	int type;
	type = tok->flags & TYPEMASK;
	switch(code) {
	case 0x2: 
		fputs(reg8[scnstr(crlist, type)], stdout);
		break;
	case 0x3: 
		fputs(reg16[scnstr(irlist, type)], stdout);
		break;
	case 0x4:
		fputs(calljmp[type - TCNZ], stdout);
		break;
	default:
		if (tok->seg) {
			for (i = 0; i < 8 && tok->name[i] != 0; i++) {
				switch(tok->name[i]) {
				case '.':
					putchar(iflag ? '@': '$');
					break;
				case '_': 
					putchar(iflag ? '?': '.');
					break;
				default: 
					putchar(tok->name[i]);
				}
			}
			if (tok->val != 0)
				printf("%+i", tok->val);
		} else if (code == TCONST) 
			 printf("%i", tok->val & 0xFF);
		else
			printf("%i", tok->val);
	}
}





void putword(token_t *tok)
{
	printf("\tdw ");
	putval(TADDR, tok);
	printf("\n");
}

