#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
	data
	8 bits	data
	9 bits	symbol
	3 bits	abs, code, data, ext
	9 bits	label -> 0 no label, 1 gen label 2-270 -> index + 2
	3 bits scan - 0 not code, 1 to code to scan, 2 code scanned, 3 code part, 4 switch

*/
#define LABELBASE	3

class loc {
public:
	unsigned char data;
	unsigned short symbol;
	unsigned char type;
	unsigned short label;
	unsigned char scan;
	char *labelName;

	loc() : data(0), symbol(0), type(0), label(0), scan(0), labelName(0) {}

	void addLabel(const char *newlabel);

};

void loc::addLabel(const char *newlabel)
{
	if (labelName == 0) {
		labelName = new char[strlen(newlabel) + 1];
		strcpy(labelName, newlabel);
	} else {
		char *s = new char[strlen(newlabel) + strlen(labelName) + 3];
		strcpy(s, newlabel);
		strcat(s, ":\n");
		strcat(s, labelName);
		delete labelName;
		labelName = s;
	}
}


	

const char *instr[2][256] = {
	{
	"nop", "lxi\tb,", "stax\tb", "inx\tb", "inr\tb", "dcr\tb", "mvi\tb,", "rlc",
	"db\t08h", "dad\tb", "ldax\tb", "dcx\tb", "inr\tc", "dcr\tc", "mvi\tc,", "rrc",
	"db\t10h", "lxi\td,", "stax\td", "inx\td", "inr\td", "dcr\td", "mvi\td,", "ral",
	"db\t18h", "dad\td", "ldax\td", "dcx\td", "inr\te", "dcr\te", "mvi\te,", "rar",
	"rim", "lxi\th,", "shld\t", "inx\th", "inr\th", "dcr\th", "mvi\th,", "daa",
	"db\t28h", "dad\th", "lhld\t", "dcx\th", "inr\tl", "dcr\tl", "mvi\tl,", "cma",
	"sim", "lxi\tsp,", "sta\t", "inx\tsp", "inr\tm", "dcr\tm", "mvi\tm,", "stc",
	"db\t38h", "dad\tsp", "lda\t", "dcx\tsp", "inr\ta", "dcr\ta", "mvi\ta,", "cmc",
	"mov\tb,b", "mov\tb,c", "mov\tb,d", "mov\tb,e", "mov\tb,h", "mov\tb,l", "mov\tb,m", "mov\tb,a",
	"mov\tc,b", "mov\tc,c", "mov\tc,d", "mov\tc,e", "mov\tc,h", "mov\tc,l", "mov\tc,m", "mov\tc,a",
	"mov\td,b", "mov\td,c", "mov\td,d", "mov\td,e", "mov\td,h", "mov\td,l", "mov\td,m", "mov\td,a",
	"mov\te,b", "mov\te,c", "mov\te,d", "mov\te,e", "mov\te,h", "mov\te,l", "mov\te,m", "mov\te,a",
	"mov\th,b", "mov\th,c", "mov\th,d", "mov\th,e", "mov\th,h", "mov\th,l", "mov\th,m", "mov\th,a",
	"mov\tl,b", "mov\tl,c", "mov\tl,d", "mov\tl,e", "mov\tl,h", "mov\tl,l", "mov\tl,m", "mov\tl,a",
	"mov\tm,b", "mov\tm,c", "mov\tm,d", "mov\tm,e", "mov\tm,h", "mov\tm,l", "hlt", "mov\tm,a",
	"mov\ta,b", "mov\ta,c", "mov\ta,d", "mov\ta,e", "mov\ta,h", "mov\ta,l", "mov\ta,m", "mov\ta,a",
	"add\tb", "add\tc", "add\td", "add\te", "add\th", "add\tl", "add\tm", "add\ta",
	"adc\tb", "adc\tc", "adc\td", "adc\te", "adc\th", "adc\tl", "adc\tm", "adc\ta",
	"sub\tb", "sub\tc", "sub\td", "sub\te", "sub\th", "sub\tl", "sub\tm", "sub\ta",
	"sbb\tb", "sbb\tc", "sbb\td", "sbb\te", "sbb\th", "sbb\tl", "sbb\tm", "sbb\ta",
	"ana\tb", "ana\tc", "ana\td", "ana\te", "ana\th", "ana\tl", "ana\tm", "ana\ta",
	"xra\tb", "xra\tc", "xra\td", "xra\te", "xra\th", "xra\tl", "xra\tm", "xra\ta",
	"ora\tb", "ora\tc", "ora\td", "ora\te", "ora\th", "ora\tl", "ora\tm", "ora\ta",
	"cmp\tb", "cmp\tc", "cmp\td", "cmp\te", "cmp\th", "cmp\tl", "cmp\tm", "cmp\ta",
	"rnz", "pop\tb", "jnz\t", "jmp\t", "cnz\t", "push\tb", "adi\t", "rst\t0",
	"rz", "ret", "jz\t", "db\t0cbh", "cz\t", "call\t", "aci\t", "rst\t1",
	"rnc", "pop\td", "jnc\t", "out\t", "cnc\t", "push\td", "sui\t", "rst\t2",
	"rc", "db\t0d9h", "jc\t", "in\t", "cc\t", "db\t0ddh", "sbi\t", "rst\t3",
	"rpo", "pop\th", "jpo\t", "xthl", "cpo\t", "push\th", "ani\t", "rst\t4",
	"rpe", "pchl", "jpe\t", "xchg", "cpe\t", "db\t0edh", "xri\t", "rst\t5",
	"rp", "pop\tpsw", "jp\t", "di", "cp\t", "push\tpsw", "ori\t", "rst\t6",
	"rm", "sphl", "jm\t", "ei", "cm\t", "db\t0fdh", "cpi\t", "rst\t7"
	},
	{
	"nop", "bc=", "*bc=a", "bc++", "b++", "b--", "b=", "rlc",
	"db\t08h", "hl+bc", "a=*bc", "bc--", "c++", "c--", "c=", "rrc",
	"db\t10h", "de=", "*de=a", "de++", "d++", "d--", "d=", "ral",
	"db\t18h", "hl+de", "a=*de", "de--", "e++", "e--", "e=", "rar",
	"rim", "hl=", "hl->", "hl++", "h++", "h--", "h=", "daa",
	"db\t28h", "hl+hl", "hl<-", "hl--", "l++", "l--", "l=", "~a",
	"sim", "sp=", "a->", "sp++", "*hl++", "*hl--", "*hl=", "stc",
	"db\t38h", "hl+sp", "a<-", "sp--", "a++", "a--", "a=", "cmc",
	"b=b", "b=c", "b=d", "b=e", "b=h", "b=l", "b=*hl", "b=a",
	"c=b", "c=c", "c=d", "c=e", "c=h", "c=l", "c=*hl", "c=a",
	"d=b", "d=c", "d=d", "d=e", "d=h", "d=l", "d=*hl", "d=a",
	"e=b", "e=c", "e=d", "e=e", "e=h", "e=l", "e=*hl", "e=a",
	"h=b", "h=c", "h=d", "h=e", "h=h", "h=l", "h=*hl", "h=a",
	"l=b", "l=c", "l=d", "l=e", "l=h", "l=l", "l=*hl", "l=a",
	"*hl=b", "*hl=c", "*hl=d", "*hl=e", "*hl=h", "*hl=l", "hlt", "*hl=a",
	"a=b", "a=c", "a=d", "a=e", "a=h", "a=l", "a=*hl", "a=a",
	"a+b", "a+c", "a+d", "a+e", "a+h", "a+l", "a+*hl", "a+a",
	"a+^b", "a+^c", "a+^d", "a+^e", "a+^h", "a+^l", "a+^*hl", "a+^a",
	"a-b", "a-c", "a-d", "a-e", "a-h", "a-l", "a-*hl", "a-a",
	"a-^b", "a-^c", "a-^d", "a-^e", "a-^h", "a-^l", "a-^*hl", "a-^a",
	"a&b", "a&c", "a&d", "a&e", "a&h", "a&l", "a&*hl", "a&a",
	"a^b", "a^c", "a^d", "a^e", "a^h", "a^l", "a^*hl", "a^a",
	"a|b", "a|c", "a|d", "a|e", "a|h", "a|l", "a|*hl", "a|a",
	"a::b", "a::c", "a::d", "a::e", "a::h", "a::l", "a::*hl", "a::a",
	"rnz", "bc<=sp", "jnz\t", "jmp\t", "cnz\t", "bc=>sp", "a+", "rst\t0",
	"rz", "ret", "jz\t", "db\t0cbh", "cz\t", "call\t", "a+^", "rst\t1",
	"rnc", "de<=sp", "jnc\t", "out\t", "cnc\t", "de=>sp", "a-", "rst\t2",
	"rc", "db\t0d9h", "jc\t", "in\t", "cc\t", "db\t0ddh", "a-^", "rst\t3",
	"rpo", "hl<=sp", "jpo\t", "hl><*sp", "cpo\t", "hl=>sp", "a&", "rst\t4",
	"rpe", "jmp *hl", "jpe\t", "de><hl", "cpe\t", "db\t0edh", "a^", "rst\t5",
	"rp", "af<=sp", "jp\t", "di", "cp\t", "af=>sp", "a|", "rst\t6",
	"rm", "sp=hl", "jm\t", "ei", "cm\t", "db\t0fdh", "a::", "rst\t7"
	}
};

