;==========================================================================
; MUSICLALF Ver.1.0〜1.2共通 プログラムソース
; ファイル名 : expand.asm
; 機能 : N88BASIC コマンド拡張
; PROGRAMED BY YUZO KOSHIRO
;==========================================================================
; ヘッダ編集/ソース修正 : @mucom88
;==========================================================================
	
	
	ORG	0AB00H
	
COMWK:	EQU	0F320H
MDATA:		EQU	COMWK	;ｺﾝﾊﾟｲﾙｻﾚﾀ ﾃﾞｰﾀｶﾞｵｶﾚﾙ ｹﾞﾝｻﾞｲﾉ ｱﾄﾞﾚｽ
DATTBL:		EQU	MDATA+4	;ｹﾞﾝｻﾞｲ ｺﾝﾊﾟｲﾙﾁｭｳ ﾉ MUSIC DATA TABLE TOP
OCTAVE:		EQU	DATTBL+2
SIFTDAT:	EQU	OCTAVE+1
CLOCK:		EQU	SIFTDAT+1
SECCOM:		EQU	CLOCK+1
KOTAE:		EQU	SECCOM+1
LINE:		EQU	KOTAE+2
ERRORLINE:	EQU	LINE+2
COMNOW:	EQU	ERRORLINE+2
COUNT:	EQU	COMNOW+1
MOJIBUF:EQU	COUNT+1
SEC:	EQU	MOJIBUF+4
MIN:	EQU	SEC+1
HOUR:	EQU	MIN+1
ALLSEC:	EQU	HOUR+1
T_FLAG:	EQU	ALLSEC+2
SE_SET:	EQU	T_FLAG+1
FD_FLG:	EQU	SE_SET+2
FD_EFG:	EQU	FD_FLG+1
ESCAPE:	EQU	FD_EFG+1
MINUSF:	EQU	ESCAPE+1
BEFRST:	EQU	MINUSF+1;ｾﾞﾝｶｲ ｶﾞ 'r' ﾃﾞｱﾙｺﾄｦｼﾒｽ ﾌﾗｸﾞ ｹﾝ ｶｳﾝﾀ
BEFCO:	EQU	BEFRST+1;ｾﾞﾝｶｲ ﾉ ｶｳﾝﾀ
BEFTONE:EQU	BEFCO+2	;ｾﾞﾝｶｲ ﾉ ﾄｰﾝ ﾃﾞｰﾀ
TIEFG:	EQU	BEFTONE+9;ｾﾞﾝｶｲ ｶﾞ ﾀｲﾃﾞｱﾙｺﾄｦ ｼﾒｽ
COMNO:	EQU	TIEFG+1	;ｾﾞﾝｶｲﾉ ｺﾏﾝﾄﾞ｡ ﾄｰﾝﾉﾄｷﾊ 0
ASEMFG:	EQU	COMNO+1
VDDAT:	EQU	ASEMFG+1
OTONUM:	EQU	VDDAT+1; ﾂｶﾜﾚﾃｲﾙ ｵﾝｼｮｸ ﾉ ｶｽﾞ
VOLUME:	EQU	OTONUM+1; NOW VOLUME
	
MWRITE:		EQU	9000H
MWRIT2:		EQU	MWRITE+3
ERRT:		EQU	MWRIT2+3
ERRORSN:	EQU	ERRT+3
ERRORIF:	EQU	ERRORSN+3
ERRORNF:	EQU	ERRORIF+3
ERRORFN:	EQU	ERRORNF+3
ERRORVF:	EQU	ERRORFN+3
ERROROO:	EQU	ERRORVF+3
ERRORND:	EQU	ERROROO+3
ERRORRJ:	EQU	ERRORND+3
STTONE:		EQU	ERRORRJ+3
STLIZM:		EQU	STTONE+3
REDATA:		EQU	STLIZM+3
MULT:		EQU	REDATA+3
DIV:		EQU	MULT+3
HEXDEC:		EQU	DIV+3
HEXPRT:		EQU	HEXDEC+3
ROM:		EQU	HEXPRT+3
RAM:		EQU	ROM+3
FMCOMC:		EQU	RAM+3
T_RST:		EQU	FMCOMC+3
ERRORNE:	EQU	T_RST+3
ERRORDC:	EQU	ERRORNE+3
ERRORML:	EQU	ERRORDC+3
MCMP:		EQU	ERRORML+3
ERRORVO:	EQU	MCMP+3
	
