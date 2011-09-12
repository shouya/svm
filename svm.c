#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "svm.h"

jmplbl_t* __jmp_tbl;
int __jmp_tbl_size = 0;
int __program_len = 0;

/* registers */
int __eax, __ebx, __ecx, __edx, __esi, __edi;
int *__esp, *__ebp;
int __eip;
int __flag;

int* __callstack_base = 0;
int* __callstack_top = 0;

static int typechr_len(int chr) {
    if (chr == 'r') return 1;
    if (chr == 'v') return 2;
    if (chr == 'j') return 3;
    return 0;
}

static int name_to_reg(const char* name) {
    if (!util_stricmp(name, "eax")) {
        return R_EAX;
    } else if (!util_stricmp(name, "ebx")) {
        return R_EBX;
    } else if (!util_stricmp(name, "ecx")) {
        return R_ECX;
    } else if (!util_stricmp(name, "edx")) {
        return R_EDX;
    } else if (!util_stricmp(name, "esi")) {
        return R_ESI;
    } else if (!util_stricmp(name, "edi")) {
        return R_EDI;
    } else if (!util_stricmp(name, "esp")) {
        return R_ESP;
    } else if (!util_stricmp(name, "ebp")) {
        return R_EBP;
    } else if (!util_stricmp(name, "eip")) {
        return R_EIP;
    } else if (!util_stricmp(name, "flag")) {
        return R_FLAG;
    } else {
/*        fprintf(stderr, "unknown register name: [%s]\n", name);*/
        return 0;
    }
}


static void reset_name(char* name, int* length) {
    name[0] = '\0';
    *length = 0;
}
static void reset_argument(int* argc, char** argv, int* argvlen) {
    int i = 0;
    for (; i != *argc; ++i) {
        if (argv[i] != NULL) {
            free(argv[i]);
            argv[i] = NULL;
        }
    }
    *argc = *argvlen = 0;
}


int* get_register(int reg_no) {
    static int* s_reg_arr[] = {
        &__eax, &__ebx, &__ecx, &__edx, &__esi, &__edi,
        (int*)&__esp, (int*)&__ebp, &__eip, &__flag
    };
    return (s_reg_arr[(-reg_no) - 1]);
}

int get_rvalue(int type, int val) {
    if (type == T_REG) {
        return (*get_register(val));
    } else if (type == T_VAL) {
        return val;
    } else {
        fprintf(stderr, "unknown type: %d\n", type);
        return FAILURE;
    }
}

void push_callstack(int pos) {
    if (__callstack_top - __callstack_base == CALLSTACK_SIZE) {
        fputs("callstack over flow!\n", stderr);
        return;
    }
    *(__callstack_top++) = pos;
}

int pop_callstack(void) {
    if (__callstack_top - __callstack_base == 0) { 
        fputs("callstack empty!\n", stderr);
        return FAILURE;
    }
    return (*(--__callstack_top));
}

void push_stack(int val) {
    if (__esp < __ebp) {
        fputs("stack pointer error!\n", stderr);
        return;
    }
    if (__esp - __ebp >= STACK_SIZE) {
        fputs("stack over flow!\n", stderr);
        return;
    }
    
    *(__esp++) = val;
}

int pop_stack(void) {
    if (__esp < __ebp) {
        fputs("stack pointer error!\n", stderr);
        return FAILURE;
    }
    if (__esp == __ebp) {
        fputs("stack empty!\n", stderr);
        return FAILURE;
    }
    return (*(--__esp));
}


int arg_len(int instno) {
    int i = 0;
    int arglen = strlen(__inst[instno].argument);
    int len = 0;
    
    for (; i != arglen; ++i) {
        if (__inst[instno].argument[i] == 'v') {
            len += 2;
        } else {
            ++len;
        }
    }
    return len;
}

