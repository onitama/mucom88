#ifndef __MOD_MUCOM88
#define __MOD_MUCOM88

;MUCOM88 Module

#include "mod_appconfig.as"

goto *_end_mod_mucom88

//---------------------------------------------------------

#define INIFILE "\\mucom88win.ini"

#deffunc mucom88app_init

	;	アプリ設定の初期化(起動時)
	;
	voicefile=MUCOM_DEFAULT_VOICEFILE
	pcmfile=MUCOM_DEFAULT_PCMFILE
	fontname="FixedSys"
	fontsize=14
	filedir=dir_exe	;dir_mydoc
	scci_mode=0
	author="???"
	url=""
	recent=""
	uuid=""
	weburl="https://github.com/onitama/mucom88/wiki/MML%E3%83%AA%E3%83%95%E3%82%A1%E3%83%AC%E3%83%B3%E3%82%B9"
	fastfw=0
	fgcolor=$dcdcdc
	bgcolor=0

	plg_fmedit=1
	plg_smon=1
	slowfw=0
	upd_check=1
	aplayer_auto=1
	aplayer_time=90
	aplayer_loop=150

	asave_sw=0
	asave_time=180
	asave_count=0
	asave_max=0

	wx=800:wy=600
	app_winx=-1
	app_winy=-1
	initopt=0
	langset=0

	get_appdata_path app_dir
	cfg_init app_dir+INIFILE
	cfg_load:modcfg=stat
	if modcfg {	; iniファイル読み込み
		wx=0+cfg_getvar("wx")
		wy=0+cfg_getvar("wy")
		app_winx=0+cfg_getvar("winx")
		app_winy=0+cfg_getvar("winy")
		voicefile=cfg_getvar("voicefile")
		pcmfile=cfg_getvar("pcmfile")
		fontname=cfg_getvar("fontname")
		fontsize=0+cfg_getvar("fontsize")
		filedir=cfg_getvar("filedir")
		author=cfg_getvar("author")
		url=cfg_getvar("url")
		recent=cfg_getvar("recent")
		uuid=cfg_getvar("uuid")
		scci_mode=0+cfg_getvar("scci_mode")
		fastfw=0+cfg_getvar("fastfw")
		fgcolor=0+cfg_getvar("fgcolor")
		bgcolor=0+cfg_getvar("bgcolor")
		if fgcolor=0&&bgcolor=0 : fgcolor=$dcdcdc : bgcolor=$1e1e1e

		cfg_getvari plg_fmedit,"plg_fmedit"
		cfg_getvari plg_smon,"plg_smon"
		cfg_getvari slowfw,"slowfw"
		cfg_getvari upd_check,"upd_check"
		cfg_getvari aplayer_auto,"aplayer_auto"
		cfg_getvari aplayer_time,"aplayer_time"
		cfg_getvari aplayer_loop,"aplayer_loop"
		cfg_getvari asave_sw,"asave_sw"
		cfg_getvari asave_time,"asave_time"

		cfg_getvari langset,"langset"
		cfg_getvari asave_count,"asave_count"
		cfg_getvari asave_max,"asave_max"

	}

	return

#deffunc mucom88app_term

	;	アプリ設定の保存(終了時)
	;
	if wx<640 : wx=640
	if wy<480 : wy=480
	get_appdata_path app_dir
	cfg_init app_dir+INIFILE
	cfg_seti "wx",wx
	cfg_seti "wy",wy
	cfg_seti "winx",ginfo_wx1
	cfg_seti "winy",ginfo_wy1
	cfg_sets "voicefile",voicefile
	cfg_sets "pcmfile",pcmfile
	cfg_sets "fontname",fontname
	cfg_seti "fontsize",fontsize
	cfg_sets "filedir",filedir
	cfg_sets "author",author
	cfg_sets "url",url
	cfg_sets "recent",recent
	cfg_sets "uuid",uuid
	cfg_seti "scci_mode",scci_mode
	cfg_seti "fastfw",fastfw
	cfg_seti "fgcolor",fgcolor
	cfg_seti "bgcolor",bgcolor

	cfg_seti "plg_fmedit",plg_fmedit
	cfg_seti "plg_smon",plg_smon
	cfg_seti "slowfw",slowfw
	cfg_seti "upd_check",upd_check
	cfg_seti "aplayer_auto",aplayer_auto
	cfg_seti "aplayer_time",aplayer_time
	cfg_seti "aplayer_loop",aplayer_loop

	cfg_seti "asave_sw",asave_sw
	cfg_seti "asave_time",asave_time

	cfg_seti "langset",langset
	cfg_seti "asave_count",asave_count
	cfg_seti "asave_max",asave_max

	cfg_save

	return

//---------------------------------------------------------

*_end_mod_mucom88

#endif

