#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "svm.h"

#define arrlen(arr) (sizeof(arr)/sizeof(*arr))

int __eax, __ebx, __ecx, __edx;
int __esp, __ebp;

int __register_list[] = {
    EAX, EBX, ECX, EDX, ESP, EBP
};

int* __register_address_list[] = {
    &__eax, &__ebx, &__ecx, &__edx,
    &__esp, &__ebp
};

char** __jump_point_name_list = NULL;
int* __jump_point_position_list = NULL;
int __jump_point_count = 0;


static const char* l_getname(int val);
static int l_stricmp(const char* s1, const char* s2);
static void reset_name(char* name, int* length);
static void reset_argument(int* argc, char** argv, int* argvlen);
static void l_parse_jmppoint(FILE* infile);

/*static*/ const char* l_getname(int val) {
    switch (val) {
    case VAL:
        return "VAL";
    case REG:
        return "REG";
    case JP:
        return "JP";
    case UK:
        return "UK";
    case EAX:
        return "EAX";
    case EBX:
        return "EBX";
    case ECX:
        return "ECX";
    case EDX:
        return "EDX";
    case ESP:
        return "ESP";
    case EBP:
        return "EBP";
    default:
        fprintf(stderr, "error: get name of %d error, no such symbol name.\n",
                val);
        return "Error";
    }
}

/*static*/ int l_stricmp(const char* s1, const char* s2) {
    while ((tolower(*s1++) == tolower(*s2++)) &&
           (*s1 != '\0') &&
           (*s2 != '\0'));
    return (tolower(*s1) - tolower(*s2));
}


int* getregister(int registername) {
    switch (registername) {
    case EAX:
        return &__eax;
    case EBX:
        return &__ebx;
    case ECX:
        return &__ecx;
    case EDX:
        return &__edx;
    case ESP:
        return &__esp;
    case EBP:
        return &__ebp;
    default:
        fprintf(stderr, "no such regisppter: %d\n", registername);
        return NULL;
    }
}

int getrvalue(int type, int value) {
    if (type == VAL) {
        return value;
    } else if (type == REG) {
        switch (value) {
        case EAX:
            return __eax;
        case EBX:
            return __ebx;
        case ECX:
            return __ecx;
        case EDX:
            return __edx;
        case ESP:
            return __esp;
        case EBP:
            return __ebp;
        default:
            fprintf(stderr, "no such register: %d\n", value);
            return 0;
        }
    } else {
        fprintf(stderr, "no such type: %d\n", type);
        return 0;
    }
}

void initregisters(void) {
    int i = 0;
    for (; i != arrlen(__register_address_list); ++i) {

        *__register_address_list[i] = 0;
    }
}

int parseargument(const char* argument, int* arg_type, int* arg_val) {
    int len;
    int i = 0;
    int _numeric_flag = 1;

    *arg_type = UK;

    /* right trim */
    len = strlen(argument);
    for (;;) {
        if (isspace(argument[len])) {
            ((char*)argument)[len--] = '\0';
        } else {
            break;
        }
    }

    /* check if == register name */
    for (; i != arrlen(__register_list); ++i) {
        if (!l_stricmp(l_getname(__register_list[i]), argument)) {
            *arg_type = REG;
            *arg_val = __register_list[i];
            return SUCCESS;
        }
    }

    /* check if == jump point name */
    for (i = 0; i != __jump_point_count; ++i) {
        if (!l_stricmp(__jump_point_name_list[i], argument)) {
            *arg_type = JP;
            *arg_val = __jump_point_position_list[i];
            return SUCCESS;
        }
    }

    /* check if == numeric */
    i = (argument[0] == '-' ? 1 : 0);
    for (; i != strlen(argument); ++i) {
        if (!isdigit(argument[i])) {
            _numeric_flag = 0;
            break;
        }
    }
    if (_numeric_flag) {
        *arg_type = VAL;
        *arg_val = atol(argument);
        return SUCCESS;
    }

    return FAILURE;
}

