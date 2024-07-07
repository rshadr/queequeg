/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include "dom/core/element.hh"

#include "dom/core/document.hh"


DOM_Document::DOM_Document(enum dom_document_format format)
: DOM_Node(nullptr, DOM_NODETYPE_DOCUMENT)
{
  // this->node_document = std::static_pointer_cast<DOM_Document>(this->shared_from_this());
  /*
   * can't be called in constructor...
   */
  this->document_format = format;

  this->parser_status = DOM_DOCUMENT_PARSER_STATUS_UNAVAILABLE;

  this->quirks_mode = DOM_QUIRKSMODE_NO_QUIRKS;
}


DOM_Document::~DOM_Document()
{
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

