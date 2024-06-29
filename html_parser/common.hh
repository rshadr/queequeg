#ifndef _queequeg_html_parser_common_hh_
#define _queequeg_html_parser_common_hh_

#include <stddef.h>

#include "dom/core/node.hh"
#include "dom/core/document.hh"
#include "dom/core/element.hh"

/*
 * Header used internally by parser components
 */

class Tokenizer;
class TreeBuilder;


class Tokenizer {
  public:
      Tokenizer(char const *input, size_t input_len);
    ~Tokenizer();

    struct {
      char const *p;
      char const *end;
    } input;

    TreeBuilder *treebuilder;

    void error(char const *errstr);

    void run(void);
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

    void **open_elements;
    void **formatting_elements;

    int mode;
    int original_mode;

    struct {
      bool fragment_parse;
      bool scripting;
      bool frameset_ok;
      bool foster_parenting;
      bool parser_pause;
    } flags;

};


struct InsertionLocation {
  DOM_Node *parent;
  DOM_Node *child;
};


#endif /* !defined(_queequeg_html_parser_common_hh_) */

