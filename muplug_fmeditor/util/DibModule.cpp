#include "DibModule.h"
#include	<stdio.h>
#include	<math.h>

#ifdef _DEBUG
#define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// コンストラクタ
DibModule::DibModule():GraphicBase()
{
	m_pBmp = NULL;
	m_hBitmap = NULL;
	m_hBeforeBitmap = NULL;
	m_hBitmapHdc = NULL;
	m_pData = NULL;
	m_hFont = NULL;
}

// デストラクタ
DibModule::~DibModule()
{
}

// 初期化
BOOL DibModule::initialize(int width,int height)
{
	// 確保済みの場合は再初期化
	uninitialize();
	// ビットマップ構造体
	LPBITMAPINFO pBmp = (LPBITMAPINFO)new BYTE[sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 254];
	ZeroMemory(pBmp,sizeof(BITMAPINFO));
	pBmp->bmiHeader.biSize			= sizeof(BITMAPINFOHEADER);	//	ヘッダサイズの指定
	pBmp->bmiHeader.biWidth			= width;					//	Ｘサイズ
	pBmp->bmiHeader.biHeight		= -height;
	pBmp->bmiHeader.biPlanes		= 1;						//	プレーン数
	pBmp->bmiHeader.biBitCount		= 32;						//	作成するDibのBit深度
	DWORD *pMask = (DWORD*)&pBmp->bmiColors;
	pBmp->bmiHeader.biCompression	= BI_BITFIELDS;				//	圧縮
	pMask[0] = 0x00ff0000;
	pMask[1] = 0x0000ff00;
	pMask[2] = 0x000000ff;
	pBmp->bmiHeader.biSizeImage		= width * 4 * height;		//	イメージサイズ
	pBmp->bmiHeader.biXPelsPerMeter	= 0;
	pBmp->bmiHeader.biYPelsPerMeter	= 0;
	pBmp->bmiHeader.biClrUsed			= 0;
	pBmp->bmiHeader.biClrImportant	= 0;
	m_hBitmap = CreateDIBSection(NULL,pBmp,DIB_RGB_COLORS,(void**)&m_pData,NULL,0);
	if(m_hBitmap == NULL){
		return FALSE;
	}
	m_hBitmapHdc = CreateCompatibleDC(NULL);
	m_hBeforeBitmap = (HBITMAP)SelectObject(m_hBitmapHdc,m_hBitmap);
	m_iWidth = width;
	m_iHeight = height;
	m_lPitch = (LONG_PTR)(width * 4);
	delete [] (BYTE*)pBmp;
	return TRUE;
}

// 解放
void DibModule::uninitialize()
{
	if(m_hFont != NULL){
		SelectObject(m_hBitmapHdc,m_hFontOld);
		DeleteObject(m_hFont);
	}
	if(m_hBitmapHdc != NULL){
		SelectObject(m_hBitmapHdc,m_hBeforeBitmap);
		ReleaseDC(NULL,m_hBitmapHdc);
		m_hBitmapHdc = NULL;
	}
	if(m_hBitmap != NULL){
		DeleteObject(m_hBitmap);
		m_hBitmap= NULL;
	}
}

// 表示
void DibModule::paint(HDC hdc)
{
	if(m_hBitmap != NULL){
		BitBlt(hdc,0,0,m_iWidth,m_iHeight,m_hBitmapHdc,0,0,SRCCOPY);
	}
}

// サイズ指定
void DibModule::paint(HDC hdc,int x,int y,int width,int height)
{
	if(m_hBitmap != NULL){
		StretchBlt(hdc,x,y,width,height,m_hBitmapHdc,0,0,m_iWidth,m_iHeight,SRCCOPY);
	}
}

// 画面クリア
void DibModule::clean()
{
	DWORD	*pAddr = (LPDWORD)m_pData;
	for(int i = 0; i < (m_iWidth * m_iHeight); i++) *pAddr++ = 0x00000000;
}

