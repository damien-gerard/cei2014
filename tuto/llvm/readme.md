
Fibonacci C++ / LLVM
====================


How to compile
--------------

	g++ -std=c++11 ./fib.cpp  $(llvm-config --cxxflags --ldflags --libs --system-libs) -o fib && ./fib


