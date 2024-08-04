/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

#include <cassert>
#include <cstdio>

#include <string>
#include <vector>
#include <unordered_map>
#include <limits>

#include <grapheme.h>

#define INFRA_SHORT_NAMES
#include <infra/ascii.h>
#include <infra/unicode.h>

#include "qglib/unicode.hh"
#include "url/url.hh"
#include "url/parser.hh"


#define LOGF(...) \
  (std::fprintf(stderr, __VA_ARGS__))


class URLBasicParser final {
  public:
    URLBasicParser(std::string const *input,
                   URLRecord *url,
                   URLRecord *base,
                   enum url_parser_state state_override);
    ~URLBasicParser() = default;


  public:
    enum handler_return_status {
      STATUS_OK,
      STATUS_FAILURE,
      STATUS_START_OVER,
      STATUS_LEAVE,
    };
    typedef enum handler_return_status (*StateHandler) (URLBasicParser *parser,
                                                        URLRecord *url);

  public:
    URLRecord *url;
    URLRecord *base;

    std::u32string::const_iterator pointer;

    std::u32string buffer = U"";

    enum url_parser_state state;

    bool at_sign_seen = false;
    bool inside_brackets = false;
    bool password_token_seen = false;


  public:
    void validation_error(char const *code) const;

    inline char32_t c(void) const;
    const std::u32string_view& remaining(void);

    inline bool have_state_override(void) const;
    inline enum url_parser_state get_state_override(void) const;

    void run(void);


  private:
    std::u32string filtered_input_;
    std::u32string::const_iterator end_pointer_;

    std::u32string::const_iterator remaining_begin_;
    std::u32string_view remaining_;

    enum url_parser_state state_override_;


  public:
    static constexpr char32_t eof = static_cast<char32_t>(-1);
  private:
    static const StateHandler k_state_handlers_[NUM_STATES];
};


URLBasicParser::URLBasicParser(std::string const *input,
                               URLRecord *url,
                               URLRecord *base,
                               enum url_parser_state state_override)
{
  /* Step 1. */
  if (url == nullptr) {
    url = new URLRecord;

    /* XXX: C0 control or space */
  }
  this->url = url;
  this->base = base;

  /* Step 2: (XXX) ASCII tab or newline; error */
  /* Step 3: (XXX) ASCII tab or newline; remove */

  QueequegLib::convert_utf8_to_utf32(input, &this->filtered_input_);
  this->filtered_input_.insert(0, U"\xFFFD"); /* provides an extra iterator slot for safety */

  /* Step 4. */
  this->state = (state_override != STATE_NONE_)
              ? state_override
              : SCHEME_START_STATE;
  this->state_override_ = state_override;

  /* Step 5: N/A */
  /* Step 6: Implicit */
  /* Step 7: Implicit */

  /* Step 8 */
  this->pointer = std::next(this->filtered_input_.cbegin()); /* skip initial U+FFFD */
  this->end_pointer_ = this->filtered_input_.cend();
  this->remaining_ = std::u32string_view(this->pointer, this->end_pointer_);

  /* Step 9: See 'URLBasicParser::run(void)' */
  /* Step 10: done in the caller */
}


void
URLBasicParser::validation_error(char const *code) const
{
  (void) code;
  /* ... */
}


inline char32_t
URLBasicParser::c(void) const
{
  if (this->pointer == this->end_pointer_)
    return URLBasicParser::eof;

  return *this->pointer;
}


const std::u32string_view&
URLBasicParser::remaining(void)
{
  if (this->remaining_begin_ != std::next(this->pointer)) {
    this->remaining_begin_ = std::next(this->pointer);
    this->remaining_ = std::u32string_view(this->remaining_begin_, this->end_pointer_);
  }

  return this->remaining_;
}


inline bool
URLBasicParser::have_state_override(void) const
{
  return (this->state_override_ != STATE_NONE_);
}


