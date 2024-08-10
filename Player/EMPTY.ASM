	ifndef	??version
?debug	macro
	endm
	endif
	?debug	S "empty.c"
_TEXT	segment	byte public 'CODE'
DGROUP	group	_DATA,_BSS
	assume	cs:_TEXT,ds:DGROUP,ss:DGROUP
_TEXT	ends
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
	?debug	C E9A4B8085907656D7074792E63
_BSS	ends
_TEXT	segment	byte public 'CODE'
;	?debug	L 3
_sample	proc	near
	push	bp
	mov	bp,sp
;	?debug	L 5
	push	word ptr [bp+4]
	call	near ptr _f
	pop	cx
;	?debug	L 6
	push	word ptr [bp+6]
	call	near ptr _f
	pop	cx
@1:
;	?debug	L 7
	pop	bp
	ret	
_sample	endp
_TEXT	ends
	?debug	C E9
_DATA	segment word public 'DATA'
s@	label	byte
_DATA	ends
_TEXT	segment	byte public 'CODE'
	extrn	_f:near
_TEXT	ends
	public	_sample
	end
