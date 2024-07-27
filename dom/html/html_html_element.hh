#ifndef _queequeg_dom_html_html_element_hh_
#define _queequeg_dom_html_html_element_hh_

#include <memory>

#include "dom/html/html_element.hh"


class DOM_Document;


class DOM_HTMLHtmlElement : public DOM_HTMLElement {
  public:
    DOM_HTMLHtmlElement(std::shared_ptr< DOM_Document> document,
                        enum InfraNamespace name_space,
                        uint16_t local_name)
    : DOM_HTMLElement(document, name_space, local_name) {}
    virtual ~DOM_HTMLHtmlElement() { }
};


#endif /* !defined(_queequeg_dom_html_html_element_hh_) */

