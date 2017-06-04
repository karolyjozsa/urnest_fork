#include <xju/io/select.hh>
#line 1 "/home/xju/urnest/xju/io/select.hcp"
#line 15
#include <xju/io/Input.hh> //impl
#include <xju/io/Output.hh> //impl

#include <xju/syscall.hh> //impl
#include <xju/unistd.hh> //impl
#include <xju/format.hh> //impl
#include <sys/select.h> //impl
namespace xju
{
namespace io
{
#line 37
std::pair<std::set<Input const*>,std::set<Output const*> > select(
  std::set<Input const*> const& inputs,
  std::set<Output const*> const& outputs,
  std::chrono::system_clock::time_point const& deadline) throw(
    std::bad_alloc)
{
  std::pair<std::set<Input const* >,std::set<Output const* > > result;
  int n;
  do {
    fd_set r;
    FD_ZERO(&r);
    for(auto x : inputs) {
      FD_SET(x->fileDescriptor(),&r);
    }
    fd_set w;
    FD_ZERO(&w);
    for(auto x : outputs) {
      FD_SET(x->fileDescriptor(),&w);
    }
    std::chrono::system_clock::duration const timeout(
      std::max(
        std::chrono::system_clock::duration(
          std::chrono::seconds(0)),
        std::min(deadline-std::chrono::system_clock::now(),
                 std::chrono::system_clock::duration(
                   std::chrono::seconds(std::numeric_limits<long>::max())))));
    std::chrono::seconds const tv_sec(
      std::chrono::duration_cast<std::chrono::seconds>(timeout));
    std::chrono::microseconds const tv_usec(
      std::chrono::duration_cast<std::chrono::microseconds>(timeout-tv_sec));
    struct timeval timeout_;
    timeout_.tv_sec=tv_sec.count();
    timeout_.tv_usec=tv_usec.count();
    n=::select(FD_SETSIZE,&r,&w,0,&timeout_);
    switch(n) {
    case 0:
      break;
    case -1:
      if (errno != EINTR)
      {
        xju::assert_equal(errno,ENOMEM);
        throw std::bad_alloc();
      }
      break;
    default:
      for(auto x : inputs) {
        if (FD_ISSET(x->fileDescriptor(),&r)) {
          result.first.insert(x);
        }
      }
      for(auto x : outputs) {
        if (FD_ISSET(x->fileDescriptor(),&w)) {
          result.second.insert(x);
        }
      }
    }
  }
  while(result.first.size()==0 &&
        result.second.size()==0 &&
        std::chrono::system_clock::now()<deadline);
  return result;
}

// as above but without any outputs
// 
std::set<Input const* > select(
  std::set<Input const* > const& inputs,
  std::chrono::system_clock::time_point const& deadline) throw(
    xju::SyscallFailed)
{
  return select(inputs,std::set<Output const* >(),deadline).first;
}

// as above but without any inputs
// 
std::set<Output const* > select(
  std::set<Output const* > const& outputs,
  std::chrono::system_clock::time_point const& deadline) throw(
    std::bad_alloc)
{
  return select(std::set<Input const*>(),outputs,deadline).second;
}


}
}