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

typedef void (func_t)(void*); /* a function that returns an int from an int */

struct ctx_s {
 //void * stack_address;
 unsigned int  ebp, esp;
 void (*f_ptr)(void*);
 void * f_args;
 int started;
};

struct ctx_s * current_ctx = NULL;

int init_ctx(struct ctx_s *ctx, int stack_size, func_t f, void *args);
void switch_to_ctx(struct ctx_s *ctx);

int init_ctx(struct ctx_s *ctx, int stack_size, func_t f, void *args)
{
 char * addr = (char*) malloc(stack_size);
 ctx->esp = (unsigned int)(addr+stack_size-8);
 ctx->ebp = ctx->esp;
 ctx->f_ptr = f;
 ctx->f_args = args;
 ctx->started = 0;
}

void switch_to_ctx(struct ctx_s *ctx)
{
  //printf("-switch-\n") ;
  static void (*f_ptr)(void*) = NULL;
  f_ptr = ctx->f_ptr;
  /*
  static int old_esp, old_ebp;
  */
  regread(esp);
  //old_esp = esp;
  regread(ebp);
  //old_ebp = ebp;
  if (current_ctx) {
    current_ctx->esp = esp;
    current_ctx->ebp = ebp;
  }
  static struct ctx_s * _ctx;
  _ctx = ctx;
  current_ctx = ctx;
  regwrite(esp,ctx->esp);
  regwrite(ebp,ctx->ebp);
  
  if (_ctx->started)
  {
    return;
  } else {
    _ctx->started = 1;
    f_ptr(_ctx->f_args);
    //for(;;) ;
    /*regwrite(esp,old_esp);
    regwrite(ebp,old_ebp);*/
}
}

struct ctx_s ctx_ping;
struct ctx_s ctx_pong;
void f_ping(void *arg);
void f_pong(void *arg);

int main(int argc, char *argv[])
{
 init_ctx(&ctx_ping, 16384, f_ping, NULL);
 init_ctx(&ctx_pong, 16384, f_pong, NULL);
 switch_to_ctx(&ctx_ping);
 exit(EXIT_SUCCESS);
}

void f_ping(void *args)
{
 while(1) {
  printf("A") ;
  switch_to_ctx(&ctx_pong);
  printf("B") ;
  switch_to_ctx(&ctx_pong);
  printf("C") ;
  switch_to_ctx(&ctx_pong);
 }
}

void f_pong(void *args)
{
 while(1) {
  printf("1") ;
  switch_to_ctx(&ctx_ping);
  printf("2") ;
  switch_to_ctx(&ctx_ping);
 }
}


