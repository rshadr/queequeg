#ifndef _queequeg_dom_html_head_element_hh_
#define _queequeg_dom_html_head_element_hh_

#include <memory>

#include "dom/html/html_element.hh"


class DOM_Document;


class DOM_HTMLHeadElement : public DOM_HTMLElement {
  public:
    DOM_HTMLHeadElement(std::shared_ptr< DOM_Document> document)
    : DOM_HTMLElement(document) { }
    virtual ~DOM_HTMLHeadElement() { }
};



#endif /* !defined(_queequeg_dom_html_head_element_hh_) */

