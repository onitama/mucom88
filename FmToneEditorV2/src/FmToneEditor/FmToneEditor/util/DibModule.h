#pragma once
#include	<Windows.h>
#include	<crtdbg.h>
#include	<cmath>
#include	"GraphicBase.h"
#include	"GraphicData.h"

#define DIBMODULE_RGB(r, g ,b) ((DWORD)(((BYTE)(b) | ((WORD)(g) << 8)) | (((DWORD)(BYTE)(r)) << 16)))


class DibModule:public GraphicBase
{
private:
	typedef struct {
		int	index;			// 番号
		int	value;
	} SortIntegerIndex;

	typedef struct {
		int	iDest;			// 加算方向
		int	iValue;			// 現在値
		int	iThreshold;		// 閾値
		int	iAddValue;		// 加算値
	} BresenhamValues;

	typedef struct {
		int	x;
		int	y;
		int	z;
		double u;
		double v;
		double w;
	} DrawPrimitive;
private:
	// プライベート変数
	BITMAPINFO	*m_pBmp;
	HBITMAP		m_hBitmap;
	HBITMAP		m_hBeforeBitmap;
	HDC			m_hBitmapHdc;
	HFONT		m_hFont;
	HFONT		m_hFontOld;
public:
	// コンストラクタ
	DibModule();
	// デストラクタ
	~DibModule();

	// 初期化
	BOOL initialize(int width,int height);
	// 解放
	void uninitialize();

	// 表示
	void	paint(HDC hdc);
	// サイズ指定
	void	paint(HDC hdc,int x,int y,int width,int height);

	// レンダリングルーチン
	void	clean();

	// フォント関連
	void SetFont(int iHeight,LPCSTR lpsFontName,int iWidth = 0);

	// フォントカラー
	void setFontColor(DWORD dColor);

	// 文字表示
	void drawFont(int x,int y,const char *pFormat,...);

	// 描画
	void	draw(int x, int y, GraphicBase *pData);
	void	draw(int x, int y, GraphicBase *pData, int sx, int sy, int w, int h);
	void	draw(int x, int y, int w, int h, GraphicBase *pData, int sx, int sy, int sw, int sh);
	void	drawAlpha(int x,int y,GraphicBase *graphic,DWORD color,BYTE bAlpha);
	void	drawAlpha(int x,int y,int w,int h,int srcX,int srcY,GraphicBase *graphic,DWORD color,BYTE bAlpha);
	void	drawAddition(int x,int y,GraphicBase *graphic,DWORD color);
	void	drawAddition(int x,int y,int w,int h,int srcX,int srcY,GraphicBase *graphic,DWORD color);
	void	fillBox(int x,int y,int w,int h,DWORD dRGB);
	void	fillBoxAlpha(int x,int y,int w,int h,DWORD dRGB,BYTE bAlpha);
	inline void	pixel(int x, int y,DWORD dRGB);
	inline void	pixel(int x, int y,DWORD dRGB,BYTE bAlpha);
	void	line(int xs,int ys,int xe,int ye,DWORD dRGB,BYTE bAlpha);
	void	line(int xs,int ys,int xe,int ye,DWORD dRGB);
	// dib→dib
	void	drawDibToDib(DibModule *srcDib,int x,int y,int w,int h,int sx,int sy);

	// 座標変換系
	inline int positionInt(double pos){
		return (int)std::floor(pos + 0.5);
	}
	// フォント関連

};

