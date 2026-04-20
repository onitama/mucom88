#pragma once
#include	<Windows.h>
#include	<crtdbg.h>
#include	"GraphicBase.h"

class GraphicData:public GraphicBase
{
public:
	GraphicData();
	~GraphicData();
	// ビットマップ読み込み
	BOOL	loadBitmap(BYTE *pData);
	BOOL	loadBitmapFile(char *pFileName);
	BOOL	loadBitmapResource(HINSTANCE hInst,HRSRC hRes);
	// PNG読み込み
	BOOL	loadPng(char *pFileName);
	BOOL	loadPngResource(HINSTANCE hInst,HRSRC hRes);
	// カラーキーの設定
	void	setColorKeyPos(int x, int y);
	void	setColorKey(DWORD dColorKey);
};

