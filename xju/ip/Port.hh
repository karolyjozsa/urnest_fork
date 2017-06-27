#ifndef _PORT_HH
#define _PORT_HH
// Copyright (c) 2017 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#include <xju/Int.hh>
#include <cinttypes>


namespace xju
{
namespace ip
{

struct PortTag{};

// port number
typedef xju::Int<PortTag,uint16_t> Port;


}
}


#endif
