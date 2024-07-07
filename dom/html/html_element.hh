#ifndef _queequeg_dom_html_element_hh_
#define _queequeg_dom_html_element_hh_

#include <memory>

#include "dom/core/element.hh"


class DOM_Document;


class DOM_HTMLElement : public DOM_Element {
  public:
    DOM_HTMLElement(std::shared_ptr< DOM_Document> document)
    : DOM_Element(document) { }
    virtual ~DOM_HTMLElement() { }
};


#endif /* !defined(_queequeg_dom_html_element_hh_) */

