//     -*- mode: c++ ; c-file-style: "xju" ; -*-
//
// Copyright (c) 2008
// Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
// C++ syntax parsing, for example usage, see:
//  - hcp-parse-file.cc
//  - hcp-split.cc
//  - test-parser.cc
//
#ifndef HCP_PARSER_H_
#define HCP_PARSER_H_

#include <hcp/ast.hh>
#include "xju/Shared.hh"
#include "xju/Exception.hh"
#include "xju/next.hh"
#include <sstream>
#include <string>
#include <utility>
#include <set>
#include <algorithm>
#include <hcp/trace.hh>
#include "xju/Optional.hh"
#include <map>
#include <vector>

namespace hcp_parser
{

typedef hcp_ast::I I;
typedef hcp_ast::IR IR;
typedef std::vector<IR> IRs;
typedef std::pair<IRs, I> PV;

class Parser;
xju::Shared<Parser> file() throw(); // reference to whole-file parser

// The simplest parsing interface, which parses the specified
// type of C++ element (default is "whole file") assumed to
// appear at the specified startOfElement, appending ast Items
// to the specified parent.items_, and returning the position
// just after the parsed element.
// 
I parse(hcp_ast::CompositeItem& parent,
        I const startOfElement,
        xju::Shared<Parser> = file(),
        bool traceToStdout = false,
        bool irsAtEnd = false)
  throw(
    // post: parent unmodified
    xju::Exception);

class Exception
{
public:
  class Cause
  {
  public:
    virtual ~Cause() throw()
    {
    }
    virtual std::string str() const throw() = 0;
  };
    
  Exception(xju::Shared<Cause const> cause, 
            I at, 
            xju::Traced const& trace,
            bool atEnd=false) throw():
    cause_(cause),
    at_(at),
    trace_(trace),
    atEnd_(atEnd) {
  }
  // gcc 4.7.2 refuses to generate the copy constructor
  Exception(Exception const& b) throw():
      cause_(b.cause_),
      at_(b.at_),
      trace_(b.trace_),
      context_(b.context_),
      atEnd_(b.atEnd_),
      irsAtEnd_(b.irsAtEnd_) {
  }

  //pre: lifetime(parser) includes lifetime(this)
  void addContext(Parser const& parser, I at, xju::Traced const& trace) throw()
  {
    context_.push_back(std::make_pair(std::make_pair(&parser, at), trace));
  }
  std::vector<std::pair<std::pair<Parser const*, I>, xju::Traced> > context_;

  xju::Shared<Cause const> const cause_;
  I const at_;
  xju::Traced const trace_;

  bool atEnd() const throw()
  {
    return atEnd_;
  }
  // add at-end irs, last-first
  void addAtEndIRs(IRs const& irs) throw()
  {
    if (atEnd_) {
      std::copy(irs.rbegin(),irs.rend(),std::back_inserter(irsAtEnd_));
    }
  }
  IRs const& getIrsAtEnd() const throw()
  {
    return irsAtEnd_;
  }
  
private:
  //indicates eof caused parse failure
  bool const atEnd_;
  //if atEnd_, collects 
  std::vector<IR> irsAtEnd_;
};
std::string readableRepr(Exception const& e) throw();
  
  

class ParseResult
{
public:
  //post: failed()
  explicit ParseResult(Exception const& e) throw():
      e_(e)
  {
  }
  //post: !failed()
  explicit ParseResult(PV v) throw():
      v_(v)
  {
  }
  bool failed() const throw()
  {
    return !v_.valid();
  }
  
  //pre: !failed()
  PV const& operator*() const throw()
  {
    return v_.value();
  }
  
  //pre: failed()
  Exception const& e() const throw()
  {
    return e_.value();
  }

  //pre: failed()
  void addContext(Parser const& p, I at, const xju::Traced& trace) throw()
  {
    e_.value().addContext(p, at, trace);
  }
  void addAtEndIRs(IRs const& irs) throw()
  {
    e_.value().addAtEndIRs(irs);
  }
private:
  xju::Optional<Exception> e_;
  xju::Optional<PV> v_;
};

class Parser;
typedef std::pair<I, Parser*const> CacheKey;
typedef std::map<CacheKey, ParseResult> CacheVal;
    
typedef xju::Shared<CacheVal> Cache;
    
class Options
{
public:
  explicit Options(bool trace,
                   Cache cache,
                   bool irsAtEnd) throw():
    trace_(trace),
    cache_(cache),
    irsAtEnd_(irsAtEnd) {
  }
  Options(Options const& y) throw():
      trace_(y.trace_),
      cache_(y.cache_),
      irsAtEnd_(y.irsAtEnd_) {
  }
    
  bool trace_;
  mutable xju::Shared<std::map<std::pair<I, Parser*const>, ParseResult> > cache_;
  bool irsAtEnd_;
};

class Parser
{
public:
  virtual ~Parser() throw() {
  }
  
  // post: 
  virtual ParseResult parse_(
    I const at, 
    Options const& options) throw() = 0;

  // What the parser matches, e.g. "string literal", "typedef", "class"
  virtual std::string target() const throw() = 0;

