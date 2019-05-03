これは英語のドキュです.日本語のドキュはREADME.mdあります.

This document is a best-effort attempt at an English introduction by massaging
an auto-translated version of README.md. It is accurate (to the extent
auto-translate is accurate) as of commit 1f306b3,
but does not strictly follow changes to README.md.

# About the OPEN MUCOM 88 Project

A Windows version of MUCOM 88 is released as part of the OPEN MUCOM 88 project.
For more information, see Open MUCOM 88 Wiki (not yet available in English). <br>
https://github.com/onitama/mucom88/wiki

MUCOM 88 for Windows is primarily distributed at the following site:

MUCOM 88 Windows <br>
https://onitama.tv/mucom88/

MUCOM 88 is a Music Macro Language (MML) compiler and music creation tool for
the NEC PC-8801 platform developed by Yuzo Koshiro in 1987. In addition to a
compiler, MUCOM 88 provides an FM synthesis driver written in z80 machine code
that understands the MML compiler output, giving an environment for playback
and feedback for the composer.

The OPEN MUCOM88 project aims to be widely used and extended by releasing the
original MUCOM 88 source code alongside OPEN MUCOM 88. Source code and assets
are released free of charge with an open license. License details are
summarized in the "License and Contact" section and "license.txt".

Disk images for the original MUCOM 88 distribution can be found on Ancient
Corp.'s [website](https://www.ancient.co.jp/~mucom88/).

# Building
* The `src` folder contains project files for Visual Studio 2017.
* The `xcode` folder contains a project file for Mac OS X XCode.
* Support for [Hot Soup Processor](hsp.tv) (Japanese only) is found in the
  `hspplugin` folder. English information for HSP is on
  [Wikipedia](https://en.wikipedia.org/wiki/Hot_Soup_Processor).
* A `Makefile` is also provided in `src`- it works on Linux and Win32 if using
  [msys2](https://www.msys2.org).

When compiling using the `Makefile`, Linux builds _require_
[SDL 1.2](https://www.libsdl.org/download-1.2.php). Using SDL is optional on
Windows, and is disabled by setting `OS_WIN32 = 1` in `Makefile.settings`.
`make` must be invoked while `src` is your working directory.

Raspberry Pi Example (Assuming a Debian-based Linux variant- adjust
accordingly if you use a different Linux distribution.):

```
sudo apt-get install libsdl1.2-dev
git clone https://github.com/onitama/mucom88
cd src
make
make mini
```

Several companion tools are available:
@BouKiCHi provides Mac OS X binaries. <br>
https://github.com/BouKiCHi/mucom88/tags

@kuma4649/kumatan provides a versatile video game music format music player
(including [VGM](https://vgmrips.net/wiki/VGM_Specification)), and a
utility to convert OPEN MUCOM 88 `.muc` files to VGM format (particularly
using the Sega Genesis chips). <br>
https://github.com/kuma4649/MDPlayer <br>
https://github.com/kuma4649/mucomMD2vgm/releases

# License and Contact
(This section mirrors both README.md and a majority of license.txt)

MUCOM 88 Windows depends on the following libraries, with credit given
to the authors:

* Portable Z80 emulation by Yasuo Kuwahara
  * http://www.geocities.jp/kwhr0/
* Fmgen by cisc
  * http://www.retropc.net/cisc/
* MUCOM 88 by Yuzo Koshiro
  * https://twitter.com/yuzokoshiro
* HSPMUCOM by Onitama (ONION software)
  * https://www.onionsoft.net/
* Adpcm converter / SCCI / FmToneEditor / mucom88DatToTxt by Gashi 3
  (gasshi)
  * http://www.pyonpyon.jp/~gasshi/fm/

Special thanks to Makoto Wada (Ancient Corporation),
[WING ☆](https://twitter.com/wing_ghost),
[OXYGEN](https://twitter.com/OXYGEN_PZ),
Gashi 3 ([gasshi](http://www.pyonpyon.jp/~gasshi/fm/)),
[MUCOM88](https://twitter.com/mucom88), and
[UME-3](https://twitter.com/ume3fmp) for their help in creating OPEN
MUCOM 88. And special thanks to
[Yuzo Koshiro](https://twitter.com/yuzokoshiro) and Ancient for creating
the original MUCOM 88 software for PC-8801.

Additional thanks goes to
[boukichi_numloc](https://twitter.com/boukichi_numloc) for WAV file
output support and cross-platform support of OPEN MUCOM 88.

OPEN MUCOM 88 for Windows (and its source code) are released under
a CC BY-NC-SA 4.0 license: <br>
https://creativecommons.org/licenses/by-nc-sa/4.0/

You can modify, copy, and redistribute this software and source code
freely for non-commercial (i.e. non-profit) purposes, provided you
include license.txt.

The sample songs (sampl1.muc, sampl2.muc, sampl3.muc) and the
accompanying data (mucompcm.bin, voice.dat) are provided by Yuzo Koshiro
of Ancient Corporation (Japanese link only). <br>
https://www.ancient.co.jp/yuzo.html

Please make sure to include a copyright license for the sample music
provided by Yuzo Koshiro. An example attribution:

"Song title (or file name) / Copyright (C) by Yuzo Koshiro"

OPEN MUCOM 88 for Windows is created by Onion Software.

ONION software Homepage <br>
https://www.onionsoft.net/

A user of OPEN MUCOM 88 retains the rights of MML files they create using
OPEN MUCOM 88.

Onion Software does is not responsible for damages caused by this
program. Use at your own risk. Also, you can freely modify and publish
the Hot Soup Processor scripts associated with OPEN MUCOM 88.

The distribution rules for the fmgen source code should follow the
license of the author, cisc. The distribution rules are as follows
(note that original text was in Japanese, and condition 4 suggests we
need the original license text):

```
The fmgen source code is copyrighted by the author (cisc@retropc.net).

This source code is provided as it is, without any implied or explicit
warranties. The author is not liable for any damages.

The source code can freely modified / embedded, distributed, and used as
long as the following conditions are met:

1. Specify the origin (author, copyright) of this software.
2. The distributed software shall remain free.
3. Document all modifications when distributing the modified source code.
4. Do not alter this text when distributing the source code; include
as-is.

The author would be happy to hear from you when you publish your
derivative work.

You must get permission from the author to use this source code,
in whole or in part, in a commercial product.
```

For more information about each library, please refer to the respective
source code and documentation. For distribution for commercial use,
please contact each author individually.
