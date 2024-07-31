#ifndef _queequeg_dom_html_element_hh_
#define _queequeg_dom_html_element_hh_

#include <memory>

#include "dom/core/element.hh"


namespace DOM {


class HTMLElement : public DOM::Element {
  public:
    HTMLElement(std::shared_ptr< DOM::Document> document,
                enum InfraNamespace name_space,
                uint16_t local_name)
    : DOM::Element(document, name_space, local_name) { }
    virtual ~HTMLElement() = default;
};


} /* namespace DOM */


#endif /* !defined(_queequeg_dom_html_element_hh_) */

