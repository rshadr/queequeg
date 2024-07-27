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

#include "qglib/unicode.hh"

#include "html_parser/common.hh"


#define LOGF(...) \
  fprintf(stderr, __VA_ARGS__)


const std::unordered_map< std::string, uint16_t> Tokenizer::k_quirky_local_names_ = {
  { "image", HTML_ELEMENT_IMG   },
  { "math",  HTML_ELEMENT_MATH_ },
  { "svg",   HTML_ELEMENT_SVG_  },
};


Tokenizer::Tokenizer(char const *input, size_t input_len)
{
  this->state = DATA_STATE;

  this->input.p   = input;
  this->input.end = &input[input_len];
}


Tokenizer::~Tokenizer()
{
}


[[nodiscard]]
char32_t
Tokenizer::getchar(void)
{
  size_t left = this->input.end - this->input.p;
  size_t read;
  char32_t ch = {0xFFFD};


  if (left > 0 && *this->input.p == '\0')
    /* libgrapheme was not designed for this, let us do it */
    return *this->input.p++;


  if (left >= 2
   && this->input.p[0] == '\r'
   && this->input.p[1] == '\n') {
    this->input.p += 2;
    return U'\n';
  }

  if (left >= 1
   && this->input.p[0] == '\r') {
    this->input.p += 1;
    return U'\n';
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
  return (this->last_start_tag_name_.compare(this->tag.tag_name) == 0);
}


void
Tokenizer::flush_char_ref_codepoints(void)
{

  for (char32_t ch : this->temp_buffer) {
    if (this->is_char_ref_in_attr())
      QueequegLib::append_c32_as_utf8(this->attr_value, ch);
    else
      this->emit_character(ch);
  }

}


void
Tokenizer::create_doctype(void)
{
  struct doctype_token *doctype = &this->doctype;

  doctype->name.clear();
  doctype->public_id.clear();
  doctype->system_id.clear();

  doctype->public_id_missing = false;
  doctype->system_id_missing = false;
  doctype->force_quirks_flag = false;

}


void
Tokenizer::create_tag_(enum token_type tag_type)
{
  struct tag_token *tag = &this->tag;

  tag->tag_name.clear();
  tag->local_name = 0;
  tag->attributes.clear();
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
Tokenizer::start_new_attr(void)
{
  this->attr_name.clear();
}


void
Tokenizer::create_comment(std::string data)
{
  this->comment = data;
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
Tokenizer::attr_name_check_hook(void)
{
  if (this->tag.attributes.contains(this->attr_name)) {
    this->error("duplicate-attribute");
    return;
  }

  /*
   * SIDE EFFECT: creates a new map entry
   */
  this->attr_value = &this->tag.attributes[this->attr_name];
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
  } else if (Tokenizer::k_quirky_local_names_.contains(tag->tag_name)) {
    /*
     * When a tag token falling under this condition gets inserted, its temporary
     * element index is ignored.
     */
    tag->local_name = Tokenizer::k_quirky_local_names_.at(tag->tag_name);
  }

  LOGF("emitting %s tag with tag_name '%s', local_name %d\n",
    (this->tag_type == TOKEN_START_TAG) ? "start" : "end",
    tag->tag_name.c_str(), tag->local_name);

  for (auto const& [k, v] : tag->attributes)
    LOGF("  %s = %s\n", k.c_str(), v.c_str());

  if (this->tag_type == TOKEN_START_TAG)
    this->last_start_tag_name_ = tag->tag_name;

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
        ch = {0xFFFD};
        break;

      default:
        ch = this->getchar();
        break;
    }

    do { status = Tokenizer::k_state_handlers_[this->state](this, ch); }
      while (status == TOKENIZER_STATUS_RECONSUME);
  }

}