MUSIC:	EQU	0B000H
DRIVE:	EQU	MUSIC+3*5
WKGET:	EQU	MUSIC+3*8
	
MUC88:	EQU	09600H
MUSICSTART:	EQU	MUC88+3*3
	
CURSOR:	EQU	0EF86H
	
CLS1:	EQU	5F0EH
SCEDIT:	EQU	5F92H	;ｽｸﾘｰﾝ ｴﾃﾞｨｯﾄ
STOPKC:	EQU	35C2H	;ｽﾄｯﾌﾟｷｰ ﾁｪｯｸ
BUFCLR:	EQU	35D9H	;ｷｰﾊﾞｯﾌｧｸﾘｱ
EDBUF:	EQU	08C10H	; T_CLKﾅﾄﾞﾄ ｷｮｳﾂｳ ﾉ ﾜｰｸ
TXTEND:	EQU	0EB18H	;ﾃｷｽﾄ ｴﾝﾄﾞ
LNKSET:	EQU	05BDH	;ﾘﾝｸﾎﾟｲﾝﾀ ｾｯﾄ
CHGWA:	EQU	044D5H	; ｳｨﾝﾄﾞｳ->ｼﾞﾂｱﾄﾞﾚｽ ﾍﾝｶﾝ
	
; -- CLEAR FROM COMPI1	-->
	
T_CLK:	EQU	08C10H
UDFLG:	EQU	T_CLK+4*11
BEFMD:	EQU	UDFLG+1
PTMFG:	EQU	BEFMD+2
PTMDLY:	EQU	PTMFG+1
SPACE:	EQU	PTMDLY+2	;2*8BYTE ｱｷ ｶﾞ ｱﾙ
DEFVOICE:EQU	SPACE+2*8
DEFVSSG:EQU	DEFVOICE+32
JCLOCK:	EQU	DEFVSSG+32
JPLINE:	EQU	JCLOCK+2
	
;-<
	
SMON:	EQU	0DE00H
CONVERT:EQU	SMON+3*2
	
	JP	DSPMSG
	JP	FOUND
	JP	PRNFAC
	JP	FVTEXT
	JP	COLOR
	JP	KEYCHK
	JP	REPLACE
	JP	CULPTM
	
; **	ﾃｷｽﾄ ｶﾗ ﾄｸﾃｲﾉ ﾓｼﾞﾚﾂｦ ｻｶﾞｽ	**
	
FOUND:
	LD	HL,PROMPT
	CALL	DSPMSG
	CALL	SCEDIT
	RET	C
	INC	HL
	LD	DE,EDBUF
	LD	BC,32
	LDIR			;EDBUF ﾆ ﾃﾝｿｳ
	
	CALL	UPCLS
	CALL	PRSWD
	CALL	CLS1
	XOR	A
	LD	(REPFLG),A
FO0:
	XOR	A
	LD	(RETK),A
	DI
	CALL	RAM
	LD	HL,1
FO1:
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	INC	HL
	LD	(LINKPT),DE
	LD	(LINEADR),HL
	LD	A,E
	OR	D	;BASIC END?
	JP	Z,NOTFOUND
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	INC	HL
	LD	(LINE2),DE
	LD	(BEGIN),HL
	INC	HL
	INC	HL
	INC	HL
	EX	DE,HL
FO12:
	LD	HL,EDBUF
	LD	B,33
