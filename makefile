PROGRAM = Bookstore

CXX = g++
CXXFLAGS = -std=c++17 -O3

CPP_FILES = $(wildcard *.cpp)
H_FILES = $(wildcard *.h)

all : code

code:
	$(CXX) -o code $(CXXFLAGS) $(CPP_FILES) $(H_FILES)

clean:
	rm -f *.o code

