CXX=clang++
CXXFLAGS=-std=c++20 -ltbb -O3 
#-Wall -Wextra -Werror -pedantic

puzzle_maker: puzzle_maker.cpp
	$(CXX) ./puzzle_maker.cpp $(CXXFLAGS) -o puzzle_maker

clean:
	rm -f puzzle_maker

run: puzzle_maker
	./puzzle_maker