FO2:
	PUSH	HL
	LD	HL,(LINKPT)
	DEC	HL
	AND	A
	SBC	HL,DE
	POP	HL
	JR	Z,FO6
	LD	A,(DE)
	INC	DE
	CP	(HL)
	JR	NZ,FO12
	INC	HL
	LD	A,(HL)
	OR	A	; BUFFER END?
	JR	Z,FO_OK
	DJNZ	FO2
	JR	FO_OK
FO6:
	LD	HL,(LINKPT)
	JR	FO1	; ﾂｷﾞ ﾉ ｷﾞｮｳﾍ
	
; --	ﾓｼﾞﾚﾂ ﾊｯｹﾝ	--
	
FO_OK:
	EI
	CALL	ROM
	PUSH	DE
	CALL	FOCUR
	PUSH	HL
	
	LD	DE,(LINE2)
	CALL	PRNFAC
FOK2:
	LD	A,20H
	NOP
	LD	HL,FOK2+1
	RST	18H
	LD	HL,(LINEADR)
	INC	HL
	INC	HL
	CALL	44A4H
	CALL	194CH
	LD	HL,0E9B9H
	CALL	DSPMSG
	LD	HL,LF
	CALL	DSPMSG
	
	LD	HL,(CURSOR)
	LD	(CUR2),HL
	POP	HL
	LD	(CURSOR),HL
FOK3:
	CALL	4290H
	IN	A,(9)		;STOPKEY
	BIT	0,A
	JR	Z,NOTF2
	LD	HL,RETK
	LD	BC,7F01H
	CALL	KEYCHK		;ﾘﾀｰﾝｷｰ ﾁｪｯｸ
	JR	Z,FOK32
	BIT	7,A
	JR	Z,FOK4
FOK32:
	LD	HL,SPACEK
	LD	BC,0BF09H
	CALL	KEYCHK
	JR	Z,FOK3
	BIT	6,A
	JR	NZ,FOK3
	POP	DE
	JR	FOK5	;ｽﾍﾟｰｽ ｶﾞ ｵｻﾚﾀﾅﾗ ﾘﾌﾟﾚｰｽ ｽｷｯﾌﾟ
FOK4:
	POP	DE
	
	LD	A,(REPFLG)
	OR	A
	CALL	NZ,REP2		; ﾘﾌﾟﾚｰｽ ﾅﾗ ﾘﾀｰﾝｷｰﾃﾞ ﾘﾌﾟﾚｰｽﾌﾟﾛｾｽ
FOK5:
	PUSH	DE
	CALL	CLS1
	POP	DE
	DI
	CALL	RAM
	JP	FO12
	
NOTFOUND:
	EI
	CALL	ROM
	CALL	BUFCLR
	RET
NOTF2:
	POP	DE	;DUMMY
	LD	DE,(CUR2)
	LD	(CURSOR),DE
	JR	NOTFOUND
	
; --	ﾐﾂｹﾀﾄｺﾛﾉ ｶｰｿﾙｻﾞﾋｮｳ ｦ ｴﾙ	--
	
	;IN:DE<=ﾃｷｽﾄ ｱﾄﾞﾚｽ
	;EXIT:	HL
	
FOCUR:
	PUSH	DE
	
	LD	DE,(LINE2)
	CALL	STRFAC
	LD	BC,0FFFFH
	INC	HL
FOC2:
	INC	HL
	LD	A,(HL)
	OR	A
	JR	Z,FOC3
	INC	BC
	JR	FOC2
FOC3:
	LD	HL,EDBUF+1
FOC4:
	LD	A,(HL)
	OR	A
	JR	Z,FOC5
	DEC	BC
	INC	HL
	JR	FOC4
FOC5:
	POP	DE
	
	EX	DE,HL
	ADD	HL,BC
	EX	DE,HL
	
	LD	HL,(BEGIN)
	EX	DE,HL
	AND	A
	SBC	HL,DE
	LD	D,0
	LD	E,80
	CALL	DIV
	LD	D,E	; ｱﾏﾘ ﾊ Xｻﾞﾋｮｳ
	LD	E,L	; ｼｮｳ ﾊ Y ｻﾞﾋｮｳ
	
	LD	HL,(CURSOR)
	ADD	HL,DE
	RET
	
	
