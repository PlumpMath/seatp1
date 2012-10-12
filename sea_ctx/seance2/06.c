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
  struct ctx_s * next, * prev;
  int id;
  struct ctx_s * next_waiting;
};

struct ctx_s * current_ctx = NULL;

struct sem_s {
  int  count;
  struct ctx_s *first_waiting, *last_waiting;
};

void disp_procs()
{
  printf("procs: ");
  struct ctx_s * ctx = current_ctx;
  do {
    printf(" %d ",ctx->id);
    ctx = ctx->next;
  } while(ctx != current_ctx);
  printf("\n");
}

int create_ctx(int stack_size, func_t f, void *args);
void yield();

void sem_init(struct sem_s * sem, unsigned int val)
{
  sem->count = val;
  sem->first_waiting = NULL;
  sem->last_waiting = NULL;
}

void sem_up(struct sem_s * sem)
{
  irq_disable();
  //printf("-SemUp: %d (cid: %d)\n",sem->count+1,current_ctx->id);
  //++sem->count;
  if (++sem->count <= 0)
  {
    struct ctx_s * ctx = sem->first_waiting;
    struct ctx_s * next = current_ctx->next;
    struct ctx_s * prev = current_ctx;
    prev->next = ctx;
    next->prev = ctx;
    ctx->next = next;
    ctx->prev = prev;
    //current_ctx = sem->first_waiting;
    sem->first_waiting = sem->first_waiting->next_waiting;
    if (!sem->first_waiting) sem->last_waiting = NULL;
    //yield();
  }
  irq_enable();
}

void sem_down(struct sem_s * sem)
{
  irq_disable();
  //printf("-SemDown: %d (cid: %d)\n",sem->count-1,current_ctx->id);
  if (--sem->count < 0)
  {
    if (current_ctx->next == current_ctx)
    {
      fprintf(stderr,"DEADLOCK!!! Exiting\n");
      exit(-1);
    }
    struct ctx_s * ctx = current_ctx;
    current_ctx->prev->next = current_ctx->next;
    current_ctx->next->prev = current_ctx->prev;
    ctx->next_waiting = NULL;
    //if (sem->last_waiting)
    if (sem->first_waiting)
      sem->last_waiting->next_waiting = ctx;
    else sem->first_waiting = ctx;
    sem->last_waiting = ctx;
    yield();
  }
  irq_enable();
}

int create_ctx(int stack_size, func_t f, void *args)
{
  irq_disable();
  struct ctx_s * ctx = malloc(sizeof(struct ctx_s));
  if (current_ctx) {
    struct ctx_s * next = current_ctx->next;
    struct ctx_s * prev = current_ctx;
    prev->next = ctx;
    next->prev = ctx;
    ctx->next = next;
    ctx->prev = prev;
  } else {
    ctx->next = ctx;
    ctx->prev = ctx;
  }
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
  printf("-Yield: current id: %d, next id: %d-\n",current_ctx->id,current_ctx->next->id);
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
  disp_procs();
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

static struct sem_s gsem;

int main(int argc, char *argv[])
{
  sem_init(&gsem,1);
  create_ctx(16384, f_ping, NULL);
  create_ctx(16384, f_pong, NULL);
  start_sched();
  for(;;);
  exit(EXIT_SUCCESS);
}

void f_ping(void *args)
{
  while(1) {
    sem_down(&gsem);
    /*
    printf("A\n") ;
    printf("B\n") ;
    printf("C\n") ;
    */
    /*
    printf("A") ;
    printf("B") ;
    printf("C") ;*/
    char i;
    for (i = 'A'; i <= 'z'; i++) printf("%c",i);
    printf("\n");
    sem_up(&gsem);
  }
}

void f_pong(void *args)
{
  while(1) {
    sem_down(&gsem);
    printf("1") ;
    printf("2") ;
    printf("\n");
    sem_up(&gsem);
  }
}