int inst[256] = {
	1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 3, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
	1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1,
	1, 3, 3, 1, 1, 1, 2, 1, 1, 1, 3, 1, 1, 1, 2, 1,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 6, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,

	1, 1, 4, 5, 4, 1, 2, 1, 1, 6, 4, 1, 4, 4, 2, 1,
	1, 1, 4, 2, 4, 1, 2, 1, 1, 1, 4, 2, 4, 1, 2, 1,
	1, 1, 4, 1, 4, 1, 2, 1, 1, 6, 4, 1, 4, 1, 2, 1,
	1, 1, 4, 1, 4, 1, 2, 1, 1, 1, 4, 1, 4, 1, 2, 1

};



typedef struct {
	short type;
	char name[10];
	int value;
} sym_t;

int getword(FILE *fp)
{
	int n;

	n = getc(fp);
	return n + getc(fp) * 256;
}

int getword(unsigned char* &buf)
{
	int n;
	n = *buf++ & 0xff;
	return n + *buf++ * 256;
}

void markCode(loc *codeblk, int len);
void buildmem(FILE *fp, loc *mem, int len);
void markrel(FILE *fp, loc *mem, int daddr, int eaddr);
void scanSwitch(loc *codeblk, int len, int swlabel);
void dump(FILE *fp, loc *mem, int start, int end, sym_t *syms);

