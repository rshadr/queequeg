/*
 * Copyright (c) 2024 Adrien Ricciardi
 *
 * This file is part of the queequeg distribution available at:
 *  https://github.com/rshadr/queequeg
 *
 * See LICENSE for details
 *
 *
 * File: html_parser/tokenizer_states.cc
 *
 *
 *  Description:
 *
 * This file implements handlers for the states of the HTML tokenizer. They are
 * grouped together in a jump table (Tokenizer::k_state_handlers_) and accessed
 * by the tokenizer main loop from there correspondingly.
 *
 * When a state emits tokens, they are immediately processed by the connected
 * TreeBuilder instance.
 *
 *
 *  TODO:
 * - Attribute support
 * - Handle named character references properly
 * - Remove all uses of 'InfraString' and replace with 'std::string'
 * - Scripting support
 */
#include <unordered_map>

#include <stdio.h>

#define INFRA_SHORT_NAMES
#include <infra/util.h>
#include <infra/ascii.h>
#include <infra/unicode.h>

#include "qglib/unicode.hh"

#include "html_parser/common.hh"



static enum tokenizer_status
data_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '&':
      tokenizer->ret_state = DATA_STATE;
      tokenizer->state = CHAR_REF_STATE;
      return TOKENIZER_STATUS_OK;

    case '<':
      tokenizer->state = TAG_OPEN_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      return tokenizer->emit_eof();

    default:
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
rcdata_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '&':
      tokenizer->ret_state = RCDATA_STATE;
      tokenizer->state = CHAR_REF_STATE;
      return TOKENIZER_STATUS_OK;

    case '<':
      tokenizer->state = RCDATA_LT_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      return tokenizer->emit_eof();

    default:
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
rawtext_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '<':
      tokenizer->state = RAWTEXT_LT_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      return tokenizer->emit_eof();

    default:
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
script_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '<':
      tokenizer->state = SCRIPT_LT_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      return tokenizer->emit_eof();

    default:
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
plaintext_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
     return tokenizer->emit_eof();

    default:
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
tag_open_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_alpha(c)) {
    tokenizer->create_start_tag();
    tokenizer->state = TAG_NAME_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }

  switch (c) {
    case '!':
      tokenizer->state = MARKUP_DECL_OPEN_STATE;
      return TOKENIZER_STATUS_OK;

    case '/':
      tokenizer->state = END_TAG_OPEN_STATE;
      return TOKENIZER_STATUS_OK;

    case '?':
      tokenizer->error("unexpected-question-mark-instead-of-tag-name");
      tokenizer->create_comment();
      tokenizer->state = BOGUS_COMMENT_STATE;
      return TOKENIZER_STATUS_RECONSUME;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-before-tag-name");
      tokenizer->emit_character('<');
      return tokenizer->emit_eof();

    default:
      tokenizer->error("invalid-first-character-of-tag-name");
      tokenizer->emit_character('<');
      tokenizer->state = DATA_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
end_tag_open_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_alpha(c)) {
    tokenizer->create_end_tag();
    tokenizer->state = TAG_NAME_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }

  switch (c) {
    case '>':
      tokenizer->error("missing-end-tag-name");
      tokenizer->state = DATA_STATE;
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-before-tag-name");
      tokenizer->emit_character('<');
      tokenizer->emit_character('/');
      return tokenizer->emit_eof();

    default:
      tokenizer->error("invalid-first-character-of-tag-name");
      tokenizer->create_comment();
      tokenizer->state = BOGUS_COMMENT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
tag_name_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_upper_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c|0x20);
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      tokenizer->state = BEFORE_ATTR_NAME_STATE;
      return TOKENIZER_STATUS_OK;

    case '/':
      tokenizer->state = SELF_CLOSING_START_TAG_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_tag();
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, 0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-tag");
      return tokenizer->emit_eof();

    default:
      QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
rcdata_lt_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '/':
      tokenizer->temp_buffer.clear();
      tokenizer->state = RCDATA_END_TAG_OPEN_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->emit_character('<');
      tokenizer->state = RCDATA_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
rcdata_end_tag_open_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_alpha(c)) {
    tokenizer->create_end_tag();
    tokenizer->state = RCDATA_END_TAG_NAME_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }

  {
    tokenizer->emit_character('<');
    tokenizer->emit_character('/');
    tokenizer->state = RCDATA_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
rcdata_end_tag_name_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_upper_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c|0x20);
    tokenizer->temp_buffer.push_back(c);
    return TOKENIZER_STATUS_OK;
  }

  if (ascii_is_lower_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c);
    tokenizer->temp_buffer.push_back(c);
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = BEFORE_ATTR_NAME_STATE;
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    case '/':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = SELF_CLOSING_START_TAG_STATE;
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    case '>':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = DATA_STATE;
        tokenizer->emit_current_tag();
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    default:
anything_else:
      tokenizer->emit_character('<');
      tokenizer->emit_character('/');
      /* ... */
      tokenizer->state = RCDATA_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
rawtext_lt_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '/':
      tokenizer->temp_buffer.clear();
      tokenizer->state = RAWTEXT_END_TAG_OPEN_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->emit_character('<');
      tokenizer->state = RAWTEXT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
