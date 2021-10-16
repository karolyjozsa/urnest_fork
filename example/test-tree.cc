// Copyright (c) 2020 Trevor Taylor
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appear in all.
// Trevor Taylor makes no representations about the suitability of this
// software for any purpose.  It is provided "as is" without express or
// implied warranty.
//
#include <example/tree.hh>

#include <iostream>
#include <xju/assert.hh>

namespace example
{
namespace tree{

struct config_tag{};
template<>
struct config<std::string,config_tag>{
  typedef std::list<std::string> children_type;
};
typedef node<std::string, config_tag> Tree;

void test1() {
  Tree const page_template_(
    { { "html",
        { "body",
          { "div class=q",
            { "p class=q", { "cdata", { "2 + 2 =", {} } } },
            { "p class=a", { "cdata", { "4" } } } },
          { "div class=q",
            { "p class=q", { "cdata", { "6 + 1 =", {} } } },
            { "p class=a", { "cdata", { "7" } } } } } } });
  
  auto real_questions = { {"8 * 8", "64"},
                          {"7 * 2", "14"},
                          {"9 - 3", "6"} };

  auto page = page_template_;

  const Tree qa_template(
    page_.remove_all([](std::string const& x)
                     {
                       return x == "div class=q" ?
                         H::disposition::yes:
                         H::disposition::no_recurse;
                     }).front());

  // put in real questions with no answers
  Tree::children_type qs;
  std::transform(real_questions.begin(),
                 real_qustions.end(),
                 std::inserter(qs, qs.end()),
                 [](auto const& qa){
                   auto q = qa_template;
                   q.find_only("p class=a").find_only("cdata") = "?";
                   return q;
                 });

  // REVISIT: simplistic, need more complicated demo, e.g. common
  // example is to have table with header row that has to be left
  // in place, can either append rows after header or perhaps better
  // is to leave one row (the template) in place and replace that
  // with the real rows.
  page.children_=qs;

  Tree const expect({
    { "html",
      { "body", 
        { "div class=q",
          { "p class=q", { "cdata", { "6 * 8 =", {} } } },
          { "p class=a", { "cdata", { "?" } } } },
        { "div class=q",
          { "p class=q", { "cdata", { "7 * 2 =", {} } } },
          { "p class=a", { "cdata", { "?" } } } },
        { "div class=q",
          { "p class=q", { "cdata", { "9 - 3 =", {} } } },
          { "p class=a", { "cdata", { "?" } } } } } } });

  xju::assert_equal(page, expect);
}

// demonstrate css selector as a select_by_path function, using
// it with find, find_all, remove_selected, apply_to_selected etc
// including inserting and removing nodes
void test2(){
}

// demonstrate same css selector use with tour to "precede" and "follow"
void test2(){
}

// demonstrate same css selector use with tour is more efficient
void test3(){
}

}

using namespace example;

int main(int argc, char* argv[])
{
  unsigned int n(0);
  test1(), ++n;
  std::cout << "PASS - " << n << " steps" << std::endl;
  return 0;
}