int asmfmt = 0;

void relv2(FILE *infp, FILE *outfp)
{
	int syms = getword(infp) / 12;
	int code = getword(infp);
	int data = getword(infp);
	int j1 = getword(infp);
	int j2 = getword(infp);
	int j3 = getword(infp);
	int j4 = getword(infp);

	int swlabel = -1;

	sym_t *symtab = new sym_t[syms];

	loc *mem = new loc[code + data];
	// load mem
	for (int i = 0; i < code + data; i++)
		mem[i].data = getc(infp);

	// load symbols

	for (int i = 0; i < syms; i++) {
		symtab[i].value = getword(infp);
		symtab[i].type = getc(infp);
		fread(symtab[i].name, 1, 9, infp);
		symtab[i].name[9] = 0;
		if (symtab[i].type == 0xd) {
			mem[symtab[i].value].scan = 1;
			mem[symtab[i].value].label = i + LABELBASE;
			mem[symtab[i].value].addLabel(symtab[i].name);
			fprintf(outfp, "\tpublic");
		} else if(symtab[i].type == 0xe) {
			mem[symtab[i].value].label = i + LABELBASE;
			mem[symtab[i].value].addLabel(symtab[i].name);
			fprintf(outfp, "\tpublic");
		} else {
			fprintf(outfp, "\textern");
			if (strcmp(symtab[i].name, "c.switch") == 0)
				swlabel = i;
		}
		fprintf(outfp, "\t%s\n", symtab[i].name);
	}
	markrel(infp, mem, code, code + data);
	if (swlabel >= 0)
		scanSwitch(mem, code, swlabel);
	markCode(mem, code);
	fprintf(outfp, "CODE\n");
	dump(outfp, mem, 0, code, symtab);
	fprintf(outfp, "DATA\n");
	dump(outfp, mem, code, code + data, symtab);
	fprintf(outfp, "L%04X:\n", code + data);
}


