
;	hspmucom.dll header(3.0)

#ifndef __hspmucom__
#define __hspmucom__

#uselib "hspmucom.dll"

#func mucominit mucominit $0
#func mucombye mucombye $100

#func mucomreset mucomreset $0
#func mucomplay mucomplay $0
#func mucomstop mucomstop $0
#func mucomfade mucomfade $0
#func mucomsetoption mucomsetoption $202

#func mucomload mucomload $16
#func mucomloadpcm mucomloadpcm $16
#func mucomloadvoice mucomloadvoice $16

#func mucomcomp mucomcomp $202
#func mucommml mucommml $202
#func mucomres mucomres $202
#func mucomstat mucomstat $202
#func mucomtag mucomtag $202
#func mucomloadtag mucomloadtag $202

#func mucomcnvpcm mucomcnvpcm $202

#func getuuid getuuid $202
#func mucomsetuuid mucomsetuuid $202
#func mucomsetfastfw mucomsetfastfw $202
#func mucomsetvolume mucomsetvolume $202

#func mucomgetchdata mucomgetchdata $202

#func mucomrecord mucomrecord $202
#func mucomplg_init mucomplg_init $202
#func mucomplg_notice mucomplg_notice $202

#func mucomedit_reset mucomedit_reset $202
#func mucomedit_setfile mucomedit_setfile $202
#func mucomedit_getstat mucomedit_getstat $202
#func mucomedit_getline mucomedit_getline $202
#func mucomedit_save mucomedit_save $202
#func mucomedit_getreq mucomedit_getreq $202
#func mucomedit_update mucomedit_update $202
#func mucomedit_proc mucomedit_proc $202
#func mucomedit_flush mucomedit_flush $202

#func mucomgetdriver mucomgetdriver $202
#func mucomsetdriver mucomsetdriver $0
#func mucomdumpvoice mucomdumpvoice $202

#define MUCOM_DEFAULT_PCMFILE "mucompcm.bin"
#define MUCOM_DEFAULT_VOICEFILE "voice.dat"

#define MUCOM_RESET_PLAYER 0
#define MUCOM_RESET_EXTFILE 1
#define MUCOM_RESET_COMPILE 2

#define MUCOM_COMPILE_NORMAL 0

#define MUCOM_MUSICBUFFER_MAX 16

#define MUCOM_STATUS_PLAYING 0
#define MUCOM_STATUS_INTCOUNT 1
#define MUCOM_STATUS_PASSTICK 2
#define MUCOM_STATUS_MAJORVER 3
#define MUCOM_STATUS_MINORVER 4
#define MUCOM_STATUS_COUNT 5
#define MUCOM_STATUS_MAXCOUNT 6
#define MUCOM_STATUS_MUBSIZE 7
#define MUCOM_STATUS_MUBRATE 8
#define MUCOM_STATUS_BASICSIZE 9
#define MUCOM_STATUS_BASICRATE 10

#define MUCOM_OPTION_FMMUTE 1
#define MUCOM_OPTION_SCCI 2
#define MUCOM_OPTION_FASTFW 4
#define MUCOM_OPTION_STEP 8

#define MUCOM_CHDATA_LENGTH 0
#define MUCOM_CHDATA_VNUM 1
#define MUCOM_CHDATA_WADR 2
#define MUCOM_CHDATA_TADR 4
#define MUCOM_CHDATA_VOL 6
#define MUCOM_CHDATA_ALG 7
#define MUCOM_CHDATA_CH 8
#define MUCOM_CHDATA_DETUNE 9
#define MUCOM_CHDATA_TLLFO 11
#define MUCOM_CHDATA_REVERB 12
#define MUCOM_CHDATA_QUANTIZE 18
#define MUCOM_CHDATA_LFODELAY 19
#define MUCOM_CHDATA_LFOCOUNT 21
#define MUCOM_CHDATA_LFODIFF 23
#define MUCOM_CHDATA_LFOPEAK 27
#define MUCOM_CHDATA_FNUM1 29
#define MUCOM_CHDATA_FNUM2 30
#define MUCOM_CHDATA_FLAG 31
#define MUCOM_CHDATA_CODE 32
#define MUCOM_CHDATA_FLAG2 33
#define MUCOM_CHDATA_PAN 36
#define MUCOM_CHDATA_KEYON 37
#define MUCOM_CHDATA_VNUMORIG 38
#define MUCOM_CHDATA_VOLORIG 39

#define MUCOM_NOTICE_MMLCHANGE 1	// MMLコード変更の通知
#define MUCOM_NOTICE_VOICECHANGE 2	// 音色変更の通知
#define MUCOM_NOTICE_PCMCHANGE 4	// PCM変更の通知
#define MUCOM_NOTICE_MMLERROR 0x100	// MMLが原因のエラー通知

#define MUCOM_NOTICE_SYSERROR 0x1000	// 不明なシステムエラー通知

#define MUCOM_EDIT_STATUS_NONE 0	// 編集中MMLなし
#define MUCOM_EDIT_STATUS_SAVED 1	// 編集中MML(保存済み)
#define MUCOM_EDIT_STATUS_CHANGED 2	// 編集中MML(未保存)

#define MUCOM_EDIT_OPTION_SJIS 0	// 編集中MMLの文字コードはSJIS
#define MUCOM_EDIT_OPTION_UTF8 1	// 編集中MMLの文字コードはUTF-8

#define MUCOM_EDIT_GETSTAT_STATUS 0
#define MUCOM_EDIT_GETSTAT_NOTICE 1
#define MUCOM_EDIT_GETSTAT_OPTION 2
#define MUCOM_EDIT_GETSTAT_UPDATE 3

//	プラグインに通知するコード
#define MUCOM88IF_NOTICE_TOOLSTART 3	// プラグインツール起動リクエスト
#define MUCOM88IF_NOTICE_TOOLHIDE 10	// プラグインツール非表示リクエスト

//	ドライバー
#define MUCOM_DRIVER_UNKNOWN -1 	// 不明
#define MUCOM_DRIVER_NONE 0 		// 未定義
#define MUCOM_DRIVER_MUCOM88 1		// オリジナルのドライバ(1.7)
#define MUCOM_DRIVER_MUCOM88E 2		// オリジナルのドライバ(1.5)
#define MUCOM_DRIVER_MUCOM88EM 4	// 拡張メモリ版ドライバ(1.7)
#define MUCOM_DRIVER_MUCOMDOTNET 8	// MucomDotNET

#endif

