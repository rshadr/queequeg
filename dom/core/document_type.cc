#include <infra/string.h>

#include "dom/core/document_type.hh"


DOM_DocumentType::DOM_DocumentType(std::shared_ptr< DOM_Document> node_document)
: DOM_Node(node_document, DOM_NODETYPE_DOCUMENT_TYPE)
{
}


DOM_DocumentType::~DOM_DocumentType()
{

  infra_string_clearref(&this->name);
  infra_string_clearref(&this->public_id);
  infra_string_clearref(&this->system_id);

}

