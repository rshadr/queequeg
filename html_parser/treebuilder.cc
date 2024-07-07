/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include <assert.h>

#include <infra/string.h>

#include "html_parser/common.hh"
#include "dom/core/document.hh"

#define TREEBUILDER_PROCESS_TOKENS
// #undef TREEBUILDER_PROCESS_TOKENS


TreeBuilder::TreeBuilder(std::shared_ptr< DOM_Document> document)
{

  this->document = document;

  this->mode = INITIAL_MODE;

  this->flags = {
    .fragment_parse   = false,
    .scripting        = false,
    .frameset_ok      = true,
    .foster_parenting = false,
    .parser_pause     = false,
  };

  this->skip_newline = false;

}


TreeBuilder::~TreeBuilder()
{
}


void
TreeBuilder::process_token(union token_data *token_data,
                           enum token_type token_type)
{
#ifdef TREEBUILDER_PROCESS_TOKENS
  enum treebuilder_status status;


  if (this->skip_newline) {
    this->skip_newline = false;

    if (token_type == TOKEN_WHITESPACE
     && token_data->ch == '\n')
      return;
  }


  do {
    assert( TreeBuilder::k_insertion_mode_handlers_[this->mode] != nullptr );
    status = TreeBuilder::k_insertion_mode_handlers_[this->mode](this, token_data, token_type);
  } while (status == TREEBUILDER_STATUS_REPROCESS);
#endif
}


[[nodiscard]]
enum treebuilder_status
TreeBuilder::tree_construction_dispatcher_(union token_data *token_data,
                                           enum token_type token_type)
{
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


void
TreeBuilder::error(void)
{
}


InsertionLocation
TreeBuilder::appropriate_insertion_place(std::shared_ptr< DOM_Node> override_target)
{
  InsertionLocation location = { nullptr, nullptr };

  std::shared_ptr< DOM_Node> target = override_target != nullptr
                                    ? override_target
                                    : std::static_pointer_cast< DOM_Node>(this->current_node());


  (void) override_target;

  /* ... */

  return location;
}


[[nodiscard]]
std::shared_ptr< DOM_Element>
TreeBuilder::create_element_for_token(struct tag_token const *tag,
                                      enum InfraNamespace name_space,
                                      std::shared_ptr< DOM_Node> intended_parent)
{
  std::shared_ptr< DOM_Document> document = intended_parent->node_document.lock();
  int16_t local_name = 0;
  void *is = nullptr;
  void *definition = nullptr;

  bool exec_script = (definition != nullptr && this->context == nullptr);

  /* ... */

  std::shared_ptr< DOM_Element> element =
   document->create_element(local_name, name_space, nullptr, is, exec_script);

  /* ... */


  return element;
}


void
TreeBuilder::insert_element_at_adjusted_insertion_location(std::shared_ptr< DOM_Element> element)
{
  InsertionLocation location = this->appropriate_insertion_place();
  /* XXX: check if can insert */

  /* ... */
  /* XXX: insert */
  /* ... */
}


std::shared_ptr< DOM_Element>
TreeBuilder::insert_foreign_element(struct tag_token const *tag,
                                    enum InfraNamespace name_space,
                                    bool only_add_to_element_stack)
{
  (void) tag;
  (void) name_space;
  (void) only_add_to_element_stack;

  std::shared_ptr< DOM_Element> element = nullptr;
  /* ... */
  return element;

}


std::shared_ptr< DOM_Element>
TreeBuilder::insert_html_element(struct tag_token const *tag)
{
  return this->insert_foreign_element(tag, INFRA_NAMESPACE_HTML, false);
}


void
TreeBuilder::insert_character(char32_t ch)
{
  (void) ch;
  /* ... */
}


void
TreeBuilder::insert_comment(std::string *data,
                            InsertionLocation where)
{
  // if (where.parent == nullptr)
  //   where = this->appropriate_node_place();

  (void) data;
  (void) where;

  /* ... */
}


[[nodiscard]]
enum treebuilder_status
TreeBuilder::generic_raw_text_parse(struct tag_token *tag)
{
  this->insert_html_element(tag);

  this->tokenizer->state = RAWTEXT_STATE;

  this->original_mode = this->mode;
  this->mode = TEXT_MODE;

  return TREEBUILDER_STATUS_OK;
}


[[nodiscard]]
enum treebuilder_status
TreeBuilder::generic_rcdata_parse(struct tag_token *tag)
{
  this->insert_html_element(tag);

  this->tokenizer->state = RCDATA_STATE;

  this->original_mode = this->mode;
  this->mode = TEXT_MODE;

  return TREEBUILDER_STATUS_OK;
}

