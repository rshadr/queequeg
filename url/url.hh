#ifndef _queequeg_url_url_hh_
#define _queequeg_url_url_hh_


#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>


struct URLHost final {
  enum host_type {
    HOST_EMPTY = 0,
    HOST_DOMAIN,
    HOST_IPV4,
    HOST_IPV6,
    HOST_OPAQUE,
  };

  /*
   *  can't use union for 'domain' and 'opaque' because we have non-trivial
   * std::basic_string<T> ctor/dtor
   */
  std::u32string domain;
  std::u32string opaque;
  union {
    uint32_t ipv4;
    uint16_t ipv6[8];
  };

  enum host_type type = HOST_EMPTY;
};


class URLRecord final {
  public:
    URLRecord() = default;
    ~URLRecord() = default;


  public:
    std::string scheme = "";

    std::string username = "";
    std::string password = "";

    URLHost host;

    std::vector< std::string> path = { };

    std::string query;
    std::string fragment;

    uint16_t port = 0;

    bool have_host = false;
    bool have_query = false;
    bool have_fragment = false;


  public:
    bool is_special(void) const;
    bool includes_credentials(void) const;


  public:
    static const std::unordered_map< std::string, uint16_t> k_special_scheme_ports;
};


#endif /* !defined(_queequeg_url_url_hh_) */

