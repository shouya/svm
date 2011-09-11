#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "inst.h"
#include "svm.h"


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
    {21, "shr", _inst_shr, "rv"}
};


int _inst_mov(INST_ARG) {
    int* arg0, arg1;

    arg0 = getregister(val_arr[0]);
    arg1 = getrvalue(type_arr[1], val_arr[1]);

    *arg0 = arg1;

    return SUCCESS;
}
 
int _inst_prn(INST_ARG) {
    int vt, vv, v;

    vt = type_arr[0];
    vv = val_arr[0];

    v = getrvalue(vt, vv);

    printf("%d", v);

    return SUCCESS;
}

int _inst_pchr(INST_ARG) {
    int vt, vv, v;

    vt = type_arr[0];
    vv = val_arr[0];

    v = getrvalue(vt, vv);

    printf("%c", v);

    return SUCCESS;
}

int _inst_jmp(INST_ARG) {
    int target = val_arr[0];
    if (target == 0) {
        fprintf(stderr, "no such jump point.\n");
        return FAILURE;
    }
    fseek(src_file, target, SEEK_SET);
    return SUCCESS;
}

int _inst_add(INST_ARG) {
    int* arg0, arg1;
    arg0 = getregister(val_arr[0]);
    arg1 = getrvalue(type_arr[1], val_arr[1]);

    *arg0 += arg1;
    return SUCCESS;
}

int _inst_sub(INST_ARG) {
    int* arg0, arg1;
    arg0 = getregister(val_arr[0]);
    arg1 = getrvalue(type_arr[1], val_arr[1]);
    
    *arg0 -= arg1;
    return SUCCESS;
}

int _inst_mul(INST_ARG) {
    int* arg0, arg1;
    arg0 = getregister(val_arr[0]);
    arg1 = getrvalue(type_arr[1], val_arr[1]);
    
    *arg0 *= arg1;
    return SUCCESS;
}

int _inst_div(INST_ARG) {
    int* arg0, arg1;
    arg0 = getregister(val_arr[0]);
    arg1 = getrvalue(type_arr[1], val_arr[1]);
    
    *arg0 /= arg1;
    return SUCCESS;
}

int _inst_mod(INST_ARG) {
    int* arg0, arg1;
    arg0 = getregister(val_arr[0]);
    arg1 = getrvalue(type_arr[1], val_arr[1]);

    *arg0 %= arg1;
    return SUCCESS;
}

int _inst_cmp(INST_ARG) {
    int* flg = getregister(FLAG);
    int arg0, arg1;

    arg0 = getrvalue(type_arr[0], val_arr[0]);
    arg1 = getrvalue(type_arr[1], val_arr[1]);

    if (arg0 < arg1) *flg = LESS;
    else if (arg0 > arg1) *flg = GREAT;
    else *flg = EQUAL;

    return SUCCESS;
}

int _inst_je(INST_ARG) {
    if (*getregister(FLAG) == EQUAL) {
        fseek(src_file, val_arr[0], SEEK_SET);
    }
    return SUCCESS;
}
int _inst_jne(INST_ARG) {
    if (*getregister(FLAG) != EQUAL) {
        fseek(src_file, val_arr[0], SEEK_SET);
    }
    return SUCCESS;
}
int _inst_jg(INST_ARG) {
    if (*getregister(FLAG) == GREAT) {
        fseek(src_file, val_arr[0], SEEK_SET);
    }
    return SUCCESS;
}
int _inst_jge(INST_ARG) {
    if (*getregister(FLAG) == EQUAL || *getregister(FLAG) == GREAT) {
        fseek(src_file, val_arr[0], SEEK_SET);
    }
    return SUCCESS;
}
int _inst_jl(INST_ARG) {
    if (*getregister(FLAG) == LESS) {
        fseek(src_file, val_arr[0], SEEK_SET);
    }
    return SUCCESS;
}
int _inst_jle(INST_ARG) {
    if (*getregister(FLAG) == EQUAL || *getregister(FLAG) == LESS) {
        fseek(src_file, val_arr[0], SEEK_SET);
    }
    return SUCCESS;
}
int _inst_not(INST_ARG) {
    int* val = getregister(val_arr[0]);
    *val = ~*val;
    return SUCCESS;
}
int _inst_and(INST_ARG) {
    *getregister(val_arr[0]) &= getrvalue(type_arr[1], val_arr[1]);
    return SUCCESS;
}
int _inst_or(INST_ARG) {
    *getregister(val_arr[0]) |= getrvalue(type_arr[1], val_arr[1]);
    return SUCCESS;
}
int _inst_xor(INST_ARG) {
    *getregister(val_arr[0]) ^= getrvalue(type_arr[1], val_arr[1]);
    return SUCCESS;
}
int _inst_shl(INST_ARG) {
    *getregister(val_arr[0]) <<= getrvalue(type_arr[1], val_arr[1]);
    return SUCCESS;
}
int _inst_shr(INST_ARG) {
    *getregister(val_arr[0]) >>= getrvalue(type_arr[1], val_arr[1]);
    return SUCCESS;
}
