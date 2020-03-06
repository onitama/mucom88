#ifndef __MOD_APPCONFIG
#define __MOD_APPCONFIG
;
;
;============================================================
; get_appdata_path
;  APP が使う Application Data の Path
;  (S.Programs HDL汎用ライブラリ)

#module

#ifndef _HSP3DISH

; winapi
#uselib "kernel32"
#func GetEnvironmentVariable	"GetEnvironmentVariableA" sptr, sptr, sptr
;--------------------------------------------------
#deffunc get_appdata_path array v1, str _appname
	org_dir = dir_exe
	sdim v1, 1000
	GetEnvironmentVariable "APPDATA", varptr(v1), 990

	if "" ! v1 {
		subdir = _appname
		if subdir="" : subdir="Hot Soup Processor"

		chdir v1
		dirlist astr, subdir, 5 : if stat = 0 : mkdir subdir // MAKEDIR subdir
		chdir org_dir
		v1 += "\\" + subdir
	}

	if "" = v1 : v1 = org_dir
	return

#endif


;
;	easy config data support module
;
#deffunc cfg_init str fname, str appname

#ifndef _HSP3DISH
	get_appdata_path app_dir, appname
#else
	app_dir = dir_exe
#endif
	cfg_name = app_dir+"\\"+fname
	sdim sbuf,$1000
	sdim s1,$100
	sdim s2,$100
	sbuf=";appcfg v1.0\n"
	cur=0
	return

#deffunc cfg_seti str varname, int val

	sbuf+=";%"+varname+"="+val+"\n"
	return

#deffunc cfg_setd str varname, double val

	sbuf+=";%"+varname+"="+val+"\n"
	return

#deffunc cfg_sets str varname, str val

	sbuf+=";%"+varname+"="+val+"\n"
	return

#deffunc cfg_data str vdata

	sbuf+=vdata+"\n"
	return

#deffunc cfg_save

	notesel sbuf
	notesave cfg_name
	return

#defcfunc cfg_getvar str varname

	res=""
	notesel sbuf
	repeat notemax
	noteget s1,cnt
	if wpeek(s1,0)=$253b {
		getstr s2,s1,2,'='
		if s2=varname {
			getstr res,s1,2+strsize
			break
		}
	}
	loop
	return res

#deffunc cfg_getvari var v1, str varname

	res=""
	notesel sbuf
	repeat notemax
	noteget s1,cnt
	if wpeek(s1,0)=$253b {
		getstr s2,s1,2,'='
		if s2=varname {
			getstr res,s1,2+strsize
			v1=0+res
			break
		}
	}
	loop
	return

#deffunc cfg_getvars var v1, str varname

	res=""
	notesel sbuf
	repeat notemax
	noteget s1,cnt
	if wpeek(s1,0)=$253b {
		getstr s2,s1,2,'='
		if s2=varname {
			getstr res,s1,2+strsize
			v1=res
			break
		}
	}
	loop
	return

#deffunc cfg_load

	exist cfg_name
	if strsize<=0 : return 0
	;
	notesel sbuf
	noteload cfg_name
	cur=0
	repeat notemax
	noteget s1,cnt
	if peek(s1,0)!=';' : cur=cnt : break
	loop
	return 1

#deffunc cfg_getdata var p1

	notesel sbuf
	if cur>=notemax : p1="" : return
	noteget p1,cur
	cur++
	return




#global
#endif

