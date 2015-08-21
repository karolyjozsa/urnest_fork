// Copyright (c) 2014 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#ifndef XJU_SNMP_HH
#define XJU_SNMP_HH

#include <string>
#include "xju/Exception.hh"
#include <vector>
#include <stdint.h>
#include <memory>
#include <xju/Int.hh>
#include "xju/Tagged.hh"
#include <set>
#include <map>
#include "xju/MicroSeconds.hh"

namespace xju
{
namespace snmp
{

// The intended use of this module for snmp-get is:
//
//   SnmpV1GetRequest request(...);
//   std::vector<uint8_t> requestData(encode(request));
//   ... send requestData to server
//   ... receive responseData as std::vector<uint8_t>
//   std::map<Oid, std::shared_ptr<Value> > values(
//     validateResponse(request,decodeSnmpV1Response(responseData)));
//   ... use values
//
// ... and for snmp-set is:
//
//   SnmpV1SetRequest request(...);
//   std::vector<uint8_t> requestData(encode(request));
//   ... send requestData to server
//   ... receive responseData as std::vector<uint8_t>
//   validateResponse(request,decodeSnmpV1Response(responseData));
//
// ... and for snmp-get-next is:
//   SnmpV1Table t{oid1,oid2,oid3};
//   while(t.incomplete()) {
//     SnmpV1GetNextRequest request(..., t.nextOids());
//     std::vector<uint8_t> requestData(encode(request);
//     ... send requestData to server
//     ... receive responseData as std::vector<uint8_t>
//     t.add(validateResponse(request,decodeSnmpV1Response(responseData));
//   }
//   for(auto i=0; i!=t.size(); ++i)
//   {
//     ... do something with t[oid1][i], t[oid2][i], t[oid3][i]
//   }
//  


// RFC 1157 OID
class Oid
{
public:
  // construct from dotted, eg .1.3.6.1.4.364
  explicit Oid(std::string const& dotted) throw(
    xju::Exception);
  explicit Oid(std::vector<uint32_t> const& components) throw():
      components_(components)
  {
  }
  std::string toString() const throw();
  std::vector<uint32_t> const& components() const throw()
  {
    return components_;
  }
  bool contains(Oid const& y) const throw();
  
private:
  std::vector<uint32_t> components_;

  friend bool operator==(Oid const& x, Oid const& y) throw()
  {
    return x.components_ == y.components_;
  }
  friend bool operator!=(Oid const& x, Oid const& y) throw()
  {
    return x.components_ != y.components_;
  }
  friend bool operator<(Oid const& x, Oid const& y) throw()
  {
    return x.components_ < y.components_;
  }
  friend Oid operator+(Oid const& a, Oid const& b) throw();

  friend std::ostream& operator<<(std::ostream& s, Oid const& x) throw()
  {
    return s << x.toString();
  }
};

// RFC 1157 request-id
class RequestIdTag{};
typedef xju::Int<RequestIdTag,uint64_t> RequestId;

// RFC 1157 community
class CommunityTag{};
typedef xju::Tagged<std::string,CommunityTag> Community;

struct IPv4Address
{
  // octets in host byte-order
  explicit IPv4Address(uint32_t octets) throw():
      _(octets)
  {
  }
  //octets with octets[0] msb
  //pre: octets.size()==4
  explicit IPv4Address(std::vector<uint8_t> const& octets) throw():
      _((octets[0]<<24)|(octets[1]<<16)|(octets[2]<<8)|(octets[3]))
  {
  }
  
  uint32_t _;
  
  // i'th octet, i=0 gets msb
  // pre: 0<=i<=3
  uint8_t operator[](int i) const throw()
  {
    return (_ >> (8*(3-i)))&0xff;
  }

  friend std::ostream& operator<<(std::ostream& s, IPv4Address const& x) 
    throw();
  