// フォントの設定
void DibModule::SetFont(int iHeight,LPSTR lpsFontName,int iWidth){
	m_hFont = CreateFont(	iHeight,				// フォント高さ
							iWidth,					// フォント幅
							0,						// 角度
							0,						// 文字の角度
							FW_REGULAR,				// 太さ
							FALSE,					// イタリック
							FALSE,					// アンダーライン
							FALSE,					// 取り消し線
							SHIFTJIS_CHARSET,		//文字セット
							OUT_DEFAULT_PRECIS,		//出力精度
							CLIP_DEFAULT_PRECIS,	//クリッピング精度
							PROOF_QUALITY,			//出力品質
							FIXED_PITCH,			//ピッチとファミリー
							lpsFontName);    	//書体名
	if(m_hFont != NULL){
		m_hFontOld = (HFONT)SelectObject(m_hBitmapHdc,m_hFont);
		SetTextColor(m_hBitmapHdc, RGB(255, 255, 255));
		SetBkMode(m_hBitmapHdc,TRANSPARENT);
	}
}

// フォントカラーの設定
void DibModule::setFontColor(DWORD dColor){
	if(m_hFont != NULL){
		SetTextColor(m_hBitmapHdc, RGB((dColor >> 16) & 0xff, (dColor >> 8) & 0xff, dColor & 0xff));
	}
}

// フォントの表示
void DibModule::drawFont(int x,int y,char *pFormat,...){
	va_list ap;
	char	drawStr[2048];
	//	---------print main---------
	va_start(ap, pFormat);
	wvsprintf(drawStr, pFormat, ap);
	va_end(ap);
	// テキスト表示
	TextOut(m_hBitmapHdc,x,y,drawStr,lstrlen(drawStr));
}

// 描画
void DibModule::draw(int x, int y, GraphicBase *pData)
{
	draw(x,y,pData,0,0,pData->m_iWidth,pData->m_iHeight);
}

// 描画
void DibModule::draw(int x, int y, GraphicBase *pData, int sx, int sy, int w, int h)
{
	void	*disaddr,*srcaddr;
	LONG_PTR	srcPitch,disPitch;
	DWORD	dColorKey = pData->m_dColorKey;
	// クリッピング
	if(x >= m_iWidth) return;	// 右画面外
	if(y >= m_iHeight) return;	// 下画面外
	if(x + w <= 0) return;		// 左画面外
	if(y + h <= 0) return;		// 上部画面外
	// 補正
	if(x < 0){
		w += x;
		sx -= x;
		x = 0;
	}
	if(y < 0){
		h += y;
		sy -= y;
		y = 0;
	}
	if(x + w >= m_iWidth) w -= (x + w) - m_iWidth;
	if(y + h >= m_iHeight) h -= (y + h) - m_iHeight;

	// 元画像の開始位置及び加算値を算出
	srcaddr = (void*)((LONG_PTR)(pData->m_pData) + (sx * 4) + (pData->m_lPitch * sy));
	srcPitch = (pData->m_lPitch - (w * 4)) >> 2;
	// 書込み先の位置及びピッチの計算
	disaddr = (void*)((LONG_PTR)(m_pData) + (x * 4) + (m_lPitch * y));
	disPitch = (m_lPitch - (w * 4)) >> 2;
	DWORD *pSrcAddr = static_cast<DWORD*>(srcaddr);
	DWORD *pDisAddr = static_cast<DWORD*>(disaddr);
	if(dColorKey == 0xffffffff){
		for(int iy = 0; iy < h; iy++){
			for(int ix = 0; ix < w; ix++){
				*pDisAddr++ = *pSrcAddr++;
			}
			pSrcAddr += srcPitch;
			pDisAddr += disPitch;
		}
	}else{
		for(int iy = 0; iy < h; iy++){
			for(int ix = 0; ix < w; ix++){
				if(*pSrcAddr != dColorKey){
					*pDisAddr = *pSrcAddr;
				}
				pSrcAddr++;
				pDisAddr++;
			}
			pSrcAddr += srcPitch;
			pDisAddr += disPitch;
		}
	}
}

