CHESS_SRCS := $(wildcard *cpp)
CHESS_H := $(wildcard *h)
CHESS_OBJS := $(subst .cpp,.o, $(CHESS_SRCS))
TARGET_NAME := cchess
LIB_NAME := c:/MinGW/lib/libws2_32.a

$(TARGET_NAME):$(CHESS_OBJS)
	g++ -mwindows $(CHESS_OBJS) $(LIB_NAME) -l Gdi32 -o $@

%.o:%.cpp $(CHESS_H)
	g++ -mwindows -c -o $@ $<

clean:
	rm -rf $(CHESS_OBJS) $(TARGET_NAME)
