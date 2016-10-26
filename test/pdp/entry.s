.globl _start
.text
	_start:
		mov $0160000, sp
		jsr pc, __start2
		halt
		jmp 0

