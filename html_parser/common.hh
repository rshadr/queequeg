#ifndef _queequeg_html_parser_common_hh_
#define _queequeg_html_parser_common_hh_

#include <unordered_map>

#include <stddef.h>
#include <uchar.h>

#include <infra/string.h>
#include <infra/stack.h>
#include <infra/namespace.h>

#include "dom/core/node.hh"
#include "dom/core/document.hh"
#include "dom/core/element.hh"

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
  InfraString *name;
  InfraString *public_id;
  InfraString *system_id;

  bool public_id_missing;
  bool system_id_missing;
  bool force_quirks_flag;
};


struct tag_token {
  InfraString *tagname;
  InfraStack *attrs;
  enum InfraNamespace name_space;
  uint16_t local_name;

  bool self_closing_flag;
  bool ack_self_closing_flag_;
};


union token {
  InfraString *         comment;
  struct doctype_token  doctype;
  struct tag_token      tag;
  char32_t              ch;
};



enum tokenizer_state {
  DATA_STATE,
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
  AFTER_ATTR_VALUE_UNQUOTED_STATE,
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
    InfraString *tmpbuf;
    InfraString *comment;
    uintmax_t char_ref;

    enum tokenizer_state state;
    enum tokenizer_state ret_state;

    enum token_type tag_type;


    /*
     * Methods
     */
    [[nodiscard]] char32_t getchar(void);
    [[nodiscard]] bool match(char const *s, size_t slen);
    [[nodiscard]] bool match_insensitive(char const *s, size_t slen);
    void error(char const *errstr);

    bool have_appropriate_end_tag(void) const;

    inline bool
    char_ref_in_attr(void) const
    {
      return (this->ret_state == ATTR_VALUE_DOUBLE_QUOTED_STATE
           || this->ret_state == ATTR_VALUE_SINGLE_QUOTED_STATE
           || this->ret_state == ATTR_VALUE_UNQUOTED_STATE);
    }

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
    void destroy_tag_(void);
    void create_tag_(enum token_type tag_type);
    void destroy_doctype_(void);
    void emit_token_(union token *token_data, enum token_type token_type);

    static const std::unordered_map<enum tokenizer_state, state_handler_cb_t> k_state_handlers_;

};



enum insertion_mode {
  INITIAL_MODE,
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
  // IN_FOREIGN_CONTENT_MODE,

  NUM_MODES
};


class TreeBuilder {
  public:
      TreeBuilder(DOM_Document *document);
    ~TreeBuilder();


    Tokenizer *tokenizer;

    DOM_Document *document;
    DOM_Element *context;

    DOM_Element *head;
    DOM_Element *form;

    InfraStack *open_elements;
    InfraStack *formatting_elements;

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


    inline DOM_Element *
    current_node(void) const
    {
      return static_cast<DOM_Element *>(infra_stack_peek(this->open_elements));
    }


    inline DOM_Element *
    adjusted_current_node(void) const
    {
      if (this->context != NULL)
        return this->context;

      return this->current_node();
    }


    void error(void);

    void insert_character(char32_t ch);
};


struct InsertionLocation {
  DOM_Node *parent;
  DOM_Node *child;
};


extern Tokenizer::state_handler_cb_t k_html_tokenizer_states_[NUM_STATES];
// extern TreeBuilder::insertion_mode_handler_cb_t k_insertion_modes[NUM_MODES];

#endif /* !defined(_queequeg_html_parser_common_hh_) */

