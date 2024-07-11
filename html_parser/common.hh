#ifndef _queequeg_html_parser_common_hh_
#define _queequeg_html_parser_common_hh_

/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

#include <memory>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>

#include <string.h>

#include <infra/namespace.h>

#include "dom/core/node.hh"
#include "dom/core/document.hh"
#include "dom/core/element.hh"

#include "dom/html/html_head_element.hh"

#include "html/elements.hh"

/*
 * Header used internally by parser components
 */

class Tokenizer;
class TreeBuilder;


enum token_type {
  TOKEN_CHARACTER,
  TOKEN_WHITESPACE,
  TOKEN_DOCTYPE,
  TOKEN_START_TAG,
  TOKEN_END_TAG,
  TOKEN_COMMENT,
  TOKEN_EOF,
};


struct doctype_token {
  std::string name;
  std::string public_id;
  std::string system_id;

  bool public_id_missing;
  bool system_id_missing;
  bool force_quirks_flag;
};


/*
 * This enum is only relevant for the "in body" insertion mode; it adds support
 * for foreign top-level elements by treating them as though they were regular
 * HTML elements in our index-based system.
 *
 * When inserting the corresponding tag tokens, they are transparently turned
 * into regular tokens with an std::string tag name which will translate to
 * the correct MathML/SVG element index.
 */
enum html_element_index_parser_internal : uint16_t {
  HTML_ELEMENT_MATH_ = NUM_HTML_BUILTIN_ELEMENTS,
  HTML_ELEMENT_SVG_,
};


struct tag_token {
  std::string tag_name;
  // InfraStack *attrs;
  uint16_t local_name;

  bool self_closing_flag;
  bool ack_self_closing_flag_;
};


union token_data {
  std::string           comment;
  struct doctype_token  doctype;
  struct tag_token      tag;
  char32_t              ch;
};


struct InsertionLocation {
  std::shared_ptr< DOM_Node> parent;
  std::shared_ptr< DOM_Node> child;
};


enum tokenizer_state {
  /*
   * 0 needed for jump table to work properly
   */
  DATA_STATE = 0,
  RCDATA_STATE,
  RAWTEXT_STATE,
  SCRIPT_STATE,
  PLAINTEXT_STATE,
  TAG_OPEN_STATE,
  END_TAG_OPEN_STATE,
  TAG_NAME_STATE,
  RCDATA_LT_STATE,
  RCDATA_END_TAG_OPEN_STATE,
  RCDATA_END_TAG_NAME_STATE,
  RAWTEXT_LT_STATE,
  RAWTEXT_END_TAG_OPEN_STATE,
  RAWTEXT_END_TAG_NAME_STATE,
  SCRIPT_LT_STATE,
  SCRIPT_END_TAG_OPEN_STATE,
  SCRIPT_END_TAG_NAME_STATE,
  SCRIPT_ESCAPE_START_STATE,
  SCRIPT_ESCAPE_START_DASH_STATE,
  SCRIPT_ESCAPED_STATE,
  SCRIPT_ESCAPED_DASH_STATE,
  SCRIPT_ESCAPED_DASH_DASH_STATE,
  SCRIPT_ESCAPED_LT_STATE,
  SCRIPT_ESCAPED_END_TAG_OPEN_STATE,
  SCRIPT_ESCAPED_END_TAG_NAME_STATE,
  SCRIPT_DOUBLE_ESCAPE_START_STATE,
  SCRIPT_DOUBLE_ESCAPED_STATE,
  SCRIPT_DOUBLE_ESCAPED_DASH_STATE,
  SCRIPT_DOUBLE_ESCAPED_DASH_DASH_STATE,
  SCRIPT_DOUBLE_ESCAPED_LT_STATE,
  SCRIPT_DOUBLE_ESCAPE_END_STATE,
  BEFORE_ATTR_NAME_STATE,
  ATTR_NAME_STATE,
  AFTER_ATTR_NAME_STATE,
  BEFORE_ATTR_VALUE_STATE,
  ATTR_VALUE_DOUBLE_QUOTED_STATE,
  ATTR_VALUE_SINGLE_QUOTED_STATE,
  ATTR_VALUE_UNQUOTED_STATE,
  AFTER_ATTR_VALUE_QUOTED_STATE,
  SELF_CLOSING_START_TAG_STATE,
  BOGUS_COMMENT_STATE,
  MARKUP_DECL_OPEN_STATE,
  COMMENT_START_STATE,
  COMMENT_START_DASH_STATE,
  COMMENT_STATE,
  COMMENT_LT_STATE,
  COMMENT_LT_BANG_STATE,
  COMMENT_LT_BANG_DASH_STATE,
  COMMENT_LT_BANG_DASH_DASH_STATE,
  COMMENT_END_DASH_STATE,
  COMMENT_END_STATE,
  COMMENT_END_BANG_STATE,
  DOCTYPE_STATE,
  BEFORE_DOCTYPE_NAME_STATE,
  DOCTYPE_NAME_STATE,
  AFTER_DOCTYPE_NAME_STATE,
  AFTER_DOCTYPE_PUBLIC_KEYWORD_STATE,
  BEFORE_DOCTYPE_PUBLIC_ID_STATE,
  DOCTYPE_PUBLIC_ID_DOUBLE_QUOTED_STATE,
  DOCTYPE_PUBLIC_ID_SINGLE_QUOTED_STATE,
  AFTER_DOCTYPE_PUBLIC_ID_STATE,
  BETWEEN_DOCTYPE_PUBLIC_SYSTEM_IDS_STATE,
  AFTER_DOCTYPE_SYSTEM_KEYWORD_STATE,
  BEFORE_DOCTYPE_SYSTEM_ID_STATE,
  DOCTYPE_SYSTEM_ID_DOUBLE_QUOTED_STATE,
  DOCTYPE_SYSTEM_ID_SINGLE_QUOTED_STATE,
  AFTER_DOCTYPE_SYSTEM_ID_STATE,
  BOGUS_DOCTYPE_STATE,
  CDATA_SECTION_STATE,
  CDATA_SECTION_BRACKET_STATE,
  CDATA_SECTION_END_STATE,
  CHAR_REF_STATE,
  NAMED_CHAR_REF_STATE,
  AMBIGUOUS_AMPERSAND_STATE,
  NUMERIC_CHAR_REF_STATE,
  HEX_CHAR_REF_START_STATE,
  DEC_CHAR_REF_START_STATE,
  HEX_CHAR_REF_STATE,
  DEC_CHAR_REF_STATE,
  NUMERIC_CHAR_REF_END_STATE,