void	DibModule::draw(int x, int y, int w, int h, GraphicBase *pData, int sx, int sy, int sw, int sh)
{
	void		*disaddr,*srcaddr;
	LONG_PTR	srcPitch,disPitch;

	// 元画像の開始位置及び加算値を算出
	srcaddr = (void*)((LONG_PTR)(pData->m_pData) + (sx * 4) + (pData->m_lPitch * sy));
	srcPitch = pData->m_lPitch;
	// 書込み先の位置及びピッチの計算
	disaddr = (void*)((LONG_PTR)(m_pData) + (x * 4) + (m_lPitch * y));
	disPitch = m_lPitch - (w * 4);
	// カラーキー
	// 書き込みます
	DWORD *pSrcAddr = static_cast<DWORD*>(srcaddr);
	DWORD *pDisAddr = static_cast<DWORD*>(disaddr);
	DWORD *pSrcAddrSave = pSrcAddr;
	DWORD dColorKey = pData->m_dColorKey;
	int iYDiff = 0;
	for(int iy = 0; iy < h; iy++){
		int iXDiff = 0;
		for(int ix = 0; ix < w; ix++){
			if(*pSrcAddr != dColorKey){
				*pDisAddr = *pSrcAddr;
			}
			iXDiff += sw;
			while(iXDiff >= w){
				pSrcAddr++;
				iXDiff -= w;
			}
			pDisAddr++;
		}
		iYDiff += sh;
		while(iYDiff >= h){
			pSrcAddrSave += srcPitch / 4;
			iYDiff -= h;
		}
		pSrcAddr = pSrcAddrSave;
		pDisAddr += disPitch / 4;
	}
}

void	DibModule::drawAlpha(int x,int y,GraphicBase *graphic,DWORD color,BYTE bAlpha){
	drawAlpha(x,y,graphic->m_iWidth,graphic->m_iHeight,0,0,graphic,color,bAlpha);
}

void	DibModule::drawAlpha(int x,int y,int w,int h,int srcX,int srcY,GraphicBase *graphic,DWORD color,BYTE bAlpha)
{
	void	*disaddr,*srcaddr;
	LONG_PTR	srcPitch,disPitch;
	DWORD	dAlpha;
	// クリッピング
	if(x >= m_iWidth) return;	// 右画面外
	if(y >= m_iHeight) return;	// 下画面外
	if(x + w <= 0) return;		// 左画面外
	if(y + h <= 0) return;		// 上部画面外
	// 補正
	if(x < 0){
		w += x;
		srcX -= x;
		x = 0;
	}
	if(y < 0){
		h += y;
		srcY -= y;
		y = 0;
	}
	if(x + w >= m_iWidth) w -= (x + w) - m_iWidth;
	if(y + h >= m_iHeight) h -= (y + h) - m_iHeight;

	// 元画像の開始位置及び加算値を算出
	srcaddr = (void*)((LONG_PTR)(graphic->m_pData) + (srcX * 4) + (graphic->m_lPitch * srcY));
	srcPitch = graphic->m_lPitch - (w * 4);
	// 書込み先の位置及びピッチの計算
	disaddr = (void*)((LONG_PTR)(m_pData) + (x * 4) + (m_lPitch * y));
	disPitch = m_lPitch - (w * 4);
	// αを設定します
	RGBQUAD *pRgb = (RGBQUAD*)&dAlpha;
	pRgb->rgbRed = 255 - bAlpha;
	pRgb->rgbBlue = 255 - bAlpha;
	pRgb->rgbGreen = 255 - bAlpha;
	// ソースの色？
	pRgb = (RGBQUAD*)&color;
	pRgb->rgbRed = (BYTE)((pRgb->rgbRed * bAlpha) / 255);
	pRgb->rgbGreen = (BYTE)((pRgb->rgbGreen * bAlpha) / 255);
	pRgb->rgbBlue = (BYTE)((pRgb->rgbBlue * bAlpha) / 255);
	// 書き込みます
	RGBQUAD *pSrc = static_cast<RGBQUAD*>(srcaddr);
	RGBQUAD *pDis = static_cast<RGBQUAD*>(disaddr);
	RGBQUAD *pColor = reinterpret_cast<RGBQUAD*>(&color);
	for(int iy = 0; iy < h; iy++){
		for(int ix = 0; ix < w; ix++){
			if(*(DWORD*)pSrc != graphic->m_dColorKey || graphic->m_dColorKey == 0xffffff){
				pDis->rgbRed	= (pDis->rgbRed * (255 - bAlpha) + ((pSrc->rgbRed * pColor->rgbRed / 255) * bAlpha)) / 255;
				pDis->rgbBlue	= (pDis->rgbBlue * (255 - bAlpha) + ((pSrc->rgbBlue * pColor->rgbBlue / 255) * bAlpha)) / 255;
				pDis->rgbGreen	= (pDis->rgbGreen * (255 - bAlpha) + ((pSrc->rgbGreen * pColor->rgbGreen / 255)* bAlpha)) / 255;
			}
			pSrc++;
			pDis++;
		}
		pSrc += srcPitch / 4;
		pDis += disPitch / 4;
	}
}

