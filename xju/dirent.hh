// Copyright (c) 2020 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#ifndef XJU_DIRENT_H
#define XJU_DIRENT_H

#include <xju/syscall.hh>
#include <dirent.h>


namespace xju
{
extern const SyscallF1<DIR*, const char*> opendir;
extern const SyscallF1<DIR*, DIR*> readdir;

}

#endif

