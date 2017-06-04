#ifndef _XJU_FILE_MKDIR_HCP
#define _XJU_FILE_MKDIR_HCP
// Copyright (c) 2017 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#include <xju/path.hh>
#include "xju/Exception.hh"

namespace xju
{
namespace file
{
void mkdir(std::pair<xju::path::AbsolutePath,xju::path::DirName> const& d,
           mode_t mode)
  throw(xju::Exception);


}
}
#endif