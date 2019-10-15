;==========================================================================
; MUSICLALF Ver.1.0 �v���O�����\�[�X
; �t�@�C���� : msub
; �@�\ : �R���p�C��(�T�u)
; PROGRAMED BY YUZO KOSHIRO
;==========================================================================
; �w�b�_�ҏW/�\�[�X�C�� : @mucom88
; ���{�\�[�X��MUSICLALF Ver.1.1��msub���獷���C���ɂč쐬�������ł��B
;==========================================================================
	
	
	ORG	09000H
	
MDATA:		EQU	0F320H
DATTBL:		EQU	MDATA+4
OCTAVE:		EQU	DATTBL+2
SIFTDAT:	EQU	OCTAVE+1
CLOCK:		EQU	SIFTDAT+1
SECCOM:		EQU	CLOCK+1
KOTAE:		EQU	SECCOM+1
LINE:		EQU	KOTAE+2
ERRORLINE:	EQU	LINE+2
COMNOW:		EQU	ERRORLINE+2
COUNT:		EQU	COMNOW+1
MOJIBUF:	EQU	COUNT+1
SEC:		EQU	MOJIBUF+4
MIN:		EQU	SEC+1
HOUR:		EQU	MIN+1
ALLSEC:		EQU	HOUR+1
T_FLAG:		EQU	ALLSEC+2
SE_SET:		EQU	T_FLAG+1
FD_FLG:		EQU	SE_SET+2
FD_EFG:		EQU	FD_FLG+1
ESCAPE:		EQU	FD_EFG+1
MINUSF:		EQU	ESCAPE+1
;MEMEND:		EQU	0E3F0H	;0DDF0H		;���C���O
MEMEND:		EQU	0DDF0H				;���C����
ERRORTBL:	EQU	08800H
	
	JP	MWRITE
	JP	MWRIT2
	JP	ERRT
	JP	ERRORSN
	JP	ERRORIF
	
	JP	ERRORNF
	JP	ERRORFN
	JP	ERRORVF
	JP	ERROROO
	JP	ERRORND
	
	JP	ERRORRJ	;+3*10
	JP	STTONE
	JP	STLIZM
	JP	REDATA
	JP	MULT
	
	JP	DIV
	JP	HEXDEC
	JP	HEXPRT
	JP	ROM
	JP	RAM
	
	JP	FMCOMC	;+3*20
	JP	T_RST
	JP	ERRORNE
	JP	ERRORDC
	JP	ERRORML
	
	JP	MCMP
	JP	ERRORVO
	JP	ERRORMD
	JP	ERRORME
	JP	CULSEC
	
	JP	CULTIM
	JP	GETTIME
	JP	ERRNMC
	JP	ERREMC
	
; ***	MDATA � COMMABD � DATA � ����	***
	
;	IN: A<= COMMAND No.
;           E<= COMMAND DATA
	
MWRITE:
	LD	BC,(MDATA)
	LD	(BC),A
	INC	BC
	LD	A,E
	LD	(BC),A
	INC	BC
	LD	(MDATA),BC
	PUSH	HL
	LD	HL,MEMEND
	AND	A
	SBC	HL,BC
	POP	HL
	JR	C,MAD2
MADR:
	PUSH	IY
	PUSH	HL
	PUSH	DE
	PUSH	BC
	LD	L,C
	LD	H,B
	CALL	HEXPRT
	LD	HL,MOJIBUF
	LD	DE,0F3C8H+36
	LD	BC,4
	LDIR
	POP	BC
	POP	DE
	POP	HL
	POP	IY
	RET
MAD2:
	POP	DE
	JP	ERRORME
	
; ***	MDATA � COMMAND ���� DATA � ����   ***
	
;	IN: A<= COMMAND No. or DATA
	
MWRIT2:
	PUSH	BC
	LD	BC,(MDATA)
	LD	(BC),A
	INC	BC
	LD	(MDATA),BC
	PUSH	HL
;	LD	HL,MEMEND				;���C���O
	LD	HL,MEMEND+20CH				;���C����
	AND	A
	SBC	HL,BC
	POP	HL
	JR	C,MAD2
	CALL	MADR
	POP	BC
	RET
	
; ***	ERROR	TRAP	***
	
ERRT:
	INC	HL
	CALL	REDATA
	JR	C,ERRSN
	OR	A
	JR	NZ,ERRIF
	RET
ERRSN:
	POP	DE
	JP	ERRORSN
ERRIF:
	POP	DE
	JP	ERRORIF
	
	
; ***   ERROR PROCESS   ***
	
ERRORSN:XOR	A
	JR	ERROR
ERRORIF:LD	A,1
	JR	ERROR
ERRORNF:LD	A,2
	JR	ERROR
ERRORFN:LD	A,3
	JR	ERROR
ERRORVF:LD	A,4
	JR	ERROR
ERROROO:LD	A,5
	JR	ERROR
ERRORND:LD	A,6
	JR	ERROR
ERRORRJ:LD	A,7
	JR	ERROR
ERRORNE:LD	A,8
	JR	ERROR
ERRORDC:LD	A,9
	JR	ERROR
ERRORML:LD	A,0AH
	JR	ERROR
ERRORVO:LD	A,0BH
	JR	ERROR
ERROROF:LD	A,0CH
	JR	ERROR
ERRORMD:LD	A,0DH
	JR	ERROR
ERRORME:LD	A,0EH
	JR	ERROR
ERRNMC:	LD	A,0FH
	JR	ERROR
ERREMC:	LD	A,10H
	JR	ERROR
	
;	IN:A<=ERROR CODE
	
ERROR:
	LD	(0EFBAH),HL
	
	EX	AF,AF'
	XOR	A
	LD	(COMNOW),A
	LD	HL,(MDATA)
	LD	(HL),A
	
	LD	HL,(MDATA+2)
	LD	(MDATA),HL
	
 	LD	HL,(LINE)
 	LD	E,(HL)
 	INC	HL
 	LD	D,(HL)  	; DE AS ERROR LINE
  	LD	(ERRORLINE),DE
	
	CALL	ROM
	
	POP	HL
	
 	PUSH	HL
 	LD	HL,(0EF86H)
 	PUSH	HL		; STORE CURSOR
	
	LD	HL,0F3C8H
	LD	DE,0F3C9H
	LD	(HL),0
	LD	BC,79
	LDIR
	
	EX	AF,AF'		; A as ERROR CODE
	ADD	A,A
	LD	HL,ERRORTBL
	LD	E,A
	LD	D,0
	ADD	HL,DE
	LD	A,(HL)
	INC	HL
	LD	H,(HL)
	LD	L,A
	
	LD	DE,16*256+1
	LD	(0EF86H),DE
	CALL	5550H
	
 	LD	DE,1*256+1	; CURSOR X,Y
 	LD	(0EF86H),DE
 	LD	HL,ERRORMSG
 	CALL	5550H
	
 	LD	HL,(ERRORLINE)
	LD	(0E656H),HL
	
	POP	HL
 	LD	(0EF86H),HL
 	POP	HL
	LD	E,2 		; ERROR CODE
	JP	03B3H		; ERROR PROCESS
	
	RET
ERLI:
	DB	20H,0,0,0,0,0,0
	
; ***	TONE SET	***
	
; 	IN  :A<= MML DATA
;	EXIT:A<= KEY CODE DATA
	
STTONE:
	PUSH	DE
	PUSH	HL
	
	LD	A,(HL)
	LD	C,A
	LD	HL,TONES
	LD	B,7	; TONE NUMBER (c-b)
TNLP0:
	LD	A,(HL)
	CP	C
	JR	Z,TONEXT
	
	INC	HL
	INC	HL
	DJNZ	TNLP0
	
	POP HL
	POP DE
	SCF		;ERROR CODE
	RET
TONEXT:
	INC HL
	LD C,(HL)	; GET KEY CODE DATA
	
	LD A,(OCTAVE)
	LD B,A		; STORE A
	
	POP HL
	
	INC HL
	
	LD A,(HL)
	
; ---   SHARP PROCESS   ---
	
	CP '+'
	JR NZ,TONEX0
	
	LD A,C
	CP 11           ; KEY='b'?
	JR NZ,TONEX3
	
	LD C,0FFH
	INC B           ; OCTAVE=OCTAVE+1
	LD A,B
	CP 8		; OCTAVE=9?
	JR NZ,TONEX3
	
	LD B,7          ; OCTAVE=8
TONEX3:
	INC C           ; KEY CODE +1
	JR TONEX2
	
; ---   FLAT PROCESS   ---
	
TONEX0:
	CP '-'
	JR NZ,TONEX1
	
	LD A,C
	OR A            ; KEY='c'?
	JR NZ,TONEX4
	
	LD C,12
	DEC B           ; OCTAVE=OCTAVE-1
	JR NC,TONEX4
	
	LD B,0		; OCTAVE=0
TONEX4:
	DEC C           ; KEY CODE -1
	JR TONEX2
	
TONEX1:
	DEC HL
	
; ---   MAKE TONE DATA   ---
	
TONEX2:
	PUSH	HL
	CALL	KEYSIFT
	POP	HL
	
	LD A,B          ; RESTORE A
	
	RLCA
	RLCA
	RLCA
	RLCA
	
	OR C     	; A=OCTAVE & KEY CODE
	
	POP	DE
	RET
	
KEYSIFT:
	LD	A,(SIFTDAT)
	OR	A
	RET	Z
	
	
	PUSH	BC
	
	LD      H,0
	LD	L,B	; OCTAVE
	LD	DE,12
	CALL	MULT
	
	POP	BC
	
	LD	A,C
	ADD	A,L
	LD	L,A
	ADC	A,H
	SUB	L
	LD	H,A	; HL=OCTAVE*12+KEYCODE
	
	LD	A,(SIFTDAT)
	CP	128
	JR	C,KYS2
	
	LD	D,0FFH
	JR	KYS3
KYS2:
	LD	D,0
KYS3:
	LD	E,A
	ADD	HL,DE
	
	LD	DE,12
	CALL	DIV
	
	LD	B,L	; B as OCTAVE
	LD	C,E	; C as KEY CODE
	
	RET
	
	
; ***   ؽ�� ��ò   ***
	
;	IN  : HL<= TEXT ADR
;	EXIT: A <= LIZM COUNT DATA
	
STLIZM:
	LD	A,(HL)
	CP	'%'
	JR	NZ,SL2
	INC	HL
	CALL	REDATA
	JR	C,SL3
	OR	A
	JR	NZ,STLIER
	LD	A,E
	RET
SL2:
	CALL	REDATA
	JR	C,STLIZ2
	OR	A
	JR	NZ,STLIER
	LD	A,(CLOCK)
	CP	E
	JR	C,STLIER	; CLOCK < E �� ERROR
	
	PUSH	HL
	LD	H,0
	LD	L,A	; HL=CLOCK : DE = LIZM
	CALL	DIV
	
	LD	A,(KOTAE)	; GET COUNTER
	POP	HL
	LD	E,0
	LD	C,A
STLIZ0:
	LD	A,(HL)
	CP	'.'
	JR	NZ,STLIZ1
	INC	HL
	SRL	C	; /2
	LD	A,C
	ADD	A,E
	LD	E,A
	LD	A,C
	JR	STLIZ0
STLIZ1:
	LD	A,(KOTAE)
	ADD	A,E
	RET	NC
	POP	DE	;DUM
	JP	ERROROF
STLIZ2:
	LD	E,0
	LD	A,(COUNT)
	LD	C,A
STLIZ4:
	LD	A,(HL)
	CP	'.'
	JR	NZ,STLIZ5
	INC	HL
	SRL	C
	LD	A,C
	ADD	A,E
	LD	E,A
	LD	A,C
	JR	STLIZ4
STLIZ5:
	LD	A,(COUNT)  	; NOW COUNT
	ADD	A,E
	RET	NC
	POP	DE	;DUM
	JP	ERROROF
STLIER:
	POP	DE	;DUMMY POP
	JP	ERRORIF
SL3:
	POP	DE
	JP	ERRORSN
	
; ***	�ް� �к� ٰ�� (2 BYTE) ***
	
;	IN	: HL<= TEXT ADR
;	EXIT	: DE<= DATA : HL<= NEXT TEXT ADR
;		: NON DATA �� SET CARRY�� A� ·� � Ӽ���
;		: ERROR ... A=1 : NON ERROR ... A=0
;	exam	: c16 �� 16 � ���� DE � ���
	
REDATA:
	PUSH	BC
	PUSH	HL
	LD	HL,SCORE
	LD	DE,SCORE+1
	LD	(HL),0
	LD	BC,5
	LDIR		; INIT SCORE
	POP	HL
	LD	B,5	; 5�� ���
	
	XOR	A
	LD	(HEXFG),A
	LD	(MINUSF),A
	
READ0:			; FIRST CHECK
	LD	A,(HL)
	CP	20H
	JR	Z,RE01
	CP	'$'
	JR	Z,READH
	CP	'-'
	JR	Z,READ9
	CP	'0'
	JP	C,READE	; 0��ޮ�  � ��׸��� ·�
	CP	':'	; 9���� ·�
	JP	NC,READE
	JR	READ7
RE01:	INC	HL
	JR	READ0
READH:
	INC	HL
	LD	A,1
	LD	(HEXFG),A
	JR	READ7
	
READ9:			; MINUS CHECK
	INC	HL
	LD	A,(HL)
	
	CP	'0'
	JP	C,READE		; 0��ޮ�  � ��׸��� ·�
	CP	':'		; 9���� ·�
	JP	NC,READE
	LD	A,1
	LD	(MINUSF),A	; SET MINUS FLAG
	
READ7:
	LD	A,(HL)		; SECOND CHECK
	LD	D,A
	
	LD	A,(HEXFG)
	OR	A
	LD	A,D
	JR	Z,READC
	
	CP	'a'
	JR	C,READG
	CP	'g'
	JR	NC,READG
	SUB	32
READG:
	CP	'A'
	JR	C,READC
	CP	'G'
	JR	NC,READC
	SUB	7
	JR	READF
READC:
	CP	'0'
	JR	C,READ1		; 0��ޮ�  � ��׸��� ·�
	CP	':'		; 9���� ·�
	JR	NC,READ1
READF:
	PUSH	HL
	PUSH	BC
	LD	HL,SCORE+1
	LD	DE,SCORE
	LD	BC,5
	LDIR
	POP	BC
	POP	HL
	
	SUB	30H	; A= 0 - 9
	LD	(SCORE+4),A
	
	INC	HL	; NEXT TEXT
	DJNZ	READ7
	
	LD	A,(HL)	; THIRD CHECK
	CP	'0'
	JR	C,READ1	; 0��ޮ�  � ��׸��� ·�
	CP	':'	; 9���� ·�
	JR	NC,READ1
READ8:
	AND	A	; CY=0
	LD	A,1	; ERROR SIGN
	POP	BC
	RET	; 7����ޮ� � �װ
READ1:
	LD	A,(HEXFG)
	OR	A
	JR	Z,READD
	
	LD	A,(SCORE+1)
	RLCA
	RLCA
	RLCA
	RLCA
	LD	D,A
	LD	A,(SCORE+2)
	OR	D
	LD	D,A
	LD	A,(SCORE+3)
	RLCA
	RLCA
	RLCA
	RLCA
	LD	E,A
	LD	A,(SCORE+4)
	OR	E
	LD	E,A
	JR	READA
READD:
	PUSH	HL
	LD	HL,0
	
	LD	DE,10000	; 10000 � ��
	LD	A,(SCORE)
	OR	A
	JR	Z,READSEN
	LD	B,A
READMAN:
	ADD	HL,DE
	DJNZ	READMAN
READSEN:
	LD	DE,1000
	LD	A,(SCORE+1)	; 1000 � ��
	OR	A
	JR	Z,READHYAKU
	LD	B,A
	
READSEN2:
	ADD	HL,DE
	DJNZ	READSEN2
READHYAKU:
	LD	DE,100
	LD	A,(SCORE+2)	; 100 � ��
	OR	A
	JR	Z,READ4
	LD	B,A
READ2:
	ADD	HL,DE
	DJNZ	READ2
READ4:
	LD	A,(SCORE+3)	; 10 � ��
	OR	A
	JR	Z,READ5
	
	LD	B,A
	LD	A,0
	LD	C,10
READ3:
	ADD	A,C
	DJNZ	READ3
	LD	C,A
	JR	READ6
READ5:
	LD	C,0
READ6:
	LD	A,(SCORE+4)	; 1 � ��
	ADD	A,C
	
	ADD	A,L
	LD	L,A
	ADC	A,H
	SUB	L
	LD	H,A
	
	EX	DE,HL
	POP	HL
	
	LD	A,(MINUSF); CHECK MINUS FLAG
	OR	A
	JR	Z,READA	; NON MINUS
	
	PUSH	HL
	XOR	A	; CY=0
	LD	HL,0
	SBC	HL,DE	; DE � ν� � ��
	EX	DE,HL
	POP	HL
READA:
	XOR	A	; CY=0
	POP	BC
	RET
	
READE:
	LD	(SECCOM),A
	SCF		; NON DATA
	POP	BC
	RET
	
SCORE:
	DB	0,0,0,0,0,0
HEXFG:
	DB	0	; HEX FLAG
	
TONES:
	DB	'c'	,0
	DB	'd'	,2
	DB	'e'	,4
	DB	'f'	,5
	DB	'g'	,7
	DB	'a'	,9
	DB	'b'	,11
	
; ***	����� & �ػ��	***
	
	
;
;	HL = HL * E  ,  HL = HL / DE
;
	
MULT:
	LD	D,0
	LD	A,E
	OR	D
	JR	Z,MULT4
	
	LD	B,E
	EX	DE,HL
	LD	HL,0
MULT2:
	ADD	HL,DE
	DJNZ	MULT2
MULT3:
	LD	(KOTAE),HL
	RET
MULT4:
	LD	HL,0
	JR	MULT3
	
DIV:
	LD	A,L
	OR	H
	JR	NZ,MULT0
	LD	DE,0
	JR	MULT4
MULT0:
	LD	A,E
	OR	D
	JR	Z,MULT4
	AND	A
	SBC	HL,DE
	JR	NC,DIV1
	ADD	HL,DE
	EX	DE,HL
	JR	MULT4
DIV1:
	LD	BC,0
DIV2:
	INC	BC
	AND	A
	SBC	HL,DE
	JR	NC,DIV2
        ADD	HL,DE
	EX	DE,HL
	LD	L,C
	LD	H,B
	JR	MULT3
	
; ***   10 �� �ݶ� ٰ��   ***
	
; 	ENTRY: HL<= DATA
	;EXIT:HL<=NUMBUF ADR
	
HEXDEC:
	PUSH	BC
	
	LD	IY,NUMBUF
	LD	BC,10000
	CALL	DEVS
	LD	BC,1000
	CALL	DEVS
	LD	BC,100
	CALL	DEVS
	LD	BC,10
	CALL	DEVS
	LD	A,L
	ADD	A,30H
	LD	(IY),A
	LD	(IY+1),0	; END OF DATA
	LD	HL,NUMBUF
	
	POP	BC
	
	RET
	
; ---	DIVISION SCORE	---
	
DEVS:
	XOR	A
DEVS1:
	SBC	HL,BC
	JR	C,DEVS2
	INC	A
	JR	DEVS1
DEVS2:
	ADD	HL,BC
	ADD	A,30H
	LD	(IY),A
	INC	IY
	RET
	
;
NUMBUF:
	DB	0,0,0,0,0,0,0,0,0,0
	
	
	
; ***   16 �� DATA ˮ���   ***
	
;	IN:<= HL DATA
	
HEXPRT:
	PUSH	DE
	PUSH	BC
	PUSH	AF
	
	LD	IY,MOJIBUF
	LD	A,H
	CALL	HEXPRT2
	LD	A,L
	CALL	HEXPRT2
	LD	HL,MOJIBUF
	
	POP	AF
	POP	BC
	POP	DE
	RET
	
HEXPRT2:
	
	LD	C,A
	AND	11110000B
	RRCA
	RRCA
	RRCA
	RRCA
	
	CALL	MOJISAVE
	LD	(IY),A
	INC	IY
	
	LD	A,C
	AND	00001111B
	CALL	MOJISAVE
	LD	(IY),A
	INC	IY
	
	RET
	
	
MOJISAVE:
	
	LD	DE,MOJIDATA
	ADD	A,E
	LD	E,A
	ADC	A,D
	SUB	E
	LD	D,A
	
	LD	A,(DE)
	
	RET
	
MOJIDATA:
	
	DB	'0','1','2','3','4','5','6','7'
	DB	'8','9','A','B','C','D','E','F'
	
	
; ***	TO ROM/RAM  MODE	***
	
ROM:
	LD	A,(0E6C2H)
	OUT	(31H),A
	RET
RAM:
	LD	A,(0E6C2H)
	OR	2
	OUT	(31H),A
	RET
	
; ***	COMMAND ����	***
	
;	IN A<= DATA :  IX<= COMMAND TABLE ADR
;	EXIT: C <= FCOMS � ������ � ����� (1 - n)
;       NOT COMMAND �� C <= 0
	
FMCOMC:
	LD	IX,FCOMS
	LD	C,1
	LD	D,A
FCMLP:
	LD	A,(IX)
	OR	A
	JR	Z,FCMLP2
	CP	D
	RET	Z		; SEARCH COM.
	INC	IX
	INC	C
	JR	FCMLP
FCMLP2:
	LD	C,A
	RET		; NO COM.
	
	
; ***	TIME RESET	***
	
T_RST:
	XOR	A
	LD	(HOUR),A
	LD	(SEC),A
	LD	(MIN),A
	DI
	CALL	GETTIME	;TIME READ
	EI
	LD	HL,0F00FH;TIME DATA WORK
	CALL	CULSEC	; CONVERT SEC ALL
	LD	(ALLSEC),HL
	LD	A,0FFH
	LD	(T_FLAG),A
	RET
	
GETTIME:
	LD	HL,0F00DH
	LD	A,3
	CALL	CLKCOM
	LD	A,1
	CALL	CLKCOM
	LD	D,5
GETT1:
	LD	B,8
	LD	E,0
GETT2:
	IN	A,(40H)
	RRCA
	RRCA
	RRCA
	RRCA
	AND	1
	OR	E
	RRCA
	LD	E,A
	CALL	CLKSFT
	DJNZ	GETT2
	LD	(HL),E
	INC	HL
	DEC	D
	JR	NZ,GETT1
	RET
CLKCOM:
	OUT	(10H),A
	LD	C,02H
	JR	CLKSND
CLKSFT:
	LD	C,04H
CLKSND:
	LD	A,(0E6C1H)	; PORT 40H DATA
	AND	11111001B
	OR	C
	OUT	(40H),A
	AND	11111001B
	PUSH	BC
	POP	BC
	NOP
	LD	(0E6C1H),A
	OUT	(40H),A
	RET
	
; --	CULCRATE ALLSEC->TIME	--
	
	;IN:HL<=SEC DATA
	;EXIT:TIME WORK<=DATA
	
CULTIM:
	LD	DE,3600
	CALL	DIV
	LD	A,L
	LD	(HOUR),A
	EX	DE,HL
	LD	DE,60
	CALL	DIV
	LD	A,L
	LD	(MIN),A
	LD	A,E
	LD	(SEC),A
	RET
	
; --	CULCRATE TIME->HOUR*3600+MIN*60+SEC (ALLSEC)	--
	
	;IN:HL<=HOUR ADR
	;EXIT:HL
	
CULSEC:
	LD	A,(HL)	;HOUR
	DEC	HL
	PUSH	HL
	CALL	BCDHEX
	CP	13
	JR	C,CULS2
	SUB	12
CULS2:
	LD	E,A
	LD	HL,3600
	CALL	MULT
	EX	DE,HL
	POP	HL
	PUSH	DE
	LD	A,(HL)	;MIN
	DEC	HL
	PUSH	HL
	CALL	BCDHEX
	LD	E,A
	LD	HL,60
	CALL	MULT
	EX	DE,HL
	POP	HL
	LD	A,(HL)	;SEC
	CALL	BCDHEX
	LD	L,A
	LD	H,0
	ADD	HL,DE	;SEC+MIN*60
	POP	DE
	ADD	HL,DE	;SEC+MIN*60+HOUR*3600
	RET
	
	
; --	CONVERT BCD CODE INTO HEX	--
	
	;IN:A / EXIT:A
BCDHEX:
	PUSH	DE
	LD	E,A
	SRL	A
	SRL	A
	SRL	A
	SRL	A
	ADD	A,A
	LD	D,A
	ADD	A,A
	ADD	A,A
	ADD	A,D	; *10
	LD	D,A
	LD	A,E
	AND	0FH
	ADD	A,D
	POP	DE
	RET
	
	
; ***
	
	;IN:HL<=TEXT ADR:DE<= STRING DATA TOP ADR
	;(DE) �� ʼ��� Ӽ��ް��� � (HL)�� ʼ��� ÷�� � �ް��� � ˶�
	;EXIT: CY=1�� FAULT. Z=1�� Ӽ���ʯ��
	
MCMP:
	
MCMP1:
	LD	A,(DE)
	OR	A
	RET	Z
	LD	C,(HL)
	CP	C
	JR	NZ,MCMP4
	INC	DE
	INC	HL
	JR	MCMP1
MCMP4:
	SCF
	RET
	
	
;	COMMAND
	
FCOMS:			; COMMANDs
	DB	'l'	; LIZM
	DB	'o'	; OCTAVE
	DB	'D'	; DETUNE
	DB	'v'	; VOLUME
	DB	'@'	; SOUND COLOR
	
	DB	'>'	; OCTAVE UP
	DB	'<'	; OCTAVE DOWN
	DB	')'	; VOLUME UP
	DB	'('	; VOLUME DOWN
	DB	'&'	; TIE
	
	DB	'y'	; REGISTER WRITE
	DB	'M'	; MODURATION (LFO)
	DB	'r'	; REST
	DB	'['	; LOOP START
	DB	']'	; LOOP END
	
	DB	'S'	; SE DETUNE
	DB	'L'	; JUMP RESTART ADR
	DB	'q'	; COMMAND OF 'q'
	DB	'E'	; SOFT ENV
	DB	'P'	; MIX PORT
	
	DB	'w'	; NOIZE WAVE
	DB	't'	; TEMPO (DIRECT CLOCK)
	DB	'C'	; SET CLOCK
	DB	'!'	; COMPILE END
	DB	'K'	; KEY SHIFT
	
	DB      '/'	; REPEAT JUMP
	DB	'V'	; TOTAL VOLUME OFFSET
	DB	'\'	; BEFORE CODE
	DB	's'	; HARD ENVE SET
	DB	'm'	; HARD ENVE PERIOD
	
	DB	'%'	; SET LIZM(DIRECT CLOCK)
	DB	'p'	; STEREO PAN
	DB	'H'	; HARD LFO
	DB	'T'	; TEMPO
	DB	'J'	; TAG SET & JUMP TO TAG
	
	DB	';'	; ���Ը ֳ
	DB	'R'	; ��ް��
	DB	'*'	; MACRO
	DB	':'	;RETURN
	DB	'^'	;&� �ż�
	
	DB	'|'	; �����
	DB	'}'	; ϸ۴���
	DB	'{'	; ������Ľ���
	DB	'#'	; FLAG SET
	
	DB	0
	
ERRORMSG:
	DB 'ERROR MESSAGE :',0