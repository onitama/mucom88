#
# Makefile for mucom88
#
# make test ... test生成
# make mini ... miniplay生成
# make tool ... pcmtool作成


CXX = g++
CC = gcc
CFLAGS = 
CXXFLAGS = 
# CXXFLAGS += -Wno-write-strings
COMMON_FLAGS = 
# COMMON_FLAGS += -Wall -Werror
LDFLAGS = 
DEFINES = 
OBJS_DIR = objs
TARGET_NAME = mucom88
LIBS =
EXT_LIBS =

# 設定ファイル
include Makefile.setting

#
ifeq ($(WIN32),1)
EXE_SUFFIX = .exe
else
EXE_SUFFIX =
endif
TARGET = $(TARGET_NAME)$(EXE_SUFFIX)
TARGET_MODTEST = mod_test$(EXE_SUFFIX)
TARGET_DEPTEST = dep_test$(EXE_SUFFIX)
TARGET_MINIPLAY = miniplay$(EXE_SUFFIX)
TARGET_PCMTOOL = pcmtool$(EXE_SUFFIX)


ifeq ($(DEBUG),1)
TARGET = $(TARGET_NAME)_d$(EXE_SUFFIX)
LDFLAGS = -g
COMMON_FLAGS = -O0 -g
OBJS_DIR = objs_d
endif

.PHONY: all test makedir $(OBJS_DIR)

all: makedir $(TARGET)
test: makedir $(TARGET_DEPTEST) $(TARGET_MODTEST)
mini: makedir $(TARGET_MINIPLAY)
tool: makedir $(TARGET_PCMTOOL)


DIRS = z80 fmgen plugin module tests

SRCS = membuf.cpp adpcm.cpp


SRCS += md5.c
SRCS += soundbuf.cpp

SRCS += cmucom.cpp
SRCS += mucomvm.cpp

ifeq ($(WIN32),1)
SRCS += mucomerror.cpp
else
SRCS += mucomerror_utf8.cpp
endif

SRCS += plugin/plugin.cpp

SRCS += Z80/Z80.cpp

SRCS += fmgen/file.cpp fmgen/fmgen.cpp fmgen/fmtimer.cpp 
SRCS += fmgen/opm.cpp fmgen/opna.cpp fmgen/psg.cpp

SRCS += callback.cpp
SRCS += wavout.cpp
SRCS += osdep.cpp

# ログ機能
SRCS += utils/s98write.cpp
SRCS += utils/vgmwrite.cpp

# WAV出力機能
SRCS += utils/wavwrite.cpp


MAIN_SRCS = main.cpp

MOD_SRCS = module/mucom_module.cpp
MODTEST_SRCS = tests/module_test.cpp
DEPTEST_SRCS = tests/depent_test.cpp

# miniplayはSDLに依存します
MINIPLAY_SRCS = sdl/miniplay.cpp sdl/audiosdl.cpp

# pcmtool
PCMTOOL_SRCS = utils/pcmtool.cpp utils/pcmentry.cpp utils/pcmtool_main.cpp

DIRS += utils

# OS依存
ifeq ($(OS_WIN32),1)
DEFINES += -DMUCOM88WIN
DIRS += win32
DEP_SRCS += win32/realchip.cpp
DEP_SRCS += win32/soundds.cpp
DEP_SRCS += win32/osdep_win.cpp

LIBS += -ldsound 
LIBS += -lole32 -lwinmm -luuid 
LIBS += -ldsound -ldxguid
LIBS += -static -lstdc++
LIBS += -mconsole

# SDL
else ifeq ($(OS_SDL),1)
DIRS += sdl
DEP_SRCS += sdl/osdep_sdl.cpp
DEP_SRCS += sdl/audiobuffer.cpp
DEP_SRCS += sdl/audiotime.cpp

SDL_CFLAGS = $(shell sdl-config --cflags) -DUSE_SDL
COMMON_FLAGS += $(SDL_CFLAGS)

ifeq ($(STATIC),1)
SDL_LIBS =  $(shell sdl-config --static-libs)
else
SDL_LIBS =  $(shell sdl-config --libs)
endif