void	DibModule::drawAddition(int x,int y,GraphicBase *graphic,DWORD color){
	drawAddition(x,y,graphic->m_iWidth,graphic->m_iHeight,0,0,graphic,color);
}

void	DibModule::drawAddition(int x,int y,int w,int h,int srcX,int srcY,GraphicBase *graphic,DWORD color)
{
	void	*disaddr,*srcaddr;
	LONG_PTR	srcPitch,disPitch;
	// クリッピング
	if(x >= m_iWidth) return;	// 右画面外
	if(y >= m_iHeight) return;	// 下画面外
	if(x + w <= 0) return;		// 左画面外
	if(y + h <= 0) return;		// 上部画面外
	// 補正
	if(x < 0){
		w += x;
		srcX -= x;
		x = 0;
	}
	if(y < 0){
		h += y;
		srcY -= y;
		y = 0;
	}
	if(x + w >= m_iWidth) w -= (x + w) - m_iWidth;
	if(y + h >= m_iHeight) h -= (y + h) - m_iHeight;

	// 元画像の開始位置及び加算値を算出
	srcaddr = (void*)((LONG_PTR)(graphic->m_pData) + (srcX * 4) + (graphic->m_lPitch * srcY));
	srcPitch = graphic->m_lPitch - (w * 4);
	// 書込み先の位置及びピッチの計算
	disaddr = (void*)((LONG_PTR)(m_pData) + (x * 4) + (m_lPitch * y));
	disPitch = m_lPitch - (w * 4);
	// 書き込みます
	RGBQUAD *pSrc = static_cast<RGBQUAD*>(srcaddr);
	RGBQUAD *pDis = static_cast<RGBQUAD*>(disaddr);
	RGBQUAD *pColor = reinterpret_cast<RGBQUAD*>(&color);
	for(int iy = 0; iy < h; iy++){
		for(int ix = 0; ix < w; ix++){
			if(*(DWORD*)pSrc != graphic->m_dColorKey || graphic->m_dColorKey == 0xffffff){
				DWORD red = pDis->rgbRed + ((pSrc->rgbRed * pColor->rgbRed) / 255);
				if(red < 256){
					pDis->rgbRed = (BYTE)red;
				}else{
					pDis->rgbRed = 0xff;
				}
				DWORD green = pDis->rgbGreen + ((pSrc->rgbGreen * pColor->rgbGreen) / 255);
				if(green < 256){
					pDis->rgbGreen = (BYTE)green;
				}else{
					pDis->rgbGreen = 0xff;
				}
				DWORD blue = pDis->rgbBlue + ((pSrc->rgbBlue * pColor->rgbBlue) / 255);
				if(blue < 256){
					pDis->rgbBlue = (BYTE)blue;
				}else{
					pDis->rgbBlue = 0xff;
				}
			}
			pSrc++;
			pDis++;
		}
		pSrc += srcPitch / 4;
		pDis += disPitch / 4;
	}
}


