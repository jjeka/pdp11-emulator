kb_iv_pc_addr	= 0160000
kb_iv_sw_addr	= kb_iv_pc_addr + 2
kb_iv_sw_val	= 0200
kb_data_addr	= 0161000

.text
	.global __kb_init
	__kb_init:
		mov $kb_iv_sw_val, kb_iv_sw_addr
		mov $kb_int_handler, kb_iv_pc_addr
		rts pc
		
	.global _set_kb_handler
	_set_kb_handler:
		mov 2(sp), kb_handler
		rts pc
		
	kb_int_handler:
		tst kb_handler
		beq kb_int_handler_ret
		
		# <3 gcc
		mov r0, -(sp)
		mov r1, -(sp)
		mov r2, -(sp)
		mov r3, -(sp)
		mov r4, -(sp)
		mov r5, -(sp)
		
		mov kb_data_addr, -(sp)
		jsr pc, @kb_handler
		add $2, sp
		
		mov (sp)+, r5
		mov (sp)+, r4
		mov (sp)+, r3
		mov (sp)+, r2
		mov (sp)+, r1
		mov (sp)+, r0
		
		kb_int_handler_ret:
		rti
		
.data
	kb_handler:
		.word 0
		
