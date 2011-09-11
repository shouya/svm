#ifndef __SVM_H__
#define __SVM_H__

#include "inst.h"

#define MAX_ARGC 10
#define BUF_SIZE 64
#define STACK_SIZE 32

#define SUCCESS (0)
#define FAILURE (-1)

#define JP (3) /* jump point */
#define VAL (2) /* lexical integer value */
#define REG (1) /* register */
#define UK (0) /* unknown */

#define EAX (-1)
#define EBX (-2)
#define ECX (-3)
#define EDX (-4)

#define ESP (-5)
#define EBP (-6)
#define EIP (-7)
#define FLAG (-8)

#define LESS (-1)
#define GREAT (1)
#define EQUAL (0)

int* getregister(int registername);
int getrvalue(int type, int value);
int getjumppoint(const char* name);
void pushcallstack(int value);
int popcallstack(void);
void pushstack(int value);
int popstack(void);
int stacktop(void);

int execfile(FILE* infile);
int execinst(const char* name, int argc, char* const* argv, FILE* srcfile);
int checkarg(const char* argtype, int argc, int* type_arr, int* val_arr);
void initregisters(void);

int main(int argc, char** argv);

#endif /* __SVM_H__ */
