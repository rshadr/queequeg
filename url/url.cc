/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include <cstdint>
#include <string>
#include <unordered_map>

#include "url/url.hh"


bool
URLRecord::is_special(void) const
{
  return URLRecord::k_special_scheme_ports.contains(this->scheme);
}


bool
URLRecord::includes_credentials(void) const
{
  return false;
}



const std::unordered_map< std::string, uint16_t> URLRecord::k_special_scheme_ports = {
  { "ftp",    21 },
  { "file",    0 },
  { "http",   80 },
  { "https", 443 },
  { "ws",     80 },
  { "wss",   443 },
};