void relv1(FILE *infp, FILE *outfp)
{
	int syms = getword(infp);
	int code = getword(infp);
	int data = getword(infp);
	int swlabel = -1;

	sym_t *symtab = new sym_t[syms];

	loc *mem = new loc[code + data];

	for (int i = 0; i < syms; i++) {
		symtab[i].type = getc(infp);
		fread(symtab[i].name, 1, 7, infp);
		symtab[i].name[7] = 0;
		symtab[i].value = getword(infp);
		if (symtab[i].type == 0xd) {
			mem[symtab[i].value].scan = 1;
			mem[symtab[i].value].label = i + LABELBASE;
			mem[symtab[i].value].addLabel(symtab[i].name);
			fprintf(outfp, "\tpublic");
		} else if(symtab[i].type == 0xe) {
			mem[symtab[i].value].label = i + LABELBASE;
			mem[symtab[i].value].addLabel(symtab[i].name);
			fprintf(outfp, "\tpublic");
		} else {
			fprintf(outfp, "\textern");
			if (strcmp(symtab[i].name, "c.switc") == 0)
				swlabel = i;

		}
		fprintf(outfp, "\t%s\n", symtab[i].name);
	}
	buildmem(infp, mem, code + data);
	if (swlabel >= 0)
		scanSwitch(mem, code, swlabel);
	markCode(mem, code);
	fprintf(outfp, "CODE\n");
	dump(outfp, mem, 0, code, symtab);
	fprintf(outfp, "DATA\n");
	dump(outfp, mem, code, code + data, symtab);
	fprintf(outfp, "L%04X:\n", code+data);
}





void scanSwitch(loc *codeblk, int len, int swlabel)
{
	for (int i = 4; i < len; i++) {
		if (codeblk[i].type == 3 && codeblk[i].symbol == swlabel) {
			if (codeblk[i-3].type == 1) {
				int swloc = codeblk[i-3].data + codeblk[i-2].data * 256;
				codeblk[swloc].scan = 4;
				while (codeblk[swloc].type == 1) {
					int target = codeblk[swloc].data + codeblk[swloc+1].data * 256;
					codeblk[target].scan = 1;
					swloc += 4;
				}
				swloc += 2;
				// default
				if (codeblk[swloc].type == 1) {
					int target = codeblk[swloc].data + codeblk[swloc+1].data * 256;
					codeblk[target].scan = 1;
				}
			}
		}
	}
}


void markCode(loc *codeblk, int len)
{
	bool rescan = true;

	while (rescan) {
		rescan = false;
		for (int i = 0; i < len - 1; i++) {
			if (codeblk[i].scan == 1) {
				codeblk[i].scan = 2;
				switch (inst[codeblk[i].data]) {
				case 3:	// simple 3 byte code
					if (codeblk[i+1].scan == 0 && codeblk[i+2].scan == 0)
						codeblk[i+1].scan = 3;
					i++;
					// fall through to 2 byte code
				case 2:
					if (codeblk[i+1].scan == 0)
						codeblk[i+1].scan = 3;
					i++;
				case 1:
					if (codeblk[i+1].scan == 0)
						codeblk[i+1].scan = 1;
				case 6:
					break;
				case 4:
					if (codeblk[i+3].scan == 0)
						codeblk[i+3].scan = 1;
				case 5:
					if (codeblk[i+1].scan == 0 && codeblk[i+2].scan == 0) {
						codeblk[i+1].scan = 3;
						codeblk[i+2].scan = 3;
						if (codeblk[i+1].type == 1) {
							int off = codeblk[i+1].data + codeblk[i+2].data * 256;
							if (codeblk[off].scan == 0) {
								codeblk[off].scan = 1;
								if (off < i)
									rescan = true;
							}
						}
					}

				}
			}
		}
	}
	if (codeblk[len-1].scan == 1)
		codeblk[len-1].scan = 2;
}

