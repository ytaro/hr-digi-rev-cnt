; h8/300h Tiny�Υ֡��ȥ⡼���ѥ������ȥ��åץ롼����
; Copyleft,1999,Yukio Mituiwa

	.h8300h
	.section .text
	.global	_start
_start:
	;; �����å��ݥ��󥿤�����
	mov.l	#_stack,sp
	;; ROM��DATA��RAM��ž��
	mov.l	#___dtors_end,er0
	mov.l	#___data,er1
	mov.l	#_edata,er2
.mvdata:
	mov.w	@er0,r3
	mov.w	r3,@er1
	adds	#2,er0
	adds	#2,er1
	cmp.l	er2,er1
	blo	.mvdata
	;; main�롼����
	jsr	@_main
	bra	_start

	.section .stack
_stack:	.long 	1