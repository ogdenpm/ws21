typedef unsigned char uchar;
typedef unsigned short ushort;


typedef struct _token_t {
	struct _token_t *link;
	unsigned short flags;
	union {
		short val;
		char *lit;
	};
	struct _token_t *seg;
	char name[8];
} token_t;

typedef struct _literal_t {
	struct _literal_t *link;
	unsigned short flags;
	union {
		short val;
		char *lit;
	};
	token_t *seg;
	char name[8];
	short id;
} literal_t;

typedef struct _tabent_t {
	short n;
	short m;
	char *s;
} tabent_t;

typedef struct _lookup_t {
	char *s;
	short v;
} lookup_t;


token_t *addsym(char *s);
void putsp(int n);
void putval(int type, token_t *tok);
void putword(token_t *tok);
void error(char *arg1, ...);
token_t *litlbl(token_t *tok, short n);
void addch(int c);
void error(char *arg1, ...);
int tolow(int c);
int scntab(lookup_t *arg1, unsigned arg2, char *arg3, int arg4);
int nn(int c);
int isdig(int c);
int isal(int c);
int gtok();
int scnstr(char *buf, int ch);
int bmatch(short arg1, token_t *tok);
char *opname(int arg1);
void gstring(token_t *arg1, char *arg2);
int gterm(token_t *arg1, char *arg2);
int gexpr(token_t *tok, int arg2);
int getop(int arg1);
int getal(int arg1);
int gchar();
void dounop(token_t *tok, short opcode);
void dosub(token_t *arg1, token_t *arg2);
int bmatch(short arg1, token_t *tok);
void bchar(int c);
void dobin(token_t *lhs, token_t *rhs, short opcode);
void define(token_t *lhs, token_t *rhs);
void puthdr(char *s);
void putftr(token_t *tok);
void putbyte(int n);
void dolit(token_t *tok);

//extern struct {
//	char len;
//	char str[126];
//} string;
extern char string[128];
extern int bincode[];
extern uchar crlist[], irlist[];
extern tabent_t *tabtab[];
extern literal_t *littab;
extern char *sname;
extern int iflag;

extern token_t *symtab;
extern int lno;
extern int nerrors;
extern int tval;


#define TPUBLIC 0xAC
#define TRPTR 0x109
#define TRDPTR 0x10F
#define TLDPTR 0x10D
#define TCOLONEQ 0x10B
#define TAND 0x182
#define TSTAR 0x83
#define TPLUS 0x184
#define TMINUS 0x185
#define TEQUALS 0x186
#define TCARET 0x187
#define TCNZ 0x198
#define TCOMMA 0x1AD
#define TJMP 0x1A9
#define TJGROUP 0x1AA

#define TSYM 0x49
#define TCONST	0x48
#define TADDR	0x41
#define TSTRING 0x40
#define TEOF -1
#define TSEP -2
#define TLPAREN -3
#define TRPAREN -4
#define TLBRACKET -5
#define TRBRACKET -6
#define TCOLON -7

#define TISUSED 0x800
#define TISDEFINED 0x400
#define TISPUBLIC 0x200
#define TBINOP 0x100
#define TYPEMASK 0x1FF