void	DibModule::fillBox(int x,int y,int w,int h,DWORD dRGB)
{
	DWORD		dx,dy,dw,dheight;			//	送り先の情報
	LONG_PTR	pitch;
	//	---------- 画面外チェック ----------
	if(x >= m_iWidth) return;
	if(y >= m_iHeight) return;
	if(x <= -w) return;
	if(y <= -h) return;
	if(w == 0 || h == 0) return;
	//	---------- クリッピング計算 ----------
	if(x < 0){		//	左にはみ出た場合
		//	----- 表示開始Ｘ座標計算 -----
		dx = 0;						//	表示先
		dw = w + x;
	}else if(x + w >= m_iWidth){	//	横幅がはみ出る場合
		dx = x;
		dw = m_iWidth - x;
	}else{
		dx = x;
		dw = w;
	}
	if(y < 0){		//	左にはみ出た場合
		//	----- 表示開始Ｘ座標計算 -----
		dy = 0;						//	表示先
		dheight = h + y;
	}else if(y + h >= m_iHeight){	//	横幅がはみ出る場合
		dy = y;
		dheight = m_iHeight - y;
	}else{
		dy = y;
		dheight = h;
	}
	// 描画処理
	DWORD *pDisaddr = (DWORD*)((LONG_PTR)(m_pData) + (dx * 4) + (dy * m_lPitch));
	pitch = m_lPitch - (dw * 4);
	for(DWORD dYPos = 0; dYPos < dheight; dYPos++){
		for(DWORD dXPos = 0; dXPos < dw; dXPos++){
			*pDisaddr++ = dRGB;
		}
		pDisaddr += (pitch / 4);
	}
}

//	---------- DIBにFillする（アルファ付き） ----------
void	DibModule::fillBoxAlpha(int x,int y,int w,int h,DWORD dRGB,BYTE bAlpha)
{
	DWORD		dx,dy,dw,dheight;			//	送り先の情報
	LONG_PTR	pitch;
	void		*disaddr;
	LONG_PTR	disPitch;
	if(x >= m_iWidth) return;
	if(y >= m_iHeight) return;
	if(x <= -w) return;
	if(y <= -h) return;
	if(w == 0 || h == 0) return;
	//	---------- クリッピング計算 ----------
	if(x < 0){		//	左にはみ出た場合
		//	----- 表示開始Ｘ座標計算 -----
		dx = 0;						//	表示先
		dw = w + x;
	}else if(x + w >= m_iWidth){	//	横幅がはみ出る場合
		dx = x;
		dw = m_iWidth - x;
	}else{
		dx = x;
		dw = w;
	}
	if(y < 0){		//	左にはみ出た場合
		//	----- 表示開始Ｘ座標計算 -----
		dy = 0;						//	表示先
		dheight = h + y;
	}else if(y + h >= m_iHeight){	//	横幅がはみ出る場合
		dy = y;
		dheight = m_iHeight - y;
	}else{
		dy = y;
		dheight = h;
	}
	// 書込み先の位置及びピッチの計算
	disaddr = (void*)((LONG_PTR)(m_pData) + (dx * 4) + (m_lPitch * dy));
	disPitch = m_lPitch - (dw * 4);
	
	RGBQUAD *pSrcRGB = (RGBQUAD*)&dRGB;

	DWORD *pDisaddr = (DWORD*)((LONG_PTR)(m_pData) + (dx * 4) + (dy * m_lPitch));
	pitch = m_lPitch - (dw * 4);
	for(DWORD dYPos = 0; dYPos < dheight; dYPos++){
		RGBQUAD *pRgb = (RGBQUAD *)disaddr;
		for(DWORD dXPos = 0; dXPos < dw; dXPos++){
			pRgb->rgbRed = ((pRgb->rgbRed * (255 - bAlpha)) + pSrcRGB->rgbRed * bAlpha) / 255;
			pRgb->rgbGreen = ((pRgb->rgbGreen * (255 - bAlpha)) + pSrcRGB->rgbGreen * bAlpha) /255;
			pRgb->rgbBlue = ((pRgb->rgbBlue * (255 - bAlpha)) + pSrcRGB->rgbBlue * bAlpha) /255;
			pRgb++;
		}
		disaddr = (LPVOID)((LONG_PTR)pRgb + disPitch);
	}
}

// ピクセルを打つ
void	DibModule::pixel(int x, int y,DWORD dRGB)
{
	// クリップ
	if(x < 0 || y < 0 || x >= m_iWidth || y >= m_iHeight) return;
	DWORD *pAddr = (DWORD*)((LONG_PTR)m_pData + x * 4 + y * m_lPitch);
	*pAddr = dRGB;
}

