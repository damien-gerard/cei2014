#include "../include/builder.h"


using namespace llvm;

Builder::Builder()
: _irb(getGlobalContext())
{
    //ctor
}

Builder::~Builder()
{
    //dtor
}