; **	REPLACE TEXT	**
	
	
REPLACE:
	LD	HL,REPWD
	CALL	DSPMSG
	LD	HL,PROMPT
	CALL	DSPMSG
	CALL	SCEDIT
	RET	C
	INC	HL
	LD	DE,EDBUF
	LD	BC,32
	LDIR			;EDBUF ﾆ ﾃﾝｿｳ
	
	CALL	UPCLS
	CALL	PRSWD
	
	LD	HL,PROM2
	CALL	DSPMSG
	CALL	SCEDIT
	RET	C
	INC	HL
	LD	DE,EDBUF+32
	LD	BC,32
	LDIR
	
	CALL	PRRWD
	
	CALL	CLS1
	LD	A,0FFH
	LD	(REPFLG),A
	JP	FO0
	
; **	REPLACE PROCESS MAIN	**
	
	;ON:DE<= TEXT ADR
	
REP2:
	PUSH	DE
	
; --	ﾍﾝｶﾝｽﾙﾓﾉﾄ ﾍﾝｶﾝｻﾚﾙﾓﾉﾉ ﾓｼﾞｽｳﾉ ｻ	--
	
	LD	HL,EDBUF+1
	LD	C,1
REP21:
	DEC	DE
	LD	A,(HL)
	OR	A
	JR	Z,REP22
	INC	HL
	INC	C
	JR	REP21
REP22:
	LD	(HENTOP),DE
	LD	HL,EDBUF+32
	LD	B,0
REP23:
	LD	A,(HL)
	OR	A
	JR	Z,REP24
	INC	B
	INC	HL
	JR	REP23
REP24:
	LD	A,B
	OR	A
	JR	NZ,REP25
	LD	(EDBUF+33),A
	LD	A,20H
	LD	(EDBUF+32),A
	LD	A,1
REP25:
	LD	(HENCO),A
	SUB	C
	JR	NC,REPB1	;ﾍﾝｶﾝｻﾚﾙｶﾞﾜｶﾞ ｵｵｷｲﾄｷﾊ REPB1ﾍ
REPS1:
	POP	DE
	PUSH	DE
	EX	DE,HL
	LD	E,A
	LD	D,0FFH
	ADD	HL,DE
	PUSH	HL
	LD	HL,(LINKPT)
	ADD	HL,DE
	LD	(LINKPT),HL
	POP	HL
	EX	DE,HL
	POP	HL
	PUSH	HL
	EXX
	
	LD	HL,(TXTEND)
	POP	DE
	AND	A
	SBC	HL,DE
	LD	C,L
	LD	B,H
	PUSH	BC
	EXX
	POP	BC
	
	PUSH	DE
	DI
	CALL	RAM
	LDIR
	
	LD	DE,(HENTOP)
	LD	HL,EDBUF+32
	LD	A,(HENCO)
	LD	C,A
	LD	B,0
	LDIR
REPS2:
	CALL	ROM
	EI
	CALL	LNKSET
	INC	HL
	CALL	CHGWA
	LD	(TXTEND),HL
	POP	DE
	RET
	
REPB1:
	OR	A
	JR	Z,REPB3
	
	LD	HL,(LINKPT)
	LD	E,A
	LD	D,0
	ADD	HL,DE
	LD	(LINKPT),HL
	LD	HL,(TXTEND)
	POP	DE
	PUSH	DE
	PUSH	HL
	AND	A
	SBC	HL,DE
	INC	HL
	LD	C,L
	LD	B,H
	POP	HL
	PUSH	HL
	LD	E,A
	LD	D,0
	ADD	HL,DE
	EX	DE,HL
	POP	HL
	
	DI
	CALL	RAM
	LDDR
