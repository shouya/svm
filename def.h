#ifndef __DEF_H__
#define __DEF_H__

#define BUF_SIZE 50
#define MAX_ARGC 8
#define CALLSTACK_SIZE 64
#define STACK_SIZE 64


#define FAILURE (-1)
#define SUCCESS 0


#define S_NOP 0
#define S_INST_NAME 1
#define S_INST_ARG_BEG 2
#define S_INST_ARG 3
#define S_COMMENT 4
#define S_UNKNOWN (-1)


#define T_REG (-1)
#define T_VAL (-2)


#define R_EAX (-1)
#define R_EBX (-2)
#define R_ECX (-3)
#define R_EDX (-4)
#define R_ESI (-5)
#define R_EDI (-6)
#define R_ESP (-7)
#define R_EBP (-8)
#define R_EIP (-9)
#define R_FLAG (-10)


#define LESS (-1)
#define GREAT (1)
#define EQUAL (0)


#endif /* __DEF_H__ */
