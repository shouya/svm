#ifndef __INST_H__
#define __INST_H__

#define INST_ARG int argc, int* type_arr, int* val_arr,\
                 FILE* src_file, int* reserve

typedef int (*instruction_t)(INST_ARG);
    

int _inst_mov(INST_ARG);
int _inst_prn(INST_ARG);
int _inst_pchr(INST_ARG);
int _inst_jmp(INST_ARG);

/* modify following 3 arrays and the count variable to add/del instructions */
/* in the source file */
extern char* __inst_name[];
extern instruction_t __inst_callback[];
extern char* __inst_arg[];
extern int __inst_count;


#endif /* __INST_H__ */
