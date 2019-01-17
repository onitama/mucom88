
//
//		HSP MUCOM88 support DLL ( for ver3.0 )
//				onion software/onitama 2018/11
//

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <rpc.h>

#include "hspdll.h"
#include "../src/cmucom.h"
#include "../src/wavout.h"

#define MUCOM_USE_UUID

#ifdef MUCOM_USE_UUID
#pragma comment(lib,"rpcrt4.lib")
#endif

static void Alertf(char *format, ...)
{
	char textbf[1024];
	va_list args;
	va_start(args, format);
	vsprintf(textbf, format, args);
	va_end(args);
	MessageBox(NULL, textbf, "error", MB_ICONINFORMATION | MB_OK);
}

static void GetUUID(char *res)
{
#ifdef MUCOM_USE_UUID
	UUID newId;
	RPC_CSTR p;
	UuidCreate(&newId);
	UuidToString(&newId, &p);
	strcpy(res, (char *)p);
	RpcStringFree(&p);
#else
	*res = 0;
#endif
}

/*------------------------------------------------------------*/

static CMucom *mucom = NULL;

static void mucom_term(void)
{
	if (mucom == NULL) return;

	mucom->Stop(1);
	delete mucom;
	mucom = NULL;
}

static int mucom_init(void)
{
	mucom_term();

	mucom = new CMucom;
	return 0;
}

/*------------------------------------------------------------*/

int WINAPI DllMain (HINSTANCE hInstance, DWORD fdwReason, PVOID pvReserved)
{
	if ( fdwReason==DLL_PROCESS_ATTACH ) {
		mucom = NULL;
	}
	if ( fdwReason==DLL_PROCESS_DETACH ) {
		mucom_term();
	}
	return TRUE ;
}

char *getvptr( HSPEXINFO *hei, PVal **pval, int *size )
{
	//		変数ポインタを得る
	//
	APTR aptr;
	PDAT *pdat;
	HspVarProc *proc;
	aptr = hei->HspFunc_prm_getva( pval );
	proc = hei->HspFunc_getproc( (*pval)->flag );
	(*pval)->offset = aptr;
	pdat = proc->GetPtr( *pval );
	return (char *)proc->GetBlockSize( *pval, pdat, size );
}

/*
static int valsize( PVAL2 *pv )
{
	//		calc object memory used size
	//			result : size(byte)
	int i,j;
	int vm=1;
	i=1;
	while(1) {
		j=pv->len[i];if (j==0) break;
		vm*=j;i++;if (i==5) break;
	}
	return vm<<2;
}
*/
/*------------------------------------------------------------*/

EXPORT BOOL WINAPI mucominit( int p1, int p2, int p3, int p4 )
{
	//	DLL mucominit hwnd, option (type$00)
	//
	int res;
	res = mucom_init();
	mucom->Init((HWND)p1,p2);
	return res;
}


EXPORT BOOL WINAPI mucombye(int p1, int p2, int p3, int p4)
{
	//	DLL mucombye (type$00)
	//
	mucom_term();
	return 0;
}


