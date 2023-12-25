------------------------------------------------------------------------------
MUCOM88 Windows document (Japanese UTF8 text)

OpenMucom88 Ver.1.7d Copyright 1987-2020(C) Yuzo Koshiro
Z80 emulation by Yasuo Kuwahara 2002-2018(C)
FM Sound Generator by cisc 1998, 2003(C)
Windows version by ONION software/onitama 2018-2024(C)
------------------------------------------------------------------------------

・はじめに

	MUCOM88 Windowsは、NECのパソコンPC-8801シリーズで動作していた、
	古代祐三氏によるFM音源のツール/ドライバーである、MUCOM88を
	Windows上で動作させるためのシステムです。
	コマンドライン版、GUI版の両方が用意されています。

		mucom88win.exe         Windows GUI版エディタ
		mucom88.exe            Windows コマンドライン版

	MUCOM88 Windowsを使用することにより、音楽記述に特化した言語
	MML(Music Macro Language)として記述された音楽を、
	PC-8801の音源と同様に演奏させることができます。
	また、SCCI(Sound Chip Common Interface)を経由することで
	実際のFM音源チップ(YM2608/YM2203)による演奏をサポートしています。

	MUCOM88 Windowsについての詳細な説明、及びMMLリファレンスは
	Open MUCOM88 WiKiにて提供されています。
	WEBブラウザで、以下のURLをご覧ください。
	https://github.com/onitama/mucom88/wiki


	MUCOM88 Windowsの最新情報及び、ファイルのダウンロードは
	以下のサイトにて行っています。

	MUCOM88 Windows
	https://onitama.tv/mucom88/

	オリジナルのPC-8801版MUCOM88も、株式会社エインシャント様のサイトで
	ディスクイメージが公開されています。

	OPEN MUCOM88 for PC-8801
	https://www.ancient.co.jp/~mucom88/


・動作環境

	MUCOM88 Windowsは、サウンド再生が可能なWindows 7以降のシステム上で
	動作します。
	Windowsのサウンド再生処理には、DirectX8以降のDirectSoundを
	使用しています。Windows 7以降の環境では、追加のコンポーネントを
	用意することなく動作させることができます。

	FM音源、PSGサウンドのエミュレーションは32bit/55KHzで合成し、
	出力されています。古いマシンでは、負荷が大きい場合があります。

	MUCOM88 Windowsは、オリジナルのMUCOM88で使用されていたコード(Z80)を
	仮想的にエミュレーションし、FM音源ジェネレーター(fmgen)を介して
	演奏します。オリジナルと同様の動作で、MMLの解釈、演奏をさせることが
	可能になっています。
	動作に際して、PC-8801のBIOSやROM、ディスクイメージは必要ありません。

	アプリケーションの更新履歴については、同梱されている「history.txt」を
	参照してください。


・使い方の概要

	Windows GUIベースでMMLの編集と演奏が可能です。
	「mucom88win.exe」を起動すると、エディタの画面となりMMLの編集が
	できるようになります。基本的な操作は、以下のキーかボタンで行います。

		メニュースクリーン呼び出し ([F1]キー)
		編集中のMMLを保存 ([ctrl]+[S]キー)
		編集中のMMLを演奏 ([F5]または[F12]キー)
		演奏の停止・再開 ([ESC]キー)
		演奏の早送り([ctrl]+[F1]キー)
		音色エディタの起動(V.EDITボタン)

	MMLの編集画面では、

	A t190@30v15 cdefgab>c

	など「チャンネル(A～K)」「スペース」「MML記述」という組み合わせで
	各行に書いたものが、そのまま演奏されます。

	基本的には、自由にMMLを記述し[F5]で演奏(MMLコンパイル)させながら
	曲を作成するスタイルになります。画面下にある数行のウインドウに、
	MMLコンパイルの結果やメッセージが表示されます。
	MMLの記述にエラーがあった場合も、そこに表示されます。

	MMLは、テキストファイルとして読み込み・保存ができるので
	気軽な気持ちで読み書きが可能です。

	MML記述についての詳細は、github上のWiKiにて紹介していますので
	参考にしてください。このページは随時更新されます。
	https://github.com/onitama/mucom88/wiki/MML%E3%83%AA%E3%83%95%E3%82%A1%E3%83%AC%E3%83%B3%E3%82%B9

	また、サンプルMML(sampl1～sampl3)も、実際の作成に役立つはずです。

	MML編集以外の操作は、[F1]キーで呼び出されるメニュースクリーンから
	行います。

	MUCOM88 Windowsについての詳細な説明、及びMMLリファレンスは
	Open MUCOM88 WiKiにて提供されています。
	WEBブラウザで、以下のURLをご覧ください。
	https://github.com/onitama/mucom88/wiki


