#include "html_parser/common.hh"

#include <stddef.h>


Tokenizer::Tokenizer(char const *input, size_t input_len)
{
  this->input.p   = input;
  this->input.end = &input[input_len];
}


Tokenizer::~Tokenizer()
{
}


void
Tokenizer::run(void)
{
  // ...
}

