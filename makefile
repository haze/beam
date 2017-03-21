COMPILER?=clang++
SRCS=$(wildcard src/*.cpp)
HDRS=$(wildcard src/*.hpp)
OBJS=$(patsubst %.c,%.o,$(SRC))

comp:
	clang++ -std=c++1y -stdlib=libc++ -I/usr/local/Cellar/tclap/1.2.1/include -L/usr/local/Cellar/tclap/1.2.1/lib -O2 -o beam $(SRCS)

run:
	./beam

install:
	cp beam /usr/bin/beam

clean:
	rm beam

