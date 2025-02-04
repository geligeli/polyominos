# CXX=clang++-19

CXXFLAGS=-std=c++20 -O3 -ffast-math -march=native -DNDEBUG -flto -masm=intel
CXXPCH_FLAGS=$(CXXFLAGS)
LDFLAGS=-ltbb
#-Wall -Wextra -Werror -pedantic


avx_match.o: avx_match.hpp avx_match.cpp
	$(CXX) $(CXXFLAGS) -c avx_match.cpp avx_match.hpp

avx_match_test: avx_match_test.cpp avx_match.o
	$(CXX) $(CXXFLAGS) avx_match_test.cpp avx_match.o -o avx_match_test

loggers.hpp.pch loggers.o: loggers.cpp loggers.hpp
	$(CXX) $(CXXFLAGS) -c loggers.cpp loggers.hpp

partition_function.hpp.pch: partition_function.hpp
	$(CXX)  $(CXXPCH_FLAGS) ./partition_function.hpp

polyominos.hpp.pch: polyominos.hpp
	$(CXX)  $(CXXPCH_FLAGS) -c ./polyominos.hpp

puzzle_maker: puzzle_maker.cpp partition_function.hpp.pch loggers.hpp.pch loggers.o loggers.hpp polyominos.hpp.pch
	$(CXX) $(CXXFLAGS) loggers.o $(LDFLAGS) -include-pch partition_function.hpp.pch loggers.hpp.pch polyominos.hpp.pch ./puzzle_maker.cpp -o ./puzzle_maker

clean:
	rm -f puzzle_maker bench *.pch *.o

bench: bench.cpp polyominos.hpp.pch loggers.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -include-pch polyominos.hpp.pch -lbenchmark loggers.o bench.cpp -o bench 

run_bench: bench
	./bench

run: puzzle_maker
	./puzzle_maker