int inst_len(const char* name) {
    int i = 0;
    for (; i != INST_COUNT; ++i) {
        if (!util_stricmp(__inst[i].name, name)) {
            /* argument's length & instruction's length(1) */
            return arg_len(i) + 1;
        }
    }

    fprintf(stderr, "not found function named: [%s]\n", name);
    return 0;
}

int util_stricmp(const char* s1, const char* s2) {
    for (;;) {
        if (*s1 == 0 && *s2 != 0) return (-1);
        else if (*s1 != 0 && *s2 == 0) return (1);
        else if (*s1 == 0 && *s2 == 0) return (0);
        else if (tolower(*s1) < tolower(*s2)) return (-1);
        else if (tolower(*s1) > tolower(*s2)) return (1);
        else {
            s1++, s2++;
        }
    }
    /* impossible launch to here */
    return 0;
}

int util_isnumeric(const char* text) {
    int i = (text[0] == '-' ? 1 : 0);
    for (; i != strlen(text); ++i) {
        if (!isdigit(text[i])) {
            return 0;
        }
    }
    return 1;
}


int parse_jmplbl(FILE* infile) {
    int chr = 0;
    char* _inst_name = malloc(BUF_SIZE);
    int _inst_name_len = 0;
    int _state = S_NOP;

    fseek(infile, 0, SEEK_SET);
    for (;;) {
        chr = fgetc(infile);

        if (isalnum(chr) || chr == '_') {
            switch (_state) {
            case S_NOP: /* name from new line */
                _state = S_INST_NAME;
                _inst_name[0] = chr;
                _inst_name_len = 1;
                break;
            case S_INST_NAME: /* store new char into name buffer */
                if (_inst_name_len == BUF_SIZE - 1) {
                    fputs("the jump point's name is too long.\n", stderr);
                    free(_inst_name);
                    return FAILURE;
                }
                _inst_name[_inst_name_len++] = chr;
                break;
            case S_UNKNOWN:
                break;
            }
        } else if (chr == '\n') {
            if (_state == S_INST_NAME) {
                _state = S_UNKNOWN;
                _inst_name[_inst_name_len] = '\0';
                __program_len += inst_len(_inst_name);
                _inst_name[0] = '\0';
                _inst_name_len = 0;
            }
            _state = S_NOP;
        } else if (isspace(chr)) {
            switch (_state) {
            case S_INST_NAME:
                _state = S_UNKNOWN;
                _inst_name[_inst_name_len] = '\0';
                __program_len += inst_len(_inst_name);
                _inst_name[0] = '\0';
                _inst_name_len = 0;
                break;
            case S_UNKNOWN:
            case S_NOP:
                break;
            }
        } else if (chr == ':') {
            switch (_state) {
            case S_INST_NAME:
                _state = S_NOP;
                _inst_name[_inst_name_len] = '\0';
                ++__jmp_tbl_size;

                __jmp_tbl = realloc(
                    __jmp_tbl, sizeof(struct jmplbl_t) * __jmp_tbl_size);
                __jmp_tbl[__jmp_tbl_size-1].no = __jmp_tbl_size - 1;
                __jmp_tbl[__jmp_tbl_size-1].name =
                    malloc(_inst_name_len);
                strcpy(__jmp_tbl[__jmp_tbl_size-1].name, _inst_name);
                __jmp_tbl[__jmp_tbl_size-1].jmppos = __program_len;
/* DEBUG
                printf("jp[%d]: [%s] -- [%d]\n", __jmp_tbl_size - 1,
                       __jmp_tbl[__jmp_tbl_size-1].name,
                    __jmp_tbl[__jmp_tbl_size-1].jmppos);
*/
                
                _inst_name[0] = '\0';
                _inst_name_len = 0;
                break;
            case S_NOP:
            case S_UNKNOWN:
                break;
            } /* switch */
        } else if (chr == EOF) {
            break;
        } else {
            _state = S_UNKNOWN;
        }/* if */
    } /* for (;;) */
    return SUCCESS;
}