  friend bool operator<(IPv4Address const& x, IPv4Address const& y) throw()
  {
    return x._ < y._;
  }
  friend bool operator>(IPv4Address const& x, IPv4Address const& y) throw()
  {
    return y < x;
  }
  friend bool operator!=(IPv4Address const& x, IPv4Address const& y) throw()
  {
    return y < x || x < y;
  }
  friend bool operator==(IPv4Address const& x, IPv4Address const& y) throw()
  {
    return !(y!=x);
  }
  friend bool operator<=(IPv4Address const& x, IPv4Address const& y) throw()
  {
    return !(x>y);
  }
  friend bool operator>=(IPv4Address const& x, IPv4Address const& y) throw()
  {
    return !(x<y);
  }
};

struct SnmpV1GetRequest
{
  SnmpV1GetRequest(Community const& community,
                   RequestId const id,
                   std::set<Oid> const& oids) throw():
      community_(community),
      id_(id),
      oids_(oids) {
  }
  Community community_;
  RequestId id_;
  std::set<Oid> oids_;

  friend std::ostream& operator<<(std::ostream& s, 
                                  SnmpV1GetRequest const& x) throw();
};
  
std::vector<uint8_t> encode(SnmpV1GetRequest const& request) throw();

class Value
{
public:
  virtual ~Value() throw(){}

  explicit Value(size_t encodedLength) throw():
      encodedLength_(encodedLength)
  {
  }
  
  // encodeTo(x)-x
  size_t const encodedLength_;
  
  // convenience functions that do type and range checking
  operator std::string() const throw(xju::Exception);
  operator int() const throw(xju::Exception);
  operator unsigned int() const throw(xju::Exception);
  operator long() const throw(xju::Exception);
  operator unsigned long() const throw(xju::Exception);

  // return length of encoded value
  // ie return encodeTo(x)-x
  size_t encodedLength() const throw()
  {
    return encodedLength_;
  }

  // encode at begin, returning end of encoding
  virtual std::vector<uint8_t>::iterator encodeTo(
    std::vector<uint8_t>::iterator begin) const throw()=0;

  // human readable
  virtual std::string str() const throw()=0;
  
  friend std::ostream& operator<<(std::ostream& s, Value const& x) throw()
  {
    return s << x.str();
  }
};

class IntValue : public Value
{
public:
  ~IntValue() throw(){}
  
  explicit IntValue(int64_t const& val) throw();
  
  int64_t const val_;

  // Value::
  std::vector<uint8_t>::iterator encodeTo(
    std::vector<uint8_t>::iterator begin) const throw() override;

  // Value::
  virtual std::string str() const throw() override;
};

class StringValue : public Value
{
public:
  ~StringValue() throw(){}
  
  explicit StringValue(std::string const& val) throw();
  std::string const val_;

  // Value::
  std::vector<uint8_t>::iterator encodeTo(
    std::vector<uint8_t>::iterator begin) const throw() override;

  // Value::
  virtual std::string str() const throw() override;
};

class OidValue : public Value
{
public:
  ~OidValue() throw(){}
  
  explicit OidValue(Oid const& val) throw();
  Oid const val_;

  // Value::
  std::vector<uint8_t>::iterator encodeTo(
    std::vector<uint8_t>::iterator begin) const throw() override;

  // Value::
  virtual std::string str() const throw() override
  {
    return val_.toString();
  }
};

class NullValue : public Value
{
public:
  ~NullValue() throw(){}
  
  explicit NullValue() throw():
      Value(2)
  {
  }

  // Value::
  std::vector<uint8_t>::iterator encodeTo(
    std::vector<uint8_t>::iterator begin) const throw() override;

  // Value::
  virtual std::string str() const throw() override
  {
    return "null";
  }
};

class IPv4AddressValue : public Value
{
public:
  explicit IPv4AddressValue(IPv4Address val) throw();

  IPv4Address const val_;

  // Value::
  virtual std::vector<uint8_t>::iterator encodeTo(
    std::vector<uint8_t>::iterator begin) const throw();
  
