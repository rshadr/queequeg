#ifndef _queequeg_qglib_algoritm_hh
#define _queequeg_qglib_algoritm_hh


#include <algorithm>
#include <iterator>

namespace QueequegLib {

  template< class T, class U> requires std::input_iterator<T>
  check_contains(T first, T last, const U& value)
  {
    return (std::find_if(first, last, value) != last);
  }

};


#endif /* !defined(_queequeg_qglib_algoritm_hh) */