/**** STATE MACHINES *****/
int sm_symbol_name(SM_ARGS) {
    switch (*state) {
    case S_NOP: /* name from new line */
        *state = S_INST_NAME;
        name[0] = chr;
        *name_len = 1;
        break;
    case S_INST_NAME: /* store new char into name buffer */
        if (*name_len == BUF_SIZE - 1) {
            fprintf(stderr, "instruction name too long at line: %d\n", line);
            return FAILURE;
        }
        name[(*name_len)++] = chr;
        break;
    case S_INST_ARG: /* store new char into argument buffer */
        if (*argv_len == BUF_SIZE - 1) {
            fprintf(stderr,
                    "instruction argument value too long at line: %d\n", line);
            return FAILURE;
        }
        argv[*argc - 1][(*argv_len)++] = chr;
        break;
    case S_INST_ARG_BEG: /* new argument */
        *state = S_INST_ARG;
        if (*argc == MAX_ARGC) {
            fprintf(stderr, "to many arguments at line: %d\n", line);
            return FAILURE;
        }
        argv[(*argc)++] = malloc(BUF_SIZE);
        argv[*argc - 1][0] = chr;
        *argv_len = 1;
        break;
    case S_COMMENT:
        break;
    }
    return SUCCESS;
}
int sm_space(SM_ARGS) {
    switch (*state) {
    case S_NOP:
        break;
    case S_INST_NAME:
        *state = S_INST_ARG_BEG;
        name[*name_len] = '\0';
        break;
    case S_INST_ARG: /* cont. store, such as [mov eax, dword|123] */
        if (*argv_len == BUF_SIZE - 1) {
            fprintf(stderr,
                    "instruction argument value too long at line: %d\n", line);
            return FAILURE;
        }
        argv[*argc - 1][(*argv_len)++] = chr;
        break;
    case S_COMMENT:
    case S_INST_ARG_BEG: /* cont, such as [mov eax,|ebx]*/
        break;
    }
    return SUCCESS;
}
int sm_newline(SM_ARGS) {
    int _ti_result = 0;

    switch (*state) {
    case S_NOP:
        break;
    case S_INST_NAME: /* execute simple instruction, such as ret */
        *state = S_NOP;
        name[*name_len] = '\0';
        /* execinst(_inst_name, 0, NULL, infile); */
        _ti_result = trans_inst(name, 0, NULL, &out_bin[*pos]);
        if (_ti_result == FAILURE) {
            fprintf(stderr, "error while parsing argument, line: %d\n", line);
            return FAILURE;
        } else {
            *pos += _ti_result;
        }
        /* always reserve a BUF_SIZE space to new instructions */
        reset_name(name, name_len);
        break;
    case S_INST_ARG:
        *state = S_NOP;
        argv[*argc - 1][*argv_len] = '\0';
        _ti_result = trans_inst(name, *argc, argv, &out_bin[*pos]);
        if (_ti_result == FAILURE) {
            fprintf(stderr, "error while parsing argument, line: %d\n", line);
            return FAILURE;
        } else {
            *pos += _ti_result;
        }
        reset_name(name, name_len);
        reset_argument(argc, argv, argv_len);
        break;
    case S_COMMENT:
        *state = S_NOP;
        break;
    case S_INST_ARG_BEG:
        *state = S_NOP;
        if (*argc > 0) { /* such as [mov eax,\n] */
            fprintf(stderr, "syntax error at line %d\n", line);
            return FAILURE;
        }
        /* such as [nop \n] */
        _ti_result = trans_inst(name, 0, NULL, &out_bin[*pos]);
        if (_ti_result == FAILURE) {
            fprintf(stderr, "error while parsing argument, line: %d\n", line);
            return FAILURE;
        } else {
            *pos += _ti_result;
        }
        /*execinst(_inst_name, 0, NULL, infile);*/
        reset_name(name, name_len);
        break;
    }
    return SUCCESS;
}
int sm_comma(SM_ARGS) {
    switch (*state) {
    case S_NOP: /* [  , ] */
        fprintf(stderr, "syntax error at line %d\n", line);
        return FAILURE;
    case S_INST_NAME: /* [mov,] */
        fprintf(stderr, "syntax error at line %d\n", line);
        return FAILURE;
    case S_INST_ARG: /* [mov eax,] */
        *state = S_INST_ARG_BEG;
        argv[*argc - 1][*argv_len] = '\0';
        break;
    case S_COMMENT:
        break;
    case S_INST_ARG_BEG: /* [mov ,] or [mov eax,,] */
        fprintf(stderr, "syntax error at line %d\n", line);
        return FAILURE;
    }
    return SUCCESS;
}
int sm_colon(SM_ARGS) {
    switch (*state) {
    case S_INST_NAME:
        *state = S_NOP;
        reset_name(name, name_len);
        break;
    case S_INST_ARG:
    case S_INST_ARG_BEG:
    case S_NOP:
        fprintf(stderr, "syntax error at line %d\n", line);
        return FAILURE;
    case S_COMMENT:
        break;
    }
    return SUCCESS;
}
int sm_semicolon(SM_ARGS) {
    int _ti_result;
    switch (*state) {
    case S_NOP:
        *state = S_COMMENT;
        break;
    case S_INST_NAME: /* [nop; some comments] */
        *state = S_COMMENT;
        name[*name_len] = '\0';
        _ti_result = trans_inst(name, 0, NULL, &out_bin[*pos]);
        if (_ti_result == FAILURE) {
            fprintf(stderr, "error while parsing argument, line: %d\n", line);
            return FAILURE;
        } else {
            *pos += _ti_result;
        }
        reset_name(name, name_len);
        break;
    case S_INST_ARG:
        *state = S_COMMENT;
        argv[*argc - 1][*argv_len] = '\0';
        _ti_result = trans_inst(name, *argc, argv, &out_bin[*pos]);
        if (_ti_result == FAILURE) {
            fprintf(stderr, "error while parsing argument, line: %d\n", line);
            return FAILURE;
        } else {
            *pos += _ti_result;
        }
        reset_name(name, name_len);
        reset_argument(argc, argv, argv_len);
        break;
    case S_COMMENT:
        break;
    case S_INST_ARG_BEG:
        *state = S_COMMENT;
        if (*argc > 0) { /* such as [mov eax,;] */
            fprintf(stderr, "syntax error at line %d\n", line);
            return FAILURE;
        }
        /* such as [nop ;] */
        _ti_result = trans_inst(name, 0, NULL, &out_bin[*pos]);
        if (_ti_result == FAILURE) {
            fprintf(stderr, "error while parsing argument, line: %d\n", line);
            return FAILURE;
        } else {
            *pos += _ti_result;
        }
        reset_name(name, name_len);
        break;
    }
    return SUCCESS;
}
int sm_eof(SM_ARGS) {
    int _ti_result;
    switch (*state) {
    case S_NOP:
        *state = S_COMMENT;
        break;
    case S_INST_NAME: /* [nop; some comments] */
        *state = S_COMMENT;
        name[*name_len] = '\0';
        _ti_result = trans_inst(name, 0, NULL, &out_bin[*pos]);
        if (_ti_result == FAILURE) {
            fprintf(stderr, "error while parsing argument, line: %d\n", line);
            return FAILURE;
        } else {
            *pos += _ti_result;
        }
        reset_name(name, name_len);
        break;
    case S_INST_ARG:
        *state = S_COMMENT;
        argv[*argc - 1][*argv_len] = '\0';
        _ti_result = trans_inst(name, *argc, argv, &out_bin[*pos]);
        if (_ti_result == FAILURE) {
            fprintf(stderr, "error while parsing argument, line: %d\n", line);
            return FAILURE;
        } else {
            *pos += _ti_result;
        }
        reset_name(name, name_len);
        reset_argument(argc, argv, argv_len);
        break;
    case S_COMMENT:
        break;
    case S_INST_ARG_BEG:
        *state = S_COMMENT;
        if (*argc > 0) { /* such as [mov eax,;] */
            fprintf(stderr, "syntax error at line %d\n", line);
            return FAILURE;
        }
        /* such as [nop ;] */
        _ti_result = trans_inst(name, 0, NULL, &out_bin[*pos]);
        if (_ti_result == FAILURE) {
            fprintf(stderr, "error while parsing argument, line: %d\n", line);
        } else {
            *pos += _ti_result;
        }
        reset_name(name, name_len);
        break;
    }
    return SUCCESS;
}

