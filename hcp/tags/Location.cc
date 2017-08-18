#include <hcp/tags/Location.hh>
//generated from "/home/xju/tmp/oc52-2/xjutv/FILES/d/l/Location.hcp.1888127.hcp"
#include <cxy/copyContext.hh> //impl

namespace hcp
{
namespace tags
{
Location::Location(
    ::hcp::tags::AbsolutePath const& p1,
    ::hcp::tags::FileName const& p2,
    ::hcp::tags::LineNumber const& p3) throw():
      directory(p1),
      file(p2),
      line(p3) {
  }
  
}
}
