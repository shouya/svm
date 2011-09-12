#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "inst.h"
#include "svm.h"
#include "def.h"


inst_t __inst[] = {
    {0, "mov", _inst_mov, "rv"},
    {1, "prn", _inst_prn, "v"},
    {2, "pchr", _inst_pchr, "v"},
    {3, "jmp", _inst_jmp, "j"},
/* math operation */
    {4, "add", _inst_add, "rv"},
    {5, "sub", _inst_sub, "rv"},
    {6, "mul", _inst_mul, "rv"},
    {7, "div", _inst_div, "rv"},
    {8, "mod", _inst_mod, "rv"},
    {9, "cmp", _inst_cmp, "vv"},
/* condition jump operationn */
    {10, "je", _inst_je, "j"},
    {11, "jne", _inst_jne, "j"},
    {12, "jg", _inst_jg, "j"},
    {13, "jge", _inst_jge, "j"},
    {14, "jl", _inst_jl, "j"},
    {15, "jle", _inst_jle, "j"},
/* binary operation */
    {16, "not", _inst_not, "r"},
    {17, "and", _inst_and, "rv"},
    {18, "or", _inst_or, "rv"},
    {19, "xor", _inst_xor, "rv"},
    {20, "shl", _inst_shl, "rv"},
    {21, "shr", _inst_shr, "rv"},
/* cont. math */
    {22, "inc", _inst_inc, "r"},
    {23, "dec", _inst_dec, "r"},
/* function call */
    {24, "call", _inst_call, "j"},
    {25, "ret", _inst_ret, ""},
/* stack operation */
    {26, "push", _inst_push, "v"},
    {27, "pop", _inst_pop, "r"},
    {28, "pushf", _inst_pushf, ""},
    {29, "popf", _inst_popf, "r"},
/* nop */
    {30, "nop", _inst_nop, ""}
};


int _inst_mov(INST_ARG) {
    *get_register(arg[0]) = get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}
 
int _inst_prn(INST_ARG) {
    printf("%d", get_rvalue(arg[0], arg[1]));
    return SUCCESS;
}

int _inst_pchr(INST_ARG) {
    putchar(get_rvalue(arg[0], arg[1]));
    return SUCCESS;
}

int _inst_jmp(INST_ARG) {
    *cur_ptr = *base_ptr + arg[0];
    return SUCCESS;
}

int _inst_add(INST_ARG) {
    *get_register(arg[0]) += get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}

int _inst_sub(INST_ARG) {
    *get_register(arg[0]) -= get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}

int _inst_mul(INST_ARG) {
    *get_register(arg[0]) *= get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}

int _inst_div(INST_ARG) {
    *get_register(arg[0]) /= get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}

int _inst_mod(INST_ARG) {
    *get_register(arg[0]) %= get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}

int _inst_cmp(INST_ARG) {
    int* flg = get_register(R_FLAG);
    int result = get_rvalue(arg[0], arg[1]) - get_rvalue(arg[2], arg[2]);

    if (result > 0) *flg = GREAT;
    else if (result < 0) *flg = LESS;
    else *flg = EQUAL;

    return SUCCESS;
}

int _inst_je(INST_ARG) {
    if (*get_register(R_FLAG) == EQUAL) {
        _inst_jmp(arg, base_ptr, cur_ptr);
    }
    return SUCCESS;
}
int _inst_jne(INST_ARG) {
    if (*get_register(R_FLAG) != EQUAL) {
        _inst_jmp(arg, base_ptr, cur_ptr);
    }
    return SUCCESS;
}
int _inst_jg(INST_ARG) {
    if (*get_register(R_FLAG) == GREAT) {
        _inst_jmp(arg, base_ptr, cur_ptr);
    }
    return SUCCESS;
}
int _inst_jge(INST_ARG) {
    if (*get_register(R_FLAG) == EQUAL || *get_register(R_FLAG) == GREAT) {
        _inst_jmp(arg, base_ptr, cur_ptr);
    }
    return SUCCESS;
}
int _inst_jl(INST_ARG) {
    if (*get_register(R_FLAG) == LESS) {
        _inst_jmp(arg, base_ptr, cur_ptr);
    }
    return SUCCESS;
}
int _inst_jle(INST_ARG) {
    if (*get_register(R_FLAG) == EQUAL || *get_register(R_FLAG) == LESS) {
        _inst_jmp(arg, base_ptr, cur_ptr);
    }
    return SUCCESS;
}
int _inst_not(INST_ARG) {
    int* val = get_register(arg[0]);
    *val = ~*val;
    return SUCCESS;
}
int _inst_and(INST_ARG) {
    *get_register(arg[0]) &= get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}
int _inst_or(INST_ARG) {
    *get_register(arg[0]) |= get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}
int _inst_xor(INST_ARG) {
    *get_register(arg[0]) ^= get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}
int _inst_shl(INST_ARG) {
    *get_register(arg[0]) <<= get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}
int _inst_shr(INST_ARG) {
    *get_register(arg[0]) >>= get_rvalue(arg[1], arg[2]);
    return SUCCESS;
}

int _inst_inc(INST_ARG) {
    ++*get_register(arg[0]);
    return SUCCESS;
}
int _inst_dec(INST_ARG) {
    --*get_register(arg[0]);
    return SUCCESS;
}
int _inst_call(INST_ARG) {
    push_callstack(cur_ptr - base_ptr);
    _inst_jmp(arg, base_ptr, cur_ptr);
    return SUCCESS;
}
int _inst_ret(INST_ARG) {
    int pos = pop_callstack();
    _inst_jmp(&pos, base_ptr, cur_ptr);
    return SUCCESS;
}
int _inst_push(INST_ARG) {
    push_stack(get_rvalue(arg[0], arg[1]));
    return SUCCESS;
}
int _inst_pop(INST_ARG) {
    *get_register(arg[0]) = pop_stack();
    return SUCCESS;
}
int _inst_pushf(INST_ARG) {
    push_stack(*get_register(R_FLAG));
    return SUCCESS;
}
int _inst_popf(INST_ARG) {
    *get_register(arg[0]) = *get_register(R_FLAG);
    return SUCCESS;
}
int _inst_nop(INST_ARG) {
    return SUCCESS;
}