REPB3:
	DI
	CALL	RAM
	POP	DE
	LD	DE,(HENTOP)
	LD	HL,EDBUF+32
	LD	A,(HENCO)
	LD	C,A
	LD	B,0
	LDIR
	PUSH	DE
	JR	REPS2
	
	
; --	PRINT"SEARCH...."	--
	
PRSWD:
	LD	HL,PROMPT
	LD	DE,0F3C8H
	LD	BC,6
	LDIR
	LD	HL,EDBUF
	CALL	PRS2
	RET
PRRWD:
	LD	HL,PROM2
	LD	DE,0F3C8H+23
	LD	BC,15
	LDIR
	LD	HL,EDBUF+32
	CALL	PRS2
	RET
PRS2:
	LD	A,(HL)
	OR	A
	RET	Z
	LD	(DE),A
	INC	HL
	INC	DE
	JR	PRS2
	
; **	PRINT TO DISPLAY	**
	
DSPMSG:
	LD	A,(HL)
	AND	A
	RET	Z
	RST	18H
	INC	HL
	JR	DSPMSG
	
; **	PRINT FACC DATA	**
	
	;IN:DE
	
PRNFAC:
	CALL	STRFAC
	INC	HL
	CALL	DSPMSG
	RET
STRFAC:
	LD	(0EC41H),DE
	LD	A,2
	LD	(0EABDH),A
	CALL	28D0H
	RET
	
; **	FIND VOICE FROM TEXT	**
	
	;IN:A<= VOICE NUMBER
	;STORE:6001Hｶﾗ 25BYTE
	;NOTFOUND:SCF
	
FVTEXT:
	EXX
	LD	(FV3+1),A
	XOR	A
	LD	(FVFG),A
	LD	HL,6001H
	EXX
	LD	HL,1
FV1:
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	INC	HL
	LD	(LINKPT),DE
	LD	(LINEADR),HL
	LD	A,E
	OR	D	;BASIC END?
	JP	Z,FVF2
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	INC	HL
	LD	(LINE2),DE
	INC	HL
	INC	HL
	INC	HL
FV2:
	LD	A,(HL)
	INC	HL
	CP	20H
	JR	NZ,FV4
	INC	HL
	LD	A,(HL)
	CP	'@'
	JR	NZ,FV4
	INC	HL
	LD	A,(HL)
	CP	'%'
	JR	NZ,FV22
	INC	HL
	LD	(FVFG),A
FV22:
	CALL	REDATA
FV3:
	LD	A,0
	CP	E
	JR	Z,FV5
FV4:
	LD	HL,(LINKPT)
	JR	FV1
	
; ---	%(25BYTEｼｷ) ﾉ ﾄｷ ﾉ ﾖﾐｺﾐ	---
	
FV5:
	LD	A,(FVFG)
	OR	A
	JR	Z,FV52
	LD	A,6	;ﾀﾃ
	LD	(FV6+1),A
	LD	A,4	;ﾖｺ
	LD	(FV63+1),A
	CALL	FV6
	LD	HL,(LINKPT)
	LD	DE,9
	ADD	HL,DE
	CALL	REDATA
	LD	A,E
	EXX
	LD	(HL),A
	EXX
	AND	A
	RET
	
; --	38ﾊﾞｲﾄﾍﾞｰｼｯｸﾎｳｼｷﾉﾄｷﾉ ﾖﾐｺﾐ	--
FV52:
	LD	HL,(LINKPT)
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	LD	(LINKPT),DE
	LD	DE,8
	ADD	HL,DE
	CALL	REDATA
	PUSH	DE
	INC	HL
	CALL	REDATA
	LD	A,E
	POP	DE
	LD	D,A
	PUSH	DE
	
	LD	A,4	;ﾀﾃ
	LD	(FV6+1),A
	LD	A,9	;ﾖｺ
	LD	(FV63+1),A
	CALL	FV6
	
	EXX
	POP	DE
	LD	(HL),E	;FB
	INC	HL
	LD	(HL),D	;ALGO
	EXX
	
	LD	HL,6001H
	CALL	CONVERT	;38BYTE->25BYTE
	AND	A
	RET
	