int execinst(const char* name, int argc, char* const* argv, FILE* srcfile) {
    int func_no = 0;
    instruction_t func = NULL;
    int* type_arr, *val_arr;
    int argi = 0;

    for (; func_no != __inst_count; ++func_no) {
        if (!l_stricmp(name, __inst_name[func_no])) {
            func = __inst_callback[func_no];
            break;
        }
    }
    if (func == NULL) {
        fputs("no match function name found.]n", stderr);
        return FAILURE;
    }

    if (argc > 0 && argc != strlen(__inst_arg[func_no])) {
        fputs("argument count not matched.\n", stderr);
        return FAILURE;
    }


    val_arr = (int*)malloc(sizeof(int) * argc);
    type_arr = (int*)malloc(sizeof(int) * argc);
    
/*
    if (__inst_arg_count[func_no] != -1) {
    } else {
        / * TODO * /
        fputs("incomplete function: variable argument instruction.\n", stderr);
        return FAILURE;
    }
*/
    for (; argi != argc; ++argi) {
        parseargument(argv[argi], &type_arr[argi], &val_arr[argi]);
    }

    if (checkarg(__inst_arg[func_no], argc,
                 type_arr, val_arr) != SUCCESS) {
        fputs("check argument failed\n", stderr);
        free(type_arr);
        free(val_arr);
        return FAILURE;
    }
    
    (*func)(argc, type_arr, val_arr, srcfile, 0);

    free(val_arr);
    free(type_arr);

    return SUCCESS;
    /* TODO: how to call function with uncertain arguments */
}

/* read state machine */
#define S_NOP 0
#define S_INST_NAME 1
#define S_INST_ARG 2
#define S_INST_ARG_BEG 3
#define S_COMMENT 4

/*static*/ void reset_name(char* name, int* length) {
    name[0] = '\0';
    *length = 0;
}
/*static*/ void reset_argument(int* argc, char** argv, int* argvlen) {
    int i = 0;
    for (; i != *argc; ++i) {
        if (argv[i] != NULL) {
            free(argv[i]);
            argv[i] = NULL;
        }
    }
    *argc = *argvlen = 0;
}

