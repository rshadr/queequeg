/*
 * Copyright 2024 Adrien Ricciardi
 *
 * This file is part of the queequeg distribution available at:
 *  https://github.com/rshadr/queequeg
 *
 * See LICENSE for details
 *
 *
 * File: html_parser/insertion_modes.cc
 *
 */
#include <utility>
#include <ranges>

#include <stdio.h>

#include "dom/core/document_type.hh"
#include "dom/html/html_html_element.hh"
#include "html/elements.hh"

#include "html_parser/common.hh"


#define LOGF(...) \
  fprintf(stderr, __VA_ARGS__)



/*
 * Forward declarations
 */

static enum treebuilder_status initial_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status before_html_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status before_head_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_head_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_head_noscript_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status after_head_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_body_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status text_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_table_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_table_text_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_caption_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_column_group_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_table_body_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_row_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_cell_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_select_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_select_in_table_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_template_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status after_body_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_frameset_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status after_frameset_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status after_after_body_mode(TreeBuilder *, union token_data *, enum token_type);
static enum treebuilder_status in_foreign_content_mode(TreeBuilder *, union token_data *, enum token_type);



static enum treebuilder_status
initial_mode(TreeBuilder *treebuilder,
             union token_data *token_data,
             enum token_type token_type)
{
  LOGF("initial mode\n");
  
  if (token_type == TOKEN_WHITESPACE) {
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment,
     InsertionLocation{std::dynamic_pointer_cast<DOM_Node>(treebuilder->document), nullptr});

    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    struct doctype_token *token = &token_data->doctype;

    if ((token->name.compare("html") != 0)
     || !token->public_id_missing
     || (!token->system_id_missing
      && (token->system_id.compare("about:legacy-compat"))) != 0)
      treebuilder->error();

    std::shared_ptr< DOM_DocumentType> doctype = std::make_shared<DOM_DocumentType>(treebuilder->document);

    doctype->name = token->name;

    if (!token->public_id_missing)
      doctype->public_id = token->public_id;

    if (!token->system_id_missing)
      doctype->system_id = token->system_id;

    treebuilder->document->append_node(doctype);

    /* XXX: ... */
    LOGF("doctype node_document: %p\n", static_cast<void *>(doctype->node_document.lock().get()));

    treebuilder->mode = BEFORE_HTML_MODE;

