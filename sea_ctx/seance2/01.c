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
 unsigned int ebp, esp;
};

typedef int (func_t)(int); /* a function that returns an int from an int */
int try(struct ctx_s *pctx, func_t *f, int arg);
int throw(struct ctx_s *pctx, int r);

struct ctx_s *global_pctx;
static int ret_save;

int try(struct ctx_s *pctx, func_t *f, int arg)
{
 regread(ebp);
 pctx->ebp = ebp;
 regread(esp);
 pctx->esp = esp;
 global_pctx = pctx;
 return f(arg);
}

int throw(struct ctx_s *pctx, int r)
{
 ret_save = r;
 regwrite(ebp,pctx->ebp);
 regwrite(esp,pctx->esp);
 return ret_save;
}

int f(int p)
{
 printf("%d\n",p);
 if (p) throw(global_pctx,42);
 return 0;
}

int main()
{
 struct ctx_s pctx;
 printf("%d\n",try(&pctx,f,0));
 int r = try(&pctx,f,666);
 printf("%d\n",r);
}










