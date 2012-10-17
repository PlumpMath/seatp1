#include <stdlib.h>
#include <stdio.h>
#include "../libhw/include/hw.h"

#define regread(name) \
 void * name;                    \
 asm ("movl %%"#name", %0"                \
  : "=r"(name)                              \
  :                                    \
  : "%"#name);

#define regwrite(name,val) { \
 void * name = val;                    \
 asm ("movl %0, %%"#name                \
  :                             \
  : "r"(name)                                   \
  : "%"#name);                     \
}

#define N 10 /* nombre de places dans le tampon */

typedef void (func_t)(void*); /* a function that returns an int from an int */

struct ctx_s {
  void * stack_addr;
  unsigned int stack_size;
  void *  ebp, * esp;
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
  ctx->stack_size = stack_size;
  ctx->stack_addr = addr;
  //ctx->esp = (unsigned int)(addr+stack_size);
  ctx->esp = (addr+stack_size);
  ctx->ebp = ctx->esp;
  ctx->f_ptr = f;
  ctx->f_args = args;
  ctx->started = 0;
  static int nbp = 0;
  ctx->id = nbp++;
  irq_enable();
}

void timeup()
{
  printf(">>> TIME IS UP !!! <<<\n");
  yield();
}

void start_sched()
{
  setup_irq(TIMER_IRQ,timeup);
  start_hw();
  irq_enable();
}

struct sem_s mutex, vide, plein;

void translate_stack(struct ctx_s * ctx, unsigned char * nstack, unsigned nstack_size)
{
  
}

void yield()
{
  irq_disable();
  printf("-Yield: current id: %d, next id: %d-\n",current_ctx->id,current_ctx->next->id);
  printf("*** mutex %d\n", mutex.count);
  printf("*** vide %d\n", vide.count);
  printf("*** plein %d\n", plein.count);
  
  //if (current_ctx)
  printf(">>> esp: %u | stack: %u\n",current_ctx->esp,current_ctx->stack_addr);
  
  unsigned int min_size = (current_ctx->stack_addr + current_ctx->stack_size - current_ctx->esp);
  min_size *= 2;
  if (!check_stack(current_ctx, min_size))
  {
    
    printf(">>> REALLOCATING STACK ! <<<\n");
    void * nstack_addr = malloc(min_size);
    printf(">>> currently using %u of %u\n",(current_ctx->stack_addr + current_ctx->stack_size - current_ctx->esp), current_ctx->stack_size);
    printf(">>> requiring: %u\n",min_size);
    printf(">>> obtaining: %u\n",nstack_addr);
    //printf(">>> %u | %u\n",nstack_addr,min_size);
    memcpy(nstack_addr,current_ctx->stack_addr,1);//current_ctx->stack_size);
    translate_stack(current_ctx, nstack_addr, min_size);
    free(current_ctx->stack_addr);
    current_ctx->stack_addr = nstack_addr;
  }
  
  
  /*
  static int old_esp, old_ebp;
  */
  regread(esp);
  //old_esp = esp;
  regread(ebp);
  //old_ebp = ebp;
  printf("--> writing ctx->esp: %u\n",esp);
  current_ctx->esp = esp;
  current_ctx->ebp = ebp;
  current_ctx = current_ctx->next;
  printf("--> writing esp: %u\n",current_ctx->esp);
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

int check_stack(struct ctx_s * pctx, unsigned size)
{
  return pctx->stack_size > size;
}



void consommateur (void);
void producteur (void);

int main(int argc, char *argv[])
{
  sem_init(&mutex, 1); /* controle d'acces au tampon */
  sem_init(&vide, N); /* nb de places libres */
  sem_init(&plein, 0); /* nb de places occupees */
  create_ctx(16384, producteur, NULL);
  create_ctx(16384, consommateur, NULL);
  start_sched();
  for(;;);
  exit(EXIT_SUCCESS);
}


void producteur (void)
{
  while (1) {
    printf("produire_objet(&objet);\n"); /* produire l'objet suivant */
    sem_down(&vide); /* dec. nb places libres */
    sem_down(&mutex); /* entree en section critique */
    printf("mettre_objet(objet);\n"); /* mettre l'objet dans le tampon */
    sem_up(&mutex); /* sortie de section critique */
    sem_up(&plein); /* inc. nb place occupees */
  }
}

void consommateur (void)
{
  while (1) {
    sem_down(&plein); /* dec. nb emplacements occupes */
    sem_down(&mutex); /* entree section critique */
    printf("retirer_objet (&objet);\n"); /* retire un objet du tampon */
    sem_up(&mutex); /* sortie de la section critique */
    sem_up(&vide); /* inc. nb emplacements libres */
    printf("utiliser_objet(objet);\n"); /* utiliser l'objet */
  }
}

// void producteur (void)
// {
//   while (1) {
//     printf("produire_objet(&objet);\n"); /* produire l'objet suivant */
//     sem_down(&mutex); /* entree en section critique */
//     sem_down(&vide); /* dec. nb places libres */
//     printf("mettre_objet(objet);\n"); /* mettre l'objet dans le tampon */
//     sem_up(&plein); /* inc. nb place occupees */
//   sem_up(&mutex); /* sortie de section critique */
//     }
// }
// 
// void consommateur (void)
// {
//   while (1) {
//     sem_down(&mutex); /* entree section critique */
//     sem_down(&plein); /* dec. nb emplacements occupes */
//     printf("retirer_objet (&objet);\n"); /* retire un objet du tampon */
//     sem_up(&vide); /* inc. nb emplacements libres */
//     sem_up(&mutex); /* sortie de la section critique */
//     printf("utiliser_objet(objet);\n"); /* utiliser l'objet */
//   }
// }