inline enum url_parser_state
URLBasicParser::get_state_override(void) const
{
  return this->state_override_;
}


void
URLBasicParser::run(void)
{

  while (true)
  {
    enum URLBasicParser::handler_return_status rc;
    rc = URLBasicParser::k_state_handlers_[this->state](this, this->url);
    (void) rc;

    if (this->pointer == this->end_pointer_)
      return;

    ++this->pointer;
  }

}



/*
 * STATE HANDLERS
 */


static enum URLBasicParser::handler_return_status
scheme_start_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  LOGF("scheme start state\n");

  (void) url;

  if (ascii_is_alpha(parser->c())) {
    parser->buffer.push_back(ascii_tolower(parser->c()));
    parser->state = SCHEME_STATE;

    return URLBasicParser::STATUS_OK;
  }

  if (! parser->have_state_override() ) {
    parser->state = NO_SCHEME_STATE;
    --parser->pointer;

    return URLBasicParser::STATUS_OK;
  }

  return URLBasicParser::STATUS_FAILURE;
}


static enum URLBasicParser::handler_return_status
scheme_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  LOGF("test test\n");

  /* Step 1. */
  if (ascii_is_alnum(parser->c())
   || (parser->c() == '+' || parser->c() == '-' || parser->c() == '.')) {
    parser->buffer.push_back(ascii_tolower(parser->c()));
    return URLBasicParser::STATUS_OK;
  }

  /* Step 2. */
  if (parser->c() == ':') {
    if (parser->have_state_override()) {
      std::string buffer_utf8;
      QueequegLib::convert_utf32_to_utf8(&parser->buffer, &buffer_utf8);

      if (URLRecord::k_special_scheme_ports.contains(url->scheme)
       && !URLRecord::k_special_scheme_ports.contains(buffer_utf8))
        return URLBasicParser::STATUS_LEAVE;

      
      if (!URLRecord::k_special_scheme_ports.contains(url->scheme)
       && URLRecord::k_special_scheme_ports.contains(buffer_utf8))
        return URLBasicParser::STATUS_LEAVE;

      if ((url->includes_credentials()
        || (url->port != 0))
       && (buffer_utf8.compare("file")) == 0)
        return URLBasicParser::STATUS_LEAVE;

      if ((url->scheme.compare("file") == 0)
       && false) /* XXX */
        return URLBasicParser::STATUS_LEAVE;

    }

    /* Step 2.2. */
    url->scheme = "";
    QueequegLib::convert_utf32_to_utf8(&parser->buffer, &url->scheme);

    if (parser->have_state_override()) {
      /* Step 2.3.1. */
      if (url->port == URLRecord::k_special_scheme_ports.at(url->scheme))
        url->port = 0;

      return URLBasicParser::STATUS_LEAVE;
    }

    /* Step 2.4. */
    parser->buffer = U"";

    /* Step 2.5. */
    if (url->scheme.compare("file") == 0) {
      if (! parser->remaining().starts_with(U"//") )
        parser->validation_error("special-scheme-missing-following-solidus");

      parser->state = FILE_STATE;
      return URLBasicParser::STATUS_OK;
    }

    /* Step 2.6. */
    if (url->is_special()
     && (parser->base != nullptr)
     && (parser->base->scheme.compare(url->scheme) == 0)) {
      /* Step 2.6.1. */
      assert( parser->base->is_special() );

      /* Step 2.6.2. */
      parser->state = SPECIAL_RELATIVE_OR_AUTHORITY_STATE;
      return URLBasicParser::STATUS_OK;
    }

    /* Step 2.7. */
    if (url->is_special()) {
      parser->state = SPECIAL_AUTHORITY_SLASHES_STATE;
      return URLBasicParser::STATUS_OK;
    }

    /* Step 2.8. */
    if (parser->remaining().starts_with(U"/")) {
      parser->state = PATH_OR_AUTHORITY_STATE;
      ++parser->pointer;
      return URLBasicParser::STATUS_OK;
    }

    /* Step 2.9. */
    url->path = std::vector{ std::string("") };
    parser->state = OPAQUE_PATH_STATE;

    return URLBasicParser::STATUS_OK;
  }

  /* Step 3. */
  if (! parser->have_state_override() ) {
    parser->buffer = U"";
    parser->state = NO_SCHEME_STATE;
    return URLBasicParser::STATUS_START_OVER;
  }

  /* Step 4. */
  return URLBasicParser::STATUS_FAILURE;
}


