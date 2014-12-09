@echo off
mkdir llvm-build
cd llvm-src
sh ./configure --prefix="%CD:\=/%"
make all CXXFLAGS=-D_GLIBCXX_HAVE_FENV_H
xcopy include ..\llvm-build\include /s /e /k /p /i
make install
cd ..