
//
//		Configure for MUCOM88
//
#ifndef __mucom88config_h
#define __mucom88config_h

#define VERSION "1.7c"
#define MAJORVER 1
#define MINORVER 7

//
//		環境フラグ:以下のラベルはコンパイルオプションで設定されます
//
//#define MUCOM88WIN		// Windows(WIN32) version flag
//#define MUCOM88LINUX		// Unix/Linux version flag
//#define MUCOM88IOS		// iOS version flag
//#define MUCOM88NDK		// android NDK version flag

//		文字コードフラグ:以下のラベルは自動的に設定されます
//
//#define MUCOM88UTF8		// UTF8使用フラグ

#ifdef MUCOM88WIN
#else
#define MUCOM88UTF8
#endif

#endif
