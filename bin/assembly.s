	.text
	.file	"Basic module"
	.globl	main
	.p2align	4, 0x90
	.type	main,@function
main:
	.cfi_startproc
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc

	.globl	litt
	.p2align	4, 0x90
	.type	litt,@function
litt:
	.cfi_startproc
	retq
.Lfunc_end1:
	.size	litt, .Lfunc_end1-litt
	.cfi_endproc

	.globl	litf
	.p2align	4, 0x90
	.type	litf,@function
litf:
	.cfi_startproc
	retq
.Lfunc_end2:
	.size	litf, .Lfunc_end2-litf
	.cfi_endproc

	.globl	nullv
	.p2align	4, 0x90
	.type	nullv,@function
nullv:
	.cfi_startproc
	xorl	%eax, %eax
	retq
.Lfunc_end3:
	.size	nullv, .Lfunc_end3-nullv
	.cfi_endproc

	.globl	__invoke
	.p2align	4, 0x90
	.type	__invoke,@function
__invoke:
	.cfi_startproc
	movl	$1, %eax
	retq
.Lfunc_end4:
	.size	__invoke, .Lfunc_end4-__invoke
	.cfi_endproc

	.section	".note.GNU-stack","",@progbits