  // Value::
  virtual std::string str() const throw();
};
  
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


struct SnmpV1Response
{
  // RFC 1157 error-status
  enum ErrorStatus
  {
    NO_ERROR,
    TOO_BIG,
    NO_SUCH_NAME,
    BAD_VALUE,
    READ_ONLY,
    GEN_ERR
  };
  
  // RFC 1157 error-index
  // note error-index of first oid is 1, not 0
  class ErrorIndexTag{};
  typedef xju::Int<ErrorIndexTag,uint64_t> ErrorIndex;

  SnmpV1Response(
    uint8_t responseType,
    Community community,
    RequestId id,
    ErrorStatus error,
    ErrorIndex errorIndex,
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values)
      throw():
      responseType_(responseType),
      community_(community),
      id_(id),
      error_(error),
      errorIndex_(errorIndex),
      values_(values) {
  }
  uint8_t responseType_;
  Community community_;
  RequestId id_;
  ErrorStatus error_;
  // 1-based ie bad oid is values_[errorIndex_-1]; 
  // (0 if error_ is a non-param-specific error)
  ErrorIndex errorIndex_;
  
  std::vector<std::pair<Oid, std::shared_ptr<Value const> > > values_;

  friend std::ostream& operator<<(std::ostream& s, SnmpV1Response const& x)
    throw();
  
};

SnmpV1Response decodeSnmpV1Response(std::vector<uint8_t> const& data) throw(
    // malformed
    xju::Exception);

// exceptions corresponding to RFC 1157 error-status
class GenErr : public xju::Exception
{
public:
  explicit GenErr(const xju::Traced& trace) throw();
};
class TooBig : public xju::Exception
{
public:
  explicit TooBig(const xju::Traced& trace) throw();
};
class InvalidParam : public xju::Exception
{
public:
  InvalidParam(std::string const& cause, Oid const& param, const xju::Traced& trace) throw();
  Oid param_;
};
class NoSuchName : public InvalidParam
{
public:
  NoSuchName(Oid const& param, const xju::Traced& trace) throw();
};
class BadValue : public InvalidParam
{
public:
  BadValue(Oid const& param, const xju::Traced& trace) throw();
};
class ReadOnly : public InvalidParam
{
public:
  ReadOnly(Oid const& param, const xju::Traced& trace) throw();
};

// other exceptions
class ResponseTypeMismatch : public xju::Exception
{
public:
  ResponseTypeMismatch(uint8_t const got,
                       uint8_t const expected,
                       xju::Traced const& trace) throw();
  uint8_t got_;
  uint8_t expected_;
};
class ResponseIdMismatch : public xju::Exception
{
public:
  ResponseIdMismatch(RequestId const got,
                     RequestId const expected,
                     xju::Traced const& trace) throw();

  RequestId got_;
  RequestId expected_;
};
  

// validate reponse to specified request
// post: *result[x] valid for all x in request.oids_
// - returns the requested values
std::map<Oid, std::shared_ptr<Value const> > validateResponse(
  SnmpV1GetRequest const& request,
  SnmpV1Response const& response) throw(
    ResponseTypeMismatch,
    ResponseIdMismatch,
    NoSuchName,
    TooBig,
    GenErr,
    // response malformed eg not all requested oids present in response
    xju::Exception);

struct SnmpV1SetRequest
{
  SnmpV1SetRequest(
    Community community,
    RequestId id,
    std::map<Oid, std::shared_ptr<Value const> > values) throw():
      community_(community),
      id_(id),
      values_(values) {
  }
  Community community_;
  RequestId id_;
  std::map<Oid, std::shared_ptr<Value const> > values_;