void markrel(FILE *fp, loc *mem, int daddr, int eaddr)
{
	int c;
	int addr = 0;
	int raddr;
	int blk = 0;

	while ((c = getc(fp)) != EOF) {
		switch(c & 0xf0) {
			case 0: case 0x10:
				if (c == 0)
					if (blk++ > 0)
						return;
					else
						addr = daddr;
				else
					addr += c;
				break;
			case 0x20:
				addr += 0x20 + (c & 0xf) * 256 + getc(fp);
				break;
			case 0x50: case 0x60: case 0x70:
			case 0x80: case 0x90: case 0xA0:
			case 0xB0: case 0xC0: case 0xD0:
			case 0xE0: case 0xF0:
				if (c == 0xFF) {
					printf("addr %04X, rel %02X\n", addr, c);
					exit(1);
				}
				if (c == 0xFC)
					mem[addr].symbol = (c - 0x50) / 4 + getc(fp);
				else
					mem[addr].symbol = (c - 0x50) / 4;
				mem[addr].type = 3;
				if (addr > 0 && mem[addr-1].data == 0xcd && mem[addr-1].scan == 0)
					mem[addr-1].scan = 1;
				addr += 2;
				break;
			case 0x40:
				if (c != 0x44 && c != 0x48) {
						printf("addr %04X, rel %02X\n", addr, c);
						exit(1);
				}
				raddr = mem[addr].data + mem[addr+1].data * 256;
				mem[addr].type = (c == 0x44) ? 1 : 2;
				if (raddr < eaddr) {
					if (mem[raddr].label == 0)
						mem[raddr].label = 1;
					else if (mem[raddr].label >= LABELBASE) {
						mem[addr].symbol = mem[raddr].label - LABELBASE;
						mem[addr].data = 0;
						mem[addr + 1].data = 0;
						mem[addr].type = 3;
					}
				}
				addr += 2;
				break;
			case 0x30:
				printf("addr %04X, rel %02X\n", addr, c);
				exit(1);
		}
	}
}

void addCase(loc *mem, const char *lab, unsigned target)
{
	char tlabel[6];
	if (mem[target].labelName == 0) {	// no label so far, so add proper jump label
		sprintf(tlabel, "L%04X", target);
		mem[target].addLabel(tlabel);
	}
	mem[target].addLabel(lab);
}


void dump(FILE *fp, loc *mem, int start, int end, sym_t *syms)
{
	int off;
	int cnt;
	char txt[17];

	for (int i = start; i < end; i++) {
		if (mem[i].labelName)
			fprintf(fp, "\n%s:", mem[i].labelName);
		else if (mem[i].label == 1)
			fprintf(fp, "\nL%04X:", i);
		putc('\t', fp);
		switch (mem[i].scan) {
		case 0:
			switch(mem[i].type) {
		    case 0:
				fprintf(fp, "db ");
				cnt = 0;
				while (i < end) {
					if (mem[i].data < 10)
						fprintf(fp, "%d", mem[i].data);
					else
						fprintf(fp, "0x%X", mem[i].data);
					if (' ' <= mem[i].data && mem[i].data <= '~')
						txt[cnt] = mem[i].data;
					else
						txt[cnt] = '.';
					if (++cnt == 16 || i == end - 1 || mem[i+1].label != 0 || mem[i+1].scan != 0 || mem[i+1].type != 0) {
						txt[cnt] = 0;
						fprintf(fp, "\t; %s", txt);
						break;
					}
					i++;
					fprintf(fp, ",");
				}
				break;
			case 1:
			case 2:
				fprintf(fp, "dw L%04X", mem[i].data + mem[i+1].data * 256);
				i++;
				break;
			case 3:
				off = mem[i].data + mem[i+1].data * 256;
				if (off == 0)
					fprintf(fp, "dw %s", syms[mem[i].symbol].name);
				else if (off < 32)
					fprintf(fp, "dw %s+%d", syms[mem[i].symbol].name, off);
				else if (off >= 0xF800)
					fprintf(fp, "dw %s-%d", syms[mem[i].symbol].name, 0x10000 - off);
				else
					fprintf(fp, "dw %s+0x%X", syms[mem[i].symbol].name, off);
				i++;
				break;
			}
			break;
		case 2:
			fprintf(fp, "%s", instr[asmfmt][mem[i].data]);
			if (i + 1 == end || mem[i + 1].scan != 3)
				break;
			i++;
			if (i == end - 1 || mem[i+1].scan != 3) {
				if (mem[i].data < 10)
					fprintf(fp, "%d", mem[i].data);
				else
					fprintf(fp, "0x%X", mem[i].data);
			} else {
				switch (mem[i].type) {
				case 0:
					off = mem[i].data + mem[i+1].data * 256;
					if (off < 1024)
						fprintf(fp, "%d", off);
					else if (off >= 0xFC00)
						fprintf(fp, "-%d", 0x10000 - off);
					else
						fprintf(fp, "0x%X", off);
					break;
				case 1:
				case 2:
					fprintf(fp, "L%04X", mem[i].data + mem[i+1].data * 256);
					break;
				case 3:
					off = mem[i].data + mem[i+1].data * 256;
					if (off == 0)
						fprintf(fp, "%s", syms[mem[i].symbol].name);
					else if (off < 1024)
						fprintf(fp, "%s+%d", syms[mem[i].symbol].name, off);
					else if (off > 0xFC00)
						fprintf(fp, "%s-%d", syms[mem[i].symbol].name, 0x10000 - off);
					else
						fprintf(fp, "%s+0x%X", syms[mem[i].symbol].name, off);
					break;
				}
				i++;
			}
			break;
		case 4:
			{
				unsigned caseV, caseT;
				char caseTxt[14];
				fprintf(fp, "/*\n\t");
				while (mem[i].type == 1) {
					caseV = mem[i+2].data + mem[i+3].data * 256;
					caseT = mem[i].data + mem[i+1].data * 256;
					sprintf(caseTxt, ";case 0x%X", caseV);
					addCase(mem, caseTxt, caseT);
					fprintf(fp, " case 0x%X: goto L%04X\n\t", caseV, caseT);
					i += 4;
				}
				caseT = mem[i+2].data + mem[i+3].data * 256;
				addCase(mem, ";default", caseT);
				fprintf(fp, " default: goto L%04X\n\t*/", mem[i+2].data + mem[i+3].data * 256);

				i += 3;
				break;
			}
		default: fprintf(fp, "oops scan = %d", mem[i].scan);
		}
		putc('\n', fp);
	}
}

