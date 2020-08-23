
enum {  // low level tokens
    SQSTRING = 1, NL = 2, ID = 3, NUMBER = 4, PUNCT = 5, DQSTRING = 6
};
enum {  // preprocessor keywords
    P_DEFINE = 0xa, P_ELSE = 0xb, P_ENDIF = 0xc,
    P_IF = 0xd, P_IFDEF = 0xe, P_IFNDEF = 0xf,
    P_INCLUDE = 0x10, P_LINE = 0x11, P_HASH = 0x12,
    P_UNDEF = 0x13
};


enum {  // C keywords
    C_AUTO = 0x21, C_BREAK = 0x22, C_CASE = 0x23, C_CHAR = 0x24, C_CONTINUE = 0x25,
    C_DEFAULT = 0x26, C_DO = 0x27, C_DOUBLE = 0x28, C_ELSE = 0x29, C_EXTERN = 0x2a,
    C_FLOAT = 0x2b, C_FOR = 0x2c, C_GOTO = 0x2d, C_IF = 0x2e, C_INT = 0x2f,
    C_LONG = 0x30, C_REGISTER = 0x31, C_RETURN = 0x32, C_SHORT = 0x33, C_STATIC = 0x34,
    C_STRUCT = 0x35, C_SWITCH = 0x36, C_TYPEDEF = 0x37, C_UNION = 0x38, C_UNSIGNED = 0x39,
    C_WHILE = 0x3a, C_PARAM=0x3b, C_EXTERNINIT = 0x3c, C_STATICINIT = 0x3d, C_LABEL = 0x3e,
    C_SIZEOF = 0x61
};

enum { // punctuation
    C_COLON = 0x1, C_COMMA = 0x2, C_DOT = 0x3, C_LBRACKET = 0x4, C_LBRACE = 0x5,
    C_LPAREN = 0x6, C_ARROW = 0x7, C_RBRACKET = 0x8, C_RBRACE = 0x9, C_RPAREN = 0xa,
    C_SEMI = 0xb,
    C_NOT = 0x43, C_DEC = 0x44, C_INC = 0x53, C_LNOT = 0x5a,
    C_ADDRESSOF = 0x64, C_CAST = 0x66, C_ENDLIST = 0x69, C_DEREF = 0x6b, C_UMINUS = 0x6c, C_UPLUS = 0x6d,
    C_ANDAND = 0x82, C_DIV = 0x85, C_EQAND = 0x86, C_EQDIV = 0x87, C_EQUAL = 0x88,
    C_EQLSHIFT = 0x89, C_EQRSHIFT = 0x8a, C_EQMINUS = 0x8b, C_EQMOD = 0x8c, C_EQOR = 0x8d,
    C_EQPLUS = 0x8e, C_GT = 0x8f, C_GE = 0x90, C_EQMUL = 0x91, C_EQXOR = 0x92,
    C_EQEQ = 0x94, C_LT = 0x95, C_LE = 0x96, C_LSHIFT = 0x97, C_MOD = 0x99,
    C_NE = 0x9b, C_OR = 0x9c, C_OROR = 0x9d, C_QMARK = 0x9f, C_RSHIFT = 0xa0,
    C_XOR = 0xa3, C_LISTNEXT = 0xa5, C_POSTDEC = 0xa7, C_NEXTEXPR = 0xa8, C_STARTLIST = 0xa9, 
    C_POSTINC = 0xaa, C_AND = 0xc1, C_MINUS = 0xd8, C_PLUS = 0xde, C_STAR = 0xe2
};

enum { // data types
    D_FUNC=1, D_ARRAY=2, D_PTR=3,
    D_CHAR=4, D_UCHAR=8, D_SHORT=0xc, D_FSHORT=0x10, D_USHORT=0x14,
    D_LONG=0x18, D_FLONG=0x1c, D_ULONG=0x20,
    D_FLOAT=0x24, D_DOUBLE=0x28, D_STRUCT=0x2c, D_UNION=0x30
};


enum { // numbers
    C_DBL = 0x11, C_ID = 0x12, C_FILE = 0x13, C_LINENO = 0x14, C_INT32 = 0x15, C_INT16 = 0x16, C_STRING = 0x17,
    C_INT8 = 0x18, C_UINT32 = 0x19, C_UINT16 = 0x1a, C_UINT8 = 0x1b // C_UINT8 not used by cpp
};

/*
    double      0x11 8 byte double          byte pairwise swap 
    id          0x12 len string             len byte
    file        0x13 len string             len byte
    linkno      0x14 lineno                 lineno short little edian
    int32       0x15 longval                longval high word, low word both little edian
    int16       0x16 longval
    string      0x17 len string             len short little edian
    int8        0x18 longval
    uint32      0x19 longval
    uint16      0x1a longval

*/