int execfile(FILE* infile) {
    int chr = 0;
    int _line = 1;

    char* _inst_name = malloc(BUF_SIZE);
    int _inst_name_len = 0;
    int _inst_argc = 0;
    char* _inst_argv[MAX_ARGC] = {NULL};
    int _inst_argv_len = 0;

    char _state = S_NOP;

    fseek(infile, 0, SEEK_SET);
    for (;;) {
        chr = fgetc(infile);

        if (isalnum(chr)) {
            switch (_state) {
            case S_NOP: /* name from new line */
                _state = S_INST_NAME;
                _inst_name[0] = chr;
                _inst_name_len = 1;
                break;
            case S_INST_NAME: /* store new char into name buffer */
                if (_inst_name_len == BUF_SIZE - 1) {
                    fprintf(stderr, "instruction name too long at line: %d\n", _line);
                    free(_inst_name);
                    return FAILURE;
                }
                _inst_name[_inst_name_len++] = chr;
                break;
            case S_INST_ARG: /* store new char into argument buffer */
                if (_inst_argv_len == BUF_SIZE - 1) {
                    fprintf(stderr, "instruction argument value too long at line: %d\n", _line);
                    reset_argument(&_inst_argc, _inst_argv, &_inst_argv_len);
                    free(_inst_name);
                    return FAILURE;
                }
                _inst_argv[_inst_argc - 1][_inst_argv_len++] = chr;
                break;
            case S_INST_ARG_BEG: /* new argument */
                _state = S_INST_ARG;
                if (_inst_argc == MAX_ARGC) {
                    fprintf(stderr, "to many arguments at line: %d\n", _line);
                    reset_argument(&_inst_argc, _inst_argv, &_inst_argv_len);
                    free(_inst_name);
                    return FAILURE;
                }
                _inst_argv[_inst_argc++] = malloc(BUF_SIZE);
                _inst_argv[_inst_argc - 1][0] = chr;
                _inst_argv_len = 1;
                break;
            case S_COMMENT:
                break;
            }
        } else if (chr == '\n') {
            switch (_state) {
            case S_NOP:
                break;
            case S_INST_NAME: /* execute simple instruction, such as ret */
                _state = S_NOP;
                _inst_name[_inst_name_len] = '\0';
                execinst(_inst_name, 0 /*argc*/, NULL /*argv*/, infile);
                reset_name(_inst_name, &_inst_name_len);
                break;
            case S_INST_ARG:
                _state = S_NOP;
                _inst_argv[_inst_argc - 1][_inst_argv_len] = '\0';
                execinst(_inst_name, _inst_argc, _inst_argv, infile);
                reset_name(_inst_name, &_inst_name_len);
                reset_argument(&_inst_argc, _inst_argv, &_inst_argv_len);
                break;
            case S_COMMENT:
                _state = S_NOP;
                break;
            case S_INST_ARG_BEG:
                _state = S_NOP;
                if (_inst_argc > 0) { /* such as [mov eax,\n] */
                    fprintf(stderr, "syntax error at line %d\n", _line);
                    reset_argument(&_inst_argc, _inst_argv, &_inst_argv_len);
                    free(_inst_name);
                    return FAILURE;
                }
                /* such as [nop \n] */
                execinst(_inst_name, 0 /*argc*/, NULL /*argv*/, infile);
                reset_name(_inst_name, &_inst_name_len);
                break;
            }
            ++_line;
        } else if (isspace(chr)) {
            switch (_state) {
            case S_NOP:
                break;
            case S_INST_NAME:
                _state = S_INST_ARG_BEG;
                _inst_name[_inst_name_len] = '\0';
                break;
            case S_INST_ARG: /* cont. store, such as [mov eax, dword|123] */
                if (_inst_argv_len == BUF_SIZE - 1) {
                    fprintf(stderr, "instruction argument value too long at line: %d\n", _line);
                    reset_argument(&_inst_argc, _inst_argv, &_inst_argv_len);
                    free(_inst_name);
                    return FAILURE;
                }
                _inst_argv[_inst_argc - 1][_inst_argv_len++] = chr;
                break;
            case S_COMMENT:
            case S_INST_ARG_BEG: /* cont, such as [mov eax,|ebx]*/
                break;
            }
        } else if (chr == ',') {
            switch (_state) {
            case S_NOP: /* [  , ] */
                fprintf(stderr, "syntax error at line %d\n", _line);
                free(_inst_name);
                return FAILURE;
            case S_INST_NAME: /* [mov,] */
                fprintf(stderr, "syntax error at line %d\n", _line);
                free(_inst_name);
                return FAILURE;
            case S_INST_ARG: /* [mov eax,] */
                _state = S_INST_ARG_BEG;
                _inst_argv[_inst_argc - 1][_inst_argv_len] = '\0';
                break;p
            case S_COMMENT:
                break;
            case S_INST_ARG_BEG: /* [mov ,] or [mov eax,,] */
                fprintf(stderr, "syntax error at line %d\n", _line);
                free(_inst_name);
                reset_argument(&_inst_argc, _inst_argv, &_inst_argv_len);
                return FAILURE;
            }
        } else if (chr == ';') {
            switch (_state) {
            case S_NOP:
                _state = S_COMMENT;
                break;
            case S_INST_NAME: /* [nop; some comments] */
                _state = S_COMMENT;
                _inst_name[_inst_name_len] = '\0';
                execinst(_inst_name, 0 /*argc*/, NULL/*argv*/, infile);
                reset_name(_inst_name, &_inst_name_len);
                break;
            case S_INST_ARG:
                _state = S_COMMENT;
                _inst_argv[_inst_argc - 1][_inst_argv_len] = '\0';
                execinst(_inst_name, _inst_argc, _inst_argv, infile);
                reset_name(_inst_name, &_inst_name_len);
                reset_argument(&_inst_argc, _inst_argv, &_inst_argv_len);
                break;
            case S_COMMENT:
                break;
            case S_INST_ARG_BEG:
                _state = S_COMMENT;
                if (_inst_argc > 0) { /* such as [mov eax,;] */
                    fprintf(stderr, "syntax error at line %d\n", _line);
                    free(_inst_name);
                    return FAILURE;
                }
                /* such as [nop ;] */
                execinst(_inst_name, 0 /*argc*/, NULL /*argv*/, infile);
                reset_name(_inst_name, &_inst_name_len);
                break;
            }
        } else if (chr == EOF) {
            switch (_state) {
            case S_NOP:
                break;
            case S_INST_NAME: /* [nop; some comments] */
                _inst_name[_inst_name_len] = '\0';
                execinst(_inst_name, 0 /*argc*/, NULL/*argv*/, infile);
                reset_name(_inst_name, &_inst_name_len);
                break;
            case S_INST_ARG:
                _inst_argv[_inst_argc - 1][_inst_argv_len] = '\0';
                execinst(_inst_name, _inst_argc, _inst_argv, infile);
                reset_name(_inst_name, &_inst_name_len);
                reset_argument(&_inst_argc, _inst_argv, &_inst_argv_len);
                break;
            case S_COMMENT:
                break;
            case S_INST_ARG_BEG:
                if (_inst_argc > 0) {
                    fprintf(stderr, "syntax error at line %d\n", _line);
                    free(_inst_name);
                    return FAILURE;
                }
                execinst(_inst_name, 0 /*argc*/, NULL /*argv*/, infile);
                reset_name(_inst_name, &_inst_name_len);
                break;
            }
            free(_inst_name);
            return SUCCESS;
        } else if (chr == ':') {
            switch (_state) {
            case S_INST_NAME:
                _state = S_NOP;
                reset_name(_inst_name, &_inst_name_len);
                break;
            case S_INST_ARG:
            case S_INST_ARG_BEG:
            case S_NOP:
                fprintf(stderr, "syntax error at line %d\n", _line);
                free(_inst_name);
                return FAILURE;
            case S_COMMENT:
                break;
            }
        } else {
            if (_state != S_COMMENT) {
                fprintf(stderr, "unknown character: %d[%c]", chr, chr);
            }
        }
    }
}