・リズム音源について

	MUCOM88 Windowsと同じフォルダに、以下のファイルがあった場合は、
	リズム音源用のサンプルファイルとして使用されます。
	リズム音源部を使用する場合は、別途ファイルをご用意頂く必要があります。

	2608_BD.WAV
	2608_HH.WAV
	2608_RIM.WAV
	2608_SD.WAV
	2608_TOM.WAV
	2608_TOP.WAV

	リズム音源用のサンプルファイルは、実際のボードからサンプリングして
	作成するなどの他に、サイトからダウンロードすることが可能です。
	https://sites.google.com/site/ym2608rhythm/


・実チップの使用について

	MUCOM88 Windowsでは、SCCI(Sound Chip Common Interface)を経由して
	実際のFM音源チップ(YM2608/YM2203)による演奏をサポートしています。
	同梱されているSCCIのシステムDLLは、以下のハードウェアに実チップ
	(YM2608またはYM2203)を搭載したものに対応しています。

	・SPFM FMの塔
	・SPFM Light
	・RE:Birth 
	・G.I.M.I.C
	・C86BOX

	実チップの使用については、最新のSCCI関連情報を参照ください。

	SCCI(Sound Chip Common Interface)
	http://www.pyonpyon.jp/~gasshi/fm/scci.html

	MUCOM88 WindowsからSCCIを使用する際は、メニュー→オプションで
	「SCCI使用」のチェックをONにして、ツールを再起動してください。
	(SCCIオプションは、ツールを再度起動した際に有効になります)

	実際にに使用する際には、PCにハードウェアを接続した状態で、最初に
	SCCIの設定を行っておく必要があります。
	メニュー→オプション「SCCI設定」ボタンから、SCCIの設定メニュー
	を起動することができます。(scciconfig.exeからも起動できます)

	SCCI使用時は、Windows上でのFM音源再生は行わず、外部のチップのみで
	再生を行います。


・ドライバの選択について

	ver0.53から、#driverタグによるドライバの選択が可能になっています。
	#driverタグに続いてドライバ名を記述することで、MMLのコンパイル及び演奏に
	特定のドライバを使用することができます。

	#driver mucom88		-> MUCOM88 1.7(デフォルト)
	#driver mucom88E	-> MUCOM88 1.5
	#driver mucom88EM	-> MUCOM88em
	#driver mucomDotNET	-> mucomDotNETによるコンパイルと演奏

	何も指定しない場合は、標準のドライバが選択されます。
	mucomDotNETを指定した場合は、kumatan氏により作成されたmucomDotNETを
	使用します。この場合、再生時にコマンドプロンプトを起動するため
	早送りやプレイヤーなどMUCOM88Win側の機能は使えなくなります。
	mucomDotNETは、デフォルトでMucom88Windows以下のmucomDotNETフォルダ内に
	格納されているツールセットを使用します。
	(設定により他のパスから実行させることも可能です。)

	ドライバによる主な差異は以下の通りです。

	#driver名       バージョン     実機サポート     備考
	---------------------------------------------------------------
	mucom88         MUCOM88 1.7    〇               MUSIC LALF相当
	mucom88E        MUCOM88 1.5    〇               PSGハードエンベロープ対応
	mucom88EM       MUCOM88em      〇(拡張メモリ)   使用できるメモリを拡張
	mucomDotNET     mucomDotNET    ×               独自に拡張された実装

	mucomDotNETを除く3つのバージョンは、PC-8801実機でも同様のMMLを
	演奏可能です。mucomDotNETでは、実機の再生はサポートされませんが、
	より広いメモリ領域を使用することができます。
	標準となるMUCOM88 1.7では、使用できるメモリサイズに制限があり、大きなMMLを
	作成することが難しいという問題があります。これを緩和するものが、MUCOM88emで、
	さらにメモリの制約を緩和に機能を拡張したものがmucomDotNETとなります。
	必要に合わせてドライバの選択を行ってください。
	大きなサイズのデータを作成しなければ、特にドライバを選択せず通常の状態で
	ご使用頂いて問題ありません。


