COMPILER?=clang++
SRCS=$(wildcard src/*.cpp)
HDRS=$(wildcard src/*.hpp)
OBJS=$(patsubst %.c,%.o,$(SRC))

comp:
	clang++ -std=c++1y -stdlib=libc++ -O2 -o bm $(SRCS)

run:
	./bm

install:
	cp bm /usr/bin/bm

clean:
	rm bm