EXPORT BOOL WINAPI mucomreset(int p1, int p2, int p3, int p4)
{
	//	DLL mucomreset option (type$00)
	//
	if (mucom) {
		mucom->Reset(p1);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomplay(int p1, int p2, int p3, int p4)
{
	//	DLL mucomplay id (type$00)
	//
	if (mucom) {
		mucom->Play(p1);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomstop(int p1, int p2, int p3, int p4)
{
	//	DLL mucomstop mode (type$00)
	//
	if (mucom) {
		mucom->Stop(p1);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomfade(int p1, int p2, int p3, int p4)
{
	//	DLL mucomfade speed (type$00)
	//
	if (mucom) {
		mucom->Fade();
	}
	return 0;
}


EXPORT BOOL WINAPI mucomloadpcm(void *bmscr, char *p1, int p2, int p3)
{
	//	DLL mucomloadpcm "filename" (type$16)
	//
	if (mucom) {
		int res;
		if (*p1 == 0) {
			res = mucom->LoadPCM();
		}
		else {
			res = mucom->LoadPCM(p1);
		}
		if (res) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucomloadvoice(void *bmscr, char *p1, int p2, int p3)
{
	//	DLL mucomloadvoice "filename" (type$16)
	//
	if (mucom) {
		int res;
		res = mucom->LoadFMVoice(p1);
		if (res) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucomload(void *bmscr, char *p1, int p2, int p3)
{
	//	DLL mucomload "filename",num (type$16)
	//
	if (mucom) {
		int res;
		res = mucom->LoadMusic(p1, p2);
		if (res) return -1;
		mucom->LoadTagFromMusic(p2);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomres(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomres var (type$202)
	//
	PVal *pv;
	APTR ap;
	const char *res;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	res = "";
	if (mucom) {
		res = mucom->GetMessageBuffer();
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_STR, res);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomstat(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomstat var,option (type$202)
	//
	PVal *pv;
	APTR ap;
	int ep1;
	int res;
	ap = hei->HspFunc_prm_getva(&pv);	// パラメータ1:変数
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ2:数値
	res = 0;
	if (mucom) {
		res = mucom->GetStatus(ep1);
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_INT, &res);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomcomp(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomcomp "mmlfile","outfile",option (type$202)
	//
	int ep1;
	int res;
	char mmlfile[_MAX_PATH];
	char outfile[_MAX_PATH];
	char *p;
	p = hei->HspFunc_prm_gets();			// パラメータ1:文字列
	strncpy( mmlfile,p, _MAX_PATH );
	p = hei->HspFunc_prm_gets();			// パラメータ2:文字列
	strncpy(outfile, p, _MAX_PATH);
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ3:数値
	res = 0;
	if (mucom) {
		res = mucom->CompileFile(mmlfile, outfile, ep1);
		if (res) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucomtag(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomtag var,"tagname" (type$202)
	//
	PVal *pv;
	APTR ap;
	const char *res;
	char *p;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	p = hei->HspFunc_prm_getds("");			// パラメータ2:文字列
	res = "";
	if (mucom) {
		if (*p == 0) {
			res = mucom->GetInfoBuffer();
		}
		else {
			res = mucom->GetInfoBufferByName(p);
		}
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_STR, res);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomloadtag(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomloadtag var,"mmlfile" (type$202)
	//
	int res;
	const char *p;
	PVal *pv;
	APTR ap;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	p = hei->HspFunc_prm_gets();			// パラメータ2:文字列
	if (mucom) {
		res = mucom->ProcessFile(p);
		if (res == 0) {
			p = mucom->GetInfoBuffer();
		}
		else {
			p = "";
		}
	}
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_STR, p);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomcnvpcm(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomcnvpcm "WAVfile","ADPCMfile" (type$202)
	//
	int res;
	char wavfile[_MAX_PATH];
	char outfile[_MAX_PATH];
	char *p;
	p = hei->HspFunc_prm_gets();			// パラメータ1:文字列
	strncpy(wavfile, p, _MAX_PATH);
	p = hei->HspFunc_prm_gets();			// パラメータ2:文字列
	strncpy(outfile, p, _MAX_PATH);
	if (mucom) {
		res = mucom->ConvertADPCM(wavfile, outfile);
		if (res<0) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI getuuid(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL getuuid var (type$202)
	//
	PVal *pv;
	APTR ap;
	char uuid[64];
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	GetUUID(uuid);
	hei->HspFunc_prm_setva(pv, ap, HSPVAR_FLAG_STR, uuid);	// 変数に値を代入
	return 0;
}


EXPORT BOOL WINAPI mucomsetuuid(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomsetuuid "UUID" (type$202)
	//
	char *p;
	p = hei->HspFunc_prm_gets();			// パラメータ1:文字列
	if (mucom) {
		mucom->SetUUID(p);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomsetoption(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomsetoption option, mode (type$202)
	//		mode : 0=set/1=add/2=sub
	//
	int ep1, ep2;
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ1:数値
	ep2 = hei->HspFunc_prm_getdi(0);		// パラメータ1:数値
	if (mucom) {
		mucom->SetVMOption(ep1, ep2);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomsetfastfw(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomsetfastfw speed (type$202)
	//
	int ep1;
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ1:数値
	if (mucom) {
		mucom->SetFastFW(ep1);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomsetvolume(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomsetvolume fmvol, ssgvol (type$202)
	//
	int ep1, ep2;
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ1:数値
	ep2 = hei->HspFunc_prm_getdi(0);		// パラメータ1:数値
	if (mucom) {
		mucom->SetVolume(ep1, ep2);
	}
	return 0;
}


EXPORT BOOL WINAPI mucomgetchdata(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomgetchdata var, ch (type$202)
	//
	int ep1,ep2;
	PVal *pv;
	APTR ap;
	PCHDATA *pt;
	ap = hei->HspFunc_prm_getva(&pv);		// パラメータ1:変数
	ep1 = hei->HspFunc_prm_getdi(0);		// パラメータ2:数値
	if (pv->len[1] < 64) return -1;
	if (mucom) {
		pt = (PCHDATA *)pv->pt;
		ep2 = mucom->GetChannelData( ep1,pt );
		if (ep2 != 0) return -1;
	}
	return 0;
}


EXPORT BOOL WINAPI mucomrecord(HSPEXINFO *hei, int p1, int p2, int p3)
{
	//	DLL mucomrecord "filename",time (type$202)
	//
	int ep1;
	char *p;
	char outfile[_MAX_PATH];
	p = hei->HspFunc_prm_gets();			// パラメータ1:文字列
	strncpy(outfile, p, _MAX_PATH);
	ep1 = hei->HspFunc_prm_getdi(90);	// パラメータ1:数値
	if (mucom) {
		RecordWave(mucom, outfile, MUCOM_AUDIO_RATE, ep1);
	}
	return 0;
}


