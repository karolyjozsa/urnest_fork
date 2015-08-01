// Copyright (c) 2014 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#include "snmp.hh"

#include <iostream>
#include <xju/assert.hh>

namespace xju
{
namespace snmp
{

void test1() throw() {
  Oid x(".1.3.6.1.4.364");
  xju::assert_equal(x.toString(), ".1.3.6.1.4.364");
  std::vector<uint32_t> b;
  b.push_back(1);
  b.push_back(3);
  b.push_back(6);
  b.push_back(1);
  b.push_back(4);
  b.push_back(364);
  xju::assert_equal(x.components(),b);
  Oid y(b);
  xju::assert_equal(y.toString(), x.toString());
  xju::assert_equal(x,y);
  std::vector<uint32_t> c(b);
  c.push_back(3);
  Oid a(c);
  xju::assert_not_equal(x, a);
  xju::assert_less(x, a);

  Oid a2(x+Oid(".3"));
  xju::assert_equal(a2, a);
}

void test2()
{
  try {
    Oid x("");
    xju::assert_abort();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e), "Failed to parse \"\" assumed to be an SNMP OID in dotted notation, eg .1.3.6.1.4.364 because\n\"\" does not start with '.'.");
  }
  try {
    Oid x("34.53");
    xju::assert_abort();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e), "Failed to parse \"34.53\" assumed to be an SNMP OID in dotted notation, eg .1.3.6.1.4.364 because\n\"34.53\" does not start with '.'.");
  }
  try {
    Oid x(".34.");
    xju::assert_abort();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e), "Failed to parse \".34.\" assumed to be an SNMP OID in dotted notation, eg .1.3.6.1.4.364 because\nfailed to parse component \"\" (at offset 4) because\nfailed to convert \"\" to a base-10 unsigned integer because\n\"\" is null.");
  }
  try {
    Oid x(".ac");
    xju::assert_abort();
  }
  catch(xju::Exception const& e) {
    xju::assert_equal(readableRepr(e), "Failed to parse \".ac\" assumed to be an SNMP OID in dotted notation, eg .1.3.6.1.4.364 because\nfailed to parse component \"ac\" (at offset 1) because\nfailed to convert \"ac\" to a base-10 unsigned integer because\ncharacter 1 ('a') of \"ac\" unexpected.");
  }
  
}

void test3() throw()
{
  // NullValue encoding
  {
    NullValue const x;
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),2U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x05,0});
  }
   
  // IntValue encoding
  // 0
  {
    IntValue const x(0);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,1,0});
  }
 
  // -1, 1
  {
    IntValue const x(-1);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,1,0xff});
  }
  {
    IntValue const x(1);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,1,0x01});
  }
  // -128,127
  {
    IntValue const x(-128);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,1,0x80});
  }
  {
    IntValue const x(127);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,1,0x07f});
  }
  // -129,128
  {
    IntValue const x(-129);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,2,0xff,0x7f});
  }
  {
    IntValue const x(128);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,2,0x00,0x80});
  }
  // -32768,32767
  {
    IntValue const x(-32768);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,2,0x80,0x00});
  }
  {
    IntValue const x(32767);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,2,0x7f,0xff});
  }
  // -32769,32768
  {
    IntValue const x(-32769);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),5U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,3,0xff,0x7f,0xff});
  }
  {
    IntValue const x(32768);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),5U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,3,0x00,0x80,0x00});
  }
  // INT32_MAX,INT32_MIN
  {
    IntValue const x(0x7fffffff);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),6U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,4,0x7f,0xff,0xff,0xff});
  }
  {
    int32_t xx(0x80000000);
    IntValue const x(xx);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),6U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,4,0x80,0x00,0x00,0x00});
  }
  // INT64_MAX,INT64_MIN
  {
    IntValue const x(0x7fffffffffffffff);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),10U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,8,0x7f,0xff,0xff,0xff,0xff,0xff,0xff,0xff});
  }
  {
    IntValue const x(0x8000000000000000);
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),10U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x02,8,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00});
  }

  // StringValue encoding
  // null string
  {
    StringValue const x("");
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),2U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x04,0});
  }
  // fred
  {
    StringValue const x("fred");
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),6U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x04,4,'f','r','e','d'});
  }
  // 127 byte string
  {
    StringValue const x(std::string(127U,'x'));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),129U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y[0],0x04);
    xju::assert_equal(y[1],127);
    xju::assert_equal(std::string(y.begin()+2,y.end()),
                      std::string(127U,'x'));
  }
  // 128 byte string
  {
    StringValue const x(std::string(128U,'x'));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),131U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y[0],0x04);
    xju::assert_equal(y[1],0x81);
    xju::assert_equal(y[2],128);
    xju::assert_equal(std::string(y.begin()+3,y.end()),
                      std::string(128U,'x'));
  }

  // OidValue encoding
  // .1.3
  {
    OidValue const x(Oid(".1.3"));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),3U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x06,1,40*1+3});
  }
  // .1.3.4
  {
    OidValue const x(Oid(".1.3.4"));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x06,2,40*1+3,0x04});
  }
  // .1.3.127
  {
    OidValue const x(Oid(".1.3.127"));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),4U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x06,2,40*1+3,0x7f});
  }
  // .1.3.128
  {
    OidValue const x(Oid(".1.3.128"));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),5U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x06,3,40*1+3,0x81,0x00});
  }
  // .1.3.128.4
  {
    OidValue const x(Oid(".1.3.128.4"));
    std::vector<uint8_t> y(x.encodedLength(),0U);
    xju::assert_equal(y.size(),6U);
    xju::assert_equal(x.encodeTo(y.begin()),y.end());
    xju::assert_equal(y,std::vector<uint8_t>{0x06,4,40*1+3,0x81,0x00,0x04});
  }

}

}
}


using namespace xju::snmp;

int main(int argc, char* argv[])
{
  unsigned int n(0);
  test1(), ++n;
  test2(), ++n;
  test3(), ++n;
  std::cout << "PASS - " << n << " steps" << std::endl;
  return 0;
}