void buildmem(FILE *fp, loc *mem, int len)
{

	for (int addr = 0; addr < len; ) {
		int blk = getc(fp);
		if (blk >= 0x80) {
			switch (blk & 0xf8) {
				case 0x80:
					mem[addr].symbol = getc(fp) + 13;
					mem[addr].type = 3;
					break;
				case 0x88:
					mem[addr].type = 1;
					break;
				case 0x90:
					mem[addr].type = 2;
					break;
				default:
					mem[addr].symbol = (blk - 0x98) >> 3;
					mem[addr].type = 3;
					break;
			}
			blk = blk & 7;
			if (mem[addr].type == 3) {
				blk += 2;
				if (addr > 0 && mem[addr-1].data == 0xcd && mem[addr-1].scan == 0)
					mem[addr-1].scan = 1;
			} else {
				int off = getword(fp);
				if (mem[off].label == 0)
					mem[off].label = 1;
				else if (mem[off].label >= LABELBASE) {
					mem[addr].symbol = mem[off].label - LABELBASE;
					mem[addr].type = 3;
					if (addr > 0 && mem[addr-1].data == 0xcd && mem[addr-1].scan == 0)
						mem[addr-1].scan = 1;
					off = 0;
				}
				mem[addr++].data = off % 256;
				mem[addr++].data = off / 256;
			}
		}
		while (blk-- > 0)
			mem[addr++].data = getc(fp);
	}
}






int main(int argc, char **argv)
{
	FILE *fp, *fpout;
	char fname[255];


	if (argc == 3)
		asmfmt = strcmp(argv[1], "-8") == 0;
	if (argc < 2 || (fp = fopen(argv[argc - 1], "rb")) == NULL)
		exit(0);
	int type = getword(fp);
	if (type != 0x106 && type != 0x1499) {
		fprintf(stderr, "unsupported rel file\n");
		exit(1);
	}
	strcpy(fname, argv[argc - 1]);
	if (char *s = strrchr(fname, '.'))
		*s = 0;
	strcat(fname, asmfmt == 0 ? ".asm" : ".8");
	if ((fpout = fopen(fname, "wt")) == NULL)
		fprintf(stderr, "can't create %s\n", fname);
	else {
		if (type == 0x106)
			relv1(fp, fpout);
		else
			relv2(fp, fpout);
		fclose(fpout);
	}
	fclose(fp);
}
