#ifndef __INST_H__
#define __INST_H__

#define INST_ARG int argc, int* type_arr, int* val_arr,\
                 FILE* src_file, int* reserve
#define DECL_INST(name) int _inst_##name(INST_ARG);

typedef int (*instruction_t)(INST_ARG);

typedef struct inst_t {
    int instno;
    const char* name;
    instruction_t callback;
    /*
     * r -- register
     * v -- register or a numeric lexical
     * j -- jump point
     */
    const char* argument;
} inst_t;

extern inst_t __inst[];

#define INST_COUNT 31 /*(sizeof(__inst)/sizeof(*__inst))*/

DECL_INST(mov);
DECL_INST(prn);
DECL_INST(pchr);
DECL_INST(jmp);
DECL_INST(add);
DECL_INST(sub);
DECL_INST(mul);
DECL_INST(div);
DECL_INST(mod);
DECL_INST(cmp);
DECL_INST(je);
DECL_INST(jne);
DECL_INST(jg);
DECL_INST(jge);
DECL_INST(jl);
DECL_INST(jle);
DECL_INST(not);
DECL_INST(and);
DECL_INST(or);
DECL_INST(xor);
DECL_INST(shl);
DECL_INST(shr);
DECL_INST(inc);
DECL_INST(dec);
DECL_INST(call);
DECL_INST(ret);
DECL_INST(push);
DECL_INST(pop);
DECL_INST(pushf);
DECL_INST(popf);
DECL_INST(nop);

#endif /* __INST_H__ */
