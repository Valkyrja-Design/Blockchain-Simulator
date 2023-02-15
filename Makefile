INCLUDE = ./include 
SOURCES = ./src/*.cpp
CXXSTD = -std=c++17
CFLAGS = -I ${INCLUDE} ${CXXSTD}
OUTFILE = simulator

all:
	g++ ${SOURCES} ${CFLAGS} -o ${OUTFILE}

clean:
	rm simulator stats trace edges
