/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include <algorithm>
#include <iterator>
#include <ranges>
#include <set>

#include <cassert>
#include <cstdio>

#include "html_parser/internal.hh"

#include "dom/core/document.hh"
#include "dom/core/text.hh"
#include "dom/core/comment.hh"

#include "qglib/unicode.hh"


#define LOGF(fmt, ...) \
  std::fprintf(stderr, (fmt), __VA_ARGS__)

#define TREEBUILDER_PROCESS_TOKENS
// #undef TREEBUILDER_PROCESS_TOKENS


TreeBuilder::TreeBuilder(std::shared_ptr< DOM::Document> document)
{
  this->document = document;
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


  if (this->flags.skip_newline) {
    this->flags.skip_newline = false;

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

  for (int i = static_cast<int>(this->open_elements.size()) - 1; i >= 0; i--)
  {
    const int elem_idx = i;
    const int node_idx = elem_idx;

    std::shared_ptr< DOM::Element> elem = this->open_elements[elem_idx];
    /*
     * Separate pointer copy for the fragment case
     */
    std::shared_ptr< DOM::Element> node = elem;

    if (node == this->open_elements.front())
      last = true;

    if (this->flags.fragment_parse)
      node = this->context;


    if (node->has_html_element_index(HTML_ELEMENT_SELECT)) {
      if (! last) {
        int ancestor_idx = node_idx;
        std::shared_ptr< DOM::Element> ancestor = this->open_elements[ancestor_idx];

        while (ancestor_idx > 0) {
          ancestor = this->open_elements[--ancestor_idx];

          if (ancestor->has_html_element_index(HTML_ELEMENT_TEMPLATE))
            break;

          if (ancestor->has_html_element_index(HTML_ELEMENT_TABLE)) {
            this->mode = IN_SELECT_IN_TABLE_MODE;
            return;
          }

        }

      }

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
TreeBuilder::is_special_element(std::shared_ptr< DOM::Element> element) const
{
  /* XXX: other namespaces */

  if (element->name_space == INFRA_NAMESPACE_HTML)
    return k_special_html_elements_set_.contains(element->local_name);


  return false;
}


/*
 * This is used to determine whether we allocate a copy of the tag_token or
 * not when creating the element.
 */
bool
TreeBuilder::is_formatting_element(std::shared_ptr< DOM::Element> element) const
{
  if (element->name_space != INFRA_NAMESPACE_HTML)
    return false;

  switch (element->local_name)
  {
    case HTML_ELEMENT_A:     case HTML_ELEMENT_B:      case HTML_ELEMENT_BIG:
    case HTML_ELEMENT_CODE:  case HTML_ELEMENT_EM:     case HTML_ELEMENT_FONT:
    case HTML_ELEMENT_I:     case HTML_ELEMENT_NOBR:   case HTML_ELEMENT_S:
    case HTML_ELEMENT_SMALL: case HTML_ELEMENT_STRIKE: case HTML_ELEMENT_STRONG:
    case HTML_ELEMENT_TT:    case HTML_ELEMENT_U:
      return true;

    default:
      return false;
  }

}


/*
 * "in scope" algorithms; see 'html_parser/common.h' for details
 */
bool
TreeBuilder::scope_node_equals_(std::shared_ptr< DOM::Element> node,
                                std::shared_ptr< DOM::Element> target) const
{
  return (node == target);
}


bool
TreeBuilder::scope_node_equals_(std::shared_ptr< DOM::Element> node,
                                std::initializer_list< enum html_element_index> html_local_names) const
{
  if (node->name_space != INFRA_NAMESPACE_HTML)
    return false;

  for (uint16_t local_name : html_local_names)
    if (node->local_name == local_name)
      return true;

  return false;
}


/*
 * XXX: check if only HTML namespace
 */
bool
TreeBuilder::scope_batch_contains_(std::vector< std::pair< uint16_t, uint16_t>> const *list,
                                   std::shared_ptr< DOM::Element> elem) const
{
  for (const auto& [name_space, local_name] : *list)
    if ((name_space == INFRA_NAMESPACE_HTML)
     && elem->has_html_element_index(local_name))
      return true;

  return false;
}


bool
TreeBuilder::scope_batch_contains_(std::vector< std::pair< uint16_t, uint16_t>> const *list,
                                   std::initializer_list< enum html_element_index> html_local_names) const
{
  for (const auto& [name_space, local_name] : *list)
  for (const uint16_t cur_html_element : html_local_names)
    if (name_space == INFRA_NAMESPACE_HTML
     && local_name == cur_html_element)
      return false;

  return false;
}


template< ScopeTargetable T>
bool
TreeBuilder::have_target_node_in_scope_(std::vector< std::pair< uint16_t, uint16_t>> const *list,
                                        T targets) const
{

  for (const auto& node : std::ranges::views::reverse(this->open_elements))
  {
    if (this->scope_node_equals_(node, targets))
      return true;

    if (this->scope_batch_contains_(list, targets))
      return false;
  }

  // std::unreachable();
  return false;
}


/*
 * Only these can be instantiated
 */
template
bool TreeBuilder::have_target_node_in_scope_< std::shared_ptr< DOM::Element>>(
      std::vector< std::pair< uint16_t, uint16_t>> const *list,
      std::shared_ptr< DOM::Element> elem) const;
template
bool TreeBuilder::have_target_node_in_scope_< std::initializer_list< enum html_element_index>>(
      std::vector< std::pair< uint16_t, uint16_t>> const *list,
      std::initializer_list< enum html_element_index> html_local_names) const;
template
bool TreeBuilder::have_target_node_in_scope_< enum html_element_index>(
      std::vector< std::pair< uint16_t, uint16_t>> const *list,
      enum html_element_index html_local_name) const;


#define PARTICULAR_ELEMENT_SCOPE_DEF \
  { INFRA_NAMESPACE_HTML,   HTML_ELEMENT_APPLET           }, \
  { INFRA_NAMESPACE_HTML,   HTML_ELEMENT_CAPTION          }, \
  { INFRA_NAMESPACE_HTML,   HTML_ELEMENT_HTML             }, \
  { INFRA_NAMESPACE_HTML,   HTML_ELEMENT_TABLE            }, \
  { INFRA_NAMESPACE_HTML,   HTML_ELEMENT_TD               }, \
  { INFRA_NAMESPACE_HTML,   HTML_ELEMENT_TH               }, \
  { INFRA_NAMESPACE_HTML,   HTML_ELEMENT_MARQUEE          }, \
  { INFRA_NAMESPACE_HTML,   HTML_ELEMENT_OBJECT           }, \
  { INFRA_NAMESPACE_HTML,   HTML_ELEMENT_TEMPLATE         }

  // { INFRA_NAMESPACE_MATHML, MATHML_ELEMENT_MI             },
  // { INFRA_NAMESPACE_MATHML, MATHML_ELEMENT_MO             },
  // { INFRA_NAMESPACE_MATHML, MATHML_ELEMENT_MN             },
  // { INFRA_NAMESPACE_MATHML, MATHML_ELEMENT_MS             },
  // { INFRA_NAMESPACE_MATHML, MATHML_ELEMENT_MTEXT          },
  // { INFRA_NAMESPACE_MATHML, MATHML_ELEMENT_ANNOTATION_XML },
  // { INFRA_NAMESPACE_SVG,    SVG_ELEMENT_FOREIGN_OBJECT    },
  // { INFRA_NAMESPACE_SVG,    SVG_ELEMENT_DESC              },
  // { INFRA_NAMESPACE_SVG,    SVG_ELEMENT_TITLE             } 

const std::vector< std::pair< uint16_t, uint16_t>> TreeBuilder::k_particular_scope_def_ = {
  PARTICULAR_ELEMENT_SCOPE_DEF,
};

const std::vector< std::pair< uint16_t, uint16_t>> TreeBuilder::k_list_scope_def_ = {
  PARTICULAR_ELEMENT_SCOPE_DEF,
  { INFRA_NAMESPACE_HTML, HTML_ELEMENT_OL },
  { INFRA_NAMESPACE_HTML, HTML_ELEMENT_UL },
};

const std::vector< std::pair< uint16_t, uint16_t>> TreeBuilder::k_button_scope_def_ = {
  PARTICULAR_ELEMENT_SCOPE_DEF,
  { INFRA_NAMESPACE_HTML, HTML_ELEMENT_BUTTON },
};

const std::vector< std::pair< uint16_t, uint16_t>> TreeBuilder::k_table_scope_def_ = {
  { INFRA_NAMESPACE_HTML, HTML_ELEMENT_HTML     },
  { INFRA_NAMESPACE_HTML, HTML_ELEMENT_TABLE    },
  { INFRA_NAMESPACE_HTML, HTML_ELEMENT_TEMPLATE },
};

const std::vector< std::pair< uint16_t, uint16_t>> TreeBuilder::k_select_scope_def_ = {
 { INFRA_NAMESPACE_HTML, HTML_ELEMENT_OPTGROUP },
 { INFRA_NAMESPACE_HTML, HTML_ELEMENT_OPTION   },
};

#undef PARTICULAR_ELEMENT_SCOPE_DEF


void
TreeBuilder::push_formatting_marker(void)
{
  this->formatting_elements.push_back(this->FORMATTING_MARKER);
}


bool
TreeBuilder::same_parsed_elements(std::shared_ptr< DOM::Element> lhs,
                                  std::shared_ptr< DOM::Element> rhs) const
{
  if (! (lhs->name_space == rhs->name_space
      && lhs->local_name == rhs->local_name))
    return false;

  /* XXX: compare attributes */

  return true;
}


void
TreeBuilder::push_to_active_formatting_elements(std::shared_ptr< DOM::Element> element)
{
  /* Step 1. */
  std::shared_ptr< DOM::Element> fitting_entry = nullptr;
  int n_fitting_entries = 0;

  for (const auto &entry : std::ranges::views::reverse(this->formatting_elements))
  {
    if (entry == this->FORMATTING_MARKER)
      break;

    if (this->same_parsed_elements(entry, element)) {
      n_fitting_entries++;
      fitting_entry = entry;
    }

  }

  if (n_fitting_entries >= 3)
    this->formatting_elements.remove(fitting_entry);

  /* Step 2. */
  this->formatting_elements.push_back(element);
}


void
TreeBuilder::reconstruct_active_formatting_elements(void)
{
  /* Step 1. */
  if (this->formatting_elements.empty())
    return;

  /* Step 2. */
  auto& stack = this->open_elements;

  if (this->formatting_elements.back() == this->FORMATTING_MARKER
   || std::find(stack.begin(), stack.end(), this->formatting_elements.back()) != stack.end())
    return;

  /* Step 3. */
  auto entry_it = std::prev(this->formatting_elements.end());

  /* Step 4. */
rewind:
  if (entry_it == this->formatting_elements.begin())
    goto create;

  /* Step 5. */
  entry_it--;

  /* Step 6. */
  if (*entry_it != this->FORMATTING_MARKER
   && std::find(stack.begin(), stack.end(), *entry_it) == stack.end())
    goto rewind;

  /* Step 7. */
advance:
  entry_it++;

  /* Step 8. */
create:
  struct tag_token *tag = static_cast<struct tag_token *>((*entry_it)->parser_token);
  std::shared_ptr< DOM::Element> new_element = std::dynamic_pointer_cast<DOM::Element>(
   this->insert_html_element(tag));

  /* Step 9. */
  (*entry_it)->parser_token = nullptr;
  // delete tag;
  *entry_it = new_element;
  auto new_element_it = entry_it;


  /* Step 10. */
  if (new_element_it != std::prev(this->formatting_elements.end()))
    goto advance;
}


void
TreeBuilder::clear_active_formatting_elements_to_marker(void)
{

  while (true)  {
    /* Step 1. */
    auto entry_it = std::prev(this->formatting_elements.end());

    /* Step 2. */
    std::shared_ptr< DOM::Element> entry = *entry_it;
    struct tag_token *tag = static_cast<struct tag_token *>(entry->parser_token);

    entry->parser_token = nullptr;
    delete tag;

    this->formatting_elements.erase(entry_it);

    /* Step 3. */
    if (entry == this->FORMATTING_MARKER)
      return;

    /* Step 4. LOOP */
  }

}


void
TreeBuilder::acknowledge_self_closing_flag(struct tag_token *tag) const
{
  if (tag->self_closing_flag)
    tag->ack_self_closing_flag_ = true;
}


InsertionLocation
TreeBuilder::appropriate_insertion_place(std::shared_ptr< DOM::Element> override_target)
{
  InsertionLocation location = { nullptr, nullptr };

  std::shared_ptr< DOM::Element> target = override_target != nullptr
                                       ? override_target
                                       : this->current_node();

  if (this->flags.foster_parenting
   && (target->has_html_element_index(HTML_ELEMENT_TABLE)
    || target->has_html_element_index(HTML_ELEMENT_TBODY)
    || target->has_html_element_index(HTML_ELEMENT_TFOOT)
    || target->has_html_element_index(HTML_ELEMENT_THEAD)
    || target->has_html_element_index(HTML_ELEMENT_TR))) {
    std::shared_ptr< DOM::Element> last_template = nullptr;
    std::shared_ptr< DOM::Element> last_table    = nullptr;
    int last_template_idx = -1;
    int last_table_idx    = -1;

    for (int i = 0; i < static_cast<long int>(this->open_elements.size()); i++) {
      auto node = this->open_elements[i];

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
      location.parent = std::dynamic_pointer_cast<DOM::Node>(this->open_elements.front());
      location.child  = nullptr;
      goto sanitize;
    }


    std::shared_ptr< DOM::Element> prev_elem = this->open_elements[last_table_idx + 1];

    location.parent = std::dynamic_pointer_cast<DOM::Node>(prev_elem);
    location.child  = nullptr;

  } else {
    location.parent = target;
    location.child  = nullptr;
  }


sanitize:
  if (location.parent->is_element()
   && std::dynamic_pointer_cast<DOM::Element>(location.parent)->has_html_element_index(HTML_ELEMENT_TEMPLATE)) {
    /* XXX: template contents */
  }


  return location;
}


[[nodiscard]]
std::shared_ptr< DOM::Element>
TreeBuilder::create_element_for_token(struct tag_token const *tag,
                                      enum InfraNamespace name_space,
                                      std::shared_ptr< DOM::Node> intended_parent)
{
  std::shared_ptr< DOM::Document> document = intended_parent->node_document.lock();
  int16_t local_name = 0;
  void *is = nullptr;
  void *definition = nullptr;

  bool exec_script = (definition != nullptr && this->context == nullptr);

  assert( name_space == INFRA_NAMESPACE_HTML );
  fprintf(stderr, "tag_name: %s\n", tag->tag_name.c_str());
  local_name = HTML::k_local_names_table.at(tag->tag_name);
  /* ... */

  std::shared_ptr< DOM::Element> element =
   document->create_element(local_name, name_space, nullptr, is, exec_script);

  /* ... */


  /*
   * Needed for the list of active formatting elements
   */
  if (this->is_formatting_element(element))
    element->parser_token = static_cast<void *>(new tag_token(*tag));

  return element;
}


std::shared_ptr< DOM::Node>
TreeBuilder::node_before(InsertionLocation location)
{
  /*
   * XXX: very sus function
   */

  if (location.parent != nullptr
   && location.child != nullptr)
    return location.child->get_previous_sibling();

  if (location.parent != nullptr
   && location.child == nullptr)
    return location.parent->get_previous_sibling();

  return nullptr;
}


void
TreeBuilder::insert_element_at_location(InsertionLocation location,
                                        std::shared_ptr< DOM::Element> element) const
{
  location.parent->insert_node(std::dynamic_pointer_cast<DOM::Node>(element),
   location.child);
}


void
TreeBuilder::insert_element_at_adjusted_insertion_location(std::shared_ptr< DOM::Element> element)
{
  InsertionLocation location = this->appropriate_insertion_place();
  /* XXX: check if can insert */

  /* XXX: custom element reactions */

  this->insert_element_at_location(location, element);

  /* XXX: invoke custom element reactions */
}


std::shared_ptr< DOM::Element>
TreeBuilder::insert_foreign_element(struct tag_token const *tag,
                                    enum InfraNamespace name_space,
                                    bool only_add_to_element_stack)
{
  InsertionLocation location = this->appropriate_insertion_place();

  std::shared_ptr< DOM::Element> element =
   this->create_element_for_token(tag, name_space, location.parent);

  if (!only_add_to_element_stack)
    (void)0; /* XXX */

  this->open_elements.push_back(element);

  return element;
}


std::shared_ptr< DOM::Element>
TreeBuilder::insert_html_element(struct tag_token const *tag)
{
  return this->insert_foreign_element(tag, INFRA_NAMESPACE_HTML, false);
}


void
TreeBuilder::insert_character_array_(char32_t const *arr, size_t arr_len)
{
  InsertionLocation location = this->appropriate_insertion_place();

  if (location.parent->is_document())
    return;

  std::shared_ptr< DOM::Node> prev_sibling = TreeBuilder::node_before(location);

#if 0
  LOGF("{ %p, %p }\n",
    reinterpret_cast<void *>(location.parent.get()),
    reinterpret_cast<void *>(location.child.get()));
#endif

  std::shared_ptr< DOM::Text> text = nullptr;

  if ((prev_sibling != nullptr) && prev_sibling->is_text()) {
    text = std::dynamic_pointer_cast<DOM::Text>(prev_sibling);
  } else {
    text = std::make_shared<DOM::Text>(this->document);
    location.parent->insert_node(std::dynamic_pointer_cast<DOM::Node>(text),
     location.child);
  }

  for (int i = 0; i < static_cast<long int>(arr_len); i++)
    QueequegLib::append_c32_as_utf8(&text->data, arr[i]);

}


void
TreeBuilder::insert_characters(std::vector< char32_t> const *vch)
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
                            InsertionLocation location)
{
  std::shared_ptr< DOM::Comment> comment =
   std::make_shared<DOM::Comment>(location.parent->node_document.lock(),
                                 *data);

  location.parent->insert_node(std::dynamic_pointer_cast<DOM::Node>(comment),
   location.child);

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
    const std::shared_ptr< DOM::Element> cur_node = this->current_node();


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