rawtext_end_tag_open_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_alpha(c)) {
    tokenizer->create_end_tag();
    tokenizer->state = RAWTEXT_END_TAG_NAME_STATE;
    return TOKENIZER_STATUS_OK;
  }

  {
    tokenizer->emit_character('<');
    tokenizer->emit_character('/');
    return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
rawtext_end_tag_name_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_upper_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c|0x20);
    tokenizer->temp_buffer.push_back(c);
    return TOKENIZER_STATUS_OK;
  }

  if (ascii_is_lower_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c);
    tokenizer->temp_buffer.push_back(c);
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = BEFORE_ATTR_NAME_STATE;
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    case '/':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = SELF_CLOSING_START_TAG_STATE;
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    case '>':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = DATA_STATE;
        tokenizer->emit_current_tag();
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

anything_else:
    default:
      tokenizer->emit_character('<');
      tokenizer->emit_character('/');
      /* ... */
      tokenizer->state = RAWTEXT_STATE;
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
script_lt_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '/':
      tokenizer->temp_buffer.clear();
      tokenizer->state = SCRIPT_END_TAG_OPEN_STATE;
      return TOKENIZER_STATUS_OK;

    case '!':
      tokenizer->state = SCRIPT_ESCAPE_START_STATE;
      tokenizer->emit_character('<');
      tokenizer->emit_character('!');
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->emit_character('<');
      tokenizer->state = SCRIPT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
script_end_tag_open_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_alpha(c)) {
    tokenizer->create_end_tag();
    tokenizer->state = SCRIPT_END_TAG_NAME_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }

  {
    tokenizer->emit_character('<');
    tokenizer->emit_character('/');
    tokenizer->state = SCRIPT_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
script_end_tag_name_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_upper_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c|0x20);
    tokenizer->temp_buffer.push_back(c);
    return TOKENIZER_STATUS_OK;
  }

  if (ascii_is_lower_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c);
    tokenizer->temp_buffer.push_back(c);
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = BEFORE_ATTR_NAME_STATE;
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    case '/':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = SELF_CLOSING_START_TAG_STATE;
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    case '>':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = DATA_STATE;
        tokenizer->emit_current_tag();
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    anything_else:
    default:
      tokenizer->emit_character('<');
      tokenizer->emit_character('/');
      /* ... */
      tokenizer->state = SCRIPT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
script_escape_start_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = SCRIPT_ESCAPE_START_DASH_STATE;
      tokenizer->emit_character('-');
      return TOKENIZER_STATUS_OK;

    default:  
      tokenizer->state = SCRIPT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
script_escape_start_dash_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = SCRIPT_ESCAPED_DASH_DASH_STATE;
      tokenizer->emit_character('-');
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->state = SCRIPT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
script_escaped_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = SCRIPT_ESCAPED_DASH_STATE;
      tokenizer->emit_character('-');
      return TOKENIZER_STATUS_OK;

    case '<':
      tokenizer->state = SCRIPT_ESCAPED_LT_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-script-html-comment-like-text");
      return tokenizer->emit_eof();

    default:
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
script_escaped_dash_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = SCRIPT_ESCAPED_DASH_DASH_STATE;
      tokenizer->emit_character('-');
      return TOKENIZER_STATUS_OK;

    case '<':
      tokenizer->state = SCRIPT_ESCAPED_LT_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->state = SCRIPT_ESCAPED_STATE;
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-script-html-comment-like-text");
      return tokenizer->emit_eof();

    default:
      tokenizer->state = SCRIPT_ESCAPED_STATE;
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
script_escaped_dash_dash_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->emit_character('-');
      return TOKENIZER_STATUS_OK;

    case '<':
      tokenizer->state = SCRIPT_ESCAPED_LT_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = SCRIPT_STATE;
      tokenizer->emit_character('>');
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->state = SCRIPT_ESCAPED_STATE;
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-script-html-comment-like-text");
      return tokenizer->emit_eof();

    default:
      tokenizer->state = SCRIPT_ESCAPED_STATE;
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
script_escaped_lt_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_alpha(c)) {
    tokenizer->temp_buffer.clear();
    tokenizer->emit_character('<');
    tokenizer->state = SCRIPT_DOUBLE_ESCAPE_START_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }

  switch (c) {
    case '/':
      tokenizer->temp_buffer.clear();
      tokenizer->state = SCRIPT_ESCAPED_END_TAG_OPEN_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->emit_character('<');
      tokenizer->state = SCRIPT_ESCAPED_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
script_escaped_end_tag_open_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_alpha(c)) {
    tokenizer->create_end_tag();
    tokenizer->state = SCRIPT_ESCAPED_END_TAG_NAME_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }

  {
    tokenizer->emit_character('<');
    tokenizer->emit_character('/');
    tokenizer->state = SCRIPT_ESCAPED_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
script_escaped_end_tag_name_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_upper_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c|0x20);
    tokenizer->temp_buffer.push_back(c);
    return TOKENIZER_STATUS_OK;
  }

  if (ascii_is_lower_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c);
    tokenizer->temp_buffer.push_back(c);
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = BEFORE_ATTR_NAME_STATE;
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    case '/':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = SELF_CLOSING_START_TAG_STATE;
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    case '>':
      if (tokenizer->have_appropriate_end_tag()) {
        tokenizer->state = DATA_STATE;
        tokenizer->emit_current_tag();
        return TOKENIZER_STATUS_OK;
      } else goto anything_else;

    default:
