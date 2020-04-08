# test CMucom to compile pure C++ environment. 

CFLAGS =

# main 
SRCS += membuf.cpp
SRCS += adpcm.cpp
SRCS += md5.c
SRCS += soundbuf.cpp
SRCS += cmucom.cpp
SRCS += mucomvm.cpp
SRCS += mucomerror.cpp
SRCS += callback.cpp
SRCS += osdep.cpp

# Z80
SRCS += Z80/Z80.cpp

# fmgen
SRCS += fmgen/file.cpp
SRCS += fmgen/fmgen.cpp
SRCS += fmgen/fmtimer.cpp
SRCS += fmgen/opm.cpp
SRCS += fmgen/opna.cpp
SRCS += fmgen/psg.cpp

# utils
SRCS += utils/s98write.cpp
SRCS += utils/vgmwrite.cpp
SRCS += utils/wavwrite.cpp

# plugin(dummy)
SRCS += plugin/plugin.cpp

# codeconv
SRCS += utils/codeconv/codeconv_dummy.cpp

# other
SRCS += module/mucom_module.cpp
SRCS += dummy/osdep_dummy.cpp

# main
MODTEST_SRCS = tests/module_test.cpp
COMPILER_SRCS = tests/compile_test.cpp

ifdef USE_ICONV
SRCS += utils/codeconv/codeconv_iconv.cpp
CFLAGS += -DUSE_ICONV
endif

SRCDIR = ..
OBJDIR = objs

ALL_SRCS = $(SRCS) $(MODTEST_SRCS) $(COMPILER_SRCS)

SRCSUBDIR = $(sort $(dir $(ALL_SRCS)))
INCDIRS = $(addprefix $(SRCDIR)/, $(SRCSUBDIR))
INCFLAGS = -I$(SRCDIR) $(addprefix -I, $(INCDIRS))

TARGET = modtest
COMPILER = compile

OBJS = $(addprefix $(OBJDIR)/,$(addsuffix .o, $(basename $(SRCS))))

TARGET_OBJS = $(addprefix $(OBJDIR)/,$(addsuffix .o, $(basename $(MODTEST_SRCS))))
COMPILER_OBJS = $(addprefix $(OBJDIR)/,$(addsuffix .o, $(basename $(COMPILER_SRCS))))

CFLAGS += $(INCFLAGS)




all : makedir $(TARGET) $(COMPILER)

makedir: 
	mkdir -p $(OBJDIR)
	mkdir -p $(addprefix $(OBJDIR)/,$(SRCSUBDIR))

$(TARGET) : $(OBJS) $(TARGET_OBJS)
	g++ -o $@ $^

$(COMPILER) : $(OBJS) $(COMPILER_OBJS)
	g++ -o $@ $^

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	g++ -c $(CFLAGS) -o $@ $<

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	gcc -c $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
	rm -f $(COMPILER)
	rm -rf $(OBJDIR)