// ピクセルを打つ
void	DibModule::pixel(int x, int y,DWORD dRGB,BYTE bAlpha)
{
	RGBQUAD *pRGB = (RGBQUAD*)&dRGB;
	// クリップ
	if(x < 0 || y < 0 || x >= m_iWidth || y >= m_iHeight) return;
	BYTE *pAddr = (BYTE*)((LONG_PTR)m_pData + x * 4 + y * m_lPitch);
	pAddr[0] = (pAddr[0] * (255 - bAlpha) + pRGB->rgbBlue * bAlpha) / 255; 
	pAddr[1] = (pAddr[1] * (255 - bAlpha) + pRGB->rgbGreen * bAlpha) / 255; 
	pAddr[2] = (pAddr[2] * (255 - bAlpha) + pRGB->rgbRed * bAlpha) / 255; 
}


// ラインの描画
void	DibModule::line(int xs,int ys,int xe,int ye,DWORD dRGB){
		int xn = xe - xs;	// 移動量
	int yn = ye - ys;	// 移動量

	int xd,yd;
	// X移動方向
	xd = (xn > 0) ? 1 : -1;
	yd = (yn > 0) ? 1 : -1;

	// 絶対値に変換する
	xn = abs(xn);
	yn = abs(yn);

	int	iDiff;
	// 長いほうを基準として処理を実施する
	if(xn > yn){
		iDiff = xn;
		// X方向が長い場合
		for(int i = 0; i <= xn; i++){
			// ポイントを打つべし
			pixel(xs,ys,dRGB);
			// 差分を計算する
			xs += xd;			// X方向移動する
			iDiff -= yn;	// yの差分計算
			if(iDiff <= 0){
				// 移動させる
				ys += yd;
				iDiff += xn;
			}
		}
	}else{
		iDiff = yn;
		// X方向が長い場合
		for(int i = 0; i <= yn; i++){
			// ポイントを打つべし
			pixel(xs,ys,dRGB);
			// 差分を計算する
			ys += yd;			// X方向移動する
			iDiff -= xn;	// yの差分計算
			if(iDiff <= 0){
				// 移動させる
				xs += xd;
				iDiff += yn;
			}
		}
	}
}

// ラインの描画
void	DibModule::line(int xs,int ys,int xe,int ye,DWORD dRGB,BYTE bAlpha){
		int xn = xe - xs;	// 移動量
	int yn = ye - ys;	// 移動量

	int xd,yd;
	// X移動方向
	xd = (xn > 0) ? 1 : -1;
	yd = (yn > 0) ? 1 : -1;

	// 絶対値に変換する
	xn = abs(xn);
	yn = abs(yn);

	int	iDiff;
	// 長いほうを基準として処理を実施する
	if(xn > yn){
		iDiff = xn;
		// X方向が長い場合
		for(int i = 0; i <= xn; i++){
			// ポイントを打つべし
			pixel(xs,ys,dRGB,bAlpha);
			// 差分を計算する
			xs += xd;			// X方向移動する
			iDiff -= yn;	// yの差分計算
			if(iDiff <= 0){
				// 移動させる
				ys += yd;
				iDiff += xn;
			}
		}
	}else{
		iDiff = yn;
		// X方向が長い場合
		for(int i = 0; i <= yn; i++){
			// ポイントを打つべし
			pixel(xs,ys,dRGB,bAlpha);
			// 差分を計算する
			ys += yd;			// X方向移動する
			iDiff -= xn;	// yの差分計算
			if(iDiff <= 0){
				// 移動させる
				xs += xd;
				iDiff += yn;
			}
		}
	}
}

void	DibModule::drawDibToDib(DibModule *srcDib,int x,int y,int w,int h,int sx,int sy){
	DWORD	*disaddr,*srcaddr;

	// 元画像の開始位置及び加算値を算出
	srcaddr = (DWORD*)((LONG_PTR)(srcDib->m_pData)  + (sx * 4) + (srcDib->m_lPitch * sy));
	// 書込み先の位置及びピッチの計算
	disaddr = (DWORD*)((LONG_PTR)(m_pData) + (x * 4) + (m_lPitch * y));

	// コピーする
	for(int j = 0; j < h; j++){
		for(int i = 0; i < w; i++){
			disaddr[i] = srcaddr[i];
		}
		disaddr = (DWORD*)((LONG_PTR)disaddr + m_lPitch);
		srcaddr = (DWORD*)((LONG_PTR)srcaddr + srcDib->m_lPitch);
	}
}