int parse_file(FILE* infile, int** output) {
    int _chr = 0;
    int _line = 1;
    int _state = S_NOP;

    char* _name = malloc(BUF_SIZE);
    int _name_len = 0;
    int _argc = 0;
    char* _argv[MAX_ARGC] = {NULL};
    int _argv_len = 0;

    int _pos = 0;
    int* _bin = malloc(__program_len * sizeof(int));

    fseek(infile, 0, SEEK_SET);
    for (;;) {
        _chr = fgetc(infile);
#define CALL_SM(sm_name) if (                                           \
            sm_name(_chr, &_state, _name, &_name_len, _argv,            \
                    &_argv_len, &_argc, _line, _bin, &_pos) == FAILURE) \
        { goto free_and_return; }

        if (isalnum(_chr) || _chr == '_') {
            CALL_SM(sm_symbol_name);
        } else if (_chr == '\n') {
            CALL_SM(sm_newline);
            ++_line;
        } else if (isspace(_chr)) {
            CALL_SM(sm_space);
        } else if (_chr == ',') {
            CALL_SM(sm_comma);
        } else if (_chr == ';') {
            CALL_SM(sm_semicolon);
        } else if (_chr == EOF) {
            CALL_SM(sm_eof);
            goto success_return;
        } else if (_chr == ':') {
            CALL_SM(sm_colon);
        } else {
            if (_state != S_COMMENT) {
                fprintf(stderr, "unknown character: %d[%c]", _chr, _chr);
                goto free_and_return;
            } /* if */
        } /* else */
#undef CALL_SM
    } /* for */


    return FAILURE;

