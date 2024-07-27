#ifndef _queequeg_dom_html_element_hh_
#define _queequeg_dom_html_element_hh_

#include <memory>

#include "dom/core/element.hh"


class DOM_Document;


class DOM_HTMLElement : public DOM_Element {
  public:
    DOM_HTMLElement(std::shared_ptr< DOM_Document> document,
                    enum InfraNamespace name_space,
                    uint16_t local_name)
    : DOM_Element(document, name_space, local_name) { }
    virtual ~DOM_HTMLElement() { }
};


#endif /* !defined(_queequeg_dom_html_element_hh_) */

