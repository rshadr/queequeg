#ifndef _queequeg_dom_document_type_hh_
#define _queequeg_dom_document_type_hh_

#include <string>

#include "dom/core/node.hh"


namespace DOM {


class DocumentType : public DOM::Node {
  public:
    DocumentType(std::shared_ptr< DOM::Document> node_document,
                 std::string name = "",
                 std::string public_id = "",
                 std::string system_id = "")
  : DOM::Node(node_document, DOM_NODETYPE_DOCUMENT_TYPE)
    {
      this->name = name;
      this->public_id = public_id;
      this->system_id = system_id;
    }
    virtual ~DocumentType() = default;

  public:
    std::string name;
    std::string public_id;
    std::string system_id;
};


} /* namespace DOM */


#endif /* !defined(_queequeg_dom_document_type_hh_) */

