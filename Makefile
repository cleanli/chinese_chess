CHESS_SRCS := $(wildcard src/*cpp)
CHESS_H := $(wildcard *h)
CHESS_OBJS := $(subst .cpp,.o, $(CHESS_SRCS))
TARGET_NAME := cchess
LIB_NAME := c:/MinGW/lib/libws2_32.a

$(TARGET_NAME):$(CHESS_OBJS) cl_chess.o
	g++ -mwindows $(CHESS_OBJS) $(LIB_NAME) -l Gdi32 res/cl_chess.o -o $@

cl_chess.o:res/cl_chess.rc res/cl_chess.ico include/rc.h
	windres res/cl_chess.rc -Iinclude -o res/cl_chess.o

%.o:%.cpp $(CHESS_H)
	g++ -mwindows -Iinclude -c -o $@ $<

clean:
	rm -rf src/*.o $(TARGET_NAME) res/cl_chess.o
