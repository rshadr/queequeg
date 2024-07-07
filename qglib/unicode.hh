#ifndef _queequeg_qglib_unicode_hh_
#define _queequeg_qglib_unicode_hh_


#include <string>


namespace QueequegLib {

  void append_c32_as_utf8(std::string *str, char32_t ch);

};


#endif /* _queequeg_qglib_unicode_hh_ */

