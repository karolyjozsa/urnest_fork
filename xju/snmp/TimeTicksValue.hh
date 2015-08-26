// Copyright (c) 2015 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#ifndef XJU_SNMP_TIMETICKSVALUE_H
#define XJU_SNMP_TIMETICKSVALUE_H

#include "xju/snmp/Value.hh"
#include "xju/MicroSeconds.hh"

namespace xju
{
namespace snmp
{

class TimeTicksValue : public Value
{
public:
  explicit TimeTicksValue(xju::MicroSeconds val) throw();
  xju::MicroSeconds val_;

  // Value::
  virtual std::vector<uint8_t>::iterator encodeTo(
    std::vector<uint8_t>::iterator begin) const throw();

  // Value::
  virtual std::string str() const throw();
};



}
}

#endif
