/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include <cassert>
#include <cstdio>
#include <string_view>

#include "url/url.hh"
#include "url/parser.hh"


static void
validation_error(char const *code)
{
  std::fputs(code, stderr);
  std::fputc('\n', stderr);
}


int
url_domain_to_ascii(std::u32string_view input,
                    std::u32string *ascii_domain,
                    bool be_strict)
{
  /* Step 1. */
  std::u32string result;
  int rc = -1;

  /* Step 2. */
  if (rc == -1) {
    validation_error("domain-to-ASCII");
    return -1;
  }

  /* Step 3. */
  if (result.empty()) {
    validation_error("domain-to-ASCII");
    return -1;
  }

  /* Step 4. */
  *ascii_domain = result;
  return 0;
}


int
url_host_parse(std::u32string_view input,
               URLHost *host,
               bool is_opaque)
{
  /* Step 1. */
  if (input.starts_with(U"[")) {
    if (! input.ends_with(U"]") ) {
      /* XXX: validation error */
      return -1;
    }

    /* XXX: ipv6 parse */
    return 0;
  }


  /* Step 2. */
  if (is_opaque) {
    /* XXX: opaque-host parsing */
    return 0;
  }


  /* Step 3. */
  assert( ! input.empty() );

  /* Step 4.: XXX */
  std::u32string domain = std::u32string(input);

  /* Step 5. + 6. */
  std::u32string ascii_domain;
  if (url_domain_to_ascii(domain, &ascii_domain, false) != 0)
    return -1;

  /* Step 7.: XXX */
  /* Step 8.: XXX */

  /* Step 9. */
  host->domain = ascii_domain;
  return 0;
}

