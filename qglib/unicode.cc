#include <string>

#include <grapheme.h>

#include "qglib/unicode.hh"


namespace QueequegLib {


void
append_c32_as_utf8(std::string *s, char32_t ch)
{
  char utf8[16] = { 0 };
  size_t written;

  written = grapheme_encode_utf8(ch, utf8, sizeof (utf8));

  s->append(utf8, written);
}


} /* namespace QueequegLib */

