#include "../../include/util/nulostream.h"



int NulStreambuf::overflow( int c )
{
  setp( dummyBuffer, dummyBuffer + sizeof( dummyBuffer ) );
  return (c == traits_type::eof()) ? '\0' : c;
}

NulStreambuf* NulOStream::rdbuf() const
{
  return const_cast<NulOStream*>(this);
}