; --	ﾖﾐｺﾐ ｻﾌﾞﾙｰﾁﾝ	--
	
FV6:
	LD	B,6
FV62:
	PUSH	BC
	LD	HL,(LINKPT)
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	LD	(LINKPT),DE
	LD	DE,8
	ADD	HL,DE
FV63:
	LD	B,4
FV7:
	PUSH	BC
	CALL	REDATA
	INC	HL	;SKIP','
	LD	A,E
	EXX
	LD	(HL),A
	INC	HL
	EXX
	POP	BC
	DJNZ	FV7
FV8:
	POP	BC
	DJNZ	FV62
	RET
FVF2:
	SCF	;ERROR
	RET
FVFG:
	DB	0
	
; **	COLOR SET	**
	
	;IN:A<=COLOR CODE
COLOR:
	PUSH	HL
	PUSH	DE
	PUSH	BC
	LD	C,A
	ADD	A,A
	ADD	A,C
	LD	HL,CCODE
	LD	E,A
	LD	D,0
	ADD	HL,DE
	CALL	6EC6H
	POP	BC
	POP	DE
	POP	HL
	RET
	
; **	ｷｰｴﾝﾄﾘｰ ﾁｪｯｸ	**
	
KEYCHK:	;IN:HL<=WORK:C<=PORT:B<=MASK
	
	IN	A,(C)
	OR	B
	LD	B,(HL)
	LD	(HL),A
	CP	0FFH
	RET	Z
	CP	B
	RET
	
; **	ｼﾞｮｳﾌﾞ 1ｷﾞｮｳ ｦ ｹｽ	**
	
UPCLS:
	LD	HL,0F3C8H
	LD	DE,0F3C9H
	LD	BC,119
	LD	(HL),0
	LDIR
	RET
	
	
RETK:	DB	0
SPACEK:	DB	0
	
CCODE:
	DB	' 0',0,' 1',0,' 2',0,' 3',0,' 4',0,' 5',0,' 6',0,' 7',0
PROMPT:
	DB	'Find:',0
PROM2:
	DB	'Replace which:',0
REPWD:
	DB	'RET(GO)/SPACE(SKIP)',0AH,0
	
REPFLG:	DB	0
LF:	DB	0DH,0AH,0
LINKPT:	DW	0
LINE2:	DW	0
LINEADR:	DW	0
HENTOP:	DW	0
HENCO:	DB	0
BEGIN:	DW	0
CUR2:	DW	0
	
	
; **	ﾎﾟﾙﾀﾒﾝﾄ ｹｲｻﾝ	**
	
	;IN:	HL<={CG}ﾀﾞｯﾀﾗ GﾉﾃｷｽﾄADR
	;EXIT:	DE<=Mｺﾏﾝﾄﾞﾉ 3ﾊﾞﾝﾒ ﾉ ﾍﾝｶﾘｮｳ
	;	Zﾌﾗｸﾞ=1 ﾅﾗ ﾍﾝｶｼﾅｲ
	
CULPTM:
	LD	DE,(MDATA)
	PUSH	DE
	CALL	STTONE
	POP	DE
	LD	(MDATA),DE
	JR	NC,CPT2
	SCF
	RET
CPT2:
	PUSH	HL
	LD	C,A
	CALL	CULP2
	PUSH	AF
	
	LD	A,(BEFCO+1)
	LD	E,A
	LD	D,0
	CALL	DIV
	
	LD	C,E
	LD	B,D
	EX	DE,HL
	POP	AF
	POP	HL
	RET	NC
	PUSH	HL
	LD	HL,0
	AND	A
	SBC	HL,DE
	EX	DE,HL
	POP	HL
	AND	A
	RET
	
