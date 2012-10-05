#include <setjmp.h>
#include <stdio.h>
static int i = 0;
static jmp_buf buf;
int
main()
{
	int j = 0;
	if (setjmp(buf))
		for (; j<5; j++)
			i++,
	printf("%d\n", j );
	else {
		for (; j<5; j++)
			i--,
	printf("%d\n", j );
		longjmp(buf,~0);
	}
	printf("%d\n", i );
}
 