  // post: result contains references to at or beyond, even if parsing
  //       fails (ie if result.failed())
  ParseResult parse(I const at, Options const& options) throw() 
  {
    CacheKey const k(at, this);
    CacheVal::const_iterator i((*options.cache_).find(k));
    if (i == (*options.cache_).end()) {
      ParseResult result(parse_(at, options));
      if (result.failed()) {
        result.addContext(*this, at, XJU_TRACED);
        }
        CacheVal::value_type v(k, result);
        i=(*options.cache_).insert(v).first;
    }
    return (*i).second;
  }

  template<class T>
  bool isA() const throw() {
    return dynamic_cast<T const*>(this);
  }
  // pre: isA<T>()
  template<class T>
  T& asA() throw() {
    return *dynamic_cast<T*>(this);
  }
  // pre: isA<T>()
  template<class T>
  T const& asA() const throw() {
    return *dynamic_cast<T const*>(this);
  }
};
typedef xju::Shared<Parser> PR;

class ZeroOrMore{};

PR operator*(ZeroOrMore a, PR b) throw();

ZeroOrMore zeroOrMore() throw();

// result parses an a then a b
PR operator+(PR a, PR b) throw();

// result parses an a or a b
PR operator|(PR a, PR b) throw();

// result throws if x succeeds; otherwise does
// not consume anything
PR operator!(PR x) throw();

// same as x except that if x fails we get name in exception context
PR anon(std::string const& name, PR const x) throw();

class NamedParser_ : public Parser
{
public:
  virtual ~NamedParser_() throw()
  {
  }
};

// pre: ItemType is e.g. a hcp_ast::CompositeItem
template<class ItemType>
class NamedParser : public NamedParser_
{
public:
  std::string const name_;
  PR x_;
  
  virtual ~NamedParser() throw() {
  }

  explicit NamedParser(std::string const& name, PR const x) throw():
    name_(name),
    x_(x) {
  }

  // Parser::
  // Parse x and:
  // - if x fails, put name in excetion context
  // - if x succeeds, wrap result in ItemType and return that as result
  virtual ParseResult parse_(I const at, Options const& o) throw() 
  {
    std::unique_ptr<hcp_trace::Scope> scope;
    if (o.trace_) {
      std::ostringstream s;
      s << "parse " << target() << " at " << at;
      scope = std::unique_ptr<hcp_trace::Scope>(
        new hcp_trace::Scope(s.str(), XJU_TRACED));
    }
    ParseResult r(x_->parse(at, o));
    if (!r.failed()) {
      PV a(*r);
      if (!a.first.size()) {
        // composite needs an item
        a.first.push_back(IR(new hcp_ast::String(at, at)));
      }
      return ParseResult(PV(IRs(1U, new ItemType(a.first)), a.second));
    }
    if (o.trace_) {
      scope->fail();
    }
    return r;
  }
  // Parser::
  virtual std::string target() const throw() {
    return name_;
  }
};

PR atLeastOne(PR const x) throw();
PR optional(PR x) throw();

PR parseAnyChar() throw();
PR parseOneOfChars(std::string const& chars) throw();
PR parseAnyCharExcept(std::string const& chars) throw();
PR charInRange(char const min, char const max) throw();

PR parseLiteral(std::string const& x) throw();


// parse match repeatedly until specified until
// - does not consume until
PR parseUntil(PR const match, PR const until) throw();

// shorthand for parseUntil(parseAnyChar(),x)
// ... downside of this is poor exception message, so prefer
//     above parseUntil
// - does not consume x
PR parseUntil(PR const x) throw();

// Parse text, balancing (), [], {}, stringLiteral and optionally <>, 
// up to first match of until.
PR balanced(PR until, bool angles=false) throw();

PR whitespaceChar() throw();
PR doubleQuote() throw();
PR doubleColon() throw();
PR backslash() throw();
PR oneChar() throw(); //any single char
PR bracketed(PR x) throw(); //x inside brackets, with optional whitespace preceding x

// rest are C++-specific
PR comments() throw();
PR eatWhite() throw(); // matches nothing or something; eats C++ comments
PR identifier() throw(); //C++ identifier
PR stringLiteral() throw(); //C++ string literal
PR hashIncludeImpl() throw(); // include preprocessor directive, with trailing "// impl" marker
PR hashInclude() throw(); // include preprocessor directive
PR hash() throw();        // other preprocessor directive
PR cv() throw(); //C++ const/volatile qualifiers
PR type_name() throw(); // examples: int, typename x::y::Z, X<Q>::size, Y<Z>
PR type_qual() throw(); //C++ const/volatile/*/& qualifier + whitespace
PR type_ref() throw(); //type ref egs "int const&", "Y<Z>* const&"
PR typedef_statement() throw();    // restriction: no anon class/struct/enum
PR using_statement() throw();       // using statement
PR scoped_enum_def() throw();
PR enum_def() throw();
PR conversion_operator_name() throw(); // eg operator constT&
PR function_proto() throw();
PR function_decl() throw();
PR template_function_def() throw();
PR function_def() throw(); // matches template, so try template_function_def first
PR var_fp() throw(); // function pointer eg int (T::*f)()
PR var_def() throw();
PR global_var_def() throw();
PR static_var_def() throw();
PR extern_var_def() throw();
PR class_decl() throw();   // template/non-template
PR class_def() throw();    // template/non-template
PR anonymous_namespace() throw();
PR namespace_def() throw();    // matches anonymous, so try anonymous_namespace first

PR endOfFile() throw(); // matches end of file

}

#endif
