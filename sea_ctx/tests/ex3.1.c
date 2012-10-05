#include <stdio.h>
/*
int
main(void)
{
 int x = 10, y;
 asm ("movl %1, %%eax" "\n\t" "movl %%eax, %0"
 : "=r"(y) // y is output operand
 : "r"(x) // x is input operand
 : "%eax"); // %eax is a clobbered register
 printf("%d",y);
}
*/
/*
#define reg(name) { \
 unsigned int y;                    \
 asm ("movl %%"#name", %0"                \
 : "=r"(y)                              \
 :                                    \
 : "%"#name);                           \
 printf(""#name": %u\n",y); \
}*/
#define reg(name) \
 unsigned int name;                    \
 asm ("movl %%"#name", %0"                \
 : "=r"(name)                              \
 :                                    \
 : "%"#name);                           \
 printf(""#name": %u\n",name); \


void bar(int x, int y, int z) {
 printf("lol\n");
 printf("&x:  %u\n",&x);
 printf("&z:  %u\n",&z);
 reg(ebp);
 reg(esp);
}

void foo(int p) {
 int x;
 reg(ebp);
 reg(esp);
 printf("&x:  %u\n",&x);
 printf("&p:  %u\n",&p);
 //unsigned int y;
 
 x++;
 printf("prout\n");
 
}

int
main(void)
{
 //printf(reg(test));
 foo(42);
 bar(42,0,0);
}




