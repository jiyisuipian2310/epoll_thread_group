TARGET_DIR=../../bin
TARGET=$(TARGET_DIR)/main
TARGET_DEBUG=$(TARGET_DIR)/$(TARGET).debug
#objcopy -add-gnu-debuglink=$(TARGET_DEBUG) $(TARGET)
#find ./ -type f -name "*.cpp" ! -path "*/Tcp*.cpp"

CXX=g++
CXX_FLAGS=-O0 -g -lpthread -std=gnu++11 -D_DEBUG
INCS=  -I/usr/local/include -I./

SERVER_SRC=$(shell find ../ -type f -name "*.cpp")
TOTAL_SRC=$(SERVER_SRC)
OBJS := $(TOTAL_SRC:.cpp=.o)
OBJSABS = $(notdir $(OBJS))

all: $(TARGET_DIR) $(TARGET)
	#objcopy --only-keep-debug $(TARGET) $(TARGET_DEBUG)
	#objcopy --strip-debug $(TARGET)
	cp ./HostInfo.cfg ../../bin
	#find . -name "*.o" | xargs rm -rf

$(TARGET_DIR):
	mkdir -p $@

$(TARGET): $(OBJS)
	$(CXX) $(CXX_FLAGS) $(OBJSABS) $(LIBDIRS) $(LIBS) -o $@

$(OBJS) : %.o : %.cpp
	$(CXX) $(CXX_FLAGS) $(INCS) -c $<

clean:
	find . -name "*.o" | xargs rm -rf
	rm -rf $(TARGET) $(TARGET_DEBUG)
