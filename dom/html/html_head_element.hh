#ifndef _queequeg_dom_html_head_element_hh_
#define _queequeg_dom_html_head_element_hh_

#include <memory>

#include "dom/html/html_element.hh"


namespace DOM {


class Document;


class HTMLHeadElement : public DOM::HTMLElement {
  public:
    HTMLHeadElement(std::shared_ptr< DOM::Document> document,
                    enum InfraNamespace name_space,
                    uint16_t local_name)
    : DOM::HTMLElement(document, name_space, local_name) { }
    virtual ~HTMLHeadElement() = default;
};


} /* namespace DOM */


#endif /* !defined(_queequeg_dom_html_head_element_hh_) */

