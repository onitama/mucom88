#ifndef __voiceformat_h
#define __voiceformat_h

// 音声フォーマット
typedef struct {
	unsigned char hed;		// ヘッダ
	// DT/ML
	unsigned char ml_op1 : 4;
	unsigned char dt_op1 : 3;
	unsigned char p1_op1 : 1;

	unsigned char ml_op3 : 4;
	unsigned char dt_op3 : 3;
	unsigned char p1_op3 : 1;

	unsigned char ml_op2 : 4;
	unsigned char dt_op2 : 3;
	unsigned char p1_op2 : 1;

	unsigned char ml_op4 : 4;
	unsigned char dt_op4 : 3;
	unsigned char p1_op4 : 1;
	// TL
	unsigned char tl_op1 : 7;
	unsigned char p2_op1 : 1;

	unsigned char tl_op3 : 7;
	unsigned char p2_op3 : 1;

	unsigned char tl_op2 : 7;
	unsigned char p2_op2 : 1;

	unsigned char tl_op4 : 7;
	unsigned char p2_op4 : 1;
	// KS/AR
	unsigned char ar_op1 : 5;
	unsigned char p3_op1 : 1;
	unsigned char ks_op1 : 2;
	
	unsigned char ar_op3 : 5;
	unsigned char p3_op3 : 1;
	unsigned char ks_op3 : 2;

	unsigned char ar_op2 : 5;
	unsigned char p3_op2 : 1;
	unsigned char ks_op2 : 2;

	unsigned char ar_op4 : 5;
	unsigned char p3_op4 : 1;
	unsigned char ks_op4 : 2;
	// AMON(OPNA)/DR
	unsigned char dr_op1 : 5;
	unsigned char p4_op1 : 2;
	unsigned char am_op1 : 1;

	unsigned char dr_op3 : 5;
	unsigned char p4_op3 : 2;
	unsigned char am_op3 : 1;

	unsigned char dr_op2 : 5;
	unsigned char p4_op2 : 2;
	unsigned char am_op2 : 1;

	unsigned char dr_op4 : 5;
	unsigned char p4_op4 : 2;
	unsigned char am_op4 : 1;

	// SR
	unsigned char sr_op1 : 5;
	unsigned char p5_op1 : 3;

	unsigned char sr_op3 : 5;
	unsigned char p5_op3 : 3;

	unsigned char sr_op2 : 5;
	unsigned char p5_op2 : 3;

	unsigned char sr_op4 : 5;
	unsigned char p5_op4 : 3;
	// SL/RR
	unsigned char rr_op1 : 4;
	unsigned char sl_op1 : 4;

	unsigned char rr_op3 : 4;
	unsigned char sl_op3 : 4;

	unsigned char rr_op2 : 4;
	unsigned char sl_op2 : 4;

	unsigned char rr_op4 : 4;
	unsigned char sl_op4 : 4;

	// FB/AL
	unsigned char al : 3;
	unsigned char fb : 3;
	unsigned char p6 : 2;
	// 文字列
	char name[6];
} MUCOM88_VOICEFORMAT;


#endif