・<>、()動作の反転について

	MML上で指定された'<'、'>'(オクターブの-+)及び、'('、')'(音量の-+)の動作を
	反転させることが可能です。(ver0.53以降)
	#invertタグに続いてonを記述することで、-+方向を反転させることが可能です。

	#invert on

	を行の先頭に記述してください。#invertタグの動作は、MML全体に対して影響します。

	MML記号    デフォルト             #invert on
	----------------------------------------------------------
	   <       オクターブを下げる     オクターブを上げる
	   >       オクターブを上げる     オクターブを下げる
	   (       音量を下げる           音量を上げる
	   )       音量を上げる           音量を下げる

	この機能は、他のMMLとの互換性を確保するための補助的な機能です。
	特に必要ない場合は、何も記述せずデフォルトの状態でお使いください。


・音色定義の追加機能について

	ver0.53から、「ツール」メニューに「MMLの末尾に音色定義を追加」ボタンが
	追加されています。これにより、MML内で使用しているすべてのFM音色の
	定義をMML末尾に追加することが可能です。
	MMLに音色定義を追加することにより、FM音色データファイルを参照することなく
	必要な音色のみをMMLで使用することが可能になります。


・コマンドライン版について

	コマンドラインから「mucom88.exe」を呼び出して使用することが可能です。
	以下の書式でオプションを指定することができます。

	mucom88 [options] [filename]

	オプション

	       -p [filename] 	読み込まれるPCMファイルを指定する
	       -v [filename]	読み込まれる音色ファイルを指定する
	       -o [filename]	出力されるバイナリファイル名を指定する
	       -w [filename]	出力されるWAVファイル名を指定する
	       -b [filename]	MMLファイルをVGM/S98形式ファイルに変換する
	       -c [filename]	MMLファイルをMUB形式ファイルに変換する
	       -i [filename]	MMLファイルの概要を出力する
	       -r [pathname]	リズム音源のWAVファイルがあるパスを指定する
	       -a [filename]	指定されたDLLをプラグインとして読み込む(Windows版のみ)
	       -f [drivername]	明示されたドライバ名を強制的に適用する
	       -e           	MUCOM88システムファイルを外部から読み込む
	       -s           	SCCIを経由して実チップでの演奏を行う
	       -k           	PCMファイルの読み込みをスキップする
	       -x		WAV/VGM/S98のレコーディングを行う
	       -l [n]		レコーディングファイル出力時の時間(秒)をnに指定する
	       -d		演奏に使用しているFM音色パラメーターを表示する
	       -g		コンパイル動作のみを行う
	       -h -?		コマンドのヘルプを表示する

	コマンドライン版は、WindowsだけでなくLinuxやMacOSX/Raspberry Pi等でも
	動作させることができます。詳しくは、OPEN MUCOM88のgithub repositoryを
	参照してください。

	OPEN MUCOM88 github repository
	https://github.com/onitama/mucom88

	Linux上でコンパイルする場合は、sdl1.2-devパッケージを導入した上で、
	リポジトリのsrcフォルダでmakeを実行してください。
	Raspberry Piで導入する例は、以下の通りです。

	例:
		sudo apt-get install libsdl1.2-dev
			(※OSによりパッケージマネージャは異なります)
		git clone https://github.com/onitama/mucom88
		cd src
		make
		make mini


・HSPプラグインについて

	同梱されている、hspmucom.dllはスクリプト言語HSP(HotSoupProcessor)から
	呼び出して使用することが可能なプラグインになっています。
	MUCOM88 Windowsアプリケーションも、HSPにより記述されています。
	hspluginフォルダにスクリプトのソース及びプラグインドキュメントが
	含まれています。
	HSPについての詳細は、以下を参照してください。

	Hot Soup Processor公式HP
	https://hsp.tv/


・OPEN MUCOM88プロジェクトについて

	MUCOM88 Windowsは、OPEN MUCOM88プロジェクトの一部として公開されています。

	MUCOM88は、もともと1987年・古代祐三氏によって開発・発表された
	NEC PC-8801プラットフォーム用のMML形式による音楽作成ツール、及び
	再生用のプログラム(ドライバー)環境です。

	OPEN MUCOM88プロジェクトは、オリジナルのMUCOM88ソースコードを公開する
	ことで、幅広く活用・継承することを目的としています。
	無償で公開されたソースコードや資産などは、オープンなライセンスにより
	自由に活用することが可能です。
	ライセンスの詳細は、「ライセンスおよび連絡先」項目及び「license.txt」
	にまとめられています。


