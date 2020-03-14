CHESS_SRCS := $(wildcard *cpp)
CHESS_H := $(wildcard *h)
CHESS_OBJS := $(subst .cpp,.o, $(CHESS_SRCS))
TARGET_NAME := cchess
LIB_NAME := c:/MinGW/lib/libws2_32.a

$(TARGET_NAME):$(CHESS_OBJS) cl_chess.o
	g++ -mwindows $(CHESS_OBJS) $(LIB_NAME) -l Gdi32 cl_chess.o -o $@

cl_chess.o:cl_chess.rc cl_chess.ico
	windres cl_chess.rc -o cl_chess.o

%.o:%.cpp $(CHESS_H)
	g++ -mwindows -c -o $@ $<

clean:
	rm -rf *.o $(TARGET_NAME) cl_chess.o
