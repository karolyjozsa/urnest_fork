#include <hcp/tags/augmentRootNamespace.hh>
#line 1 "/home/xju/urnest/hcp/tags/augmentRootNamespace.hcp"
#line 12
#include <hcp/parser.hh> //impl
#line 15
#include <xju/stringToUInt.hh> //impl
#include <hcp/ast.hh> //impl
#include "xju/assert.hh" //impl
#line 19
#include <xju/readFile.hh> //impl
#line 21
#include <hcp/tags/splitSymbol.hh> //impl

namespace hcp
{
namespace tags
{
#line 28
namespace
{
typedef hcp_parser::PR PR_;
typedef hcp_parser::IR IR_;

PR_ eatWhite_(hcp_parser::zeroOrMore()*hcp_parser::whitespaceChar());
PR_ stringValue(hcp_parser::parseUntil(hcp_parser::doubleQuote()));
struct SymbolTag{};
typedef hcp_ast::TaggedCompositeItem<SymbolTag> Symbol;
PR_ symbol(new hcp_parser::NamedParser<Symbol>(
            "symbol",
            stringValue));
struct FileNameTag{};
typedef hcp_ast::TaggedCompositeItem<FileNameTag> ParsedFileName;
PR_ fileName(new hcp_parser::NamedParser<ParsedFileName>(
              "file name",
              stringValue));

class ParsedLineNumber : public hcp_ast::CompositeItem
{
public:
  ParsedLineNumber(std::vector<hcp_ast::IR> const& items) throw(xju::Exception):
      CompositeItem(items),
      lineNumber_(xju::stringToUInt(hcp_ast::reconstruct(*this)))
  {
  }
  hcp::tags::LineNumber const lineNumber_;
  operator hcp::tags::LineNumber() const throw()
  {
    return lineNumber_;
  }
};
  
PR_ digit(hcp_parser::charInRange('0','9'));
PR_ lineNumber(new hcp_parser::NamedParser<ParsedLineNumber>(
                "line number",
                // we do not use parse::atLeastOne as it uses
                // CompositeItem, which is redundent
                digit+hcp_parser::zeroOrMore()*digit));

PR_ openBrace(hcp_parser::parseLiteral("{"));
PR_ closeBrace(hcp_parser::parseLiteral("}"));
PR_ f(hcp_parser::parseLiteral("\"f\""));
PR_ l(hcp_parser::parseLiteral("\"l\""));
PR_ colon(hcp_parser::parseLiteral(":"));
PR_ comma(hcp_parser::parseLiteral(","));

struct LocationTag{};
typedef hcp_ast::TaggedCompositeItem<LocationTag> ParsedLocation;
PR_ location(new hcp_parser::NamedParser<ParsedLocation>(
              "location",
              openBrace+eatWhite_+f+eatWhite_+
              colon+eatWhite_+
              hcp_parser::doubleQuote()+fileName+hcp_parser::doubleQuote()+
              eatWhite_+comma+eatWhite_+
              l+eatWhite_+colon+eatWhite_+
              lineNumber+eatWhite_+closeBrace+eatWhite_));

PR_ openSquare(hcp_parser::parseLiteral("["));
PR_ closeSquare(hcp_parser::parseLiteral("]"));

PR_ locations=openSquare+eatWhite_+location+eatWhite_+
  hcp_parser::zeroOrMore()*(comma+eatWhite_+location+eatWhite_)+
  closeSquare+eatWhite_;

struct EntryTag{};
typedef hcp_ast::TaggedCompositeItem<EntryTag> Entry;
PR_ entry(new hcp_parser::NamedParser<Entry>(
           "entry",
           hcp_parser::doubleQuote()+symbol+hcp_parser::doubleQuote()+eatWhite_+
           colon+eatWhite_+locations));

PR_ nextEntry(comma+eatWhite_+entry);

PR_ entries(entry+eatWhite_+hcp_parser::zeroOrMore()*nextEntry+closeBrace+eatWhite_);

PR_ tagsFile=openBrace+eatWhite_+
  (entries|
    (closeBrace+eatWhite_))+
  hcp_parser::endOfFile();

template<class T>
T const& find(hcp_ast::IRs::const_iterator begin,
              hcp_ast::IRs::const_iterator end)
  throw() {
  hcp_ast::IRs::const_iterator const i(
    hcp_ast::find1stInTree(begin,end,
                           [](hcp_ast::IR x) { return x->isA<T>(); }));
  xju::assert_not_equal(i,end);
  return (*i)->asA<T>();
}
}


// augment rootNamespace with symbols from file
void augmentRootNamespace(Namespace& rootNamespace,
                          std::pair<hcp::tags::AbsolutePath, hcp::tags::FileName> const& tagsFileName,
                          bool const traceParser)
  throw(
    // pre: rootNamespace == rootNamespace@pre
    xju::Exception) {
  try {
    std::string const x(xju::readFile(xju::path::str(tagsFileName)));
    hcp_parser::I i(x.begin(), x.end());
    hcp_ast::CompositeItem root;
    
    hcp_parser::parse(root, i, tagsFile, traceParser);

    for(auto x:root.items_) {
      if (x->isA<Entry>()) {
        Entry const& entry(x->asA<Entry>());
        Symbol const& symbol(
          find<Symbol>(entry.items_.begin(),entry.items_.end()));
        std::vector<hcp::tags::Location> locations;
        for(auto x:entry.items_) {
          if (hcp_ast::isA_<ParsedLocation>(x)) {
            ParsedLocation const& location(
              x->asA<ParsedLocation>());
            auto f(find<ParsedFileName>(location.items_.begin(),
                                        location.items_.end()));
            auto l(find<ParsedLineNumber>(location.items_.begin(),
                                          location.items_.end()));
            auto fileName(xju::path::split(reconstruct(f)));
            locations.push_back(hcp::tags::Location(
                                  fileName.first,
                                  fileName.second,
                                  l));
          }
        }
        auto const ns(splitSymbol(reconstruct(symbol)));
        rootNamespace.addSymbol(ns.first,ns.second,locations);
      }
    }
  }
  catch(xju::Exception& e) {
    std::ostringstream s;
    s << "augment namespace with symbols from tags file "
      << xju::path::str(tagsFileName);
    e.addContext(s.str(),XJU_TRACED);
    throw;
  }
}

}
}
