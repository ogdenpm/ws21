#include "anat.h"

tabent_t L023A[] = {	// bincode 0x186 "="
	{3, 5, "lxi L,R"},
	{0x4A, 0x57, "stax b"},
	{0x57, 0x4A, "ldax b"},
	{0x4B, 0x57, "stax d"},
	{0x57, 0x4B, "ldax d"},
	{0x49, 0x44, "shld L"},
	{0x44, 0x49, "lhld R"},
	{0x49, 0x57, "sta L"},
	{0x57, 0x49, "lda R"},
	{2, 0x48, "mvi L,R"},
	{2, 2, "mov L,R"},
	{0x45, 0x44, "sphl"},
	{0x42, 0x43, "mov c,e;mov b,d"},
	{0x42, 0x44, "mov c,l;mov b,h"},
	{0x43, 0x42, "mov e,c;mov d,b"},
	{0x43, 0x44, "mov e,l;mov d,h"},
	{0x44, 0x42, "mov l,c;mov h,b"},
	{0x44, 0x43, "mov l,e;mov h,d"},
	{0, 0, 0}
};
tabent_t L02AC[] = {	// bincode 0x10D "<="
	{0x42, 0x45, "pop b"},
	{0x43, 0x45, "pop d"},
	{0x44, 0x45, "pop h"},
	{0x58, 0x45, "pop psw"},
	{0x45, 0x42, "push b"},
	{0x45, 0x43, "push d"},
	{0x45, 0x44, "push h"},
	{0x45, 0x58, "push psw"},
	{0, 0, 0}
};
tabent_t L02E2[] = {	// bincode 0x10E "<>"
	{0x44, 0x4D, "xthl"},
	{0x4D, 0x44, "xthl"},
	{0x44, 0x43, "xchg"},
	{0x43, 0x44, "xchg"},
	{0, 0, 0}
};
tabent_t L0300[] = {	// bincode 0x184
	{0x44, 3, "dad R"},
	{3, 1, "inx L"},
	{2, 1, "inr L"},
	{0x57, 2, "add R"},
	{0x57, 0x48, "adi R"},
	{0, 0, 0}
};
tabent_t L0324[] = {	// bincode 0x185 "-"
	{3, 1, "dcx L"},
	{2, 1, "dcr L"},
	{0x57, 2, "sub R"},
	{0x57, 0x48, "sui R"},
	{0, 0, 0}
};
tabent_t L0342[] = {	// bincode 0x108 "+^"
	{0x57, 2, "adc R"},
	{0x57, 0x48, "aci R"},
	{0, 0, 0}
};
tabent_t L0354[] = {	// bincode 0x10A "-^"
	{0x57, 2, "sbb R"},
	{0x57, 0x48, "sbi R"},
	{0, 0, 0}
};
tabent_t L0366[] = {	// bincode 0x182 "&"
	{0x57, 2, "ana R"},
	{0x57, 0x48, "ani R"},
	{0, 0, 0}
};
tabent_t L0378[] = {	// bincode 0x187 "^"
	{0x57, 2, "xra R"},
	{0x57, 0x48, "xri R"},
	{0, 0, 0}
};
tabent_t L038A[] = {	// bincode 0x181 "|"
	{0x57, 2, "ora R"},
	{0x57, 0x48, "ori R"},
	{0, 0, 0}
};
tabent_t L039C[] = {	// bincode 0x12B "::"
	{0x57, 2, "cmp R"},
	{0x57, 0x48, "cpi R"},
	{0, 0, 0}
};
tabent_t L03AE[] = {	// bincode 0x112 "<*>"
	{0x57, 1, "rlc"},
	{0x57, -1, "rrc"},
	{0, 0, 0}
};
tabent_t L03C0[] = {	// bincode 0x113 "<^>"
	{0x57, 1, "ral"},
	{0x57, -1, "rar"},
	{0, 0, 0}
};
tabent_t L03D2[] = {	// bincode 0x114 "<+>"
	{0x57, 1, "stc;ral"},
	{0x57, -1, "stc;rar"},
	{0, 0, 0}
};
tabent_t L03E4[] = {	// bincode 0x115 "<->"
	{0x57, 1, "ora a;ral"},
	{0x57, -1, "ora a;rar"},
	{0, 0, 0}
};
tabent_t L03F6[] = {	// bincode 0x1A9 "jmp"
	{4, 0x49, "jmp R"},
	{4, 0x4C, "pchl"},
	{0, 0, 0}
};
tabent_t L0408[] = {	// bincode 0x1AA jmps & calls
	{4, 0x49, "L R"},
	{0, 0, 0}
};
tabent_t L0414[] = {	// bincode 0x110 "^"
	{0x42, 0x44, "mov m,c;inx h;mov m,b"},
	{0x43, 0x44, "mov m,e;inx h;mov m,d"},
	{0, 0, 0}
};
tabent_t L0426[] = {	// bincode 0x111 "->^"
	{0x42, 0x44, "mov c,m;inx h;mov b,m"},
	{0x43, 0x44, "mov e,m;inx h;mov d,m"},
	{0, 0, 0}
};
tabent_t L0438[] = {	// bincode 0x116 "a^"
	{0x44, 0x44, "mov a,m;inx h;mov h,m;mov l,a"},
	{0, 0, 0}
};
tabent_t L0444[] = {	// bincode 0x117 "=!"
	{0x57, 0x57, "cma"},
	{0, 0, 0}
};

tabent_t *tabtab[] = {L023A, L02AC, L02E2, L0300, L0324, L0342, L0354, L0366,
		   L0378, L038A, L039C, L03AE, L03C0, L03D2, L03E4, L03F6,
		   L0408, L0426, L0414, L0438, L0444, 0};

