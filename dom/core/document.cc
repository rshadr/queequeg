#include "dom/core/element.hh"

#include "dom/core/document.hh"


DOM_Document::DOM_Document(enum dom_document_format format)
: DOM_Node(std::static_pointer_cast< DOM_Document>(this->shared_from_this()), DOM_NODETYPE_DOCUMENT)
{
  this->document_format = format;
}


[[nodiscard]]
std::shared_ptr< DOM_Element>
DOM_Document::create_element(uint16_t local_name,
                             enum InfraNamespace name_space,
                             void *prefix,
                             void *is,
                             bool sync_custom_elements)
{
  (void) prefix;
  (void) is;
  (void) sync_custom_elements;

  std::shared_ptr< DOM_Element> result = nullptr;

  /* XXX: custom definition ... */

  {
    /* XXX: element interface */
    result = std::make_shared< DOM_Element>(std::static_pointer_cast< DOM_Document>(this->shared_from_this()));

    result->local_name = local_name;
    result->name_space = name_space;

    result->custom_state = DOM_CESTATE_UNCUSTOMIZED;
    result->custom_definition = nullptr;

    /* ... */
  }

  return result;

}

