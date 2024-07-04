/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

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

  tokenizer.run();

  return 0;
}

