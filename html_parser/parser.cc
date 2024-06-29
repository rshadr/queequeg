#include <stddef.h>

#include "html_parser/parser.hh"
#include "html_parser/common.hh"

#include "dom/core/document.hh"


int
html_parse_document(DOM_Document *document,
                    char const *input, size_t input_len)
{
  (void) document;

  Tokenizer   tokenizer   = Tokenizer(input, input_len);
  TreeBuilder treebuilder = TreeBuilder(document);

  tokenizer.treebuilder = &treebuilder;
  treebuilder.tokenizer = &tokenizer;

  tokenizer.run();

  return 0;
}

