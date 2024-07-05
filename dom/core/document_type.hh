#ifndef _queequeg_dom_document_type_hh_
#define _queequeg_dom_document_type_hh_

#include <infra/string.h>

#include "dom/core/node.hh"


class DOM_DocumentType : public DOM_Node {
  public:
    DOM_DocumentType(std::shared_ptr< DOM_Document> node_document);
   ~DOM_DocumentType();

  public:
    InfraString *name;
    InfraString *public_id;
    InfraString *system_id;
};


#endif /* !defined(_queequeg_dom_document_type_hh_) */

