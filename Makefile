# file:   wifi_parser.c
# date:   Wed Jun 21 11:40:00 EST 2006
# Author: Doug Madory, Jihwang Yeo

ALL = parse_ampdu
# SRCS = parser.cpp TimeVal.cpp util.cpp crc32.cpp cpack.cpp
# OBJS = $(patsubst %.cpp,%.o,$(SRCS))

# Compiler and flags
CXX = g++
CXXFLAGS = -g -Wall
LFLAGS = -lpcap

LIBS = -lpcap

all: $(ALL)

# wifipcap.a: $(OBJS)
# 	ar rc $@ $(OBJS)

parse_ampdu: parse_ampdu.cpp parser.cpp parser.h
	$(CXX) $(CXXFLAGS)  -o $@ parse_ampdu.cpp parser.cpp ./wifipcap/wifipcap.a $(LFLAGS) 

# parser.o: parser.cpp parser.h
# 	$(CXX) $(CXXFLAGS)  -c parser.cpp
clean:
	rm -f $(ALL) *.o
