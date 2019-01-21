// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、
// または、参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#define MUCOM88WIN

#include <stdio.h>

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
// Windows ヘッダー ファイル
#include <windows.h>
#include <windows.h>

#include "../src/mucom88config.h"
#include "../src/plugin/mucom88if.h"
#include "../src/mucomvm.h"
#include "../src/cmucom.h"

// プログラムに必要な追加ヘッダーをここで参照してください
