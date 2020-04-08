
TARGET = codeconv

CXXFLAGS = -g 
CXXFLAGS = -DUSE_ICONV
# LFLAGS = -liconv

all : $(TARGET)

OBJS = test_codeconv.o
OBJS += codeconv_dummy.o
OBJS += codeconv_iconv.o

clean: 
	rm -rf $(TARGET)
	rm -rf $(OBJS)

$(TARGET) : $(OBJS)
	$(CXX) -o $(TARGET) $^ $(LFLAGS)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

%.o : %.c
	$(CXX) -c $(CXXFLAGS) -o $@ $<
	

