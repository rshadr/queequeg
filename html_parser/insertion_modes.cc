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
#include "dom/core/document_type.hh"
#include "dom/html/html_html_element.hh"
#include "html/elements.hh"

#include "html_parser/common.hh"



static enum treebuilder_status
initial_mode(TreeBuilder *treebuilder,
             union token_data *token_data,
             enum token_type token_type)
{

  if (token_type == TOKEN_WHITESPACE) {
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment,
     InsertionLocation{std::static_pointer_cast<DOM_Node>(treebuilder->document), nullptr});

    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    struct doctype_token *token = &token_data->doctype;

    if (token->name.compare("html")
     || !token->public_id_missing
     || (!token->system_id_missing
      && token->system_id.compare("about:legacy-compat")))
      treebuilder->error();

    std::shared_ptr< DOM_DocumentType> doctype = std::make_shared<DOM_DocumentType>(treebuilder->document);

    doctype->name = token->name;

    if (!token->public_id_missing)
      doctype->public_id = token->public_id;

    if (!token->system_id_missing)
      doctype->system_id = token->system_id;

    treebuilder->document->append_node(doctype);

    /* XXX: ... */

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

  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();

    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_COMMENT) {
    treebuilder->insert_comment(&token_data->comment,
     InsertionLocation{ std::static_pointer_cast<DOM_Node>(treebuilder->document), nullptr });

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
  printf("before head mode\n");

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
        /* XXX: process in body */
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_HEAD: {
        std::shared_ptr< DOM_HTMLHeadElement> head =
         std::static_pointer_cast<DOM_HTMLHeadElement>(treebuilder->insert_html_element(tag));

        treebuilder->head = head;

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
      .tag_name = { },
      .local_name = HTML_ELEMENT_HEAD,
      .self_closing_flag = false,
      .ack_self_closing_flag_ = false,
    };

    std::shared_ptr< DOM_HTMLHeadElement> head =
      std::static_pointer_cast< DOM_HTMLHeadElement>(treebuilder->insert_html_element(&dummy_token));

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
        /* XXX: process in body */
        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_BASE: case HTML_ELEMENT_BASEFONT: case HTML_ELEMENT_BGSOUND:
      case HTML_ELEMENT_LINK: {
        treebuilder->insert_html_element(tag);

        treebuilder->open_elements.pop_back();

        /* XXX: ack self-closing flag */

        return TREEBUILDER_STATUS_OK;
      }


      case HTML_ELEMENT_META: {
        treebuilder->insert_html_element(tag);

        treebuilder->open_elements.pop_back();

        /* XXX: ack self-closing flag */

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

  if (token_type == TOKEN_DOCTYPE) {
    treebuilder->error();
    return TREEBUILDER_STATUS_IGNORE;
  }


  if (token_type == TOKEN_START_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name) {
      /* ... */
      default:
        goto anything_else;
    }

  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name) {
      /* ... */
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
      default: {
        goto anything_else;
      }

    }

  }


  if (token_type == TOKEN_END_TAG) {
    struct tag_token *tag = &token_data->tag;

    switch (tag->local_name) {
      /* ... */
      default: {
        treebuilder->error();
        return TREEBUILDER_STATUS_IGNORE;
      }

    }

  }


  anything_else: {
    /* XXX: insert body */

    treebuilder->mode = IN_BODY_MODE;
    return TREEBUILDER_STATUS_REPROCESS;
  }

}


static enum treebuilder_status
in_body_mode(TreeBuilder *treebuilder,
             union token_data *token_data,
             enum token_type token_type)
{

  if (token_type == TOKEN_CHARACTER) {
    if (token_data->ch == U'\0') {
      treebuilder->error();
      return TREEBUILDER_STATUS_IGNORE;
    }

    /* XXX: reconstruct active formatting */
    treebuilder->insert_character(token_data->ch);
    treebuilder->flags.frameset_ok = false;

    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_WHITESPACE) {
    /* XXX: reconstruct active formatting */
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


  /* XXX: all the tags... */


  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
text_mode(TreeBuilder *treebuilder,
          union token_data *token_data,
          enum token_type token_type)
{
  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_table_mode(TreeBuilder *treebuilder,
              union token_data *token_data,
              enum token_type token_type)
{

  if (token_type == TOKEN_CHARACTER || token_type == TOKEN_WHITESPACE) {
    treebuilder->insert_character(token_data->ch);
    return TREEBUILDER_STATUS_OK;
  }


  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_table_text_mode(TreeBuilder *treebuilder,
                   union token_data *token_data,
                   enum token_type token_type)
{
  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_caption_mode(TreeBuilder *treebuilder,
                union token_data *token_data,
                enum token_type token_type)
{
  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_column_group_mode(TreeBuilder *treebuilder,
                     union token_data *token_data,
                     enum token_type token_type)
{
  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_table_body_mode(TreeBuilder *treebuilder,
                   union token_data *token_data,
                   enum token_type token_type)
{
  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_row_mode(TreeBuilder *treebuilder,
            union token_data *token_data,
            enum token_type token_type)
{
  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_cell_mode(TreeBuilder *treebuilder,
             union token_data *token_data,
             enum token_type token_type)
{
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
  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
in_frameset_mode(TreeBuilder *treebuilder,
                 union token_data *token_data,
                 enum token_type token_type)
{
  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
after_frameset_mode(TreeBuilder *treebuilder,
                    union token_data *token_data,
                    enum token_type token_type)
{
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
  (void) treebuilder;
  (void) token_data;
  (void) token_type;

  return TREEBUILDER_STATUS_OK;
}


static enum treebuilder_status
after_after_frameset_mode(TreeBuilder *treebuilder,
                          union token_data *token_data,
                          enum token_type token_type)
{
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

