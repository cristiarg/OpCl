
#include <cassert>

template < typename at_ >
struct TOpenThenClose {
  TOpenThenClose( at_& ac_)
      : mv_( ac_)
  {
    assert( false == mv_.mf_bIsOpen() );
    mv_.mp_Open();
  }

  ~TOpenThenClose()
  {
    mv_.mp_Close();
    assert( false == mv_.mf_bIsOpen() );
  }

private:
  at_& mv_;
};