  NUM_STATES
};


enum tokenizer_status {
  TOKENIZER_STATUS_RECONSUME,
  TOKENIZER_STATUS_OK,
  TOKENIZER_STATUS_IGNORE,
  TOKENIZER_STATUS_EOF,
};


class Tokenizer {
  public:
      Tokenizer(char const *input, size_t input_len);
    ~Tokenizer();


  public:
    /*
     * Handlers
     */
    typedef enum tokenizer_status (* const state_handler_cb_t) (Tokenizer *tokenizer, char32_t ch);

    struct {
      char const *p;
      char const *end;
    } input;

    TreeBuilder *treebuilder;

    struct doctype_token doctype;
    struct tag_token tag;
    std::u32string temp_buffer;
    std::string comment;
    uintmax_t char_ref;

    enum tokenizer_state state;
    enum tokenizer_state ret_state;

    enum token_type tag_type;


    /*
     * Methods
     */
    [[nodiscard]] char32_t getchar(void);

    [[nodiscard]] bool match(char const *s, size_t slen);

    [[nodiscard]]
    bool match(char const *c_str)
    {
      return this->match(c_str, strlen(c_str));
    }

    [[nodiscard]] bool match_insensitive(char const *s, size_t slen);

    [[nodiscard]]
    bool match_insensitive(char const *c_str)
    {
      return this->match_insensitive(c_str, strlen(c_str));
    }

    void error(char const *errstr);

    bool have_appropriate_end_tag(void) const;

    inline bool
    is_char_ref_in_attr(void) const
    {
      return (this->ret_state == ATTR_VALUE_DOUBLE_QUOTED_STATE
           || this->ret_state == ATTR_VALUE_SINGLE_QUOTED_STATE
           || this->ret_state == ATTR_VALUE_UNQUOTED_STATE);
    }

    void flush_char_ref_codepoints(void);

    void create_doctype(void);

    void create_start_tag(void);

    void create_end_tag(void);

    void create_comment(void);

    void emit_character(char32_t ch);

    void emit_current_doctype(void);

    void emit_current_tag(void);

    void emit_current_comment(void);

    [[nodiscard]] enum tokenizer_status emit_eof(void);


    void run(void);


  private:
    [[nodiscard]] bool match_fn_(int (*cmp) (char const *, char const *, size_t),
                                 char const *s,
                                 size_t slen);

    void create_tag_(enum token_type tag_type);

    void destroy_doctype_(void);

    void emit_token_(union token_data *token_data, enum token_type token_type);

    static const state_handler_cb_t k_state_handlers_[NUM_STATES];

    static const std::unordered_map< std::string, uint16_t> k_foreign_toplevel_elements_map_;
};



