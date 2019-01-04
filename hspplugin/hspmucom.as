
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


#endif

