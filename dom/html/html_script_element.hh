#ifndef _queequeg_dom_html_script_element_hh_
#define _queequeg_dom_html_script_element_hh_

#include <memory>

#include "dom/html/html_element.hh"


class DOM_Document;


enum html_script_type {
  HTML_SCRIPT_TYPE_CLASSIC,
  HTML_SCRIPT_TYPE_MODULE,
  HTML_SCRIPT_TYPE_IMPORTMAP,
};


class DOM_HTMLScriptElement : public DOM_HTMLElement {
  public:
    DOM_HTMLScriptElement(std::shared_ptr< DOM_Document> document,
                          enum InfraNamespace name_space,
                          uint16_t local_name)
    : DOM_HTMLElement(document, name_space, local_name) { }
    virtual ~DOM_HTMLScriptElement() { }

  public:
    std::weak_ptr< DOM_Document> parser_document;
    std::weak_ptr< DOM_Document> prep_time_document;

    enum html_script_type script_type;

    struct {
      bool force_async = true;
      bool from_external_file = false;
      bool ready_parser_exec = false;
      bool already_started = false;
      bool delaying_load_event = false;
    } script_flags;
};



#endif /* !defined(_queequeg_dom_html_script_element_hh_) */

