/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include <iterator>
#include <ranges>
#include <set>

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
     && token_data->ch == U'\n')
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

  for (std::shared_ptr< DOM_Element> elem : std::ranges::views::reverse(this->open_elements))
  {
    /*
     * Separate pointer copy for the fragment case
     */
    std::shared_ptr< DOM_Element> node = elem;

    if (node == this->open_elements.front())
      last = true;

    if (this->flags.fragment_parse)
      node = this->context;


    if (node->has_html_element_index(HTML_ELEMENT_SELECT)) {
      /* ... */
      this->mode = IN_SELECT_MODE;
      return;
    }


    if ((node->has_html_element_index(HTML_ELEMENT_TD)
      || node->has_html_element_index(HTML_ELEMENT_TH))
     && !last) {
      this->mode = IN_CELL_MODE;
      return;
    }


    if (node->has_html_element_index(HTML_ELEMENT_TR)) {
      this->mode = IN_ROW_MODE;
      return;
    }


    if (node->has_html_element_index(HTML_ELEMENT_TBODY)
     || node->has_html_element_index(HTML_ELEMENT_THEAD)
     || node->has_html_element_index(HTML_ELEMENT_TFOOT)) {
      this->mode = IN_TABLE_BODY_MODE;
      return;
    }


    if (node->has_html_element_index(HTML_ELEMENT_CAPTION)) {
      this->mode = IN_CAPTION_MODE;
      return;
    }


    if (node->has_html_element_index(HTML_ELEMENT_COLGROUP)) {
      this->mode = IN_COLUMN_GROUP_MODE;
      return;
    }


    if (node->has_html_element_index(HTML_ELEMENT_TABLE)) {
      this->mode = IN_TABLE_MODE;
      return;
    }


    if (node->has_html_element_index(HTML_ELEMENT_TEMPLATE)) {
      this->mode = this->template_modes.back();
      return;
    }


    if (node->has_html_element_index(HTML_ELEMENT_HEAD)
     && !last) {
      this->mode = IN_HEAD_MODE;
      return;
    }


    if (node->has_html_element_index(HTML_ELEMENT_BODY)) {
      this->mode = IN_BODY_MODE;
      return;
    }


    if (node->has_html_element_index(HTML_ELEMENT_FRAMESET)) {
      /* fragment case */
      this->mode = IN_FRAMESET_MODE;
      return;
    }


    if (node->has_html_element_index(HTML_ELEMENT_HTML)) {
      if (this->head == nullptr) {
        this->mode = BEFORE_HEAD_MODE;
        return;
      }

      this->mode = AFTER_HEAD_MODE;
      return;
    }


    if (last) {
      /* fragment case */
      this->mode = IN_BODY_MODE;
      return;
    }


  }


  // std::unreachable();
}