CULP2:
	;EXIT:	HL<=ﾍﾝｶﾊﾝｲ
	;	CY ﾅﾗ ｻｶﾞﾘﾊｹｲ
	;	Z ﾅﾗ ﾍﾝｶｾｽﾞ
	
	LD	HL,SETPM4
	LD	DE,SETPM3
	EXX
	LD	HL,FNUMB
	EXX
	LD	A,(COMNOW)
	SUB	3
	CP	3
	JR	NC,CULP4
	EX	DE,HL
	EXX
	LD	HL,SNUMB
	EXX
CULP4:
	LD	(W1+1),HL
	LD	(W2+1),DE
	LD	A,C
	EXX
	LD	C,A
	LD	A,(BEFTONE)
	AND	0FH	;KEY
	ADD	A,A
	LD	E,A
	LD	D,0
	ADD	HL,DE
	LD	E,(HL)
	INC	HL
	LD	D,(HL)
	LD	(FRQBEF),DE
	
	LD	A,C
	CALL	CTONE
	PUSH	AF
	LD	A,(BEFTONE)
	CALL	CTONE
	LD	C,A
	POP	AF
	SUB	C
	RET	Z
W1:
	JP	NC,SETPM4
	NEG
W2:
	JP	SETPM3
	
;	BEFTONE>NOWTONE	(ｻｶﾞﾘ)
	
SETPM3:
	LD	HL,0A1BBH	;FACC=BBA17180	(0.943874)
	LD	(CULLP2+1),HL
	LD	HL,08071H
	LD	(CULLP3+1),HL
	CALL	CULC
	EX	DE,HL
	LD	HL,(FRQBEF)
	AND	A
	SBC	HL,DE
	SCF
	RET
	
;	BEFTONE<NOWTONE	(ｱｶﾞﾘ)
	
SETPM4:
	LD	HL,9C62H	;FACC=629C0781	(1.05946)
	LD	(CULLP2+1),HL
	LD	HL,8107H
	LD	(CULLP3+1),HL
	CALL	CULC
	LD	DE,(FRQBEF)
	AND	A
	SBC	HL,DE
	AND	A
	RET
CULC:
	PUSH	AF
	CALL	ROM
	LD	HL,(FRQBEF)
	CALL	21FDH	;STORE HL INTO FACC
	CALL	222FH	;ﾀﾝｾｲﾄﾞ ﾆ ﾍﾝｶﾝ
	CALL	20E8H	;BCDE<=FACC
	POP	AF
CULLP:
	PUSH	AF
CULLP2:
	LD	HL,0A1BBH	;FACC=BBA17180	(0.943874)
	LD	(0EC41H),HL
CULLP3:
	LD	HL,08071H
	LD	(0EC43H),HL	;FACC=LHED
	LD	A,04
	LD	(0EABDH),A
	CALL	1F53H	;FACC=BCDE*FACC .. F_NUM*1/(2^(1/12))
	CALL	20E8H	;BCDE<=FACC(NEW F_NUM)
	POP	AF
	DEC	A
	JR	NZ,CULLP
	
	CALL	21A0H	;ｾｲｽｳ ﾆ ﾍﾝｶﾝ=>HL
	
	CALL	RAM
	
	RET
	
CTONE:
	LD	D,A
	AND	11110000B	;OCTAVE
	SRL	A
	SRL	A
	SRL	A
	SRL	A
	ADD	A,A
	ADD	A,A
	LD	E,A
	ADD	A,A
	ADD	A,E	;*12
	LD	E,A
	LD	A,D
	AND	0FH
	ADD	A,E
	RET
	
FNUMB:
	DW	26AH,28FH,2B6H,2DFH,30BH,339H,36AH,39EH
	DW	3D5H,410H,44EH,48FH
SNUMB:
	DW	0EE8H,0E12H,0D48H,0C89H,0BD5H,0B2BH,0A8AH,09F3H
	DW	0964H,08DDH,085EH,07E6H
FRQBEF:
	DW	0
