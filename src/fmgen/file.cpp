//	$Id: file.cpp,v 1.6 1999/12/28 11:14:05 cisc Exp $

#include <stdio.h>
#include <string.h>


#include "headers.h"
#include "file.h"

// ---------------------------------------------------------------------------
//	構築/消滅
// ---------------------------------------------------------------------------

FileIO::FileIO()
{
    fp = NULL;
    flags = 0;
}

FileIO::FileIO(const char* filename, uint flg)
{
    fp = NULL;
    flags = 0;
    Open(filename, flg);
}

FileIO::~FileIO()
{
    Close();
}

// ---------------------------------------------------------------------------
//	ファイルを開く
// ---------------------------------------------------------------------------

bool FileIO::Open(const char* filename, uint flg)
{
    Close();
    strncpy(path, filename, _MAXPATH);

    fp = fopen(filename, "rb");
    if (!fp) return false;

    SetLogicalOrigin(0);

    return true;
}

// ---------------------------------------------------------------------------
//	ファイルがない場合は作成
// ---------------------------------------------------------------------------

bool FileIO::CreateNew(const char* filename)
{
    Close();
    strncpy(path, filename, _MAXPATH);

    fp = fopen(filename, "wb");
    if (!fp) return false;

    SetLogicalOrigin(0);

    return !!(flags & open);
}

// ---------------------------------------------------------------------------
//	ファイルを作り直す
// ---------------------------------------------------------------------------

bool FileIO::Reopen(uint flg)
{
    return false;
}

// ---------------------------------------------------------------------------
//	ファイルを閉じる
// ---------------------------------------------------------------------------

void FileIO::Close()
{
    if (fp) fclose(fp);
    fp = NULL;
 }

// ---------------------------------------------------------------------------
//	ファイル殻の読み出し
// ---------------------------------------------------------------------------

int32 FileIO::Read(void* dest, int32 size)
{
    if (!fp) return -1;

    int len = (int)fread(dest, 1, size, fp);
    return len;
}

// ---------------------------------------------------------------------------
//	ファイルへの書き出し
// ---------------------------------------------------------------------------

int32 FileIO::Write(const void* dest, int32 size)
{
    if (!fp) return -1;

    int len = (int)fwrite(dest, 1, size, fp);
    return len;
}

// ---------------------------------------------------------------------------
//	ファイルをシーク
// ---------------------------------------------------------------------------

bool FileIO::Seek(int32 pos, SeekMethod method)
{
    if (!fp) return false;

    switch (method)
    {
    case begin:
        fseek(fp, pos, SEEK_SET);
        break;
    case current:
        fseek(fp, pos, SEEK_CUR);
        break;
    case end:
        fseek(fp, pos, SEEK_END);
        break;
    default:
        return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
//	ファイルの位置を得る
// ---------------------------------------------------------------------------

int32 FileIO::Tellp()
{
    if (!fp) return -1;

    return (int)ftell(fp);
}

// ---------------------------------------------------------------------------
//	現在の位置をファイルの終端とする
// ---------------------------------------------------------------------------

bool FileIO::SetEndOfFile()
{
    return false;
}
