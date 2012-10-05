#include <stdlib.h>
#include <stdio.h>

#define regread(name) \
 unsigned int name;                    \
 asm ("movl %%"#name", %0"                \
 : "=r"(name)                              \
 :                                    \
 : "%"#name);

#define regwrite(name,val) { \
 unsigned int name = val;                    \
 asm ("movl %0, %%"#name                \
 :                             \
 : "r"(name)                                   \
 : "%"#name);                     \
}

struct ctx_s {
 void * stack_address;
 unsigned int ebp, esp;
};

typedef void (func_t)(void*); /* a function that returns an int from an int */

int init_ctx(struct ctx_s *ctx, int stack_size, func_t f, void *args);

//static void (*f_ptr)(void*) = NULL;
//static void * f_ptr;

int main();

int init_ctx(struct ctx_s *ctx, int stack_size, func_t f, void *args)
{
 //static func_t f_ptr;
 static void (*f_ptr)(void*) = NULL;
 static int old_esp, old_ebp;
 //f_ptr = f;
 f_ptr = f;
 //f_ptr = (void*) f;
 
 char * addr = (char*) malloc(stack_size);
 //regread(eip);
 /*addr[stack_size-4] = main+8;//(char*)main;//0x8048412;
 regread(ebp);
 addr[stack_size-8] = ebp;*/
 regread(esp);
 old_esp = esp;
 regread(ebp);
 old_ebp = ebp;
 regwrite(esp,addr+stack_size-8);
 regwrite(ebp,addr+stack_size-8);
 f_ptr(args);
 //((func_t)f_ptr)(args);
 //func_t ff = (func_t)f_ptr;
 //func_t * ff = &((func_t)f_ptr);
 regwrite(esp,old_esp);
 regwrite(ebp,old_ebp);
}

void f_ping(void * p)
{
 printf("ok!\n");
}

int main()
{
 struct ctx_s ctx_ping;
 
 init_ctx(&ctx_ping, 16384, f_ping, NULL);
 
}












