#ifndef __SVM_H__
#define __SVM_H__

#include "../inst.h"
#include "def.h"

typedef struct jmplbl_t {
    int no;
    char* name;
    int jmppos;
} jmplbl_t;

#define SM_ARGS int chr, int* state, char* name, int* name_len, \
        char** argv, int* argv_len, int* argc, int line, int* out_bin, int* pos
/* syntax parser state machines */
int sm_symbol_name(SM_ARGS);
int sm_space(SM_ARGS);
int sm_newline(SM_ARGS);
int sm_comma(SM_ARGS);
int sm_colon(SM_ARGS);
int sm_semicolon(SM_ARGS);
int sm_eof(SM_ARGS);

int util_stricmp(const char* s1, const char* s2);
int inst_len(const char* name);
int parse_jmplbl(FILE* infile);
int parse_file(FILE* infile, int** output);
int exec_binary(int* bin, int len);

#endif /* __SVM_H__ */