static enum URLBasicParser::handler_return_status
no_scheme_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
special_relative_or_authority_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
path_or_authority_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
relative_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
relative_slash_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
special_authority_slashes_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) url;

  if ((parser->c() == '/')
   && parser->remaining().starts_with(U"/")) {
    parser->state = SPECIAL_AUTHORITY_IGNORE_SLASHES_STATE;
    ++parser->pointer;
  }

  parser->validation_error("special-scheme-missing-following-solidus");
  parser->state = SPECIAL_AUTHORITY_IGNORE_SLASHES_STATE;
  --parser->pointer;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
special_authority_ignore_slashes_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  LOGF("special authority ignore slashes state\n");

  (void) url;

  /* Step 1. */
  if (parser->c() != '/' && parser->c() != '\\') {
    parser->state = AUTHORITY_STATE;
    --parser->pointer;
  }

  /* Step 2. */
  parser->validation_error("special-scheme-missing-following-solidus");
  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
authority_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  LOGF("authority state\n");

  /* Step 1. */
  if (parser->c() == '@') {
    /* Step 1.1. */
    parser->validation_error("invalid-credentials");

    /* Step 1.2. */
    if (parser->at_sign_seen)
      parser->buffer.insert(0, U"%40");

    /* Step 1.3. */
    parser->at_sign_seen = true;

    /* Step 1.4. */
    for (char32_t cp : parser->buffer) {
      /* Step 1.4.1. */
      if ((cp == ':') && !parser->password_token_seen) {
        parser->password_token_seen = true;
        continue;
      }

      /* Step 1.4.2. */
      /* XXX: percent-encode stuff */
    }

    parser->buffer.clear();
    return URLBasicParser::STATUS_OK;
  }


  /* Step 2. */
  if ((parser->c() == URLBasicParser::eof
    || parser->c() == '/'
    || parser->c() == '?'
    || parser->c() == '#')
   || (url->is_special() && (parser->c() == '\\'))) {
    if (parser->at_sign_seen 
     && parser->buffer.empty()) {
      parser->validation_error("host-missing");
      return URLBasicParser::STATUS_FAILURE;
    }

    /* Step 2.2. */
    std::advance(parser->pointer, -(parser->buffer.size() + 1) );
    parser->state = HOST_STATE;

    return URLBasicParser::STATUS_OK;
  }


  /* Step 3. */
  parser->buffer.push_back( parser->c() );
  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
