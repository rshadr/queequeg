#include "html_parser/common.hh"

#include "dom/core/document.hh"

/*
 * XXX: manage stack memory and element references
 */

TreeBuilder::TreeBuilder(DOM_Document *document)
{
  this->document = document;

  this->flags = {
    .fragment_parse   = false,
    .scripting        = false,
    .frameset_ok      = true,
    .foster_parenting = false,
    .parser_pause     = false,
  };

}


TreeBuilder::~TreeBuilder()
{
}

