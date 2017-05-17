#include <hcp/tags/Namespace.hh>
#line 1 "/home/xju/urnest/hcp/tags/Namespace.hcp"
#line 16
#include <algorithm> //impl
#line 18
#include "xju/next.hh" //impl
#include <sstream> //impl
#include <xju/format.hh> //impl

namespace hcp
{
namespace tags
{
#line 32
void Namespace::addSymbol(std::vector<NamespaceName> const& namespace_,
                 UnqualifiedSymbol const& symbol,
                 std::vector<Location> const& locations) throw() {
    Namespace& n(findNamespace( {namespace_.begin(),namespace_.end()} ));
    auto i=n.symbols_.find(symbol);
    if (i==n.symbols_.end()) {
      i=n.symbols_.insert( {symbol,Locations() }).first;
    }
    std::copy(locations.begin(),locations.end(),
              std::back_inserter((*i).second));
  }

  
#line 48
Namespace::UnknownNamespace::UnknownNamespace(const std::string& cause,
                     const xju::Traced& trace) throw():
        xju::Exception(cause,trace)
    {
    }
  
#line 58
Namespace::UnknownSymbol::UnknownSymbol(const std::string& cause,
                  const xju::Traced& trace) throw():
        xju::Exception(cause,trace)
    {
    }
  
#line 83
Namespace& Namespace::findNamespace(
    std::pair<std::vector<NamespaceName>::const_iterator,
              std::vector<NamespaceName>::const_iterator> const& path) throw()
  {
    if (path.first==path.second) {
      return *this;
    }
    auto i=children_.find(*path.first);
    if (i==children_.end()) {
      i=children_.insert( {*path.first,Namespace()} ).first;
    }
    return (*i).second.findNamespace( {xju::next(path.first),path.second} );
  }

  // find namespace given by path within *this
  // post: path.size() || result===*this
  
#line 99
Namespace const& Namespace::findNamespace(
    std::pair<std::vector<NamespaceName>::const_iterator,
              std::vector<NamespaceName>::const_iterator> const& path) const
    throw(UnknownNamespace)
  {
    try {
      if (path.first==path.second) {
        return *this;
      }
      auto i=children_.find(*path.first);
      if (i==children_.end()) {
        std::vector<NamespaceName> known;
        for(auto x:children_) {
          known.push_back(x.first);
        }
        std::ostringstream s;
        s << "unknown namespace " << (*path.first)
          << ", not one of "
          << xju::format::join(known.begin(),known.end(),",");
        throw UnknownNamespace(s.str(),XJU_TRACED);
      }
      return (*i).second.findNamespace({xju::next(path.first),path.second});
    }
    catch(xju::Exception& e) {
      std::ostringstream s;
      s << "find namespace " << xju::format::join(path.first,path.second,"::");
      e.addContext(s.str(),XJU_TRACED);
      throw;
    }
  }
  
  
#line 130
std::vector<Location> const& Namespace::findSymbol(
    UnqualifiedSymbol const& symbol) const throw(
      UnknownSymbol)
  {
    try {
      auto i=symbols_.find(symbol);
      if (i==symbols_.end()) {
        throw UnknownSymbol("unknown symbol",XJU_TRACED);
      }
      return (*i).second;
    }
    catch(xju::Exception& e) {
      std::vector<UnqualifiedSymbol> known;
      for(auto x:symbols_) {
        known.push_back(x.first);
      }
      std::ostringstream s;
      s << "find symbol " << symbol << " amongst "
        << xju::format::join(known.begin(),known.end(),",");
      e.addContext(s.str(),XJU_TRACED);
      throw;
    }
  }
  

}
}
