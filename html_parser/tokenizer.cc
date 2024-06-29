#include <stddef.h>
#include <stdio.h>

#include <grapheme.h>
#include <infra/string.h>

#include "html_parser/common.hh"



Tokenizer::Tokenizer(char const *input, size_t input_len)
{
  this->input.p   = input;
  this->input.end = &input[input_len];

  this->comment = infra_string_create();
}


Tokenizer::~Tokenizer()
{
  infra_string_clearref(&this->comment);
}


[[nodiscard]]
char32_t
Tokenizer::getchar(void)
{
  size_t left = this->input.end - this->input.p;
  size_t read;
  char32_t ch = 0xFFFD;


  if (left > 0 && *this->input.p == '\0')
    /* libgrapheme was not designed for this, let us do it */
    return *this->input.p++;


  if (left >= 2
   && this->input.p[0] == '\r'
   && this->input.p[1] == '\n') {
    this->input.p += 2;
    return '\n';
  }

  if (left >= 1
   && this->input.p[0] == '\r') {
    this->input.p += 1;
    return '\n';
  }


  if (!left || !(read = grapheme_decode_utf8(this->input.p,
                         left, reinterpret_cast<uint_least32_t *>(&ch))))
    return -1;


  this->input.p += read;
  return ch;
}


void
Tokenizer::error(char const *errstr)
{
  fputs(errstr, stderr);
  fputc('\n', stderr);
  fflush(stderr);
}


void
Tokenizer::create_comment(void)
{
  infra_string_zero(this->comment);
}


void
Tokenizer::emit_character(char32_t ch)
{
  // ...
  (void) ch;
}


[[nodiscard]]
enum tokenizer_status
Tokenizer::emit_eof(void)
{
  // ...
  return TOKENIZER_STATUS_EOF;
}


void
Tokenizer::run(void)
{
  enum tokenizer_status status = TOKENIZER_STATUS_OK;


  while (status != TOKENIZER_STATUS_EOF) {
    char32_t ch;

    switch (this->state) {
      case MARKUP_DECL_OPEN_STATE:
      case AFTER_DOCTYPE_NAME_STATE:
      case AFTER_DOCTYPE_PUBLIC_KEYWORD_STATE:
      case NUMERIC_CHAR_REF_END_STATE:
        ch = 0xFFFD;
        break;

      default:
        ch = this->getchar();
        break;
    }

  }

}

