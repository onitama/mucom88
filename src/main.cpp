
//
//	mucom : OpenMucom88 Command Line Tool
//			MUCOM88 by Yuzo Koshiro Copyright 1987-2019(C) 
//			Windows version by onion software/onitama 2018/11
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "windows.h"
#include "cmucom.h"

//#define DEBUG_MUCOM

#define DEFAULT_OUTFILE "mucom88.mub"

/*----------------------------------------------------------*/

static void usage1( void )
{
static 	char *p[] = {
	"usage: mucom88 [options] [filename]",
	"       -p [filename] setload PCM file",
	"       -v [filename] set load voice file",
	"       -o [filename] set output file",
	"       -c [filename] compile mucom88 MML",
	"       -i [filename] info print mucom88 MML",
	"       -e Use external ROM files",
	"       -s Use SCCI device",
	"       -k Skip PCM load",
	NULL };
	int i;
	for (i = 0; p[i]; i++) {
		printf("%s\n", p[i]);
	}
}

/*----------------------------------------------------------*/

int main( int argc, char *argv[] )
{
	char a1,a2;
	int b,st;
	int cmpopt,ppopt;
	int scci_opt;
	char fname[1024];
	char *pcmfile;
	char *outfile;
	char *voicefile;

	//	check switch and prm

#ifdef DEBUG_MUCOM
	{
		CMucom mucom;

		mucom.Init();

		mucom.Reset(0);
		//mucom.ProcessFile("test.muc");
		//mucom.PrintInfoBuffer();

		mucom.LoadPCM();
		mucom.LoadMusic("test2.mub");
		mucom.Play(0);
		mucom.PrintInfoBuffer();

		//mucom.Reset(2);
		//mucom.LoadPCM();
		//mucom.CompileFile("sampl1.muc","test2.mub");

		puts(mucom.GetMessageBuffer());

		while (1) {
			Sleep(20);
		}


		return 0;
	}
#endif

	if (argc<2) { usage1();return -1; }

	st = 0; ppopt = 0; cmpopt = 0; scci_opt = 0;
	pcmfile = MUCOM_DEFAULT_PCMFILE;
	outfile = DEFAULT_OUTFILE;
	voicefile = NULL;
	fname[0] = 0;
	
	for (b=1;b<argc;b++) {
		a1=*argv[b];a2=tolower(*(argv[b]+1));
		if (a1!='-') {
			strcpy(fname,argv[b]);
		} else {
			switch (a2) {
			case 'p':
				pcmfile = argv[b + 1]; b++;
				ppopt = 0;
				break;
			case 'v':
				voicefile = argv[b + 1]; b++;
				break;
			case 'o':
				outfile = argv[b + 1]; b++;
				break;
			case 'c':
				cmpopt |= 2;
				break;
			case 'e':
				cmpopt |= 1;
				break;
			case 'k':
				ppopt = 1;
				break;
			case 'i':
				cmpopt |= 0x100;
				break;
			case 's':
				scci_opt = 1;
				break;
			default:
				st=1;break;
			}
		}
	}

	if (st) { printf("#Illegal switch selected.\n");return 1; }
	if (fname[0]==0) { printf("#No file name selected.\n");return 1; }

	//		call main
	CMucom mucom;

	mucom.Init();

	if (scci_opt) {
		printf("Use SCCI.\n");
		mucom.SetVMOption(MUCOM_OPTION_SCCI | MUCOM_OPTION_FMMUTE, 1);
	}

	mucom.Reset(cmpopt);
	st = 0;

	if (cmpopt & 0x100) {
		mucom.ProcessFile(fname);
		mucom.PrintInfoBuffer();
		puts(mucom.GetMessageBuffer());
		return 0;
	}
	if (cmpopt & 2) {
		if (ppopt == 0) {
			mucom.LoadPCM(pcmfile);
		}
		if (voicefile != NULL) {
			mucom.LoadFMVoice(voicefile);
		}
		if (mucom.CompileFile(fname, outfile) < 0) {
			st = 1;
		}
	}
	else {
		if (mucom.LoadMusic(fname) < 0) {
			st = 1;
		}
		else {
			mucom.Play(0);
			mucom.PrintInfoBuffer();
		}
	}
	puts(mucom.GetMessageBuffer());

	if (st) return st;

	if ((cmpopt & 2)==0) {
		while (1) {
			Sleep(20);
		}
	}

	return 0;
}