ifeq ($(SDL_WIN32),1)
# dllをすべて静的リンクします。 （Win32かつSDLはテスト用途のため)
LIBS += -static $(SDL_LIBS) -mconsole
else
LIBS += $(SDL_LIBS)
endif
else 
DIRS += dummy
DEP_SRCS += dummy/osdep_dummy.cpp
endif


# OS依存
DEP_SRCS_BASE = $(basename $(DEP_SRCS))
DEP_OBJS = $(addprefix $(OBJS_DIR)/,$(addsuffix .o,$(DEP_SRCS_BASE)))

# OS依存から参照される依存しないコード
REF_SRCS = soundbuf.cpp
REF_SRCS_BASE = $(basename $(REF_SRCS))
REF_OBJS = $(addprefix $(OBJS_DIR)/,$(addsuffix .o,$(REF_SRCS_BASE)))

# コマンドライン基本
MAIN_SRCS_BASE = $(basename $(MAIN_SRCS))
MAIN_OBJS = $(addprefix $(OBJS_DIR)/,$(addsuffix .o,$(MAIN_SRCS_BASE)))

# モジュール化
MOD_SRCS_BASE = $(basename $(MOD_SRCS))
MOD_OBJS = $(addprefix $(OBJS_DIR)/,$(addsuffix .o,$(MOD_SRCS_BASE)))

# 本体ソース
SRCS_BASE = $(basename $(SRCS))
OBJS = $(addprefix $(OBJS_DIR)/,$(addsuffix .o,$(SRCS_BASE)))
OBJS += $(DEP_OBJS) $(REF_OBJS)

# モジュールテスト
MODTEST_SRCS_BASE = $(basename $(MODTEST_SRCS))
MODTEST_OBJS = $(addprefix $(OBJS_DIR)/,$(addsuffix .o,$(MODTEST_SRCS_BASE)))

# OS依存テスト
DEPTEST_SRCS_BASE = $(basename $(DEPTEST_SRCS))
DEPTEST_OBJS = $(addprefix $(OBJS_DIR)/,$(addsuffix .o,$(DEPTEST_SRCS_BASE)))

# ミニプレイヤー
MINIPLAY_SRCS_BASE = $(basename $(MINIPLAY_SRCS))
MINIPLAY_OBJS = $(addprefix $(OBJS_DIR)/,$(addsuffix .o,$(MINIPLAY_SRCS_BASE)))

# PCMツール
PCMTOOL_SRCS_BASE = $(basename $(PCMTOOL_SRCS))
PCMTOOL_OBJS = $(addprefix $(OBJS_DIR)/,$(addsuffix .o,$(PCMTOOL_SRCS_BASE)))

INCLUDES += -I. $(addprefix -I,$(DIRS) )



COMMON_FLAGS += $(DEFINES) $(INCLUDES)
CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += $(COMMON_FLAGS)

# 依存テスト
$(TARGET_DEPTEST) : $(OBJS) $(DEPTEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

# モジュールテスト
$(TARGET_MODTEST) : $(OBJS) $(MOD_OBJS) $(MODTEST_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

# モジュールテスト
$(TARGET_MINIPLAY) : $(OBJS) $(MOD_OBJS) $(MINIPLAY_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

# モジュールテスト
$(TARGET_PCMTOOL) : $(OBJS) $(MOD_OBJS) $(PCMTOOL_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)


# 実行ファイル
$(TARGET) : $(MAIN_OBJS) $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

makedir :
	mkdir -p $(OBJS_DIR) $(addprefix $(OBJS_DIR)/,$(DIRS))

mucomerror_utf8.cpp :
	iconv -f cp932 -t utf8 mucomerror.cpp > $@ 

$(OBJS_DIR)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJS_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean :
	rm -f $(TARGET_PCMTOOL)
	rm -f $(TARGET_MINIPLAY) $(TARGET_DEPTEST) $(TARGET_MODTEST) $(TARGET)
	rm -rf $(OBJS_DIR)

