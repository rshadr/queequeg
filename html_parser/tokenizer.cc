/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

#include <stddef.h>
#include <stdio.h>

#include <grapheme.h>
#include <infra/string.h>

#include "html_parser/common.hh"



Tokenizer::Tokenizer(char const *input, size_t input_len)
{
  this->state = DATA_STATE;

  this->input.p   = input;
  this->input.end = &input[input_len];

  this->tmpbuf = infra_string_create();

  this->comment = infra_string_create();
}


Tokenizer::~Tokenizer()
{
  infra_string_clearref(&this->tmpbuf);

  infra_string_clearref(&this->comment);
}


[[nodiscard]]
char32_t
Tokenizer::getchar(void)
{
  size_t left = this->input.end - this->input.p;
  size_t read;
  char32_t ch = 0xFFFD;


  printf("we're getting there\n");

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


[[nodiscard]]
bool
Tokenizer::match(char const *s, size_t slen)
{
  (void) s;
  (void) slen;
  // ...
  return true;
}


[[nodiscard]]
bool
Tokenizer::match_insensitive(char const *s, size_t slen)
{
  (void) s;
  (void) slen;
  // ...
  return true;
}


void
Tokenizer::error(char const *errstr)
{
  fputs(errstr, stderr);
  fputc('\n', stderr);
  fflush(stderr);
}


bool
Tokenizer::have_appropriate_end_tag(void) const
{
  // ...
  return true;
}


void
Tokenizer::create_doctype(void)
{
  // ...
}


void
Tokenizer::create_start_tag(void)
{
  // ...
}


void
Tokenizer::create_end_tag(void)
{
  // ...
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


void
Tokenizer::emit_current_doctype(void)
{
  // ...
}


void
Tokenizer::emit_current_tag(void)
{
  // ...
}


void
Tokenizer::emit_current_comment(void)
{
  // ...
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

  printf("hullo");


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

    do { status = Tokenizer::k_state_handlers_.at(this->state)(this, ch); }
      while (status == TOKENIZER_STATUS_RECONSUME);
  }

}

