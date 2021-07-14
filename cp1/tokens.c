#include "cp1.h"
#include <inttypes.h>

bool needNL = false;
typedef struct {
    uint8_t code;
    char *enumStr;
} map_t;

static map_t codes[] = {
    {0x00, "C_EOF"},
    {0x01, "C_COLON"},
    {0x02, "C_COMMA"},
    {0x03, "C_DOT"},
    {0x04, "C_LBRACKET"},
    {0x05, "C_LBRACE"},
    {0x06, "C_LPAREN"},
    {0x07, "C_ARROW"},
    {0x08, "C_RBRACKET"},
    {0x09, "C_RBRACE"},
    {0x0a, "C_RPAREN"},
    {0x0b, "C_SEMI"},
    {0x11, "C_DBL"},
    {0x12, "C_ID"},
    {0x13, "C_FILE"},
    {0x14, "C_LINENO"},
    {0x15, "C_INT32"},
    {0x16, "C_INT16"},
    {0x17, "C_STRING"},
    {0x18, "C_INT8"},
    {0x19, "C_UINT32"},
    {0x1a, "C_UINT16"},
    {0x1b, "C_UINT8"},
    {0x21, "C_AUTO"},
    {0x22, "C_BREAK"},
    {0x23, "C_CASE"},
    {0x24, "C_CHAR"},
    {0x25, "C_CONTINUE"},
    {0x26, "C_DEFAULT"},
    {0x27, "C_DO"},
    {0x28, "C_DOUBLE"},
    {0x29, "C_ELSE"},
    {0x2a, "C_EXTERN"},
    {0x2b, "C_FLOAT"},
    {0x2c, "C_FOR"},
    {0x2d, "C_GOTO"},
    {0x2e, "C_IF"},
    {0x2f, "C_INT"},
    {0x30, "C_LONG"},
    {0x31, "C_REGISTER"},
    {0x32, "C_RETURN"},
    {0x33, "C_SHORT"},
    {0x34, "C_STATIC"},
    {0x35, "C_STRUCT"},
    {0x36, "C_SWITCH"},
    {0x37, "C_TYPEDEF"},
    {0x38, "C_UNION"},
    {0x39, "C_UNSIGNED"},
    {0x3a, "C_WHILE"},
    {0x3b, "C_PARAM"},
    {0x3c, "C_EXTERNINIT"},
    {0x3d, "C_STATICINIT"},
    {0x3e, "C_LABEL"},
    {0x43, "C_NOT"},
    {0x44, "C_DEC"},
    {0x53, "C_INC"},
    {0x5a, "C_LNOT"},
    {0x61, "C_SIZEOF"},
    {0x64, "C_ADDRESSOF"},
    {0x66, "C_CAST"},
    {0x69, "C_ENDLIST"},
    {0x6b, "C_DEREF"},
    {0x6c, "C_UMINUS"},
    {0x6d, "C_UPLUS"},
    {0x82, "C_ANDAND"},
    {0x85, "C_DIV"},
    {0x86, "C_EQAND"},
    {0x87, "C_EQDIV"},
    {0x88, "C_EQUAL"},
    {0x89, "C_EQLSHIFT"},
    {0x8a, "C_EQRSHIFT"},
    {0x8b, "C_EQMINUS"},
    {0x8c, "C_EQMOD"},
    {0x8d, "C_EQOR"},
    {0x8e, "C_EQPLUS"},
    {0x8f, "C_GT"},
    {0x90, "C_GE"},
    {0x91, "C_EQMUL"},
    {0x92, "C_EQXOR"},
    {0x94, "C_EQEQ"},
    {0x95, "C_LT"},
    {0x96, "C_LE"},
    {0x97, "C_LSHIFT"},
    {0x99, "C_MOD"},
    {0x9b, "C_NE"},
    {0x9c, "C_OR"},
    {0x9d, "C_OROR"},
    {0x9f, "C_QMARK"},
    {0xa0, "C_RSHIFT"},
    {0xa3, "C_XOR"},
    {0xa5, "C_LISTNEXT"},
    {0xa7, "C_POSTDEC"},
    {0xa8, "C_NEXTEXPR"},
    {0xa9, "C_STARTLIST"},
    {0xaa, "C_POSTINC"},
    {0xc1, "C_AND"},
    {0xd8, "C_MINUS"},
    {0xde, "C_PLUS"},
    {0xe2, "C_STAR"},
    {0xff, NULL}
};


static map_t dataTypes[] = {
    {1, "D_FUNC"},
    {2, "D_ARRAY"},
    {3, "D_PTR"},
    {4, "D_CHAR"},
    {8, "D_UCHAR"},
    {0x0c, "D_SHORT"},
    {0x10, "D_FSHORT"},
    {0x14, "D_USHORT"},
    {0x18, "D_LONG"},
    {0x1c, "D_FLONG"},
    {0x20, "D_ULONG"},
    {0x24, "D_FLOAT"},
    {0x28, "D_DOUBLE"},
    {0x2c, "D_STRUCT"},
    {0x30, "D_UNION"},
    {0xff, NULL}
};

static void showEnum(map_t *table, int val) {
    while (table->code != val && table->code != 0xff)
        table++;
    if (table->code != 0xff)
        fputs(table->enumStr, stdout);
    else
        printf("C_%02X", val);
};

void dumptok(tok_t *tok) {
    if (needNL) {
        putchar('\n');
        needNL = false;
    }
    printf("%s %d: ", infile, lineno);
    showEnum(codes, tok->code);
    if ((tok->code & 0xf0) == 0x10)
        switch (tok->code) {
        case C_DBL:
            printf(" %016" PRIX64, tok->dbl);
            break;
        case C_ID:
            printf(" %.8s", tok->name);
            break;
        case C_UINT32: case C_INT32: case C_UINT16: case C_INT16: case C_UINT8: case C_INT8:
            printf(" %8lX", tok->lng);
            break;
        case C_STRING:
            printf(" %.*s", tok->len, tok->str);
            break;
        default:
            break;
        }
    putchar('\n');
}
