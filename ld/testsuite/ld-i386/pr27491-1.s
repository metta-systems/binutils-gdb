	.weak __start_xx
	.weak __stop_xx

	.text
	.global foo
foo:
	movl	__start_xx@got(%ebx), %eax
	movl	__stop_xx@got(%ebx), %eax

	.section xx,"a",unique,0
	.byte 0

	.section xx,"a",unique,1
	.byte 1
