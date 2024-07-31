#ifndef _queequeg_dom_html_html_element_hh_
#define _queequeg_dom_html_html_element_hh_

#include <memory>

#include "dom/html/html_element.hh"


namespace DOM {


class Document;


class HTMLHtmlElement : public DOM::HTMLElement {
  public:
    HTMLHtmlElement(std::shared_ptr< DOM::Document> document,
                    enum InfraNamespace name_space,
                    uint16_t local_name)
    : DOM::HTMLElement(document, name_space, local_name) { }
    virtual ~HTMLHtmlElement() = default;
};


} /* namespace DOM */


#endif /* !defined(_queequeg_dom_html_html_element_hh_) */

