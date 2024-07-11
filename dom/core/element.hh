#ifndef _queequeg_dom_element_hh_
#define _queequeg_dom_element_hh_

#include <stdint.h>

#include <infra/namespace.h>

#include "dom/core/node.hh"



class DOM_Document;


enum dom_custom_element_state {
  DOM_CESTATE_UNDEFINED,
  DOM_CESTATE_FAILED,
  DOM_CESTATE_UNCUSTOMIZED,
  DOM_CESTATE_PRECUSTOMIZED,
  DOM_CESTATE_CUSTOM,
};


class DOM_Element : public DOM_Node {
  public:
    DOM_Element(std::shared_ptr< DOM_Document> document);
    virtual ~DOM_Element() { }


  public:
    uint16_t local_name;
    enum InfraNamespace name_space;

    void *custom_definition;
    enum dom_custom_element_state custom_state;


  public:
    inline bool
    has_element_index(enum InfraNamespace name_space, uint16_t local_name) const
    {
      return (this->name_space == name_space && this->local_name == local_name);
    }

    inline bool
    has_html_element_index(uint16_t local_name) const
    {
      return this->has_element_index(INFRA_NAMESPACE_HTML, local_name);
    }
    
};


#endif /* !defined(_queequeg_dom_element_hh_) */

