/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
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
    // treebuilder->insert_comment(token_data->comment,
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_DOCTYPE) {
    /* ... */

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
    treebuilder->insert_comment(token_data->data,
     InsertionLocation{ treebuilder->document, nullptr });
    return TREEBUILDER_STATUS_OK;
  }


  if (token_type == TOKEN_WHITESPACE) {
    return TREEBUILDER_STATUS_IGNORE;
  }


  /* ... */


  {
    /* ... */
    treebuilder->mode = BEFORE_HEAD_MODE;
    return TREEBUILDER_STATUS_REPROCESS;
  }

}


const TreeBuilder::insertion_mode_handler_cb_t TreeBuilder::k_insertion_mode_handlers_[NUM_MODES] = {
  initial_mode,
#if 0
  before_html_mode,
  in_head_mode,
  in_head_noscript_mode,
  after_head_mode,
  in_body_mode,
  text_mode,
  in_table_mode,
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
  // in_foreign_content_mode,
#endif
};