anything_else:
      tokenizer->emit_character('<');
      tokenizer->emit_character('/');
      /* ... */
      tokenizer->state = SCRIPT_ESCAPED_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
script_double_escape_start_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_upper_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c|0x20);
    tokenizer->temp_buffer.push_back(c);
    return TOKENIZER_STATUS_OK;
  }

  if (ascii_is_lower_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->tag.tag_name, c);
    tokenizer->temp_buffer.push_back(c);
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case '\t': case '\n': case '\f': case ' ': case '/': case '>':
      if (!tokenizer->temp_buffer.compare(U"script"))
        tokenizer->state = SCRIPT_DOUBLE_ESCAPED_STATE;
      else
        tokenizer->state = SCRIPT_ESCAPED_STATE;
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->state = SCRIPT_ESCAPED_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
script_double_escaped_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_DASH_STATE;
      tokenizer->emit_character('-');
      return TOKENIZER_STATUS_OK;

    case '<':
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_LT_STATE;
      tokenizer->emit_character('<');
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-script-html-comment-like-text");
      return tokenizer->emit_eof();

    default:
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
script_double_escaped_dash_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_DASH_DASH_STATE;
      tokenizer->emit_character('-');
      return TOKENIZER_STATUS_OK;

    case '<':
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_LT_STATE;
      tokenizer->emit_character('<');
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_STATE;
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-script-html-comment-like-text");
      return tokenizer->emit_eof();

    default:
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_STATE;
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
script_double_escaped_dash_dash_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->emit_character('-');
      return TOKENIZER_STATUS_OK;

    case '<':
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_LT_STATE;
      tokenizer->emit_character('<');
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = SCRIPT_STATE;
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_STATE;
      tokenizer->emit_character(0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-script-html-comment-like-text");
      return tokenizer->emit_eof();

    default:
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_STATE;
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
script_double_escaped_lt_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '/':
      tokenizer->temp_buffer.clear();
      tokenizer->state = SCRIPT_DOUBLE_ESCAPE_END_STATE;
      tokenizer->emit_character('/');
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
script_double_escape_end_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_upper_alpha(c)) {
    tokenizer->temp_buffer.push_back(c|0x20);
    tokenizer->emit_character(c);
    return TOKENIZER_STATUS_OK;
  }

  if (ascii_is_lower_alpha(c)) {
    tokenizer->temp_buffer.push_back(c);
    tokenizer->emit_character(c);
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case '\t': case '\n': case '\f': case ' ': case '/': case '>':
      if (!tokenizer->temp_buffer.compare(U"script"))
        tokenizer->state = SCRIPT_ESCAPED_STATE;
      else
        tokenizer->state = SCRIPT_DOUBLE_ESCAPED_STATE;
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->state = SCRIPT_DOUBLE_ESCAPED_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
before_attr_name_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      return TOKENIZER_STATUS_OK;

    case '/': case '>': case static_cast<char32_t>(-1):
      tokenizer->state = AFTER_ATTR_NAME_STATE;
      return TOKENIZER_STATUS_RECONSUME;

    case '=':
      tokenizer->error("unexpected-equals-sign-before-attribute-name");
      /* ... */
      tokenizer->state = ATTR_NAME_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      /* ... */
      tokenizer->state = ATTR_NAME_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
attr_name_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_upper_alpha(c)) {
    /* ... */
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case '\t': case '\n': case '\f': case ' ': case '/': case '>':
    case static_cast<char32_t>(-1):
      tokenizer->state = AFTER_ATTR_NAME_STATE;
      return TOKENIZER_STATUS_RECONSUME;

    case '=':
      tokenizer->state = BEFORE_ATTR_VALUE_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      /* ... */
      return TOKENIZER_STATUS_OK;

    case '\"': case '\'': case '<':
      tokenizer->error("unexpected-character-in-attribute-name");
      /* ... */
      return TOKENIZER_STATUS_OK;

    default:
      /* ... */
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
after_attr_name_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      return TOKENIZER_STATUS_OK;

    case '/':
      tokenizer->state = SELF_CLOSING_START_TAG_STATE;
      return TOKENIZER_STATUS_OK;

    case '=':
      tokenizer->state = BEFORE_ATTR_VALUE_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_tag();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-tag");
      return tokenizer->emit_eof();

    default:
      /* XXX */
      tokenizer->state = ATTR_NAME_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
before_attr_value_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      return TOKENIZER_STATUS_OK;

    case '\"':
      tokenizer->state = ATTR_VALUE_DOUBLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '\'':
      tokenizer->state = ATTR_VALUE_SINGLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("missing-attribute-value");
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_tag();
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->state = ATTR_VALUE_UNQUOTED_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
attr_value_double_quoted_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\"':
      tokenizer->state = AFTER_ATTR_VALUE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '&':
      tokenizer->ret_state = ATTR_VALUE_DOUBLE_QUOTED_STATE;
      tokenizer->state     = CHAR_REF_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      // infra_string_put_unicode(
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-tag");
      return tokenizer->emit_eof();

    default:
      /* XXX */
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
attr_value_single_quoted_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\'':
      tokenizer->state = AFTER_ATTR_VALUE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '&':
      tokenizer->ret_state = ATTR_VALUE_SINGLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      // XXX
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-tag");
      return tokenizer->emit_eof();

    default:
      // XXX
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
attr_value_unquoted_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      tokenizer->state = BEFORE_ATTR_NAME_STATE;
      return TOKENIZER_STATUS_OK;

    case '&':
      tokenizer->ret_state = ATTR_VALUE_UNQUOTED_STATE;
      tokenizer->state     = CHAR_REF_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_tag();
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      // XXX
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-tag");
      return tokenizer->emit_eof();

    case '\"': case '\'': case '<': case '=': case '`':
      tokenizer->error("unexpected-character-in-unquoted-attribute-value");
      // infra_string_put_unicode(..., c);
      return TOKENIZER_STATUS_OK;

    default:
      // infra_string_put_unicode(..., c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
after_attr_value_quoted_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      tokenizer->state = BEFORE_ATTR_NAME_STATE;
      return TOKENIZER_STATUS_OK;

    case '/':
      tokenizer->state = SELF_CLOSING_START_TAG_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_tag();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-tag");
      return tokenizer->emit_eof();

    default:
      tokenizer->error("missing-whitespace-between-attributes");
      tokenizer->state = BEFORE_ATTR_NAME_STATE;
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
self_closing_start_tag_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '>':
      tokenizer->tag.self_closing_flag = true;
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_tag();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-tag");
      return tokenizer->emit_eof();

    default:
      tokenizer->error("unexpected-solidus-in-tag");
      tokenizer->state = BEFORE_ATTR_NAME_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
bogus_comment_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_comment();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->emit_current_comment();
      return tokenizer->emit_eof();

    case '\0':
      tokenizer->error("unexpected-null-character");
      QueequegLib::append_c32_as_utf8(&tokenizer->comment, 0xFFFD);
      return TOKENIZER_STATUS_OK;

    default:
      QueequegLib::append_c32_as_utf8(&tokenizer->comment, c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
markup_decl_open_state(Tokenizer *tokenizer, char32_t c)
{
  (void) c;

  if (tokenizer->match("--")) {
    tokenizer->create_comment();
    tokenizer->state = COMMENT_START_STATE;
    return TOKENIZER_STATUS_OK;
  }

  if (tokenizer->match_insensitive("DOCTYPE")) {
    tokenizer->state = DOCTYPE_STATE;
    return TOKENIZER_STATUS_OK;
  }

  if (tokenizer->match("[CDATA[")) {
    const std::shared_ptr< DOM_Element> node = tokenizer->treebuilder->adjusted_current_node();

    if (node != nullptr && node->name_space != INFRA_NAMESPACE_HTML) {
      tokenizer->state = CDATA_SECTION_STATE;
      return TOKENIZER_STATUS_OK;
    }

    tokenizer->error("cdata-in-html-content");
    /* ... */
    tokenizer->state = BOGUS_COMMENT_STATE;
    return TOKENIZER_STATUS_OK;
  }

  {
    tokenizer->error("incorrectly-opened-comment");
    tokenizer->create_comment();
    tokenizer->state = BOGUS_COMMENT_STATE;
    return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
comment_start_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = COMMENT_START_DASH_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("abrupt-closing-of-empty-comment");
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_comment();
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->state = COMMENT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
comment_start_dash_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = COMMENT_END_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("abrupt-closing-of-empty-comment");
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_comment();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-comment");
      tokenizer->emit_current_comment();
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->comment.append("-");
      tokenizer->state = COMMENT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
comment_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '<':
      QueequegLib::append_c32_as_utf8(&tokenizer->comment, c);
      tokenizer->state = COMMENT_LT_STATE;
      return TOKENIZER_STATUS_OK;

    case '-':
      tokenizer->state = COMMENT_END_DASH_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      QueequegLib::append_c32_as_utf8(&tokenizer->comment, 0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-comment");
      return tokenizer->emit_eof();

    default:
      QueequegLib::append_c32_as_utf8(&tokenizer->comment, c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
comment_lt_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '!':
      QueequegLib::append_c32_as_utf8(&tokenizer->comment, c);
      tokenizer->state = COMMENT_LT_BANG_STATE;
      return TOKENIZER_STATUS_OK;

    case '<':
      QueequegLib::append_c32_as_utf8(&tokenizer->comment, c);
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->state = COMMENT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
comment_lt_bang_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = COMMENT_LT_BANG_DASH_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->state = COMMENT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
comment_lt_bang_dash_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = COMMENT_LT_BANG_DASH_DASH_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->state = COMMENT_END_DASH_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
comment_lt_bang_dash_dash_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '>': case static_cast<char32_t>(-1):
      tokenizer->state = COMMENT_END_STATE;
      return TOKENIZER_STATUS_RECONSUME;

    default:
      tokenizer->error("nested-comment");
      tokenizer->state = COMMENT_END_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
comment_end_dash_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->state = COMMENT_END_STATE;
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-comment");
      tokenizer->emit_current_comment();
      return tokenizer->emit_eof();

    default:
      tokenizer->comment.append("-");
      tokenizer->state = COMMENT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
comment_end_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_comment();
      return TOKENIZER_STATUS_OK;

    case '!':
      tokenizer->state = COMMENT_END_BANG_STATE;
      return TOKENIZER_STATUS_OK;

    case '-':
      tokenizer->comment.append("-");
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-comment");
      tokenizer->emit_current_comment();
      return tokenizer->emit_eof();

    default:
      tokenizer->comment.append("--");
      tokenizer->state = COMMENT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
comment_end_bang_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '-':
      tokenizer->comment.append("--!");
      tokenizer->state = COMMENT_END_DASH_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("incorrectly-closed-comment");
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_comment();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-comment");
      tokenizer->emit_current_comment();
      return tokenizer->emit_eof();

    default:
      tokenizer->comment.append("--!");
      tokenizer->state = COMMENT_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
doctype_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      tokenizer->state = BEFORE_DOCTYPE_NAME_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = BEFORE_DOCTYPE_NAME_STATE;
      return TOKENIZER_STATUS_RECONSUME;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      return tokenizer->emit_eof();

    default:
      tokenizer->error("missing-whitespace-before-doctype-name");
      tokenizer->state = BEFORE_DOCTYPE_NAME_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
before_doctype_name_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_upper_alpha(c)) {
    tokenizer->create_doctype();
    QueequegLib::append_c32_as_utf8(&tokenizer->doctype.name, c|0x20);
    tokenizer->state = DOCTYPE_NAME_STATE;
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      tokenizer->create_doctype();
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.name, 0xFFFD);
      tokenizer->state = DOCTYPE_NAME_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("missing-doctype-name-state");
      tokenizer->create_doctype();
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->create_doctype();
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      tokenizer->create_doctype();
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.name, c);
      tokenizer->state = DOCTYPE_NAME_STATE;
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
doctype_name_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_upper_alpha(c)) {
    QueequegLib::append_c32_as_utf8(&tokenizer->doctype.name, c|0x20);
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      tokenizer->state = AFTER_DOCTYPE_NAME_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.name, 0xFFFD);
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.name, c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
after_doctype_name_state(Tokenizer *tokenizer, char32_t c)
{
  (void) c;

  if (tokenizer->match_insensitive("PUBLIC")) {
    tokenizer->state = AFTER_DOCTYPE_PUBLIC_KEYWORD_STATE;
    return TOKENIZER_STATUS_OK;
  }

  if (tokenizer->match_insensitive("SYSTEM")) {
    tokenizer->state = AFTER_DOCTYPE_SYSTEM_KEYWORD_STATE;
    return TOKENIZER_STATUS_OK;
  }

  switch (tokenizer->getchar()) {
    case '\t': case '\n': case '\f': case ' ':
      return TOKENIZER_STATUS_OK; 

    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK; 

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      tokenizer->error("invalid-character-sequence-after-doctype-name");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = BOGUS_DOCTYPE_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
after_doctype_public_keyword_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      tokenizer->state = BEFORE_DOCTYPE_PUBLIC_ID_STATE;
      return TOKENIZER_STATUS_OK;

    case '\"':
      tokenizer->error("missing-whitespace-after-doctype-public-keyword");
      tokenizer->doctype.public_id_missing = false;
      tokenizer->state = DOCTYPE_PUBLIC_ID_DOUBLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '\'':
      tokenizer->error("missing-whitespace-after-doctype-public-keyword");
      tokenizer->doctype.public_id_missing = false;
      tokenizer->state = DOCTYPE_PUBLIC_ID_SINGLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("missing-doctype-public-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      tokenizer->error("missing-quote-before-doctype-public-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = BOGUS_DOCTYPE_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
before_doctype_public_id_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      return TOKENIZER_STATUS_OK;

    case '\"':
      tokenizer->doctype.public_id_missing = false;
      tokenizer->state = DOCTYPE_PUBLIC_ID_DOUBLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '\'':
      tokenizer->doctype.public_id_missing = false;
      tokenizer->state = DOCTYPE_PUBLIC_ID_SINGLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("missing-doctype-public-identifier");
      tokenizer->doctype.force_quirks_flag = false;
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = false;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      tokenizer->error("missing-quote-before-doctype-public-identifier");
      tokenizer->doctype.force_quirks_flag = false;
      tokenizer->state = BOGUS_DOCTYPE_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
doctype_public_id_double_quoted_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\"':
      tokenizer->state = AFTER_DOCTYPE_PUBLIC_ID_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.public_id, 0xFFFD);
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("abrupt-doctype-public-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.public_id, c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
doctype_public_id_single_quoted_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\'':
      tokenizer->state = AFTER_DOCTYPE_PUBLIC_ID_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.public_id, 0xFFFD);
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("abrupt-doctype-public-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = DATA_STATE;
      return tokenizer->emit_eof();

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.public_id, c);
      return TOKENIZER_STATUS_OK;
  }
}

static enum tokenizer_status
after_doctype_public_id_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      tokenizer->state = BETWEEN_DOCTYPE_PUBLIC_SYSTEM_IDS_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case '\"':
      tokenizer->error(
       "missing-whitespace-between-doctype-public-and-system-identifiers");
      tokenizer->doctype.system_id_missing = false;
      tokenizer->state = DOCTYPE_SYSTEM_ID_DOUBLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '\'':
      tokenizer->error(
       "missing-whitespace-between-doctype-public-and-system-identifiers");
      tokenizer->doctype.system_id_missing = false;
      tokenizer->state = DOCTYPE_SYSTEM_ID_SINGLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();
    default:
      tokenizer->error("missing-quote-before-doctype-system-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = BOGUS_DOCTYPE_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
between_doctype_public_system_ids_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case '\"':
      tokenizer->doctype.system_id_missing = false;
      tokenizer->state = DOCTYPE_SYSTEM_ID_DOUBLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '\'':
      tokenizer->doctype.system_id_missing = false;
      tokenizer->state = DOCTYPE_SYSTEM_ID_SINGLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      tokenizer->error("missing-quote-before-doctype-system-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = BOGUS_DOCTYPE_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
after_doctype_system_keyword_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      tokenizer->state = BEFORE_DOCTYPE_SYSTEM_ID_STATE;
      return TOKENIZER_STATUS_OK;

    case '\"':
      tokenizer->error("missing-whitespace-after-doctype-system-keyword");
      tokenizer->doctype.system_id_missing = false;
      tokenizer->state = DOCTYPE_SYSTEM_ID_DOUBLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '\'':
      tokenizer->error("missing-whitespace-after-doctype-system-keyword");
      tokenizer->doctype.system_id_missing = false;
      tokenizer->state = DOCTYPE_SYSTEM_ID_SINGLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("missing-doctype-system-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      tokenizer->error("missing-quote-before-doctype-system-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = BOGUS_DOCTYPE_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
before_doctype_system_id_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      return TOKENIZER_STATUS_OK;

    case '\"':
      tokenizer->doctype.system_id_missing = false;
      tokenizer->state = DOCTYPE_SYSTEM_ID_DOUBLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '\'':
      tokenizer->doctype.system_id_missing = false;
      tokenizer->state = DOCTYPE_SYSTEM_ID_SINGLE_QUOTED_STATE;
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("missing-doctype-system-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      tokenizer->error("missing-quote-before-doctype-system-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = BOGUS_DOCTYPE_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
doctype_system_id_double_quoted_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\"':
      tokenizer->state = AFTER_DOCTYPE_SYSTEM_ID_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.system_id, c);
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("abrupt-doctype-system-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.system_id, c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
doctype_system_id_single_quoted_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\'':
      tokenizer->state = AFTER_DOCTYPE_SYSTEM_ID_STATE;
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.system_id, 0xFFFD);
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->error("abrupt-doctype-system-identifier");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      QueequegLib::append_c32_as_utf8(&tokenizer->doctype.system_id, c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
after_doctype_system_id_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '\t': case '\n': case '\f': case ' ':
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-doctype");
      tokenizer->doctype.force_quirks_flag = true;
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      tokenizer->error("unexpected-character-after-doctype-system-identifier");
      tokenizer->state = BOGUS_DOCTYPE_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
bogus_doctype_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case '>':
      tokenizer->state = DATA_STATE;
      tokenizer->emit_current_doctype();
      return TOKENIZER_STATUS_OK;

    case '\0':
      tokenizer->error("unexpected-null-character");
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->emit_current_doctype();
      return tokenizer->emit_eof();

    default:
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
cdata_section_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case ']':
      tokenizer->state = CDATA_SECTION_BRACKET_STATE;
      return TOKENIZER_STATUS_OK;

    case static_cast<char32_t>(-1):
      tokenizer->error("eof-in-cdata");
      return tokenizer->emit_eof();

    default:
      tokenizer->emit_character(c);
      return TOKENIZER_STATUS_OK;
  }
}


static enum tokenizer_status
cdata_section_bracket_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case ']':
      tokenizer->state = CDATA_SECTION_END_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->emit_character(']');
      tokenizer->state = CDATA_SECTION_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
cdata_section_end_state(Tokenizer *tokenizer, char32_t c)
{
  switch (c) {
    case ']':
      tokenizer->emit_character(']');
      return TOKENIZER_STATUS_OK;

    case '>':
      tokenizer->state = DATA_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->emit_character(']');
      tokenizer->emit_character(']');
      tokenizer->state = CDATA_SECTION_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
char_ref_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_alnum(c)) {
    tokenizer->state = NAMED_CHAR_REF_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }

  switch (c) {
    case '#':
      tokenizer->temp_buffer.push_back(c);
      tokenizer->state = NUMERIC_CHAR_REF_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      /* XXX flush code points */
      tokenizer->state = tokenizer->ret_state;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


#include "./named_entities.cc"

static enum tokenizer_status
named_char_ref_state(Tokenizer *tokenizer, char32_t c)
{
  (void) c;

  char const *p = tokenizer->input.p - 2;
  /* ^ skipped '&' and c; works because c is always ASCII */
  const size_t left = tokenizer->input.end - p;

  for (size_t i = 0; i < INFRA_COUNTOF(k_named_char_refs_); i++)
  {
    size_t entlen = k_named_char_refs_[i].name_len;

    if (entlen > left)
      continue;

    if (!memcmp(k_named_char_refs_[i].name, p, entlen))
    {
      /* -> If there is a match: */
      tokenizer->input.p = p + entlen;

#if 0
      /* XXX: character reference! */
      infra_string_put_chunk(tokenizer->tmpbuf,
        k_named_char_refs_[i].name,
        k_named_char_refs_[i].name_len);
#endif

      if (tokenizer->is_char_ref_in_attr()
       && (tokenizer->input.p[-1] != ';')
       && ((tokenizer->input.p[0] == '=')
        || ascii_is_alnum(tokenizer->input.p[0])))
      {
        /* first case (historical) */

        /* XXX flush */
        tokenizer->state = CHAR_REF_STATE;
        return TOKENIZER_STATUS_OK;
      }

      else
      {
        /* "regular" case */

        if (tokenizer->input.p[-1] != ';')
          tokenizer->error("missing-semicolon-after-character-reference");

        tokenizer->temp_buffer.clear();

#if 0
        /* XXX: character reference! */
        infra_string_put_chunk(tokenizer->tmpbuf,
          k_named_char_refs_[i].utf8,
          k_named_char_refs_[i].utf8_len);
#endif
        /* XXX FLUSH */

        tokenizer->state = tokenizer->ret_state;
        return TOKENIZER_STATUS_OK;
      }

    }

  }

  /* -> otherwise */
  /* XXX flush */
  tokenizer->state = AMBIGUOUS_AMPERSAND_STATE;
  return TOKENIZER_STATUS_OK;
}


static enum tokenizer_status
ambiguous_ampersand_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_alnum(c)) {
    if (tokenizer->is_char_ref_in_attr())
      (void)0; // infra_string_put_char(tokenizer->attr, c);
    else
      tokenizer->emit_character(c);
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case ';':
      tokenizer->error("unknown-named-character-reference");
      tokenizer->state = tokenizer->ret_state;
      return TOKENIZER_STATUS_RECONSUME;

    default:
      tokenizer->state = tokenizer->ret_state;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
numeric_char_ref_state(Tokenizer *tokenizer, char32_t c)
{
  tokenizer->char_ref = 0;

  switch (c) {
    case 'x': case 'X':
      tokenizer->temp_buffer.push_back(c);
      tokenizer->state = HEX_CHAR_REF_START_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->state = DEC_CHAR_REF_START_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
hex_char_ref_start_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_xdigit(c)) {
    tokenizer->state = HEX_CHAR_REF_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }

  {
    tokenizer->error("absence-of-digits-in-numeric-character-reference");
    /* XXX flush */
    tokenizer->state = tokenizer->ret_state;
    return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
dec_char_ref_start_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_digit(c)) {
    tokenizer->state = DEC_CHAR_REF_STATE;
    return TOKENIZER_STATUS_RECONSUME;
  }

  {
    tokenizer->error("absence-of-digits-in-numeric-character-reference");
    /* XXX flush */
    tokenizer->state = tokenizer->ret_state;
    return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
hex_char_ref_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_digit(c)) {
    tokenizer->char_ref <<= 4;
    tokenizer->char_ref  |= c - 0x30;
    return TOKENIZER_STATUS_OK;
  }

  if (ascii_is_upper_xdigit(c)) {
    tokenizer->char_ref <<= 4;
    tokenizer->char_ref  |= c - 0x37;
    return TOKENIZER_STATUS_OK;
  }

  if (ascii_is_lower_xdigit(c)) {
    tokenizer->char_ref <<= 4;
    tokenizer->char_ref  |= c - 0x57;
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case ';':
      tokenizer->state = NUMERIC_CHAR_REF_END_STATE;
      return TOKENIZER_STATUS_RECONSUME;

    default:
      tokenizer->error("missing-semicolon-after-character-reference");
      tokenizer->state = NUMERIC_CHAR_REF_END_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static enum tokenizer_status
dec_char_ref_state(Tokenizer *tokenizer, char32_t c)
{
  if (ascii_is_digit(c)) {
    tokenizer->char_ref *= 10;
    tokenizer->char_ref += c - 0x30;
    return TOKENIZER_STATUS_OK;
  }

  switch (c) {
    case ';':
      tokenizer->state = NUMERIC_CHAR_REF_END_STATE;
      return TOKENIZER_STATUS_OK;

    default:
      tokenizer->error("missing-semicolon-after-character-reference");
      tokenizer->state = NUMERIC_CHAR_REF_END_STATE;
      return TOKENIZER_STATUS_RECONSUME;
  }
}


static const std::unordered_map<char32_t, char32_t> k_numeric_subst_ = {
  { 0x80, 0x20AC }, /* EURO SIGN */
  { 0x82, 0x201A }, /* SINGLE LOW-9 QUOTATION MARK */
  { 0x83, 0x0192 }, /* LATIN SMALL LETTER F WITH HOOK */
  { 0x84, 0x201E }, /* DOUBLE LOW-9 QUOTATION MARK */
  { 0x85, 0x2026 }, /* HORIZONTAL ELLIPSIS */
  { 0x86, 0x2020 }, /* DAGGER */
  { 0x87, 0x2021 }, /* DOUBLE DAGGER */
  { 0x88, 0x02C6 }, /* MODIFIER LETTER CIRCUMFLEX ACCENT */
  { 0x89, 0x2030 }, /* PER MILLE SIGN */
  { 0x8A, 0x0160 }, /* LATIN CAPITAL LETTER S WITH CARON */
  { 0x8B, 0x2039 }, /* SINGLE LEFT-POINTING ANGLE QUOTATION MARK */
  { 0x8C, 0x0152 }, /* LATIN CAPITAL LIGATURE OE */
  { 0x8E, 0x017D }, /* LATIN SMALL LETTER Z WITH CARON */
  { 0x91, 0x2018 }, /* LEFT SINGLE QUOTATION MARK */
  { 0x92, 0x2019 }, /* RIGHT SINGLE QUOTATION MARK */
  { 0x93, 0x201C }, /* LEFT DOUBLE QUOTATION MARK */
  { 0x94, 0x201D }, /* RIGHT DOUBLE QUOTATION MARK */
  { 0x95, 0x2022 }, /* BULLET */
  { 0x96, 0x2013 }, /* EN DASH */
  { 0x97, 0x2014 }, /* EM DASH */
  { 0x98, 0x02DC }, /* SMALL TILDE */
  { 0x9A, 0x0161 }, /* LATIN SMALL LETTER S WITH CARON */
  { 0x9B, 0x203A }, /* SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */
  { 0x9C, 0x0153 }, /* LATIN SMALL LIGATURE OE */
  { 0x9E, 0x017E }, /* LATIN SMALL LETTER W WITH CARON */
  { 0x9F, 0x0178 }, /* LATIN CAPITAL LETTERY WITH DIARESIS */
};


static enum tokenizer_status
numeric_char_ref_end_state(Tokenizer *tokenizer, char32_t ch)
{
  (void) ch;

  if (tokenizer->char_ref > 0x10FFFF) {
    tokenizer->error("character-reference-outside-of-unicode-range");
    tokenizer->char_ref = 0xFFFD;
  }

  char32_t code = tokenizer->char_ref;

  if (code == 0x00) {
    tokenizer->error("null-character-reference");
    code = 0xFFFD;
  } else if (unicode_is_surrogate(code)) {
    /* XXX */
  } else if (unicode_is_noncharacter(code)) {
    /* XXX */
  } else if (k_numeric_subst_.contains(code)) {
    code = k_numeric_subst_.at(code);
  }

  tokenizer->temp_buffer.clear();
  tokenizer->temp_buffer.push_back(code);
  /* XXX flush */

  tokenizer->state = tokenizer->ret_state;
  return TOKENIZER_STATUS_OK;
}



const Tokenizer::state_handler_cb_t Tokenizer::k_state_handlers_[NUM_STATES] = {
  data_state,
  rcdata_state,
  rawtext_state,
  script_state,
  plaintext_state,
  tag_open_state,
  end_tag_open_state,
  tag_name_state,
  rcdata_lt_state,
  rcdata_end_tag_open_state,
  rcdata_end_tag_name_state,
  rawtext_lt_state,
  rawtext_end_tag_open_state,
  rawtext_end_tag_name_state,
  script_lt_state,
  script_end_tag_open_state,
  script_end_tag_name_state,
  script_escape_start_state,
  script_escape_start_dash_state,
  script_escaped_state,
  script_escaped_dash_state,
  script_escaped_dash_dash_state,
  script_escaped_lt_state,
  script_escaped_end_tag_open_state,
  script_escaped_end_tag_name_state,
  script_double_escape_start_state,
  script_double_escaped_state,
  script_double_escaped_dash_state,
  script_double_escaped_dash_dash_state,
  script_double_escaped_lt_state,
  script_double_escape_end_state,
  before_attr_name_state,
  attr_name_state,
  after_attr_name_state,
  before_attr_value_state,
  attr_value_double_quoted_state,
  attr_value_single_quoted_state,
  attr_value_unquoted_state,
  after_attr_value_quoted_state,
  self_closing_start_tag_state,
  bogus_comment_state,
  markup_decl_open_state,
  comment_start_state,
  comment_start_dash_state,
  comment_state,
  comment_lt_state,
  comment_lt_bang_state,
  comment_lt_bang_dash_state,
  comment_lt_bang_dash_dash_state,
  comment_end_dash_state,
  comment_end_state,
  comment_end_bang_state,
  doctype_state,
  before_doctype_name_state,
  doctype_name_state,
  after_doctype_name_state,
  after_doctype_public_keyword_state,
  before_doctype_public_id_state,
  doctype_public_id_double_quoted_state,
  doctype_public_id_single_quoted_state,
  after_doctype_public_id_state,
  between_doctype_public_system_ids_state,
  after_doctype_system_keyword_state,
  before_doctype_system_id_state,
  doctype_system_id_double_quoted_state,
  doctype_system_id_single_quoted_state,
  after_doctype_system_id_state,
  bogus_doctype_state,
  cdata_section_state,
  cdata_section_bracket_state,
  cdata_section_end_state,
  char_ref_state,
  named_char_ref_state,
  ambiguous_ampersand_state,
  numeric_char_ref_state,
  hex_char_ref_start_state,
  dec_char_ref_start_state,
  hex_char_ref_state,
  dec_char_ref_state,
  numeric_char_ref_end_state,
};

