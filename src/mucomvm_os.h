
#ifdef MUCOM88WIN
#include "win32/osdep_win.h"		// とりあえず仮作成中
#define OSDEP_CLASS OsDependentWin32
#elif defined USE_SDL
#include "sdl/osdep_sdl.h"
#define OSDEP_CLASS OsDependentSdl
#else
#include "dummy/osdep_dummy.h"
#define OSDEP_CLASS OsDependentDummy
#endif


