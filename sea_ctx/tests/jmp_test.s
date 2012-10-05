	.file	"jmp_test.c"
	.local	i
	.comm	i,4,4
	.section	.rodata
.LC0:
	.string	"j = %d\n"
.LC1:
	.string	"i = %d\n"
	.text
	.type	cpt, @function
cpt:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	movl	$0, -4(%ebp)
	movl	$buf, (%esp)
	call	_setjmp
	testl	%eax, %eax
	je	.L2
	movl	-4(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC0, (%esp)
	call	printf
	movl	$0, -4(%ebp)
	jmp	.L4
.L5:
	movl	i, %eax
	addl	$1, %eax
	movl	%eax, i
	movl	i, %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	printf
	addl	$1, -4(%ebp)
.L4:
	cmpl	$4, -4(%ebp)
	jle	.L5
	jmp	.L11
.L2:
	movl	-4(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC0, (%esp)
	call	printf
	movl	$0, -4(%ebp)
	jmp	.L8
.L9:
	movl	i, %eax
	subl	$1, %eax
	movl	%eax, i
	movl	i, %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	printf
	addl	$1, -4(%ebp)
.L8:
	cmpl	$4, -4(%ebp)
	jle	.L9
.L11:
	leave
	ret
	.size	cpt, .-cpt
	.section	.rodata
.LC2:
	.string	"ok"
.LC3:
	.string	"np = %d\n"
	.text
.globl main
	.type	main, @function
main:
	leal	4(%esp), %ecx
	andl	$-16, %esp
	pushl	-4(%ecx)
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ecx
	subl	$36, %esp
	movl	$0, -8(%ebp)
	call	cpt
	movl	$.LC2, (%esp)
	call	printf
	addl	$1, -8(%ebp)
	cmpl	$1, -8(%ebp)
	jne	.L13
	movl	$-1, 4(%esp)
	movl	$buf, (%esp)
	call	longjmp
.L13:
	movl	-8(%ebp), %eax
	movl	%eax, 4(%esp)
	movl	$.LC3, (%esp)
	call	printf
	movl	i, %eax
	movl	%eax, 4(%esp)
	movl	$.LC1, (%esp)
	call	printf
	addl	$36, %esp
	popl	%ecx
	popl	%ebp
	leal	-4(%ecx), %esp
	ret
	.size	main, .-main
	.local	buf
	.comm	buf,156,32
	.ident	"GCC: (GNU) 4.1.2 20080704 (Red Hat 4.1.2-46)"
	.section	.note.GNU-stack,"",@progbits