enum insertion_mode {
  /*
   * 0 needed for jump table to work properly
   */
  INITIAL_MODE = 0,
  BEFORE_HTML_MODE,
  BEFORE_HEAD_MODE,
  IN_HEAD_MODE,
  IN_HEAD_NOSCRIPT_MODE,
  AFTER_HEAD_MODE,
  IN_BODY_MODE,
  TEXT_MODE,
  IN_TABLE_MODE,
  IN_TABLE_TEXT_MODE,
  IN_CAPTION_MODE,
  IN_COLUMN_GROUP_MODE,
  IN_TABLE_BODY_MODE,
  IN_ROW_MODE,
  IN_CELL_MODE,
  IN_SELECT_MODE,
  IN_SELECT_IN_TABLE_MODE,
  IN_TEMPLATE_MODE,
  AFTER_BODY_MODE,
  IN_FRAMESET_MODE,
  AFTER_FRAMESET_MODE,
  AFTER_AFTER_BODY_MODE,
  AFTER_AFTER_FRAMESET_MODE,
  IN_FOREIGN_CONTENT_MODE,

  NUM_MODES
};


enum treebuilder_status {
  TREEBUILDER_STATUS_REPROCESS,
  TREEBUILDER_STATUS_OK,
  TREEBUILDER_STATUS_IGNORE,
  TREEBUILDER_STATUS_STOP,
};


class TreeBuilder {
  public:
      TreeBuilder(std::shared_ptr< DOM_Document> document);
    ~TreeBuilder();

  public:
    /*
     * Handlers
     */
    typedef enum treebuilder_status (*insertion_mode_handler_cb_t)
     (TreeBuilder *treebuilder, union token_data *token_data, enum token_type);

    Tokenizer *tokenizer;

    std::shared_ptr< DOM_Document> document;
    std::shared_ptr< DOM_Element> context;

    std::shared_ptr< DOM_HTMLHeadElement> head;
    std::shared_ptr< DOM_Element> form;

    std::vector< std::shared_ptr< DOM_Element>> open_elements;
    std::list< std::shared_ptr< DOM_Element>> formatting_elements;

    std::vector< char32_t> pending_table_characters;

    std::vector< enum insertion_mode> template_modes;

    uint16_t script_nesting_level;

    enum insertion_mode mode;
    enum insertion_mode original_mode;

    struct {
      bool fragment_parse;
      bool scripting;
      bool frameset_ok;
      bool foster_parenting;
      bool parser_pause;
    } flags;

    bool skip_newline;


    /*
     * Methods
     */
    void process_token(union token_data *token_data, enum token_type token_type);

    void error(void);


    inline enum insertion_mode
    current_template_mode(void)
    {
      /* XXX: maybe throws an exception? */
      return this->template_modes.back();
    }


    void reset_insertion_mode_appropriately(void);


    inline std::shared_ptr< DOM_Element>
    current_node(void) const
    {
      return this->open_elements.back();
    }


    inline std::shared_ptr< DOM_Element>
    adjusted_current_node(void) const
    {
      if (this->context != nullptr)
        return this->context;

      return this->current_node();
    }


    bool is_special_element(std::shared_ptr< DOM_Element> element) const;


    void push_to_active_formatting_elements(std::shared_ptr< DOM_Element> element);

    void reconstruct_active_formatting_elements(void);

    void acknowledge_self_closing_flag(struct tag_token *tag) const;


    InsertionLocation appropriate_insertion_place(std::shared_ptr< DOM_Element> override_target = nullptr);

    [[nodiscard]] std::shared_ptr< DOM_Element>
    create_element_for_token(struct tag_token const *tag,
                             enum InfraNamespace name_space,
                             std::shared_ptr< DOM_Node> intended_parent);

    void insert_element_at_adjusted_insertion_location(std::shared_ptr< DOM_Element>);

    std::shared_ptr< DOM_Element> insert_foreign_element(struct tag_token const *tag,
     enum InfraNamespace name_space, bool only_add_to_element_stack);

    std::shared_ptr< DOM_Element> insert_html_element(struct tag_token const *tag);

    void insert_characters(std::vector< char32_t> *vch);
    void insert_character(char32_t ch);

    void insert_comment(std::string *data, InsertionLocation where);

    inline void
    insert_comment(std::string *data)
    {
      this->insert_comment(data, this->appropriate_insertion_place());
    }

    [[nodiscard]] enum treebuilder_status generic_raw_text_parse(struct tag_token *tag);

    [[nodiscard]] enum treebuilder_status generic_rcdata_parse(struct tag_token *tag);

    void generate_implied_end_tags(uint16_t exclude_html = 0);


  private:
    void insert_character_array_(char32_t const *arr, size_t arr_len);

    [[nodiscard]] enum treebuilder_status tree_construction_dispatcher_(union token_data *token_data,
                                                                        enum token_type token_type);

    static const insertion_mode_handler_cb_t k_insertion_mode_handlers_[NUM_MODES];
};


// extern TreeBuilder::insertion_mode_handler_cb_t k_insertion_modes[NUM_MODES];

#endif /* !defined(_queequeg_html_parser_common_hh_) */

