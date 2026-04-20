#include "GraphicData.h"

#ifdef _DEBUG
#define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

// コンストラクタ
GraphicData::GraphicData(void):GraphicBase()
{
	m_pData = NULL;
	m_dColorKey = 0xffffffff;
}

// デストラクタ
GraphicData::~GraphicData(void)
{
	// 解放
	if(m_pData != NULL){
		delete [] m_pData;
		m_pData = NULL;
	}
}

// ビットマップ読み込み
BOOL GraphicData::loadBitmap(BYTE *pData)
{
	void				*bmpdata;
	LONG_PTR			lPaletteSize;
	BITMAPFILEHEADER	*pBmpFileHeader;
	BITMAPINFOHEADER	*pBmpInfoHrader;
	BITMAPINFO			*pBmpInfo;
	int					BmpPitch;		//	ビットマップのピッチサイズ
	// 上書きする場合は解放する場合は解放
	if(m_pData != NULL){
		delete [] m_pData;
		m_pData = NULL;
		m_dColorKey = 0xffffffff;
	}
	//	---------- リソースがBMPかチェックする ----------
	pBmpFileHeader = (LPBITMAPFILEHEADER)pData;
	if(pBmpFileHeader->bfType != 'MB'){
		return	FALSE;
	}
	//	---------- ＢＭＰサイズ計算 ----------
	pBmpInfoHrader = (LPBITMAPINFOHEADER)((LONG_PTR)pBmpFileHeader + sizeof(BITMAPFILEHEADER));
	pBmpInfo = (LPBITMAPINFO)pBmpInfoHrader;
	m_lPitch		= (32 * pBmpInfoHrader->biWidth) / 8;	//	ピッチを計算
	if(m_lPitch % 4){
		m_lPitch += 4 - (m_lPitch % 4);
	}
	BmpPitch		= (pBmpInfoHrader->biBitCount * pBmpInfoHrader->biWidth) / 8;	//	ピッチを計算
	if(BmpPitch % 4){
		BmpPitch += 4 - (BmpPitch % 4);
	}
	m_iWidth	= (int)pBmpInfoHrader->biWidth;
	m_iHeight	= (int)pBmpInfoHrader->biHeight;
	//	---------- データ領域作製 ----------
	m_pData = new BYTE[m_lPitch * pBmpInfoHrader->biHeight];
	if(m_pData == NULL){
		return	FALSE;
	}
	//	----------- BMPデータを転送する -----------
	switch(pBmpInfoHrader->biBitCount){
		case	4:
			lPaletteSize = ((pBmpInfoHrader->biClrUsed) ? pBmpInfoHrader->biClrUsed : 16) * sizeof(RGBQUAD);
			break;
		case	8:
			lPaletteSize = ((pBmpInfoHrader->biClrUsed) ? pBmpInfoHrader->biClrUsed : 256) * sizeof(RGBQUAD);
			break;
		case	24:
			lPaletteSize = 0;
			break;
		case	32:
			if(pBmpInfoHrader->biCompression == BI_BITFIELDS){
				lPaletteSize = sizeof(DWORD) * 3;		//	マスク分
			}else{
				lPaletteSize = NULL;
			}
			break;
		default:
			delete [] m_pData;
			m_pData = NULL;
			return	FALSE;

	}
	//	----------- ビットマップデータ領域チェック ---------
	BYTE	*p,*q;
	int		i,j;
	bmpdata = (void*)((LONG_PTR)pBmpInfo->bmiColors + lPaletteSize);
	//	---------- トップダウンコピー ----------
	switch(pBmpInfoHrader->biBitCount){
		case	4:
			for(j = 0; j < m_iHeight; j++){
				p = (BYTE*)((LONG_PTR)bmpdata + (BmpPitch * (m_iHeight - 1 - j)));
				q = (BYTE*)((LONG_PTR)m_pData + (m_iHeight * j));
				for(i = 0; i < m_iWidth / 2; i++){
					*q++ = pBmpInfo->bmiColors[*p & 0xf].rgbRed;		//	R
					*q++ = pBmpInfo->bmiColors[*p & 0xf].rgbGreen;		//	G
					*q++ = pBmpInfo->bmiColors[*p & 0xf].rgbBlue;		//	B
					*q++ = pBmpInfo->bmiColors[*p & 0xf].rgbReserved;	//	リザーブド
					*q++ = pBmpInfo->bmiColors[*p >> 4].rgbRed;			//	R
					*q++ = pBmpInfo->bmiColors[*p >> 4].rgbGreen;		//	G
					*q++ = pBmpInfo->bmiColors[*p >> 4].rgbBlue;		//	B
					*q++ = pBmpInfo->bmiColors[*p >> 4].rgbReserved;	//	リザーブド
					*p++;
				}
			}
			break;
		case	8:
			for(j = 0; j < m_iHeight; j++){
				p = (BYTE*)((LONG_PTR)bmpdata + (BmpPitch * (m_iHeight - 1 - j)));
				q = (BYTE*)((LONG_PTR)m_pData + (m_iHeight * j));
				for(i = 0; i < m_iWidth; i++){
					*q++ = pBmpInfo->bmiColors[*p].rgbRed;		//	R
					*q++ = pBmpInfo->bmiColors[*p].rgbGreen;		//	G
					*q++ = pBmpInfo->bmiColors[*p].rgbBlue;		//	B
					*q++ = pBmpInfo->bmiColors[*p].rgbReserved;	//	リザーブド
					*p++;
				}
			}
			break;
		case	24:
			for(j = 0; j < m_iHeight; j++){
				p = (BYTE*)((LONG_PTR)bmpdata + (BmpPitch * (m_iHeight - 1 - j)));
				q = (BYTE*)((LONG_PTR)m_pData + (m_lPitch * j));
				for(i = 0; i < m_iWidth; i++){
					*q++ = *p++;	//	R
					*q++ = *p++;	//	G
					*q++ = *p++;	//	B
					*q++ = 0x00;	//	無し
				}
			}
			break;
		case	32:
			for(j = 0; j < m_iHeight; j++){
				p = (BYTE*)((LONG_PTR)bmpdata + (BmpPitch * (m_iHeight - 1 - j)));
				q = (BYTE*)((LONG_PTR)m_pData + (m_lPitch * j));
				for(i = 0; i < m_iWidth; i++){
					*q++ = *p++;	//	R
					*q++ = *p++;	//	G
					*q++ = *p++;	//	B
					*q++ = *p++;	//	無し
				}
			}
			break;
	}
	return	TRUE;
}

