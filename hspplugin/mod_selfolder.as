#ifndef __MOD_SELFOLDER
#define __MOD_SELFOLDER

;FolderSelect Module (inovia)

#module mFolderSelect

#define CLSID_FileOpenDialog "{DC1C5A9C-E88A-4dde-A5A1-60F82A20AEF7}"
#define IID_IFileOpenDialog "{d57c7288-d4ad-4768-be02-9d969532d960}"
#usecom IFileOpenDialog IID_IFileOpenDialog CLSID_FileOpenDialog

#comfunc IFileOpenDialog_Show 3 int
#comfunc IFileOpenDialog_SetOptions 9 int
#comfunc IFileOpenDialog_GetOptions 10 var
#comfunc IFileOpenDialog_SetFolder 12 int
#comfunc IFileOpenDialog_GetResult 20 var

#define IID_IShellItem "{43826d1e-e718-42ee-bc55-a1e261c37bfe}"
#usecom IShellItem IID_IShellItem
#comfunc IShellItem_GetDisplayName 5 int, var

#define FOS_PICKFOLDERS 0x20
#define SIGDN_DESKTOPABSOLUTEPARSING 0x80028000

#uselib "shell32"
#func SHCreateItemFromParsingName "SHCreateItemFromParsingName" wstr, sptr, var, var
#uselib "kernel32"
#cfunc lstrlenW "lstrlenW" sptr
#uselib "ole32"
#func CoTaskMemFree "CoTaskMemFree" sptr
#uselib "comdlg32"
#func GetOpenFileName "GetOpenFileNameA" int
#func GetSaveFileName "GetSaveFileNameA" int

#define ctype IsDisableComObj(%1) (vartype(%1) != 6 || varuse(%1) == 0)
#define global ctype FolderDialog(%1="") _FolderDialog(%1)

#defcfunc _FolderDialog str strDefPath

	newcom pFod, IFileOpenDialog
	if IsDisableComObj(pFod) : return ""

	if ( strDefPath != "") {
		ppShellItem = 0
		IShellItem_GUID = 0x43826d1e, 0x42eee718, 0xe2a155bc, 0xfe7bc361
		SHCreateItemFromParsingName strDefPath, 0, IShellItem_GUID, ppShellItem
		if ( ppShellItem != 0){
			newcom pShellItem, IID_IShellItem, -1, ppShellItem
			IFileOpenDialog_SetFolder pFod, ppShellItem
			delcom pShellItem
		}
	}
	
	dwOptions = 0
	IFileOpenDialog_GetOptions pFod, dwOptions
	IFileOpenDialog_SetOptions pFod, dwOptions | FOS_PICKFOLDERS
	IFileOpenDialog_Show pFod, hwnd
	
	ppsi = 0
	IFileOpenDialog_GetResult pFod, ppsi
	
	if ppsi == 0 {
		delcom pFod
		return ""
	}
	
	newcom pSi, IID_IShellItem, -1, ppsi
	if IsDisableComObj(pSi) : delcom pFod : return ""
	
	pszPath = 0
	IShellItem_GetDisplayName pSi, SIGDN_DESKTOPABSOLUTEPARSING, pszPath
	
	if pszPath == 0 {
		delcom pSi
		delcom pFod
		return ""
	}
	
	dupptr szPath, pszPath, lstrlenW(pszPath)*2, 2
	path = cnvwtos(szPath)

	CoTaskMemFree pszPath
	delcom pSi
	delcom pFod
	
return path


/*--------------------------------------------------------------------------

	[HSP3] 拡張したファイルの開く/保存ダイアログ モジュール
	by Kpan
	http://lhsp.s206.xrea.com/ (Let's HSP!)

	dialogEx p1, p2, p3, p4, p5
		p1=ダイアログの種類 {開く(0) / 保存(1)}
		p2=[ファイルの種類]で表示する文字列
		p3=[ファイル名]で表示する文字列
		p4=ダイアログのタイトル名
		p5=初期フォルダパス

	ファイルの開くダイアログ(dialog命令のタイプ16相当)、保存ダイア
	ログ(タイプ17相当)を表示します。ファイルの選択が正常に行われると、
	statに1、refstrにファイルパスが返ります。キャンセルやエラーの
	場合はstatに0が返ります。

	p2はダイアログの[ファイルの種類]項目で表示する文字列です。リスト
	ボックスに複数の拡張子を用意できます。｢文字列|*.拡張子|～｣という
	形式で書いてください。(詳しくはサンプル参照)

	p3はダイアログの[ファイル名]項目で表示する文字列です。dialog命令
	では拡張子が自動的に表示される部分です。

	p4はダイアログのタイトル名です。｢""｣という形で省略した場合、自動
	的に｢ファイルを開く｣、｢名前をつけて保存｣になります。

	p5は初期フォルダのパスです。chdir命令に相当します。

	(注) モジュールを使用するに当たって、スクリプトエディタの[HSP]
	メニューの[HSP拡張マクロを使用する]を有効にしておいてください。

--------------------------------------------------------------------------*/

#deffunc dialogEx int v0, str v1, str v2, str v3, str v4

	sdim lpstrFilter, 256 : lpstrFilter = v1
	sdim lpstrInitialDir, 128 : lpstrInitialDir = v4
	sdim lpstrFile, 128 : lpstrFile = v2
	sdim lpstrTitle, 64 : lpstrTitle = v3

	repeat strlen (lpstrFilter)
		x = peek (lpstrFilter, cnt)
		if x>$80 & x<$A0 | x>$DF & x<$FD : continue cnt+2
		if x = $7C : poke lpstrFilter, cnt, $00
	loop

	prm = 76, hwnd, hinstance, varptr (lpstrFilter), 0, 0, 0, varptr (lpstrFile), 256, 0, 0, varptr (lpstrInitialDir), varptr (lpstrTitle)

	if v0 {
		prm.13 = $806 : GetSaveFileName varptr (prm)
	} else {
		prm.13 = $1004 : GetOpenFileName varptr (prm)
	}

	if stat = 0 : return
	return lpstrFile


#global
#endif

