
# OPEN MUCOM88プロジェクトについて

MUCOM88 Windowsは、OPEN MUCOM88プロジェクトの一部として公開されています。
詳細については、Open MUCOM88 WiKiをご覧ください。
https://github.com/onitama/mucom88/wiki

MUCOM88 Windowsの配布パッケージは、以下のサイトにて1次配布されています。

	MUCOM88 Windows
	https://onitama.tv/mucom88/

MUCOM88は、もともと1987年・古代祐三氏によって開発・発表されたNEC PC-8801プラットフォーム用のMML形式による音楽作成ツール、及び再生用のプログラム(ドライバー)環境です。

OPEN MUCOM88プロジェクトは、オリジナルのMUCOM88ソースコードを公開することで、幅広く活用・継承することを目的としています。無償で公開されたソースコードや資産などは、オープンなライセンスにより自由に活用することが可能です。
ライセンスの詳細は、「ライセンスおよび連絡先」項目及び「license.txt」にまとめられています。

オリジナルのPC-8801版MUCOM88も、[株式会社エインシャント様](https://www.ancient.co.jp/~mucom88/)のサイトでディスクイメージが公開されています。以下を参照してください。


# ビルド方法

	srcフォルダにVisual Studio 2017用のプロジェクトファイルが含まれています
	xcodeフォルダにMacOSX版のXCode用プロジェクトファイルが含まれています
	HSPプラグイン及び、HSPソースコードはhsppluginフォルダに含まれています
	(Windows版のGUIアプリケーションはHSP3.5からコンパイルする必要があります)
	Makefileは、Linux及びWin32版に対応しています。

	Makefileからコンパイルする場合は、sdl1.2-devパッケージを導入した上で、
	リポジトリのsrcフォルダでmakeを実行してください。
	Raspberry Piで導入する例は、以下の通りです。

	例:
		sudo apt-get install libsdl1.2-dev
			(※OSによりパッケージマネージャは異なります)
		git clone https://github.com/onitama/mucom88
		cd src
		make
		make mini

	いくつかの派生ツールが公開されています

	ぼうきちさんによる、MacOSX向けの実装及びバイナリが公開されています。
	https://github.com/BouKiCHi/mucom88/tags

	kumatan氏による、MDPlayer、及びmucomMD2vgmでMUCOM88のMML(.muc)を
	サポートしています。
	https://github.com/kuma4649/MDPlayer
	https://github.com/kuma4649/mucomMD2vgm/releases


# ライセンスおよび連絡先

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

作成にあたり、ご協力頂いた和田誠(エインシャント)様、WING☆様、OXYGEN様、がし３(gasshi)様、@MUCOM88様、UME-3様そしてオリジナルのPC-8801版を作成した古代祐三様、本当にありがとうございました。

MUCOM88 Windows及びソースコードは、クリエイティブコモンズで規定されたCC BY-NC-SA 4.0ライセンスで公開されています。
https://creativecommons.org/licenses/by-nc-sa/4.0/deed.ja

無償(非営利)である限りは自由に紹介、複製、再配布が可能です。
その際には必ずドキュメントとライセンス表記(license.txt)も含めるようにしてください。

サンプル楽曲(sampl1.muc、sampl2.muc、sampl3.muc)及び付属するデータ(mucompcm.bin、voice.dat)は株式会社エインシャントの古代祐三氏により提供されています。
https://www.ancient.co.jp/yuzo.html

古代祐三氏のサンプル楽曲については、著作権のライセンスを必ず明示するようお願い致します。

	ライセンス表記の例:
	「楽曲名(またはファイル名) / Copyright(C) by Yuzo Koshiro」


MUCOM88 Windowsは、おにたま(onion software)が中心となり作成されています。

	ONION software Homepage
	https://www.onionsoft.net/

ユーザーがMUCOM88を使って作成したオリジナルの楽曲、MMLファイルの権利はそれを作成したユーザーに属します。
onion softwareは本プログラムによって生じた、いかなる損害についても保証いたしません。自己の責任の範囲で使用してください。また、付属のHSPスクリプトも自由に改変、公開していただいて構いません。


fmgenソースコードに関する配布規定は、作者であるcisc氏のライセンスに従ってください。fmgenソースコードの配布規定は以下の通りです。

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

各ライブラリについての詳細は、それぞれのソースコード及びドキュメントを参照ください。
有償・商用での配布については、別途作者までお問い合わせください。