// ビットマップ読み込み
BOOL GraphicData::loadBitmapFile(char *pFileName)
{
	return TRUE;
}

BOOL GraphicData::loadBitmapResource(HINSTANCE hInst,HRSRC hRes)
{
	BOOL	bRet;
	void	*pData;
	//	---------- リソースをロックする -----------
	HANDLE hHnd = LoadResource(hInst,hRes);
	pData = LockResource(hHnd);
	if(pData == NULL) return FALSE;
	// BMPをメモリへ展開
	bRet = loadBitmap((BYTE*)pData);
	//	----------- リソースを解放 ----------
	UnlockResource(hHnd);
	return	bRet;
}

// PNG読み込み
BOOL GraphicData::loadPng(char *pFileName)
{
	return TRUE;
}

BOOL GraphicData::loadPngResource(HINSTANCE hInst,HRSRC hRes)
{
	return TRUE;
}

// カラーキー設定（場所）
void GraphicData::setColorKeyPos(int x, int y)
{
	DWORD *pPixel = (DWORD*)(m_pData + (y * m_lPitch) + x * 4);
	m_dColorKey = *pPixel;
}

// カラーキー設定（直値）
void GraphicData::setColorKey(DWORD dColorKey)
{
	m_dColorKey = dColorKey;
}



