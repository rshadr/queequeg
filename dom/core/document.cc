/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include <cassert>

#include "dom/core/element.hh"
#include "dom/core/document.hh"
#include "dom/html/html_element.hh"

#include "html/elements.hh"


namespace DOM {


Document::Document(enum dom_document_format format)
: DOM::Node(nullptr, DOM_NODETYPE_DOCUMENT)
{
  // this->node_document = std::static_pointer_cast<DOM_Document>(this->shared_from_this());
  /*
   * can't be called in constructor...
   */
  this->document_format = format;

  this->parser_status = DOM_DOCUMENT_PARSER_STATUS_UNAVAILABLE;

  this->quirks_mode = DOM_QUIRKSMODE_NO_QUIRKS;
}


[[nodiscard]]
std::shared_ptr< DOM::Element>
DOM::Document::create_element(uint16_t local_name,
                              enum InfraNamespace name_space,
                              void *prefix,
                              void *is,
                              bool sync_custom_elements)
{
  (void) prefix;
  (void) is;
  (void) sync_custom_elements;

  assert( name_space == INFRA_NAMESPACE_HTML && "only HTML is supported" );

  std::shared_ptr< DOM::Element> result = nullptr;

  /* XXX: custom definition ... */

  {
    std::shared_ptr< DOM::Document> document =
     std::dynamic_pointer_cast<DOM::Document>(this->shared_from_this());

    /* XXX: element interface */
    result = std::dynamic_pointer_cast<DOM::Element>(std::shared_ptr<DOM::HTMLElement>(
              HTML::new_element_with_index(document, local_name)));

    result->custom_state = DOM_CESTATE_UNCUSTOMIZED;
    result->custom_definition = nullptr;

    /* ... */
  }

  return result;

}


} /* namespace DOM */