success_return:
    *output = _bin;
    return SUCCESS;

free_and_return:
    free(_name);
    reset_argument(&_argc, _argv, &_argv_len);
    return FAILURE;
}

int exec_binary(int* bin, int len, int startpos) {
    instruction_t _inst;
    int _args[MAX_ARGC*2];
    int* _p = bin + startpos;
    int i, _arglen;

    for (;;) {
        if (_p - bin == len) {
            break;
        }
        
/* DEBUG
        printf("[%s](%d,%d,%d,%d)-[%d]\t",
               __inst[*_p].name, _p[1], _p[2], _p[3], _p[4], _p - bin);
        printf("eax-edx: %d %d %d %d, esp-ebp: %d\n",
               __eax, __ebx, __ecx, __edx,
               __esp - __ebp);
        if (*_p == 9) {
            printf("cmp result: %s\n",
                   __flag == LESS ? "LESS":
                   __flag == GREAT ? "GREAT":
                   __flag == EQUAL ? "EQUAL":
                   "ELSE");
        }
*/
        _inst = __inst[*_p].callback; /* inst no */
        _arglen = arg_len(*_p++); /* inst no -> arg */
        for (i = 0; i != _arglen; ++i) {
            _args[i] = *_p++;
        }
        (*_inst)(_args, &bin, &_p);
    }
    return SUCCESS;
}

