#ifndef __SVM_H__
#define __SVM_H__

#define MAX_ARGC 10
#define BUF_SIZE 64

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


#include "inst.h"


int* getregister(int registername);
int getrvalue(int type, int value);
int getjumppoint(const char* name);

int execfile(FILE* infile);
int execinst(const char* name, int argc, char* const* argv, FILE* srcfile);
int checkarg(const char* argtype, int argc, int* type_arr, int* val_arr);
void initregisters(void);

int main(int argc, char** argv);

#endif /* __SVM_H__ */
