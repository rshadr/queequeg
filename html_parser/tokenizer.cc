/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>

#include <grapheme.h>
#include <infra/ascii.h>
#include <infra/string.h>

#include "html_parser/common.hh"


const std::unordered_map< std::string, uint16_t> Tokenizer::k_foreign_toplevel_elements_map_ = {
  { "math", HTML_ELEMENT_MATH },
  { "svg",  HTML_ELEMENT_SVG  },
};


Tokenizer::Tokenizer(char const *input, size_t input_len)
{
  this->state = DATA_STATE;

  this->input.p   = input;
  this->input.end = &input[input_len];
}


Tokenizer::~Tokenizer()
{
  this->destroy_tag_();
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
    return static_cast<char32_t>(-1);


  this->input.p += read;
  return ch;
}


[[nodiscard]]
bool
Tokenizer::match_fn_(int (*cmp) (char const *, char const *, size_t),
                     char const *s,
                     size_t slen)
{
  size_t left = this->input.end - this->input.p;


  if (left < slen)
    return false;


  if (!cmp(this->input.p, s, slen)) {
    this->input.p += slen;
    return true;
  }


  return false;

}


[[nodiscard]]
bool
Tokenizer::match(char const *s, size_t slen)
{
  return this->match_fn_(strncmp, s, slen);
}


[[nodiscard]]
bool
Tokenizer::match_insensitive(char const *s, size_t slen)
{
  return this->match_fn_(infra_ascii_strincmp, s, slen);
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
Tokenizer::flush_char_ref_codepoints(void)
{

  for (char32_t ch : this->temp_buffer) {
    if (this->is_char_ref_in_attr())
      (void)0;
    else
      this->emit_character(ch);
  }

}


void
Tokenizer::destroy_doctype_(void)
{
  struct doctype_token *doctype = &this->doctype;

  doctype->name.clear();
  doctype->public_id.clear();
  doctype->system_id.clear();

}


void
Tokenizer::create_doctype(void)
{
  this->destroy_doctype_();
  struct doctype_token *doctype = &this->doctype;

  doctype->public_id_missing = false;
  doctype->system_id_missing = false;
  doctype->force_quirks_flag = false;

}


void
Tokenizer::destroy_tag_(void)
{
  struct tag_token *tag = &this->tag;

  tag->tag_name.clear();

}


void
Tokenizer::create_tag_(enum token_type tag_type)
{
  this->destroy_tag_();

  struct tag_token *tag = &this->tag;

  tag->local_name = 0;
  tag->self_closing_flag = false;
  tag->ack_self_closing_flag_ = false;

  this->tag_type = tag_type;
}


void
Tokenizer::create_start_tag(void)
{
  this->create_tag_(TOKEN_START_TAG);
}


void
Tokenizer::create_end_tag(void)
{
  this->create_tag_(TOKEN_END_TAG);
}


void
Tokenizer::create_comment(void)
{
  this->comment.clear();
}


void
Tokenizer::emit_token_(union token_data *token_data,
                       enum token_type token_type)
{
  this->treebuilder->process_token(token_data, token_type);
}


static enum token_type
character_token_type(char32_t ch)
{
  switch (ch) {
    case '\t': case '\n': case '\f': case ' ':
      return TOKEN_WHITESPACE;
    default:
      return TOKEN_CHARACTER;
  }
}


void
Tokenizer::emit_character(char32_t ch)
{
#if 0
  char buf[16] = { 0 };
  grapheme_encode_utf8(ch, buf, sizeof (buf));

  printf("emitting character '%s'\n", buf);
#endif

  this->emit_token_(reinterpret_cast<union token_data *>(&ch),
                    character_token_type(ch));
}


void
Tokenizer::emit_current_doctype(void)
{
  struct doctype_token *doctype = &this->doctype;

  this->emit_token_(reinterpret_cast<union token_data *>(doctype),
                    TOKEN_DOCTYPE);
}


void
Tokenizer::emit_current_tag(void)
{
  struct tag_token *tag = &this->tag;


  /*
   * This step is only meant to speed up the parser when (re)processing tokens
   * multiple times; it is cheaper to hash once than string-compare often.
   */
  if (HTML::k_local_names_table.contains(tag->tag_name)) {
    tag->local_name = HTML::k_local_names_table.at(tag->tag_name);
  } else if (Tokenizer::k_foreign_toplevel_elements_map_.contains(tag->tag_name)) {
    /*
     * When a tag token falling under this condition gets inserted, its temporary
     * element index is ignored.
     */
    tag->local_name = Tokenizer::k_foreign_toplevel_elements_map_.at(tag->tag_name);
  }


  this->emit_token_(reinterpret_cast<union token_data *>(tag),
                    this->tag_type);
}


void
Tokenizer::emit_current_comment(void)
{
  std::string *comment = &this->comment;

  this->emit_token_(reinterpret_cast<union token_data *>(comment),
                    TOKEN_COMMENT);
}


[[nodiscard]]
enum tokenizer_status
Tokenizer::emit_eof(void)
{
  this->emit_token_(static_cast<union token_data *>(nullptr),
                    TOKEN_EOF);

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

    do { status = Tokenizer::k_state_handlers_[this->state](this, ch); }
      while (status == TOKENIZER_STATUS_RECONSUME);
  }

}

