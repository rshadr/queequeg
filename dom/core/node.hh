#ifndef _queequeg_dom_node_hh_
#define _queequeg_dom_node_hh_

#include "dom/events/event_target.hh"


class DOM_Document;


enum dom_node_type {
  DOM_NODETYPE_ELEMENT                = 1,
  DOM_NODETYPE_ATTRIBUTE              = 2,
  DOM_NODETYPE_TEXT                   = 3,
  DOM_NODETYPE_CDATA_SECTION          = 4,
  DOM_NODETYPE_ENTITY_REFERENCE       = 5, /* legacy */
  DOM_NODETYPE_ENTITY                 = 6, /* legacy */
  DOM_NODETYPE_PROCESSING_INSTRUCTION = 7,
  DOM_NODETYPE_COMMENT                = 8,
  DOM_NODETYPE_DOCUMENT               = 9,
  DOM_NODETYPE_DOCUMENT_TYPE          = 10,
  DOM_NODETYPE_DOCUMENT_FRAGMENT      = 11,
  DOM_NODETYPE_NOTATION               = 12, /* legacy */
};


class DOM_Node : DOM_EventTarget {

  public:
    DOM_Document *node_document;
    enum dom_node_type node_type;

    DOM_Node *get_previous_sibling(void);

    void insert_node(DOM_Node *node, DOM_Node *child, bool supp_observers = false);
    void append_node(DOM_Node *node, bool supp_observers = false);


  protected:
    typedef void (DOM_Node::*children_changed_cb_t) (void);
};


#endif /* !defined(_queequeg_dom_node_hh_) */

