#ifndef _queequeg_dom_element_hh_
#define _queequeg_dom_element_hh_

#include "dom/core/node.hh"

#include <stdint.h>


enum dom_custom_element_state {
  DOM_CESTATE_UNDEFINED,
  DOM_CESTATE_FAILED,
  DOM_CESTATE_UNCUSTOMIZED,
  DOM_CESTATE_PRECUSTOMIZED,
  DOM_CESTATE_CUSTOM,
};


class DOM_Element : DOM_Node {
  public:
    int16_t local_name;
    int16_t name_space;

    enum dom_custom_element_state custom_state;
};


#endif /* !defined(_queequeg_dom_element_hh_) */

