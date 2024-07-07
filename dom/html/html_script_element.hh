#ifndef _queequeg_dom_html_script_element_hh_
#define _queequeg_dom_html_script_element_hh_

#include <memory>

#include "dom/html/html_element.hh"


class DOM_Document;


enum html_script_type {
  HTML_SCRIPT_TYPE_CLASSIC,
  HTML_SCRIPT_TYPE_MODULE,
  HTML_SCRIPT_TYPE_IMPORTMAP;
};


class DOM_HTMLScriptElement : public DOM_HTMLElement {
  public:
    DOM_HTMLScriptElement(std::shared_ptr< DOM_Document> document);
    virtual ~DOM_HTMLScriptElement();

  public:
    std::weak_ptr< DOM_Document> parser_document;
    std::weak_ptr< DOM_Document> prep_time_document;

    enum html_script_type script_type;

    bool force_async;
    bool from_external_file;
    bool ready_parser_exec;
    bool already_started;
    bool delaying_load_event;
};



#endif /* !defined(_queequeg_dom_html_script_element_hh_) */

