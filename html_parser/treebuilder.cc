/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include <ranges>

#include <assert.h>


#include "html_parser/common.hh"
#include "dom/core/document.hh"

#define TREEBUILDER_PROCESS_TOKENS
// #undef TREEBUILDER_PROCESS_TOKENS


TreeBuilder::TreeBuilder(std::shared_ptr< DOM_Document> document)
{

  this->document = document;

  this->script_nesting_level = 0;

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
  /* ... */
}


void
TreeBuilder::reset_insertion_mode_appropriately(void)
{
  bool last = false;

  for (std::shared_ptr< DOM_Element> node : std::ranges::views::reverse(this->open_elements))
  {
    /*
     * Separate copy for the fragment case
     */
    std::shared_ptr< DOM_Element> elem = node;

    if (elem == this->open_elements.front())
      last = true;

    if (this->flags.fragment_parse)
      elem = this->context;
  }

}


bool
TreeBuilder::is_special_element(std::shared_ptr< DOM_Element> element) const
{
  (void) element;
  /* ... */
  return false;
}


void
TreeBuilder::push_to_active_formatting_elements(std::shared_ptr< DOM_Element> element)
{
  /* ... */
  (void) element;
}


void
TreeBuilder::reconstruct_active_formatting_elements(void)
{
  /* ... */
}


void
TreeBuilder::acknowledge_self_closing_flag(struct tag_token *tag) const
{
  if (tag->self_closing_flag)
    tag->ack_self_closing_flag_ = true;
}


InsertionLocation
TreeBuilder::appropriate_insertion_place(std::shared_ptr< DOM_Element> override_target)
{
  InsertionLocation location = { nullptr, nullptr };

  std::shared_ptr< DOM_Element> target = override_target != nullptr
                                       ? override_target
                                       : this->current_node();

  if (this->flags.foster_parenting
   && (target->has_html_element_index(HTML_ELEMENT_TABLE)
    || target->has_html_element_index(HTML_ELEMENT_TBODY)
    || target->has_html_element_index(HTML_ELEMENT_TFOOT)
    || target->has_html_element_index(HTML_ELEMENT_THEAD)
    || target->has_html_element_index(HTML_ELEMENT_TR))) {
    /* ... */
  } else {
    location.parent = target;
    location.child  = nullptr;
  }


  if (location.parent->is_element()
   && std::dynamic_pointer_cast<DOM_Element>(location.parent)->has_html_element_index(HTML_ELEMENT_TEMPLATE)) {
    /* XXX: template contents */
  }


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

  assert( name_space == INFRA_NAMESPACE_HTML );
  fprintf(stderr, "tag_name: %s\n", tag->tag_name.c_str());
  local_name = HTML::k_local_names_table.at(tag->tag_name);
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

  /* XXX: custom element reactions */

  location.parent->insert_node(std::static_pointer_cast<DOM_Node>(element),
                               location.child);

  /* XXX: invoke custom element reactions */
}


std::shared_ptr< DOM_Element>
TreeBuilder::insert_foreign_element(struct tag_token const *tag,
                                    enum InfraNamespace name_space,
                                    bool only_add_to_element_stack)
{
  InsertionLocation location = this->appropriate_insertion_place();

  std::shared_ptr< DOM_Element> element =
   this->create_element_for_token(tag, name_space, location.parent);

  if (!only_add_to_element_stack)
    (void)0; /* XXX */

  this->open_elements.push_back(element);

  return element;
}


std::shared_ptr< DOM_Element>
TreeBuilder::insert_html_element(struct tag_token const *tag)
{
  return this->insert_foreign_element(tag, INFRA_NAMESPACE_HTML, false);
}


void
TreeBuilder::insert_character_array_(char32_t const *arr, size_t arr_len)
{
  /* XXX: magic happens here */
  (void) arr;
  (void) arr_len;
}


void
TreeBuilder::insert_characters(std::vector< char32_t> *vch)
{
  this->insert_character_array_(vch->data(), vch->size());
}


void
TreeBuilder::insert_character(char32_t ch)
{
  this->insert_character_array_(&ch, 1);
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


void
TreeBuilder::generate_implied_end_tags(uint16_t exclude_html)
{

  while (true) {
    const std::shared_ptr< DOM_Element> cur_node = this->current_node();


    if (exclude_html != 0
     && cur_node->name_space == INFRA_NAMESPACE_HTML
     && cur_node->local_name == exclude_html)
      break;


    if ((cur_node->name_space == INFRA_NAMESPACE_HTML)
     && (cur_node->local_name == HTML_ELEMENT_DD
      || cur_node->local_name == HTML_ELEMENT_DT
      || cur_node->local_name == HTML_ELEMENT_LI
      || cur_node->local_name == HTML_ELEMENT_OPTGROUP
      || cur_node->local_name == HTML_ELEMENT_OPTION
      || cur_node->local_name == HTML_ELEMENT_P
      || cur_node->local_name == HTML_ELEMENT_RB
      || cur_node->local_name == HTML_ELEMENT_RP
      || cur_node->local_name == HTML_ELEMENT_RT
      || cur_node->local_name == HTML_ELEMENT_RTC)) {
      this->open_elements.pop_back();
      continue;
    }

    break;
  }

}

