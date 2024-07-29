#ifndef _queequeg_dom_document_type_hh_
#define _queequeg_dom_document_type_hh_

#include <string>

#include "dom/core/node.hh"


class DOM_DocumentType : public DOM_Node {
  public:
    DOM_DocumentType(std::shared_ptr< DOM_Document> node_document,
                     std::string name = "",
                     std::string public_id = "",
                     std::string system_id = "")
  : DOM_Node(node_document, DOM_NODETYPE_DOCUMENT_TYPE)
    {
      this->name = name;
      this->public_id = public_id;
      this->system_id = system_id;
    }
    virtual ~DOM_DocumentType() { }

  public:
    std::string name;
    std::string public_id;
    std::string system_id;
};


#endif /* !defined(_queequeg_dom_document_type_hh_) */