int trans_inst(const char* name, int argc, char* const* argv, int* bin_ptr) {
    int _instno = 0;
    int _argi = 0;
    int _parg = 0;

    for (; _instno != INST_COUNT; ++_instno) {
        if (!util_stricmp(name, __inst[_instno].name)) {
            break;
        }
    }
    if (_instno == INST_COUNT) {
        fprintf(stderr, "no match function [%s] found.\n", name);
        return FAILURE;
    }

    if (argc > 0 && argc != strlen(__inst[_instno].argument)) {
        fputs("argument count not matched.\n", stderr);
        return FAILURE;
    }

    *(bin_ptr++) = _instno;

    for (; _argi != argc; ++_argi) {
        _parg = parse_arg(__inst[_instno].argument[_argi],
                          argv[_argi], bin_ptr);
        if (_parg == FAILURE) {
            fputs("argument parse error.\n", stderr);
            return FAILURE;
        }
        bin_ptr += typechr_len(__inst[_instno].argument[_argi]);
    }

    return (arg_len(_instno) + 1);
}

int parse_arg(int type_chr, const char* text, int* bin_ptr) {
    int i = 0;
    switch (type_chr) {
    case 'r':
        if ((*bin_ptr = name_to_reg(text)) == 0) {
            fprintf(stderr, "not found jump label named: [%s]\n", text);
            return FAILURE;
        }
        return 1;
    case 'v':
        bin_ptr[0] = T_REG;
        if ((bin_ptr[1] = name_to_reg(text)) == 0) {
            if (util_isnumeric(text)) {
                bin_ptr[0] = T_VAL;
                bin_ptr[1] = atol(text);
            } else {
                fprintf(stderr,
                        "not a register or a lexical numeric: [%s]\n", text);
                return FAILURE;
            }
        }
        return 2;
    case 'j':
        *bin_ptr = -1;
        for (; i != __jmp_tbl_size; ++i) {
            if (!util_stricmp(__jmp_tbl[i].name, text)) {
                *bin_ptr = __jmp_tbl[i].jmppos;
                return 1;
            }
        }
        fprintf(stderr, "not found such jump label: [%s]\n", text);
        return FAILURE;
    }

    /* unknown type char */
    fprintf(stderr, "unknown type character: [%c]\n", type_chr);
    return FAILURE;
}

void init_registers(void) {
    __eax = __ebx = __ecx = __edx = 0;
    __esi = __edi = 0;
    __esp = __ebp = malloc(STACK_SIZE * sizeof(int));
    __callstack_base = malloc(CALLSTACK_SIZE * sizeof(int));
    __callstack_top = __callstack_base;
}


int main(int argc, char** argv) {
    static FILE* _input_file;
    static int* _binary = NULL;
    int retval = 0;
    int _startpos = 0;
    int i = 0;

    if (argc > 1) {
        _input_file = fopen(argv[1], "r");
        if (!_input_file) {
            fprintf(stderr, "open file failed: [%s]", argv[1]);
            exit(EXIT_FAILURE);
        }
    } else {
        _input_file = stdin;
    }

    if (parse_jmplbl(_input_file) == FAILURE) {
        fputs("error at parsing jump labels\n", stderr);
        goto free_and_exit;
    }
    if (parse_file(_input_file, &_binary) == FAILURE) {
        fputs("error at parsing file\n", stderr);
        goto free_and_exit;
    }

    for (; i != __jmp_tbl_size; ++i) {
        if (!util_stricmp(__jmp_tbl[i].name, "start")) {
            _startpos = __jmp_tbl[i].jmppos;
        }
    }

    free(__jmp_tbl);
    init_registers();

/*    for (i = 0; i != __program_len; ++i) {
        printf("[%02d]get a: %d\n", i, _binary[i]);
    }
*/
    //  exit(0);

    if (exec_binary(_binary, __program_len, _startpos) == FAILURE) {
        fputs("error at executing\n", stderr);
        free(_binary);
        goto free_and_exit;
    }
    free(_binary);

    if (_input_file != stdin) {
        fclose(_input_file);
    }

    exit(retval);

free_and_exit:
    if (_input_file != stdin) {
        fclose(_input_file);
    }
    exit(EXIT_FAILURE);
}    