  friend std::ostream& operator<<(std::ostream& s, 
                                  SnmpV1SetRequest const& x) throw();
};

std::vector<uint8_t> encode(SnmpV1SetRequest const& request) throw();

// validate reponse to specified request
// - note that RFC 1157 says that no values are modified if an error
//   is returned
void validateResponse(
  SnmpV1SetRequest const& request, 
  SnmpV1Response const& response) throw(
    // response.responseType_ != 0xA3
    ResponseTypeMismatch,
    // response.id_ != request.id_
    ResponseIdMismatch,
    // server does not know NoSuchName.oid_ 
    NoSuchName,
    // server forbids request.values_[BadValue.oid] as value of BadValue.oid
    BadValue,
    // ReadOnly.oid_ is read-only
    ReadOnly,
    // request was too big to process or responsd to
    // (note no values were set)
    TooBig,
    // SNMP General Error
    GenErr,
    // response malformed eg not all requested oids present in response
    xju::Exception);

struct SnmpV1GetNextRequest
{
  SnmpV1GetNextRequest(Community const& community,
                       RequestId const id,
                       std::vector<Oid> const& oids) throw():
      community_(community),
      id_(id),
      oids_(oids) {
  }
  Community community_;
  RequestId id_;
  std::vector<Oid> oids_;

  friend std::ostream& operator<<(std::ostream& s, 
                                  SnmpV1GetNextRequest const& x) throw();
};
  
std::vector<uint8_t> encode(SnmpV1GetNextRequest const& request) throw();

// validate reponse to specified request
// - result suitable to pass to SnmpV1Table.add()
//   note: NoSuchName error is translated to a result with all its oids
//         set to 1.3, to signal end of table (see SnmpV1Table)
std::vector<std::pair<Oid, std::shared_ptr<Value const> > > validateResponse(
  SnmpV1GetNextRequest const& request,
  SnmpV1Response const& response) throw(
    ResponseTypeMismatch,
    ResponseIdMismatch,
    TooBig,
    GenErr,
    // response malformed eg not all requested oids present in response
    xju::Exception);

class SnmpV1Table
{
public:
  // pre: cols.size()>0
  // post: cols_==cols
  //       !atEnd()
  SnmpV1Table(std::set<Oid> cols) throw();
  
  std::set<Oid> const cols_;

  struct Cell
  {
    explicit Cell(std::pair<Oid, std::shared_ptr<Value const> > v) throw():
        oid_(v.first),
        value_(v.second) {
    }
    Oid oid_;
    std::shared_ptr<Value const> value_;
  };
  
  // pre: col in cols_
  std::vector<Cell> const& operator[](Oid const& col) const throw();
  
  bool atEnd() const throw() { return atEnd_; }

  // post: result.size()==cols_.size()
  std::vector<Oid> nextOids() const throw();
  
  // pre: row.size()==cols_.size()
  void add(
    std::vector<std::pair<Oid, std::shared_ptr<Value const> > > const& row)
  throw();

private:
  std::map<Oid, std::vector<Cell> > data_;
  bool atEnd_;
};

struct SnmpV1Trap
{
  enum class GenericType
  {
    COLDSTART,
    WARMSTART,
    LINKDOWN,
    LINKUP,
    AUTHENTICATIONFAILURE,
    EGPNEIGHBORLOSS,
    ENTERPRISESPECIFIC
  };
    
  struct SpecificTypeTag{};
  typedef xju::Int<SpecificTypeTag,uint32_t> SpecificType;
  
  SnmpV1Trap(
    Community community,
    Oid trapType,
    IPv4Address origin,
    GenericType genericType,
    SpecificType specificType,
    xju::MicroSeconds timestamp,
    std::map<Oid, std::shared_ptr<Value const> > vars) throw():
      community_(community),
      trapType_(trapType),
      origin_(origin),
      genericType_(genericType),
      specificType_(specificType),
      timestamp_(timestamp),
      vars_(vars) {
  }

  Community community_;
  Oid trapType_;
  IPv4Address origin_;
  GenericType genericType_;
  SpecificType specificType_;
  xju::MicroSeconds timestamp_;

  std::map<Oid, std::shared_ptr<Value const> > vars_;
  
  friend std::ostream& operator<<(std::ostream& s, SnmpV1Trap const& x) 
    throw();
};

std::vector<uint8_t> encode(SnmpV1Trap const& trap) throw();

  
}
}

#endif