static const std::set<uint16_t> k_special_html_elements_set_ = {
  HTML_ELEMENT_ADDRESS,
  HTML_ELEMENT_APPLET,
  HTML_ELEMENT_AREA,
  HTML_ELEMENT_ARTICLE,
  HTML_ELEMENT_ASIDE,
  HTML_ELEMENT_BASE,
  HTML_ELEMENT_BASEFONT,
  HTML_ELEMENT_BGSOUND,
  HTML_ELEMENT_BLOCKQUOTE,
  HTML_ELEMENT_BODY,
  HTML_ELEMENT_BR,
  HTML_ELEMENT_BUTTON,
  HTML_ELEMENT_CAPTION,
  HTML_ELEMENT_CENTER,
  HTML_ELEMENT_COL,
  HTML_ELEMENT_COLGROUP,
  HTML_ELEMENT_DD,
  HTML_ELEMENT_DETAILS,
  HTML_ELEMENT_DIR,
  HTML_ELEMENT_DIV,
  HTML_ELEMENT_DL,
  HTML_ELEMENT_DT,
  HTML_ELEMENT_EMBED,
  HTML_ELEMENT_FIELDSET,
  HTML_ELEMENT_FIGCAPTION,
  HTML_ELEMENT_FIGURE,
  HTML_ELEMENT_FOOTER,
  HTML_ELEMENT_FORM,
  HTML_ELEMENT_FRAME,
  HTML_ELEMENT_FRAMESET,
  HTML_ELEMENT_H1,
  HTML_ELEMENT_H2,
  HTML_ELEMENT_H3,
  HTML_ELEMENT_H4,
  HTML_ELEMENT_H5,
  HTML_ELEMENT_H6,
  HTML_ELEMENT_HEAD,
  HTML_ELEMENT_HEADER,
  HTML_ELEMENT_HGROUP,
  HTML_ELEMENT_HR,
  HTML_ELEMENT_HTML,
  HTML_ELEMENT_IFRAME,
  HTML_ELEMENT_IMG,
  HTML_ELEMENT_INPUT,
  HTML_ELEMENT_KEYGEN,
  HTML_ELEMENT_LI,
  HTML_ELEMENT_LINK,
  HTML_ELEMENT_LISTING,
  HTML_ELEMENT_MAIN,
  HTML_ELEMENT_MARQUEE,
  HTML_ELEMENT_MENU,
  HTML_ELEMENT_META,
  HTML_ELEMENT_NAV,
  HTML_ELEMENT_NOEMBED,
  HTML_ELEMENT_NOFRAMES,
  HTML_ELEMENT_OBJECT,
  HTML_ELEMENT_OL,
  HTML_ELEMENT_P,
  HTML_ELEMENT_PARAM,
  HTML_ELEMENT_PLAINTEXT,
  HTML_ELEMENT_PRE,
  HTML_ELEMENT_SCRIPT,
  HTML_ELEMENT_SEARCH,
  HTML_ELEMENT_SECTION,
  HTML_ELEMENT_SELECT,
  HTML_ELEMENT_SOURCE,
  HTML_ELEMENT_STYLE,
  HTML_ELEMENT_SUMMARY,
  HTML_ELEMENT_TABLE,
  HTML_ELEMENT_TBODY,
  HTML_ELEMENT_TD,
  HTML_ELEMENT_TEMPLATE,
  HTML_ELEMENT_TEXTAREA,
  HTML_ELEMENT_TFOOT,
  HTML_ELEMENT_TH,
  HTML_ELEMENT_THEAD,
  HTML_ELEMENT_TITLE,
  HTML_ELEMENT_TR,
  HTML_ELEMENT_TRACK,
  HTML_ELEMENT_UL,
  HTML_ELEMENT_WBR,
  HTML_ELEMENT_XMP,
};


bool
TreeBuilder::is_special_element(std::shared_ptr< DOM_Element> element) const
{
  /* XXX: other namespaces */

  if (element->name_space == INFRA_NAMESPACE_HTML)
    return k_special_html_elements_set_.contains(element->local_name);


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
    std::shared_ptr< DOM_Element> last_template = nullptr;
    std::shared_ptr< DOM_Element> last_table    = nullptr;
    int last_template_idx = -1;
    int last_table_idx    = -1;

    for (int i = 0; i < static_cast<long int>(this->open_elements.size()); i++) {
      std::shared_ptr< DOM_Element> node = this->open_elements[i];

      if (node->has_html_element_index(HTML_ELEMENT_TEMPLATE)) {
        last_template     = node;
        last_template_idx = i;
        continue;
      }


      if (node->has_html_element_index(HTML_ELEMENT_TABLE)) {
        last_table     = node;
        last_table_idx = i;
        continue;
      }

    }


    if (last_template != nullptr
     && (last_table == nullptr
      || last_template_idx > last_table_idx)) {
      /* XXX: template contents */
      goto sanitize;
    }


    if (last_table == nullptr) {
      /* fragment case */
      location.parent = std::dynamic_pointer_cast<DOM_Node>(this->open_elements.front());
      location.child  = nullptr;
      goto sanitize;
    }


    std::shared_ptr< DOM_Element> prev_elem = this->open_elements[last_table_idx + 1];

    location.parent = std::dynamic_pointer_cast<DOM_Node>(prev_elem);
    location.child  = nullptr;

  } else {
    location.parent = target;
    location.child  = nullptr;
  }


sanitize:
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

