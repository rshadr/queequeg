#ifndef _queequeg_url_parser_hh_
#define _queequeg_url_parser_hh_

#include <string_view>

#include "url/url.hh"


enum url_parser_state {
  STATE_NONE_ = 0,

  SCHEME_START_STATE,
  SCHEME_STATE,
  NO_SCHEME_STATE,
  SPECIAL_RELATIVE_OR_AUTHORITY_STATE,
  PATH_OR_AUTHORITY_STATE,
  RELATIVE_STATE,
  RELATIVE_SLASH_STATE,
  SPECIAL_AUTHORITY_SLASHES_STATE,
  SPECIAL_AUTHORITY_IGNORE_SLASHES_STATE,
  AUTHORITY_STATE,
  HOST_STATE,
  HOSTNAME_STATE,
  PORT_STATE,
  FILE_STATE,
  FILE_SLASH_STATE,
  FILE_HOST_STATE,
  PATH_START_STATE,
  PATH_STATE,
  OPAQUE_PATH_STATE,
  QUERY_STATE,
  FRAGMENT_STATE,

  NUM_STATES,
};

int url_domain_to_ascii(std::u32string_view input,
                        std::u32string *ascii_domain,
                        bool be_strict);

int url_host_parse(std::u32string_view input,
                   URLHost *host,
                   bool is_opaque = false);

URLRecord *url_parse_string(std::string const *input,
                            URLRecord *base = nullptr);

URLRecord *url_basic_parse_string(std::string const *input,
                                  URLRecord *base = nullptr,
                                  URLRecord *url = nullptr,
                                  enum url_parser_state state_override = STATE_NONE_);



#endif /* !defined(_queequeg_url_parser_hh_) */

