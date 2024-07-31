/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

#include <cstdio>
#include <stdio.h>
#include <csignal>
#include <cstdlib>

#include "html_parser/parser.hh"
#include "html_parser/internal.hh"

#include "dom/core/document.hh"


static struct {
  Tokenizer   *tokenizer;
  TreeBuilder *treebuilder;
} signal_handler_shared;


/*
 * cheap hack to roughly know where we are in the source document
 */

extern "C" {


[[noreturn]]
static void
sigsegv_handler(int signal)
{
  (void) signal;

  auto *ctx = &signal_handler_shared;

  char peek_buffer[512] = { 0 };
  size_t left = ctx->tokenizer->input.end - ctx->tokenizer->input.p;
  memcpy(peek_buffer, ctx->tokenizer->input.p, std::min(left, sizeof (peek_buffer) - 1));

  std::fprintf(stderr, "==< SEGMENTATION FAULT (QUEEQUEG HANDLER) >==\n"
                       "  peek_buffer:\n"
                       "%s\n",
               peek_buffer);

  std::exit(1337);
}


} /* extern "C" */


static void
register_signal_handlers(Tokenizer *tokenizer,
                         TreeBuilder *treebuilder)
{

  signal_handler_shared.tokenizer = tokenizer;
  signal_handler_shared.treebuilder = treebuilder;

  std::signal(SIGSEGV, sigsegv_handler);

}


int
html_parse_document(std::shared_ptr< DOM::Document> document,
                    char const *input, size_t input_len)
{
  Tokenizer   tokenizer   = Tokenizer(input, input_len);
  TreeBuilder treebuilder = TreeBuilder(document);

  tokenizer.treebuilder = &treebuilder;
  treebuilder.tokenizer = &tokenizer;

  document->parser_status = DOM_DOCUMENT_PARSER_STATUS_RUNNING;

  register_signal_handlers(&tokenizer, &treebuilder);

  tokenizer.run();

  document->parser_status = DOM_DOCUMENT_PARSER_STATUS_DONE;

  printf("%d elements left on stack after parsing:\n",
    static_cast<int>(treebuilder.open_elements.size()));
  for (auto& elem : treebuilder.open_elements)
    printf("  element of index %d\n", static_cast<int>(elem->local_name));

  return 0;
}