    return TREEBUILDER_STATUS_OK;
  }


  {
    /* ... */
    treebuilder->mode = BEFORE_HTML_MODE;
    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static enum treebuilder_status
before_html_mode(TreeBuilder *treebuilder,
                 union token_data *token_data,
                 enum token_type token_type)
{
  LOGF("before html mode\n");

  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();

    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment,
     InsertionLocation{ std::dynamic_pointer_cast<DOM_Node>(treebuilder->document), nullptr });

    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_WHITESPACE) {
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;


    switch (tag->local_name)
    {
      case HTML_ELEMENT_HTML: {
        std::shared_ptr< DOM_HTMLHtmlElement> html =
         std::make_shared< DOM_HTMLHtmlElement>(treebuilder->document);

        html->name_space = INFRA_NAMESPACE_HTML;
        html->local_name = HTML_ELEMENT_HTML;

        treebuilder->document->append_node(html);

        treebuilder->open_elements.push_back(html);

        treebuilder->mode = BEFORE_HEAD_MODE;
        return TREEBUILDER_STATUS_OK;
      }

      default:
        break;
    }

    goto anything_else;
  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;


    switch (tag->local_name)
    {
      case HTML_ELEMENT_HEAD: case HTML_ELEMENT_BODY: case HTML_ELEMENT_HTML:
      case HTML_ELEMENT_BR:
        goto anything_else;

      default: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }
    }

  }


  anything_else: {
    std::shared_ptr< DOM_HTMLHtmlElement> html =
     std::make_shared< DOM_HTMLHtmlElement>(treebuilder->document);

    html->name_space = INFRA_NAMESPACE_HTML;
    html->local_name = HTML_ELEMENT_HTML;

    treebuilder->document->append_node(html);

    treebuilder->open_elements.push_back(html);

    treebuilder->mode = BEFORE_HEAD_MODE;
    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static enum treebuilder_status
before_head_mode(TreeBuilder *treebuilder,
                 union token_data *token_data,
                 enum token_type token_type)
{
  LOGF("before head mode\n");

  if (token_type == TOKEN_WHITESPACE) {
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;


    switch (tag->local_name)
    {
      case HTML_ELEMENT_HTML: {
        return in_body_mode(treebuilder, token_data, token_type);
      }


      case HTML_ELEMENT_HEAD: {
        std::shared_ptr< DOM_HTMLHeadElement> head =
         std::dynamic_pointer_cast<DOM_HTMLHeadElement>(treebuilder->insert_html_element(tag));

        treebuilder->head = head;

        LOGF("head use count: %p, %ld\n", static_cast<void *>(head.get()), head.use_count());

        treebuilder->mode = IN_HEAD_MODE;
        return TREEBUILDER_STATUS_OK;
      }


      default:
        break;
    }

    goto anything_else;
  }


  anything_else: {
    static const struct tag_token dummy_token = {
      .tag_name = "head",
      .local_name = HTML_ELEMENT_HEAD,
      .self_closing_flag = false,
      .ack_self_closing_flag_ = false,
    };

    std::shared_ptr< DOM_HTMLHeadElement> head =
      std::dynamic_pointer_cast<DOM_HTMLHeadElement>(treebuilder->insert_html_element(&dummy_token));

    treebuilder->head = head;

    treebuilder->mode = IN_HEAD_MODE;
    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static enum treebuilder_status
in_head_mode(TreeBuilder *treebuilder,
             union token_data *token_data,
             enum token_type token_type)
{
  LOGF("in head mode\n");

  if (token_type == TOKEN_WHITESPACE) {
    treebuilder->insert_character(token_data->ch);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;


    switch (tag->local_name)
    {
      case HTML_ELEMENT_HTML: {
        return in_body_mode(treebuilder, token_data, token_type);
      }


      case HTML_ELEMENT_BASE: case HTML_ELEMENT_BASEFONT: case HTML_ELEMENT_BGSOUND:
      case HTML_ELEMENT_LINK: {
        treebuilder->insert_html_element(tag);

        treebuilder->open_elements.pop_back();

        treebuilder->acknowledge_self_closing_flag(tag);

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_META: {
        treebuilder->insert_html_element(tag);

        treebuilder->open_elements.pop_back();

        treebuilder->acknowledge_self_closing_flag(tag);

        /* ... */

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_TITLE: {
        return treebuilder->generic_rcdata_parse(tag);
      }


      case HTML_ELEMENT_NOSCRIPT: {
        if (treebuilder->flags.scripting) {
          return treebuilder->generic_raw_text_parse(tag);
        } else {
          treebuilder->insert_html_element(tag);
          treebuilder->mode = IN_HEAD_NOSCRIPT_MODE;
          return TREEBUILDER_STATUS_OK;
        }
      }


      case HTML_ELEMENT_NOFRAMES: case HTML_ELEMENT_STYLE: {
        return treebuilder->generic_raw_text_parse(tag);
      }


      case HTML_ELEMENT_SCRIPT: {
        /* XXX: ... */

        treebuilder->tokenizer->state = SCRIPT_STATE;
        treebuilder->original_mode = treebuilder->mode;
        treebuilder->mode = TEXT_MODE;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_TEMPLATE: {
        /* XXX: ... */
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_HEAD: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }


      default:
        break;
    }

    goto anything_else;
  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;


    switch (tag->local_name)
    {
      case HTML_ELEMENT_HEAD: {
        treebuilder->open_elements.pop_back();
        treebuilder->mode = AFTER_HEAD_MODE;
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_BODY: case HTML_ELEMENT_HTML: case HTML_ELEMENT_BR:
        goto anything_else;


      case HTML_ELEMENT_TEMPLATE: {
        /* XXX: ... */
        return TREEBUILDER_STATUS_OK;
      }


      default: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }

    }

  }


  anything_else: {
    treebuilder->open_elements.pop_back();

    treebuilder->mode = AFTER_HEAD_MODE;

    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static enum treebuilder_status
in_head_noscript_mode(TreeBuilder *treebuilder,
                      union token_data *token_data,
                      enum token_type token_type)
{
  LOGF("in head noscript mode");

  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name) {

      case HTML_ELEMENT_HTML: {
        return in_body_mode(treebuilder, token_data, token_type);
      }


      case HTML_ELEMENT_BASEFONT: case HTML_ELEMENT_BGSOUND:  case HTML_ELEMENT_LINK:
      case HTML_ELEMENT_META:     case HTML_ELEMENT_NOFRAMES: case HTML_ELEMENT_STYLE: {
        return in_head_mode(treebuilder, token_data, token_type);
      }


      case HTML_ELEMENT_HEAD: case HTML_ELEMENT_NOSCRIPT: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }


      default:
        goto anything_else;
    }

  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name) {

      case HTML_ELEMENT_NOSCRIPT: {
        treebuilder->open_elements.pop_back();

        treebuilder->mode = IN_HEAD_MODE;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_BR:
        goto anything_else;


      default: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }

    }

  }


  if (token_type == TOKEN_WHITESPACE) {
    return in_head_mode(treebuilder, token_data, token_type);
  }


  if (token_type == TOKEN_COMMENT) {
    return in_head_mode(treebuilder, token_data, token_type);
  }


  anything_else: {
    treebuilder->error();

    treebuilder->open_elements.pop_back();

    treebuilder->mode = IN_HEAD_MODE;

    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static enum treebuilder_status
after_head_mode(TreeBuilder *treebuilder,
                union token_data *token_data,
                enum token_type token_type)
{
  LOGF("after head mode\n");

  if (token_type == TOKEN_WHITESPACE) {
    treebuilder->insert_character(token_data->ch);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name) {

      case HTML_ELEMENT_HTML: {
        return in_body_mode(treebuilder, token_data, token_type);
      }


      case HTML_ELEMENT_BODY: {
        treebuilder->insert_html_element(tag);

        treebuilder->flags.frameset_ok = false;

        treebuilder->mode = IN_BODY_MODE;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_FRAMESET: {
        treebuilder->insert_html_element(tag);

        treebuilder->mode = IN_FRAMESET_MODE;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_BASE:   case HTML_ELEMENT_BASEFONT: case HTML_ELEMENT_BGSOUND:
      case HTML_ELEMENT_LINK:   case HTML_ELEMENT_META:     case HTML_ELEMENT_NOFRAMES:
      case HTML_ELEMENT_SCRIPT: case HTML_ELEMENT_STYLE:    case HTML_ELEMENT_TEMPLATE:
      case HTML_ELEMENT_TITLE: {
        treebuilder->error();

        treebuilder->open_elements.push_back(treebuilder->head);

        enum treebuilder_status rc = in_head_mode(treebuilder, token_data, token_type);

        std::erase(treebuilder->open_elements, treebuilder->head);

        return rc;
      }


      case HTML_ELEMENT_HEAD: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }


      default: {
        goto anything_else;
      }

    }

  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name) {

      case HTML_ELEMENT_TEMPLATE: {
        return in_head_mode(treebuilder, token_data, token_type);
      }


      case HTML_ELEMENT_BODY: case HTML_ELEMENT_HTML: case HTML_ELEMENT_BR:
        goto anything_else;


      default: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }

    }

  }


  anything_else: {
    static const struct tag_token dummy_token = {
      .tag_name = "body",
      .local_name = HTML_ELEMENT_BODY,
      .self_closing_flag = false,
      .ack_self_closing_flag_ = false,
    };

    LOGF("hello darkness\n");
    treebuilder->insert_html_element(&dummy_token);

    treebuilder->mode = IN_BODY_MODE;
    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static void
close_p_element(TreeBuilder *treebuilder)
{
  treebuilder->generate_implied_end_tags(HTML_ELEMENT_P);

  if (!treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_P))
    treebuilder->error();

  std::shared_ptr< DOM_Element> popped;

  do {
    popped = treebuilder->open_elements.back();
    treebuilder->open_elements.pop_back();
  } while (!popped->has_html_element_index(HTML_ELEMENT_P));

}


static enum treebuilder_status
in_body_mode(TreeBuilder *treebuilder,
             union token_data *token_data,
             enum token_type token_type)
{
  LOGF("in body mode\n");

  /*
   * Even if not accessed, the variable is created here so that goto jumps
   * can be done properly, i.e. this shouldn't result in a segfault.
   */
  struct tag_token *tag = &token_data->tag;

  if (token_type == TOKEN_CHARACTER) {
    if (token_data->ch == U'\0') {
      treebuilder->error();
      return TREEBUILDER_STATUS_IGNORE;
    }

    treebuilder->reconstruct_active_formatting_elements();
    treebuilder->insert_character(token_data->ch);
    treebuilder->flags.frameset_ok = false;

    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_WHITESPACE) {
    treebuilder->reconstruct_active_formatting_elements();
    treebuilder->insert_character(token_data->ch);

    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_START_TAG) {

    switch (tag->local_name) {

      case HTML_ELEMENT_HTML: {
        treebuilder->error();

        /* XXX: if there is a template element... */
        /* ... otherwise */

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_BASE:   case HTML_ELEMENT_BASEFONT: case HTML_ELEMENT_BGSOUND:
      case HTML_ELEMENT_LINK:   case HTML_ELEMENT_META:     case HTML_ELEMENT_NOFRAMES:
      case HTML_ELEMENT_SCRIPT: case HTML_ELEMENT_STYLE:    case HTML_ELEMENT_TEMPLATE:
      case HTML_ELEMENT_TITLE: {
        return in_head_mode(treebuilder, token_data, token_type);
      }


      case HTML_ELEMENT_BODY: {
        treebuilder->error();

        /* ... */

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_FRAMESET: {
        treebuilder->error();

        /* XXX: ignore */

        if (!treebuilder->flags.frameset_ok)
          return TREEBUILDER_STATUS_IGNORE;

        /* XXX: remove from parent node */
        while (treebuilder->open_elements.size() > 1)
          treebuilder->open_elements.pop_back();

        treebuilder->insert_html_element(tag);

        treebuilder->mode = IN_FRAMESET_MODE;
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_ADDRESS:    case HTML_ELEMENT_ARTICLE:  case HTML_ELEMENT_ASIDE:
      case HTML_ELEMENT_BLOCKQUOTE: case HTML_ELEMENT_CENTER:   case HTML_ELEMENT_DETAILS:
      case HTML_ELEMENT_DIALOG:     case HTML_ELEMENT_DIR:      case HTML_ELEMENT_DIV:
      case HTML_ELEMENT_DL:         case HTML_ELEMENT_FIELDSET: case HTML_ELEMENT_FIGCAPTION:
      case HTML_ELEMENT_FIGURE:     case HTML_ELEMENT_FOOTER:   case HTML_ELEMENT_HEADER:
      case HTML_ELEMENT_HGROUP:     case HTML_ELEMENT_MAIN:     case HTML_ELEMENT_MENU:
      case HTML_ELEMENT_NAV:        case HTML_ELEMENT_OL:       case HTML_ELEMENT_P:
      case HTML_ELEMENT_SEARCH:     case HTML_ELEMENT_SECTION:  case HTML_ELEMENT_SUMMARY:
      case HTML_ELEMENT_UL: {
        /* XXX: close a p element */
        LOGF("yes?\n");
        treebuilder->insert_html_element(tag);
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_H1: case HTML_ELEMENT_H2: case HTML_ELEMENT_H3:
      case HTML_ELEMENT_H4: case HTML_ELEMENT_H5: case HTML_ELEMENT_H6: {
        /* XXX: close a p element */

        if (treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_H1)
         || treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_H2)
         || treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_H3)
         || treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_H4)
         || treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_H5)
         || treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_H6))
          treebuilder->open_elements.pop_back();

        treebuilder->insert_html_element(tag);

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_PRE: case HTML_ELEMENT_LISTING: {
        /* XXX: close a p element */

        treebuilder->insert_html_element(tag);

        treebuilder->insert_html_element(tag);
        treebuilder->skip_newline = true;
        treebuilder->flags.frameset_ok = false;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_FORM: {
        /* XXX: ignore token */

        /* XXX: close p element */

        /* XXX insert html element, assign to form_ptr */

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_LI: {
        treebuilder->flags.frameset_ok = false;

        /* ... */

        treebuilder->insert_html_element(tag);
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_DD: case HTML_ELEMENT_DT: {
        treebuilder->flags.frameset_ok = false;

        /* ... */

        treebuilder->insert_html_element(tag);
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_PLAINTEXT: {
        /* XXX: close p element */
        treebuilder->insert_html_element(tag);

        treebuilder->tokenizer->state = PLAINTEXT_STATE;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_BUTTON: {
        /* XXX: in scope? steps.. */
        treebuilder->reconstruct_active_formatting_elements();
        treebuilder->insert_html_element(tag);
        treebuilder->flags.frameset_ok = false;
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_A: {
        /* XXX: long pain */

        treebuilder->reconstruct_active_formatting_elements();

        auto a = treebuilder->insert_html_element(tag);
        treebuilder->push_to_active_formatting_elements(a);

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_B:      case HTML_ELEMENT_BIG:   case HTML_ELEMENT_CODE:
      case HTML_ELEMENT_EM:     case HTML_ELEMENT_FONT:  case HTML_ELEMENT_I:
      case HTML_ELEMENT_S:      case HTML_ELEMENT_SMALL: case HTML_ELEMENT_STRIKE:
      case HTML_ELEMENT_STRONG: case HTML_ELEMENT_TT:    case HTML_ELEMENT_U: {
        treebuilder->reconstruct_active_formatting_elements();
        auto elem = treebuilder->insert_html_element(tag);
        treebuilder->push_to_active_formatting_elements(elem);
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_NOBR: {
        treebuilder->reconstruct_active_formatting_elements();

        /* XXX: ... */

        auto nobr = treebuilder->insert_html_element(tag);
        treebuilder->push_to_active_formatting_elements(nobr);

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_APPLET: case HTML_ELEMENT_MARQUEE: case HTML_ELEMENT_OBJECT: {
        treebuilder->reconstruct_active_formatting_elements();

        treebuilder->insert_html_element(tag);

        /* XXX: insert marker */

        treebuilder->flags.frameset_ok = false;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_TABLE: {
        /* XXX: close a p element */

        treebuilder->insert_html_element(tag);
        treebuilder->flags.frameset_ok = false;
        treebuilder->mode = IN_TABLE_MODE;

        return TREEBUILDER_STATUS_OK;
      }


      br_start_tag:
      case HTML_ELEMENT_AREA: case HTML_ELEMENT_BR:     case HTML_ELEMENT_EMBED:
      case HTML_ELEMENT_IMG:  case HTML_ELEMENT_KEYGEN: case HTML_ELEMENT_WBR: {
        treebuilder->reconstruct_active_formatting_elements();

        treebuilder->insert_html_element(tag);
        treebuilder->open_elements.pop_back();

        treebuilder->acknowledge_self_closing_flag(tag);
        treebuilder->flags.frameset_ok = false;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_INPUT: {
        treebuilder->reconstruct_active_formatting_elements();

        treebuilder->insert_html_element(tag);
        treebuilder->open_elements.pop_back();

        treebuilder->acknowledge_self_closing_flag(tag);
        /* XXX: lookup attr */

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_PARAM: {
        treebuilder->insert_html_element(tag);
        treebuilder->open_elements.pop_back();

        treebuilder->acknowledge_self_closing_flag(tag);

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_HR: {
        /* XXX: close a p element */
        treebuilder->insert_html_element(tag);
        treebuilder->open_elements.pop_back();

        treebuilder->acknowledge_self_closing_flag(tag);
        treebuilder->flags.frameset_ok = false;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_TEXTAREA: {
        treebuilder->insert_html_element(tag);

        treebuilder->skip_newline = true;

        treebuilder->tokenizer->state = RCDATA_STATE;
        treebuilder->original_mode = treebuilder->mode;
        treebuilder->flags.frameset_ok = false;

        treebuilder->mode = TEXT_MODE;
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_XMP: {
        /* XXX: close p element */

        treebuilder->reconstruct_active_formatting_elements();
        treebuilder->flags.frameset_ok = false;
        return treebuilder->generic_raw_text_parse(tag);
      }


      case HTML_ELEMENT_IFRAME: {
        treebuilder->flags.frameset_ok = false;
        return treebuilder->generic_raw_text_parse(tag);
      }


      case HTML_ELEMENT_NOEMBED: {
        return treebuilder->generic_raw_text_parse(tag);
      }


      case HTML_ELEMENT_NOSCRIPT: {
        if (treebuilder->flags.scripting) {
          return treebuilder->generic_raw_text_parse(tag);
        }
        return TREEBUILDER_STATUS_OK; /* XXX */
      }


      case HTML_ELEMENT_SELECT: {
        treebuilder->reconstruct_active_formatting_elements();
        treebuilder->insert_html_element(tag);
        treebuilder->flags.frameset_ok = false;

        switch (treebuilder->mode) {
          case IN_TABLE_MODE:
          case IN_CAPTION_MODE:
          case IN_TABLE_BODY_MODE:
          case IN_ROW_MODE:
          case IN_CELL_MODE:
            treebuilder->mode = IN_SELECT_IN_TABLE_MODE;
            break;

          default:
            treebuilder->mode = IN_SELECT_MODE;
            break;
        }

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_OPTGROUP: case HTML_ELEMENT_OPTION: {
        /* XXX: pop current_node */

        treebuilder->reconstruct_active_formatting_elements();
        treebuilder->insert_html_element(tag);

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_RB: case HTML_ELEMENT_RTC: {
        /* XXX: ... generate implied end tags */

        treebuilder->insert_html_element(tag);

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_RP: case HTML_ELEMENT_RT: {
        /* XXX: ... */

        treebuilder->insert_html_element(tag);

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_MATH_: {
        treebuilder->reconstruct_active_formatting_elements();
        /* XXX: adjust MathML attributes */
        /* XXX: adjust foreign attributes */

        treebuilder->insert_foreign_element(tag, INFRA_NAMESPACE_MATHML, false);

        if (tag->self_closing_flag) {
          treebuilder->open_elements.pop_back();
          treebuilder->acknowledge_self_closing_flag(tag);
        }

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_SVG_: {
        treebuilder->reconstruct_active_formatting_elements();

        /* XXX: adjust SVG attributes */
        /* XXX: adjust foreign attributes */

        treebuilder->insert_foreign_element(tag, INFRA_NAMESPACE_SVG, false);

        if (tag->self_closing_flag) {
          treebuilder->open_elements.pop_back();
          treebuilder->acknowledge_self_closing_flag(tag);
        }

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_CAPTION: case HTML_ELEMENT_COL:   case HTML_ELEMENT_COLGROUP:
      case HTML_ELEMENT_FRAME:   case HTML_ELEMENT_HEAD:  case HTML_ELEMENT_TBODY:
      case HTML_ELEMENT_TD:      case HTML_ELEMENT_TFOOT: case HTML_ELEMENT_TH:
      case HTML_ELEMENT_THEAD:   case HTML_ELEMENT_TR: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }


      default: {
        treebuilder->reconstruct_active_formatting_elements();
        treebuilder->insert_html_element(tag);

        return TREEBUILDER_STATUS_OK;
      }

    }


  }


  if (token_type == TOKEN_END_TAG) {

    switch (tag->local_name)
    {
      case HTML_ELEMENT_TEMPLATE: {
        return in_head_mode(treebuilder, token_data, token_type);
      }


      case HTML_ELEMENT_BODY: {
        /* XXX: in scope */
        /* XXX: in stack */

        treebuilder->mode = AFTER_BODY_MODE;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_HTML: {
        /* XXX: in scope */
        /* XXX: in stack */

        treebuilder->mode = AFTER_BODY_MODE;

        return TREEBUILDER_STATUS_REPROCESS;
      }


      case HTML_ELEMENT_ADDRESS:    case HTML_ELEMENT_ARTICLE: case HTML_ELEMENT_ASIDE:
      case HTML_ELEMENT_BLOCKQUOTE: case HTML_ELEMENT_BUTTON:  case HTML_ELEMENT_CENTER:
      case HTML_ELEMENT_DETAILS:    case HTML_ELEMENT_DIALOG:  case HTML_ELEMENT_DIR:
      case HTML_ELEMENT_DIV:        case HTML_ELEMENT_DL:      case HTML_ELEMENT_FIELDSET:
      case HTML_ELEMENT_FIGCAPTION: case HTML_ELEMENT_FIGURE:  case HTML_ELEMENT_FOOTER:
      case HTML_ELEMENT_HEADER:     case HTML_ELEMENT_HGROUP:  case HTML_ELEMENT_LISTING:
      case HTML_ELEMENT_MAIN:       case HTML_ELEMENT_MENU:    case HTML_ELEMENT_NAV:
      case HTML_ELEMENT_OL:         case HTML_ELEMENT_PRE:     case HTML_ELEMENT_SEARCH:
      case HTML_ELEMENT_SECTION:    case HTML_ELEMENT_SUMMARY: case HTML_ELEMENT_UL: {
        /* XXX: in scope; ignore */

        treebuilder->generate_implied_end_tags();

        if (!treebuilder->current_node()->has_html_element_index(tag->local_name))
          treebuilder->error();

        std::shared_ptr< DOM_Element> popped;

        do {
          LOGF("open_elements has %d elements\n", static_cast<int>(treebuilder->open_elements.size()));
          popped = treebuilder->open_elements.back();
          LOGF("popped: %d\n", static_cast<int>(popped->local_name));
          treebuilder->open_elements.pop_back();
        }  while (!popped->has_html_element_index(tag->local_name));

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_FORM: {
        /* XXX: much things. */
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_P: {
        static const struct tag_token dummy_token = {
          .tag_name = "p",
          .local_name = HTML_ELEMENT_P,
          .self_closing_flag = false,
          .ack_self_closing_flag_ = false,
        };

        /* XXX: insert? */

        close_p_element(treebuilder);

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_LI: {
        /* XXX: ignore? */

        treebuilder->generate_implied_end_tags(HTML_ELEMENT_LI);

        if (!treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_LI))
          treebuilder->error();

        std::shared_ptr< DOM_Element> popped;

        do {
          popped = treebuilder->open_elements.back();
          treebuilder->open_elements.pop_back();
        } while (!popped->has_html_element_index(HTML_ELEMENT_LI));

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_DD: case HTML_ELEMENT_DT: {
        /* XXX: ignore? */

        treebuilder->generate_implied_end_tags(tag->local_name);

        if (!treebuilder->current_node()->has_html_element_index(tag->local_name))
          treebuilder->error();

        std::shared_ptr< DOM_Element> popped;

        do {
          popped = treebuilder->open_elements.back();
          treebuilder->open_elements.pop_back();
        } while (!popped->has_html_element_index(tag->local_name));

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_H1: case HTML_ELEMENT_H2: case HTML_ELEMENT_H3:
      case HTML_ELEMENT_H4: case HTML_ELEMENT_H5: case HTML_ELEMENT_H6: {
        /* XXX: in scope ... */

        treebuilder->generate_implied_end_tags();

        if (!treebuilder->current_node()->has_html_element_index(tag->local_name))
          treebuilder->error();

        std::shared_ptr< DOM_Element> popped;

        do {
          popped = treebuilder->open_elements.back();
          treebuilder->open_elements.pop_back();
        } while (!(popped->has_html_element_index(HTML_ELEMENT_H1)
                || popped->has_html_element_index(HTML_ELEMENT_H2)
                || popped->has_html_element_index(HTML_ELEMENT_H3)
                || popped->has_html_element_index(HTML_ELEMENT_H4)
                || popped->has_html_element_index(HTML_ELEMENT_H5)
                || popped->has_html_element_index(HTML_ELEMENT_H6)));
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_A: case HTML_ELEMENT_B: case HTML_ELEMENT_BIG:
      case HTML_ELEMENT_CODE: case HTML_ELEMENT_EM: case HTML_ELEMENT_FONT:
      case HTML_ELEMENT_I: case HTML_ELEMENT_NOBR: case HTML_ELEMENT_S:
      case HTML_ELEMENT_SMALL: case HTML_ELEMENT_STRIKE: case HTML_ELEMENT_STRONG:
      case HTML_ELEMENT_TT: case HTML_ELEMENT_U: {
        /* XXX: run adoption agency algorithm */
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_APPLET: case HTML_ELEMENT_MARQUEE: case HTML_ELEMENT_OBJECT: {
        /* XXX: in scope ... */

        treebuilder->generate_implied_end_tags();

        if (!treebuilder->current_node()->has_html_element_index(tag->local_name))
          treebuilder->error();

        std::shared_ptr< DOM_Element> popped;

        do {
          popped = treebuilder->open_elements.back();
          treebuilder->open_elements.pop_back();
        } while (!(popped->has_html_element_index(tag->local_name)));

        /* XXX: clear list of active formatting elements to last marker */

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_BR: {
        treebuilder->error();
        /* XXX: drop attributes on token */

        goto br_start_tag;
      }


      default: {

        for (std::shared_ptr< DOM_Element> node : std::ranges::views::reverse(treebuilder->open_elements))
        {
          if (node->name_space == INFRA_NAMESPACE_HTML
           && node->local_name == tag->local_name) {

            treebuilder->generate_implied_end_tags(tag->local_name);

            if (node != treebuilder->current_node())
              treebuilder->error();

            std::shared_ptr< DOM_Element> popped;

            do {
              popped = treebuilder->open_elements.back();
              treebuilder->open_elements.pop_back();
            } while (popped != node);

            break;
          }


          /* XXX: is special? */
        }

        // std::unreachable();
        return TREEBUILDER_STATUS_OK;
      }

    }

  }


  // std::unreachable();
  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
text_mode(TreeBuilder *treebuilder,
          union token_data *token_data,
          enum token_type token_type)
{
  LOGF("text mode\n");

  if (token_type == TOKEN_CHARACTER || token_type == TOKEN_WHITESPACE) {
    treebuilder->insert_character(token_data->ch);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_EOF) {
    treebuilder->error();

    /* XXX: already started */

    treebuilder->open_elements.pop_back();

    treebuilder->mode = treebuilder->original_mode;

    return TREEBUILDER_STATUS_REPROCESS;
  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;


    switch (tag->local_name) {

      case HTML_ELEMENT_SCRIPT: {
        /* XXX: hell... */
        treebuilder->open_elements.pop_back();
        treebuilder->mode = treebuilder->original_mode;
        /* XXX: hell... */
        return TREEBUILDER_STATUS_OK;
      }

      default: {
        treebuilder->open_elements.pop_back();
        treebuilder->mode = treebuilder->original_mode;
        return TREEBUILDER_STATUS_OK;
      }

    }

  }


  printf("what???\n");
  // std::unreached();
  return TREEBUILDER_STATUS_OK;
}


static void
clear_stack_to_table_context(TreeBuilder *treebuilder)
{
  while (!((treebuilder->current_node()->name_space == INFRA_NAMESPACE_HTML)
        && (treebuilder->current_node()->local_name == HTML_ELEMENT_TABLE
         || treebuilder->current_node()->local_name == HTML_ELEMENT_TEMPLATE
         || treebuilder->current_node()->local_name == HTML_ELEMENT_HTML)))
    treebuilder->open_elements.pop_back();
}


static enum treebuilder_status
in_table_mode(TreeBuilder *treebuilder,
              union token_data *token_data,
              enum token_type token_type)
{
  LOGF("in table mode\n");

  if (token_type == TOKEN_CHARACTER || token_type == TOKEN_WHITESPACE) {
    if ((treebuilder->current_node()->name_space == INFRA_NAMESPACE_HTML)
     && (treebuilder->current_node()->local_name == HTML_ELEMENT_TABLE
      || treebuilder->current_node()->local_name == HTML_ELEMENT_TBODY
      || treebuilder->current_node()->local_name == HTML_ELEMENT_TEMPLATE
      || treebuilder->current_node()->local_name == HTML_ELEMENT_TFOOT
      || treebuilder->current_node()->local_name == HTML_ELEMENT_THEAD
      || treebuilder->current_node()->local_name == HTML_ELEMENT_TR)) {
      treebuilder->pending_table_characters.clear();

      treebuilder->original_mode = treebuilder->mode;

      treebuilder->mode = IN_TABLE_TEXT_MODE;

      return TREEBUILDER_STATUS_REPROCESS;
    }

    goto anything_else;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name)
    {
      case HTML_ELEMENT_CAPTION: {
        clear_stack_to_table_context(treebuilder);
        /* XXX: insert marker */
        treebuilder->insert_html_element(tag);
        treebuilder->mode = IN_CAPTION_MODE;
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_COLGROUP: {
        clear_stack_to_table_context(treebuilder);
        treebuilder->insert_html_element(tag);
        treebuilder->mode = IN_COLUMN_GROUP_MODE;
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_COL: {
        static const struct tag_token dummy_token = {
          .tag_name = "colgroup",
          .local_name = HTML_ELEMENT_COLGROUP,
          .self_closing_flag = false,
          .ack_self_closing_flag_ = false,
        };

        clear_stack_to_table_context(treebuilder);

        treebuilder->insert_html_element(&dummy_token);

        treebuilder->mode = IN_COLUMN_GROUP_MODE;

        return TREEBUILDER_STATUS_REPROCESS;
      }


      case HTML_ELEMENT_TBODY: case HTML_ELEMENT_TFOOT: case HTML_ELEMENT_THEAD: {
        clear_stack_to_table_context(treebuilder);

        treebuilder->mode = IN_TABLE_BODY_MODE;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_TD: case HTML_ELEMENT_TH: case HTML_ELEMENT_TR: {
        static const struct tag_token dummy_token = {
          .tag_name = "tbody",
          .local_name = HTML_ELEMENT_TBODY,
          .self_closing_flag = false,
          .ack_self_closing_flag_ = false,
        };

        clear_stack_to_table_context(treebuilder);

        treebuilder->insert_html_element(tag);
        treebuilder->mode = IN_TABLE_BODY_MODE;

        return TREEBUILDER_STATUS_REPROCESS;
      }


      case HTML_ELEMENT_TABLE: {
        treebuilder->error();
        /* XXX: table in scope */

        std::shared_ptr< DOM_Element> popped;

        do {
          popped = treebuilder->open_elements.back();
          treebuilder->open_elements.pop_back();
        } while (!(popped->name_space == INFRA_NAMESPACE_HTML
                && popped->local_name == HTML_ELEMENT_TABLE));

        treebuilder->reset_insertion_mode_appropriately();

        return TREEBUILDER_STATUS_REPROCESS;
      }


      case HTML_ELEMENT_STYLE: case HTML_ELEMENT_SCRIPT: case HTML_ELEMENT_TEMPLATE: {
        return in_head_mode(treebuilder, token_data, token_type);
      }


      case HTML_ELEMENT_INPUT: {
        /* XXX: "type" attribute */
        treebuilder->error();

        treebuilder->insert_html_element(tag);

        treebuilder->open_elements.pop_back();

        treebuilder->acknowledge_self_closing_flag(tag);

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_FORM: {
        treebuilder->error();

        /* XXX: template in stack */

        treebuilder->insert_html_element(tag);

        /* XXX: set form ptr */

        treebuilder->open_elements.pop_back();

        return TREEBUILDER_STATUS_OK;
      }


      default:
        break;
    }

    goto anything_else;
  }


  if (token_type == TOKEN_EOF) {
    return in_body_mode(treebuilder, token_data, token_type);
  }


  anything_else: {
    treebuilder->error();

    treebuilder->flags.foster_parenting = true;

    enum treebuilder_status rc = in_body_mode(treebuilder, token_data, token_type);

    treebuilder->flags.foster_parenting = false;

    return rc;
  }

}


static enum treebuilder_status
in_table_text_mode(TreeBuilder *treebuilder,
                   union token_data *token_data,
                   enum token_type token_type)
{
  LOGF("in table text mode\n");

  if (token_type == TOKEN_CHARACTER || token_type == TOKEN_WHITESPACE) {
    if (token_data->ch == U'\0') {
      treebuilder->error();
      return TREEBUILDER_STATUS_IGNORE;
    }

    treebuilder->pending_table_characters.push_back(token_data->ch);
    return TREEBUILDER_STATUS_OK;
  }


  /* anything_else: */ {
    /* XXX: ... */

    treebuilder->insert_characters(&treebuilder->pending_table_characters);

    treebuilder->mode = treebuilder->original_mode;

    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static enum treebuilder_status
in_caption_mode(TreeBuilder *treebuilder,
                union token_data *token_data,
                enum token_type token_type)
{
  LOGF("in caption mode\n");


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name)
    {

      case HTML_ELEMENT_CAPTION: {
        /* XXX: ... in table scope */

        treebuilder->generate_implied_end_tags();

        if (!treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_CAPTION))
          treebuilder->error();

        std::shared_ptr< DOM_Element> popped;

        do {
          popped = treebuilder->open_elements.back();
          treebuilder->open_elements.pop_back();
        } while (!popped->has_html_element_index(HTML_ELEMENT_CAPTION));

        /* XXX: clear to last marker */

        treebuilder->mode = IN_TABLE_MODE;
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_TABLE: {
        /* XXX: ... in table scope */

        treebuilder->generate_implied_end_tags();

        if (!treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_CAPTION))
          treebuilder->error();

        std::shared_ptr< DOM_Element> popped;

        do {
          popped = treebuilder->open_elements.back();
          treebuilder->open_elements.pop_back();
        } while (!popped->has_html_element_index(HTML_ELEMENT_CAPTION));

        /* XXX: clear to last marker */

        treebuilder->mode = IN_TABLE_MODE;
        return TREEBUILDER_STATUS_REPROCESS;
      }


      case HTML_ELEMENT_BODY:  case HTML_ELEMENT_COL:   case HTML_ELEMENT_COLGROUP:
      case HTML_ELEMENT_HTML:  case HTML_ELEMENT_TBODY: case HTML_ELEMENT_TD:
      case HTML_ELEMENT_TFOOT: case HTML_ELEMENT_TH:    case HTML_ELEMENT_THEAD:
      case HTML_ELEMENT_TR: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }


      default:
        break;
    }


    goto anything_else;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;


    switch (tag->local_name)
    {
      case HTML_ELEMENT_CAPTION: case HTML_ELEMENT_COL:   case HTML_ELEMENT_COLGROUP:
      case HTML_ELEMENT_TBODY:   case HTML_ELEMENT_TD:    case HTML_ELEMENT_TFOOT:
      case HTML_ELEMENT_TH:      case HTML_ELEMENT_THEAD: case HTML_ELEMENT_TR: {
        /* XXX: ... in table scope */

        treebuilder->generate_implied_end_tags();

        if (!treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_CAPTION))
          treebuilder->error();

        std::shared_ptr< DOM_Element> popped;

        do {
          popped = treebuilder->open_elements.back();
          treebuilder->open_elements.pop_back();
        } while (!popped->has_html_element_index(HTML_ELEMENT_CAPTION));

        /* XXX: clear to last marker */

        treebuilder->mode = IN_TABLE_MODE;
        return TREEBUILDER_STATUS_REPROCESS;
      }


      default:
        break;
    }

  }


  anything_else: {
    return in_body_mode(treebuilder, token_data, token_type);
  }

}


static enum treebuilder_status
in_column_group_mode(TreeBuilder *treebuilder,
                     union token_data *token_data,
                     enum token_type token_type)
{
  LOGF("in column group mode\n");

  if (token_type == TOKEN_WHITESPACE) {
    treebuilder->insert_character(token_data->ch);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->error();
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name)
    {
      case HTML_ELEMENT_HTML: {
        return in_body_mode(treebuilder, token_data, token_type);
      }


      case HTML_ELEMENT_COL: {
        treebuilder->insert_html_element(tag);
        treebuilder->open_elements.pop_back();
        treebuilder->acknowledge_self_closing_flag(tag);
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_TEMPLATE: {
        return in_head_mode(treebuilder, token_data, token_type);
      }


      default:
        break;
    }


    goto anything_else;
  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name)
    {

      case HTML_ELEMENT_COLGROUP: {
        if (!treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_COLGROUP)) {
          treebuilder->error();
          return TREEBUILDER_STATUS_IGNORE;
        }

        treebuilder->open_elements.pop_back();

        treebuilder->mode = IN_TABLE_MODE;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_COL: {
        treebuilder->error();

        return TREEBUILDER_STATUS_IGNORE;
      }


      case HTML_ELEMENT_TEMPLATE: {
        return in_head_mode(treebuilder, token_data, token_type);
      }


      default:
        break;
    }


    goto anything_else;
  }


  anything_else: {
    if (!treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_COLGROUP)) {
      treebuilder->error();
      return TREEBUILDER_STATUS_IGNORE;
    }

    treebuilder->open_elements.pop_back();

    treebuilder->mode = IN_TABLE_MODE;
    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static void
clear_stack_to_table_body_context(TreeBuilder *treebuilder)
{
  while (!(treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_TBODY)
        || treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_TFOOT)
        || treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_THEAD)
        || treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_TEMPLATE)
        || treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_HTML)))
    treebuilder->open_elements.pop_back();
}


static enum treebuilder_status
in_table_body_mode(TreeBuilder *treebuilder,
                   union token_data *token_data,
                   enum token_type token_type)
{
  LOGF("in table body mode\n");

  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name)
    {
      case HTML_ELEMENT_TR: {
        clear_stack_to_table_body_context(treebuilder);
        treebuilder->insert_html_element(tag);
        treebuilder->mode = IN_ROW_MODE;
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_TH: case HTML_ELEMENT_TD: {
        static const struct tag_token dummy_token = {
          .tag_name = "tr",
          .local_name = HTML_ELEMENT_TR,
          .self_closing_flag = false,
          .ack_self_closing_flag_ = false,
        };

        treebuilder->error();

        clear_stack_to_table_body_context(treebuilder);

        treebuilder->insert_html_element(&dummy_token);

        treebuilder->mode = IN_ROW_MODE;

        return TREEBUILDER_STATUS_REPROCESS;
      }


      case HTML_ELEMENT_CAPTION: case HTML_ELEMENT_COL:   case HTML_ELEMENT_COLGROUP:
      case HTML_ELEMENT_TBODY:   case HTML_ELEMENT_TFOOT: case HTML_ELEMENT_THEAD: {
        /* XXX: ... in scope */

        clear_stack_to_table_body_context(treebuilder);

        treebuilder->open_elements.pop_back();

        treebuilder->mode = IN_TABLE_MODE;

        return TREEBUILDER_STATUS_REPROCESS;
      }


      default:
        break;
    }


    goto anything_else;
  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name)
    {
      case HTML_ELEMENT_TBODY: case HTML_ELEMENT_TFOOT: case HTML_ELEMENT_THEAD: {
        /* XXX: ... in scope */

        clear_stack_to_table_body_context(treebuilder);

        treebuilder->open_elements.pop_back();

        treebuilder->mode = IN_TABLE_MODE;

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_TABLE: {
        /* XXX: ... in scope */

        clear_stack_to_table_body_context(treebuilder);

        treebuilder->open_elements.pop_back();

        treebuilder->mode = IN_TABLE_MODE;

        return TREEBUILDER_STATUS_REPROCESS;
      }


      case HTML_ELEMENT_BODY:     case HTML_ELEMENT_CAPTION: case HTML_ELEMENT_COL:
      case HTML_ELEMENT_COLGROUP: case HTML_ELEMENT_HTML:    case HTML_ELEMENT_TD:
      case HTML_ELEMENT_TH:       case HTML_ELEMENT_TR: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }


      default:
        break;
    }

  }


  anything_else: {
    return in_table_mode(treebuilder, token_data, token_type);
  }

}


static enum treebuilder_status
in_row_mode(TreeBuilder *treebuilder,
            union token_data *token_data,
            enum token_type token_type)
{
  LOGF("in row mode\n");

  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static void
close_cell(TreeBuilder *treebuilder)
{
  treebuilder->generate_implied_end_tags();

  if (!(treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_TD)
     || treebuilder->current_node()->has_html_element_index(HTML_ELEMENT_TH)))
    treebuilder->error();

  std::shared_ptr< DOM_Element> popped;

  do {
    popped = treebuilder->open_elements.back();
    treebuilder->open_elements.pop_back();
  } while (!(popped->has_html_element_index(HTML_ELEMENT_TD)
          || popped->has_html_element_index(HTML_ELEMENT_TH)));

  /* XXX: clear to last marker */

  treebuilder->mode = IN_ROW_MODE;
}


static enum treebuilder_status
in_cell_mode(TreeBuilder *treebuilder,
             union token_data *token_data,
             enum token_type token_type)
{
  LOGF("in cell mode\n");

  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_select_mode(TreeBuilder *treebuilder,
               union token_data *token_data,
               enum token_type token_type)
{
  LOGF("in select mode\n");

  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_select_in_table_mode(TreeBuilder *treebuilder,
                        union token_data *token_data,
                        enum token_type token_type)
{
  LOGF("in select in table mode\n");

  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_template_mode(TreeBuilder *treebuilder,
                 union token_data *token_data,
                 enum token_type token_type)
{
  LOGF("in template mode\n");

  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
after_body_mode(TreeBuilder *treebuilder,
                union token_data *token_data,
                enum token_type token_type)
{
  LOGF("after body mode\n");

  if (token_type == TOKEN_WHITESPACE) {
    return in_body_mode(treebuilder, token_data, token_type);
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment,
     InsertionLocation{std::dynamic_pointer_cast<DOM_Node>(treebuilder->open_elements.front()), nullptr});
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name)
    {
      case HTML_ELEMENT_HTML: {
        return in_body_mode(treebuilder, token_data, token_type);
      }

      default:
        break;
    }

    goto anything_else;
  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name)
    {
      case HTML_ELEMENT_HTML: {
        if (treebuilder->flags.fragment_parse) {
          /* fragment case */
          treebuilder->error();
          return TREEBUILDER_STATUS_IGNORE;
        }

        treebuilder->mode = AFTER_AFTER_BODY_MODE;
        return TREEBUILDER_STATUS_OK;
      }

      default:
        break;
    }


    goto anything_else;
  }


  if (token_type == TOKEN_EOF) {
    return TREEBUILDER_STATUS_STOP;
  }


  anything_else: {
    treebuilder->error();

    treebuilder->mode = IN_BODY_MODE;

    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static enum treebuilder_status
in_frameset_mode(TreeBuilder *treebuilder,
                 union token_data *token_data,
                 enum token_type token_type)
{
  LOGF("in frameset mode\n");

  if (token_type == TOKEN_WHITESPACE) {
    treebuilder->insert_character(token_data->ch);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment);
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name) {
      /* ... */

      default:
        break;
    }

    goto anything_else;
  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name) {
      /* ... */

      default:
        break;
    }

    goto anything_else;
  }


  if (token_type == TOKEN_EOF) {
    if (treebuilder->current_node() != treebuilder->open_elements.front())
      treebuilder->error();

    return TREEBUILDER_STATUS_STOP;
  }


  anything_else: {
    treebuilder->error();
    return TREEBUILDER_STATUS_OK;
  }

}


static enum treebuilder_status
after_frameset_mode(TreeBuilder *treebuilder,
                    union token_data *token_data,
                    enum token_type token_type)
{
  LOGF("after frameset mode\n");

  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
after_after_body_mode(TreeBuilder *treebuilder,
                      union token_data *token_data,
                      enum token_type token_type)
{
  LOGF("after after body mode\n");

  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment,
     InsertionLocation{std::dynamic_pointer_cast< DOM_Node>(treebuilder->document), nullptr});
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    return in_body_mode(treebuilder, token_data, token_type);
  }


  if (token_type == TOKEN_WHITESPACE) {
    return in_body_mode(treebuilder, token_data, token_type);
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name)
    {
      case HTML_ELEMENT_HTML: {
        return in_body_mode(treebuilder, token_data, token_type);
      }


      default:
        break;
    }


    goto anything_else;
  }


  anything_else: {
    treebuilder->error();
    treebuilder->mode = IN_BODY_MODE;
    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static enum treebuilder_status
after_after_frameset_mode(TreeBuilder *treebuilder,
                          union token_data *token_data,
                          enum token_type token_type)
{
  LOGF("after after frameset mode\n");

  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_foreign_content_mode(TreeBuilder *treebuilder,
                        union token_data *token_data,
                        enum token_type token_type)
{
  LOGF("in foreign content mode\n");

  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


const TreeBuilder::insertion_mode_handler_cb_t TreeBuilder::k_insertion_mode_handlers_[NUM_MODES] = {
  initial_mode,
  before_html_mode,
  before_head_mode,
  in_head_mode,
  in_head_noscript_mode,
  after_head_mode,
  in_body_mode,
  text_mode,
  in_table_mode,
  in_table_text_mode,
  in_caption_mode,
  in_column_group_mode,
  in_table_body_mode,
  in_row_mode,
  in_cell_mode,
  in_select_mode,
  in_select_in_table_mode,
  in_template_mode,
  after_body_mode,
  in_frameset_mode,
  after_frameset_mode,
  after_after_body_mode,
  after_after_frameset_mode,
  in_foreign_content_mode,
};

