#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#include <libgen.h>
#define _chdir chdir
#define _getcwd getcwd
#endif

#ifndef MAX_PATH
#define	MAX_PATH	(260)
#endif

#ifndef _MAX_PATH
#define	_MAX_PATH	MAX_PATH
#define	_MAX_DRIVE	3
#define	_MAX_DIR	256
#define	_MAX_FNAME	256
#define	_MAX_EXT	256
#endif

#include "pcmtool.h"
#include "pcmentry.h"

void SplitPath(const char *path, char *drive, char *dir, char *name, char *ext) {
#ifdef _WIN32
	_splitpath(path, drive, dir, name, ext);
#else
	char buf[_MAX_PATH];
	strcpy(buf, path);
	if (drive) drive[0] = 0;
	if (dir) strcpy(dir, dirname(buf));
    if (name) {
        strcpy(name, basename(buf));
        char *p = strchr(name, '.');
        if (p) *p = 0;
    }
	if (ext) strcpy(ext, strrchr(path, '.'));
#endif

}

#define DATABIN_HEADER_LEN 0x400
#define DATABIN_BODY_MAXLEN 0x40000

PcmTool::PcmTool() {
	entryCount = 0;
	pcmBodySize = 0;
	cmucom = new CMucom();
	cmucom->Init();
}
PcmTool::~PcmTool() {
	if (header != NULL) delete[] header;
	header = NULL;

	if (body != NULL) delete[] body;
	body = NULL;


	if (cmucom != NULL) delete cmucom;
	cmucom = NULL;
}

bool PcmTool::WriteBinary(const char* outfile) {
	FILE* fp = fopen(outfile, "wb");
	if (!fp) { return false; }

	fwrite(header, DATABIN_HEADER_LEN,1, fp);
	fwrite(body, pcmBodySize,1, fp);

	fclose(fp);
	return true;
}


int PcmTool::Convert(const char* filename)
{
	FILE* fp = fopen(filename, "r");
	if (!fp) {
		printf("File open error!\n");
	}

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char name[_MAX_FNAME];
	SplitPath(filename, drive, dir, name, NULL);

	char wd[_MAX_PATH];
	char oldwd[_MAX_PATH];
	_getcwd(oldwd, _MAX_PATH);
	strcpy(wd, drive);
	strcat(wd, dir);
	// printf("wd:%s\n", wd);
	_chdir(wd);

	if (ConvertList(fp)) {
		char outbinname[_MAX_FNAME];
		strcpy(outbinname, name);
		strcat(outbinname, ".bin");
		printf("PCM:%s len=%d\n", outbinname, pcmBodySize + DATABIN_HEADER_LEN);
		if (!WriteBinary(outbinname)) {
			printf("Failed to write PCM!\n");
		}
	}

	_chdir(oldwd);
	fclose(fp);

	return 0;
}

bool PcmTool::ConvertList(FILE* fp) {
	entryCount = 0;
	pcmBodySize = 0;

	while (!feof(fp)) {
		if (entryCount >= 32) break;
		char infile[_MAX_FNAME];
		infile[0] = 0;
		fgets(infile, _MAX_FNAME, fp);
		int len = strlen(infile);
		if (len == 0) break;

		// 改行を削除(trim)
		while (len > 0 && infile[len - 1] < 0x20) {
			infile[len - 1] = 0;
			len--;
		}

		char outfile[_MAX_FNAME];
		char outname[_MAX_FNAME];
		SplitPath(infile, NULL, NULL, outname, NULL);
		strcpy(outfile, outname);
		strcat(outfile, "_adpcm.bin");
		printf("%s -> %s ", infile, outfile);
		cmucom->ConvertADPCM(infile, outfile);

		if (!entry[entryCount].SetData(outname, outfile)) {
			printf("failed to read %s!\n", outname);
			return false;
		}

		int pcmlen = entry[entryCount].GetLength();
		printf("len=%d\n", pcmlen);

		// 出力アドレス確定
		entry[entryCount].SetStart(pcmBodySize);
		pcmBodySize += NextAddress(pcmlen);

		entryCount++;
	}

	// サイズ確認
	if (pcmBodySize >= DATABIN_BODY_MAXLEN) {
		printf("over maxsize of PCM!\n");
		return false;
	}

	header = new unsigned char[DATABIN_HEADER_LEN];
	body = new unsigned char[pcmBodySize];
	memset(header, 0, DATABIN_HEADER_LEN);
	memset(body, 0, pcmBodySize);

	// エントリ作成
	int i = 0;
	int dest = 0;
	for (i = 0; i < entryCount; i++) {
		int len = entry[i].GetLength();
		memcpy(body + dest, entry[i].data, len);
		dest += NextAddress(len);
		entry[i].SetEntry(header + (i * 32));
	}

	return true;
}

int PcmTool::NextAddress(int len) {
	int a = (len & 0x03);
	return (a > 0) ? len + (4 - a) : len;
}


