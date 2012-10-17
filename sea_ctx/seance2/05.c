#include <stdlib.h>
#include <stdio.h>
#include "../libhw/include/hw.h"

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
  struct ctx_s * next;//, * prev;
  int id;
};

struct ctx_s * current_ctx = NULL;

int create_ctx(int stack_size, func_t f, void *args);
void yield();

int create_ctx(int stack_size, func_t f, void *args)
{
  irq_disable();
  struct ctx_s * ctx = malloc(sizeof(struct ctx_s));
  if (current_ctx) {
    struct ctx_s * next = current_ctx->next;
    current_ctx->next = ctx;
    ctx->next = next;
  } else ctx->next = ctx;
  current_ctx = ctx;
  char * addr = (char*) malloc(stack_size);
  ctx->esp = (unsigned int)(addr+stack_size);
  ctx->ebp = ctx->esp;
  ctx->f_ptr = f;
  ctx->f_args = args;
  ctx->started = 0;
  static int nbp = 0;
  ctx->id = nbp++;
  irq_enable();
}

void start_sched()
{
  setup_irq(TIMER_IRQ,yield);
  start_hw();
  irq_enable();
}

void yield()
{
  irq_disable();
  //printf("-Yield: current id: %d, next id: %d-\n",current_ctx->id,current_ctx->next->id);
  //struct ctx_s * ctx = current_ctx;
  /*
  static int old_esp, old_ebp;
  */
  regread(esp);
  //old_esp = esp;
  regread(ebp);
  //old_ebp = ebp;
  current_ctx->esp = esp;
  current_ctx->ebp = ebp;
  current_ctx = current_ctx->next;
  regwrite(esp,current_ctx->esp);
  regwrite(ebp,current_ctx->ebp);
  if (current_ctx->started)
  {
    irq_enable();
    return;
  } else {
    current_ctx->started = 1;
    irq_enable();
    current_ctx->f_ptr(current_ctx->f_args);
    //for(;;) ;
    /*regwrite(esp,old_esp);
    regwrite(ebp,old_ebp);*/
  }
}

//struct ctx_s ctx_ping;
//struct ctx_s ctx_pong;
void f_ping(void *arg);
void f_pong(void *arg);

int main(int argc, char *argv[])
{
 create_ctx(16384, f_ping, NULL);
 create_ctx(16384, f_pong, NULL);
 start_sched();
 for(;;);
 exit(EXIT_SUCCESS);
}

void f_ping(void *args)
{
  while(1) {
    printf("A") ;
    printf("B") ;
    printf("C") ;
  }
}

void f_pong(void *args)
{
  while(1) {
    printf("1") ;
    printf("2") ;
  }
}


