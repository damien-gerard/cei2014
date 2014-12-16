#ifndef NULOSTREAM_H
#define NULOSTREAM_H
#include <iostream>

class NulStreambuf : public std::streambuf
{
  private:
    char dummyBuffer[ 64 ];
  protected:
    virtual int overflow( int c );
};

class NulOStream : private NulStreambuf, public std::ostream
{
  public:
    NulOStream(): std::ostream( this ) {}
    NulStreambuf* rdbuf() const;
};


#endif // NULOSTREAM_H
