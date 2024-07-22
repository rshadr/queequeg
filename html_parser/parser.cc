/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

#include <stdio.h>

#include "html_parser/parser.hh"
#include "html_parser/common.hh"

#include "dom/core/document.hh"


int
html_parse_document(std::shared_ptr< DOM_Document> document,
                    char const *input, size_t input_len)
{
  Tokenizer   tokenizer   = Tokenizer(input, input_len);
  TreeBuilder treebuilder = TreeBuilder(document);

  tokenizer.treebuilder = &treebuilder;
  treebuilder.tokenizer = &tokenizer;

  document->parser_status = DOM_DOCUMENT_PARSER_STATUS_RUNNING;

  tokenizer.run();

  document->parser_status = DOM_DOCUMENT_PARSER_STATUS_DONE;

  printf("%d elements left on stack after parsing\n",
    static_cast<int>(treebuilder.open_elements.size()));

  for (std::shared_ptr< DOM_Element> elem : treebuilder.open_elements)
    printf("  element %d\n", static_cast<int>(elem->local_name));

  return 0;
}