host_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  LOGF("host state\n");

  /* Step 1. */
  if (parser->have_state_override()
   && (url->scheme.compare("file") == 0)) {
    --parser->pointer;
    parser->state = FILE_HOST_STATE;
    return URLBasicParser::STATUS_OK;
  }


  /* Step 2. */
  if ((parser->c() == ':') && !parser->inside_brackets) {
    if (parser->buffer.empty()) {
      parser->validation_error("host-missing");
      return URLBasicParser::STATUS_FAILURE;
    }

    if (parser->have_state_override()
     && (parser->get_state_override() == HOSTNAME_STATE))
      return URLBasicParser::STATUS_LEAVE;

    /* XXX: host parsing */
  }


  /* Step 3. */
  if ((parser->c() == URLBasicParser::eof
    || parser->c() == '/'
    || parser->c() == '?'
    || parser->c() == '#')
   || (url->is_special() && (parser->c() == '\\'))) {
    --parser->pointer;

    if (url->is_special() && parser->buffer.empty()) {
      parser->validation_error("host-missing");
      return URLBasicParser::STATUS_FAILURE;
    }

    if (parser->have_state_override()
     && parser->buffer.empty()
     && (url->includes_credentials() || (url->port != 0)))
      return URLBasicParser::STATUS_LEAVE;

    /* XXX: host parsing */
    /* ... */

    /* Step 3.6. */
    if (parser->have_state_override())
      return URLBasicParser::STATUS_LEAVE;
  }


  /* Step 4. */
  if (parser->c() == '[')
    parser->inside_brackets = true;

  if (parser->c() == ']')
    parser->inside_brackets = false;

  parser->buffer.push_back( parser->c() );

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
hostname_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  return host_state_handler_(parser, url);
}


static enum URLBasicParser::handler_return_status
port_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  if (ascii_is_digit(parser->c())) {
    parser->buffer.push_back(parser->c());
    return URLBasicParser::STATUS_OK;
  }

  if ((parser->c() == URLBasicParser::eof
    || parser->c() == '/'
    || parser->c() == '?'
    || parser->c() == '#')
   || (url->is_special() && (parser->c() == '\\'))
   || parser->have_state_override()) {

    if (! parser->buffer.empty() ) {
      /* Step 2.1.1. */
      uintmax_t port = { 0 };

      for (char32_t ch : parser->buffer) {
        unsigned int digit = (ch - '0');
        port = (port * 10) + digit;
      }

      /* Step 2.1.2. */
      if (port > std::numeric_limits<uint16_t>::max())
        return URLBasicParser::STATUS_FAILURE;

      /* Step 2.1.3 */
      if (url->is_special()
       && (URLRecord::k_special_scheme_ports.at(url->scheme) == port))
        url->port = 0;
      else
        url->port = port;

      /* Step 2.1.4. */
      parser->buffer.clear();
    }

    if (parser->have_state_override())
      return URLBasicParser::STATUS_LEAVE;

    /* Step 2.3. */
    parser->state = PATH_START_STATE;
    --parser->pointer;
  }

  /* Step 3. */
  parser->validation_error("port-invalid");
  return URLBasicParser::STATUS_FAILURE;
}


static enum URLBasicParser::handler_return_status
file_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
file_slash_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
file_host_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
path_start_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
path_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
opaque_path_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
query_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


static enum URLBasicParser::handler_return_status
fragment_state_handler_(URLBasicParser *parser, URLRecord *url)
{
  (void) parser;
  (void) url;

  return URLBasicParser::STATUS_OK;
}


const URLBasicParser::StateHandler URLBasicParser::k_state_handlers_[NUM_STATES] = {
  nullptr,
  scheme_start_state_handler_,
  scheme_state_handler_,
  no_scheme_state_handler_,
  special_relative_or_authority_state_handler_,
  path_or_authority_state_handler_,
  relative_state_handler_,
  relative_slash_state_handler_,
  special_authority_slashes_state_handler_,
  special_authority_ignore_slashes_state_handler_,
  authority_state_handler_,
  host_state_handler_,
  hostname_state_handler_,
  port_state_handler_,
  file_state_handler_,
  file_slash_state_handler_,
  file_host_state_handler_,
  path_start_state_handler_,
  path_state_handler_,
  opaque_path_state_handler_,
  query_state_handler_,
  fragment_state_handler_,
};


URLRecord *
url_parse_string(std::string const *input,
                 URLRecord *base)
{
  return url_basic_parse_string(input, base);
}


URLRecord *
url_basic_parse_string(std::string const *input,
                       URLRecord *base,
                       URLRecord *url,
                       enum url_parser_state state_override)
{
  URLBasicParser parser(input, url, base, state_override);

  parser.run();

  return parser.url;
}