void l_parse_jmppoint(FILE* infile) {
    int chr = 0;
    char* _inst_name = malloc(BUF_SIZE);
    int _inst_name_len = 0;
    int _state = S_NOP;

    fseek(infile, 0, SEEK_SET);
    for (;;) {
        chr = fgetc(infile);

        if (isalnum(chr)) {
            switch (_state) {
            case S_NOP: /* name from new line */
                _state = S_INST_NAME;
                _inst_name[0] = chr;
                _inst_name_len = 1;
                break;
            case S_INST_NAME: /* store new char into name buffer */
                _inst_name[_inst_name_len++] = chr;
                break;
            }
        } else if (chr == ':') {
            switch (_state) {
            case S_INST_NAME:
                _state = S_NOP;
                _inst_name[_inst_name_len] = '\0';
                __jump_point_name_list =
                    realloc(__jump_point_name_list,
                            (__jump_point_count + 1) * sizeof(int));
                __jump_point_name_list[__jump_point_count] =
                    malloc(strlen(_inst_name) + 1);
                strcpy(__jump_point_name_list[__jump_point_count],
                       _inst_name);
                __jump_point_position_list =
                    realloc(__jump_point_position_list,
                            (__jump_point_count + 1) * sizeof(int));
                __jump_point_position_list[__jump_point_count] =
                    ftell(infile);
                ++__jump_point_count;

                _inst_name_len = 0;
            } /* switch */
        } else if (chr == EOF) {
            break;
        } /* else if EOF */
    } /* for (;;) */
}

int getjumppoint(const char* name) {
    int i = 0;
    for (; i != __jump_point_count; ++i) {
        if (!l_stricmp(__jump_point_name_list[i], name)) {
            return __jump_point_position_list[i];
        }
    }
    return 0; /* no one available jump point will at position 0 */
}

int checkarg(const char* argtype, int argc, int* type_arr, int* val_arr) {
    int i = 0, len = strlen(argtype);

    for (; i != len; ++i) {
        switch (argtype[i]) {
        case 'r': /* register */
            if (type_arr[i] != REG) {
                fprintf(stderr,
                  "argument %d require for register type but gave a %s type\n",
                   i, l_getname(type_arr[i]));
                return FAILURE;
            }
            break;
        case 'v': /* value */
            if (type_arr[i] != REG && type_arr[i] != VAL) {
                fprintf(stderr,
                    "argument %d require for value type but gave a %s type\n",
                     i, l_getname(type_arr[i]));
                return FAILURE;
            }
            break;
        case 'j': /* jump point */
            if (type_arr[i] != JP) {
                fprintf(stderr,
                 "argument %d require for jump point type but gave a %s type\n",
                 i, l_getname(type_arr[i]));
                return FAILURE;
            }
            break;
        default:
            fprintf(stderr,
                    "unkown type specription: [%c]",
                    argtype[i]);
            return FAILURE;
        } /* switch */
    } /* for */

    return SUCCESS;
}



int main(int argc, char** argv) {
    static FILE* _input_file;
    int retval = 0;


    initregisters();
    if (argc > 1) {
        _input_file = fopen(argv[1], "r");
        if (!_input_file) {
            fprintf(stderr, "open file failed: [%s]", argv[1]);
        }
    } else {
        _input_file = stdin;
    }


    l_parse_jmppoint(_input_file);
    retval = execfile(_input_file);


    if (_input_file != stdin) {
        fclose(_input_file);
    }

    exit(retval);
}