_DATA	segment word public 'DATA'
_DATA	ends
_BSS	segment word public 'BSS'
_BSS	ends
_TEXT	segment	byte public 'CODE'
DGROUP	group	_DATA,_BSS
	assume	cs:_TEXT,ds:DGROUP,ss:DGROUP

; This calls the chunk of decode code
; arguments are:
;	segment we are decoding
;	offset at which the code starts
; we need to:
;	make ds point to the data
;   make sure es points to the screen
;	call ds:offset

_demo	proc	near
	push	bp
	mov	bp,sp
    push ds
	push di
	push es
	push si

	; To call we need
	; 1 - ES to the screen
	mov ax,0b000h
	mov es,ax
	; 2 - DS to the data
	mov ds,word ptr [bp+6]
	; 3 - Adrs of code
	call dword ptr [bp+4]

	pop si
	pop es
	pop di
	pop ds
	pop bp
	ret
_demo	endp
_TEXT	ends
	public	_demo
	end
