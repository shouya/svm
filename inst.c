#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "inst.h"
#include "svm.h"

char* __inst_name[] = {
    "mov",
    "prn",
    "pchr"
};

instruction_t __inst_callback[] = {
    (instruction_t)&_inst_mov,
    (instruction_t)&_inst_prn,
    (instruction_t)&_inst_pchr,
    (instruction_t)&_inst_jmp

};
/*
 * r -- register
 * v -- register or a numeric lexical
 * j -- jump point
 */
char* __inst_arg[] = {
    "rv",
    "v",
    "v",
    "j"
};

int __inst_count = 4;



int _inst_mov(INST_ARG) {
    int* lhs;
    int rhs, rhst, rhsv;

    lhs = getregister(val_arr[0]);
    rhst = type_arr[1];
    rhsv = val_arr[1];

    rhs = getrvalue(rhst, rhsv);

    *lhs = rhs;

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


