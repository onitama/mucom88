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

#global
#endif

