#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "svm.h"

jmplbl_t* __jmp_tbl;
int __jmp_tbl_size = 0;
int __program_len = 0;

int arg_len(int instno) {
    int i = 0;
    int arglen = strlen(__inst[instno].argument);
    int len = 0;
    
    for (; i != arglen; ++i) {
        if (__inst[instno].argument[i] == 'r') {
            len += 2;
        } else {
            ++len;
        }
    }
    return len;
}

int inst_len(const char* name) {
    int i = 0, j = 0, arglen, len;
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
                
                _inst_name[0] = '\0';
                _inst_name_len = 0;
                break;
            case S_NOP:
            case S_UNKNOWN:
                break;
            } /* switch */
        } else if (chr == EOF) {
            break;
        } else if (chr != '\n') {
            _state = S_NOP;
        } /* if */
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
    switch (*state) {
    case S_NOP:
        break;
    case S_INST_NAME: /* execute simple instruction, such as ret */
        *state = S_NOP;
        name[*name_len] = '\0';
        /* execinst(_inst_name, 0, NULL, infile); */
        *pos += translateinst(name, 0, NULL, &out_bin[*pos]);
        /* always reserve a BUF_SIZE space to new instructions */
        reset_name(name, name_len);
        break;
    case S_INST_ARG:
        *state = S_NOP;
        argv[*argc - 1][*argv_len] = '\0';
        *pos += translateinst(name, *argc, argv, &out_bin[*pos]);
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
        *pos += translateinst(name, 0, NULL, &out_bin[*pos]);
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
}
int sm_colon(SM_ARGS) {
    switch (*state) {
    case S_INST_NAME:
        *state = S_NOP;
        reset_name(name, &name_len);
        break;
    case S_INST_ARG:
    case S_INST_ARG_BEG:
    case S_NOP:
        fprintf(stderr, "syntax error at line %d\n", _line);
        return FAILURE;
    case S_COMMENT:
        break;
    }
    return SUCCESS;
}
int sm_semicolon(SM_ARGS) {
    switch (*state) {
    case S_NOP:
        *state = S_COMMENT;
        break;
    case S_INST_NAME: /* [nop; some comments] */
        *state = S_COMMENT;
        name[*name_len] = '\0';
        *pos += translateinst(name, 0, NULL, &out_bin[*pos]);
        reset_name(name, name_len);
        break;
    case S_INST_ARG:
        *state = S_COMMENT;
        argv[*argc - 1][*argv_len] = '\0';
        *pos += translateinst(name, *argc, argv, &out_bin[*pos]);
        reset_name(name, &name_len);
        reset_argument(argc, argv, argv_len);
        break;
    case S_COMMENT:
        break;
    case S_INST_ARG_BEG:
        *state = S_COMMENT;
        if (*argc > 0) { /* such as [mov eax,;] */
            fprintf(stderr, "syntax error at line %d\n", _line);
            return FAILURE;
        }
        /* such as [nop ;] */
        *pos += translateinst(name, 0, NULL, &out_bin[*pos]);
        reset_name(name, name_len);
        break;
    }
    return SUCCESS;
}
int sm_eof(SM_ARGS) {
    switch (*state) {
    case S_NOP:
        *state = S_COMMENT;
        break;
    case S_INST_NAME: /* [nop; some comments] */
        *state = S_COMMENT;
        name[*name_len] = '\0';
        *pos += translateinst(name, 0, NULL, &out_bin[*pos]);
        reset_name(name, name_len);
        break;
    case S_INST_ARG:
        *state = S_COMMENT;
        argv[*argc - 1][*argv_len] = '\0';
        *pos += translateinst(name, *argc, argv, &out_bin[*pos]);
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
        *pos += translateinst(name, 0, NULL, &out_bin[*pos]);
        reset_name(name, name_len);
        break;
    }
    return SUCCESS;
}

int parse_file(FILE* infile, char** output) {
    int _chr = 0;
    int _line = 1;

    char* _name = malloc(BUF_SIZE);
    int _name_len = 0;
    int _argc = 0;
    char* _argv[MAX_ARGC] = {NULL};
    int _argv_len = 0;

    int _pos = 0;
    int* _bin = malloc(__program_len * sizeof(int));


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
        } else if (isspace(_chr)) {
            CALL_SM(sm_space);
        } else if (_chr == ',') {
            CALL_SM(sm_comma);
        } else if (_chr == ';') {
            CALL_SM(sm_semicolon);
        } else if (_chr == EOF) {
            CALL_SM(sm_eof);
        } else if (_chr == ':') {
            CALL_SM(sm_colon);
        } else {
            if (_state != S_COMMENT) {
                fprintf(stderr, "unknown character: %d[%c]", chr, chr);
            } /* if */
        } /* else */
#undef CALL_SM
    } /* for */

    return SUCCESS;

free_and_return:
    free(_name);
    reset_argument(&_argc, _argv, &_argv_len);
    return FAILURE;
}

int exec_binary(int* bin, int len) {
    instruction_t _inst;
    int _args[MAX_ARGC*2];
    int* _p = bin;
    int i, _arglen;

    for (;;) {
        _inst = __inst[*_p].callback; /* inst no */
        _arglen = arg_len(*_p++); /* inst no -> arg */
        for (i = 0; i != _arglen; ++i) {
            _args[i] = *_p++;
        }
        _inst(_args, &bin, &_p);
    }
}


int main(int argc, char** argv) {
    static FILE* _input_file;
    static int* _binary;
    int retval = 0;

    if (argc > 1) {
        _input_file = fopen(argv[1], "r");
        if (!_input_file) {
            fprintf(stderr, "open file failed: [%s]", argv[1]);
        }
    } else {
        _input_file = stdin;
    }

    initregisters();

    if (parse_jmplbl(_input_file) == FAILURE) {
        fputs("error at parsing jump labels\n", stderr);
        goto free_and_exit;
    }
    if (parse_file(_input_file, &_binary) == FAILURE) {
        fputs("error at parsing file\n", stderr);
        goto free_and_exit;
    }
    if (exec_binary(_binary, __program_len) == FAILURE) {
        fputs("error at executing\n", stderr);
        goto free_and_exit;
    }

/*    retval = execfile(_input_file);*/

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