・ライセンスおよび連絡先

	MUCOM88 Windowsは、以下のサイトにて1次配布されています。

	MUCOM88 Windows
	https://onitama.tv/mucom88/

	OPEN MUCOM88 github repository
	https://github.com/onitama/mucom88


	MUCOM88 Windowsは、以下のライブラリ及びソースコード・データにより作成されています。

	・Portable Z80 emulation作者 Yasuo Kuwahara氏
	  http://www.geocities.jp/kwhr0/
	・fmgen作者 cisc氏
	  http://www.retropc.net/cisc/
	・MUCOM88作者 古代祐三氏
	  https://twitter.com/yuzokoshiro
	・HSPMUCOM作者 おにたま(ONION software)
	  https://www.onionsoft.net/
	・adpcm converter/SCCI/FmToneEditor/mucom88DatToTxt作者
	  がし３(gasshi)氏
	  http://www.pyonpyon.jp/~gasshi/fm/
	・mucomDotNET作者 kumatan氏
	  https://github.com/kuma4649/mucomDotNET
	・MUCOM88em 作者 @MUCOM88氏
	  https://github.com/MUCOM88/mucom88/

	作成にあたり、ご協力頂いた和田誠(エインシャント)様、
	WING☆様、OXYGEN様、がし３(gasshi)様、ぼうきち様( @boukichi_numloc )、kumatan様、
	@MUCOM88様、UME-3様そしてオリジナルのPC-8801版を作成した古代祐三様に
	感謝致します。本当にありがとうございました。

	MUCOM88 Windows及びソースコードは、クリエイティブコモンズで規定された
	CC BY-NC-SA 4.0ライセンスで公開されています。
	https://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja

	無償(非営利)である限りは自由に紹介、複製、再配布が可能です。
	その際には必ずドキュメントとライセンス表記(license.txt)も含めるように
	してください。

	サンプル楽曲(sampl1.muc、sampl2.muc、sampl3.muc)及び付属するデータ
	(mucompcm.bin、voice.dat)は株式会社エインシャントの古代祐三氏により
	提供されています。
	https://www.ancient.co.jp/yuzo.html

	古代祐三氏のサンプル楽曲については、著作権のライセンスを必ず明示する
	ようお願い致します。

	ライセンス表記の例:
	「楽曲名(またはファイル名) / Copyright(C) by Yuzo Koshiro」


	MUCOM88 Windowsは、おにたま(onion software)が中心となり作成されています。

	ONION software Homepage
	https://www.onionsoft.net/

	ユーザーがMUCOM88を使って作成したオリジナルの楽曲、MMLファイルの権利は
	それを作成したユーザーに属します。
	onion softwareは本プログラムによって生じた、いかなる損害についても
	保証いたしません。自己の責任の範囲で使用してください。
	また、付属のHSPスクリプトも自由に改変、公開していただいて構いません。


	fmgenソースコードに関する配布規定は、作者であるcisc氏のライセンスに
	従ってください。fmgenソースコードの配布規定は以下の通りです。

	・fmgenソースコードは作者(cisc@retropc.net) が著作権を所有しています。

	・本ソースコードはあるがままに提供されるものであり，
	  暗黙及び明示的な保証を一切含みません．

	・本ソースコードを利用したこと，利用しなかったこと，
	  利用できなかったことに関して生じたあるいは生じると予測される
	  損害について，作者は一切責任を負いません．

	・本ソースコードは，以下の制限を満たす限り自由に改変・組み込み・
	  配布・利用することができます．

	  1. 本ソフトの由来(作者, 著作権)を明記すること.
	  2. 配布する際にはフリーソフトとすること．
	  3. 改変したソースコードを配布する際は改変内容を明示すること.
	  4. ソースコードを配布する際にはこのテキストを一切改変せずに
	     そのまま添付すること．

	・公開の際に作者への連絡を頂ければ幸いです．

	・商用ソフト(シェアウェア含む) に本ソースコードの一部，または
	  全部を組み込む際には，事前に作者の合意を得る必要があります．

	各ライブラリについての詳細は、それぞれのソースコード及びドキュメントを
	参照ください。
	有償・商用での配布については、別途作者までお問い合わせください。


-------------------------------------------------------------------------------
                                            MUCOM88 users manual / end of file 
-------------------------------------------------------------------------------
