#ifndef _queequeg_html_parser_parser_hh_
#define _queequeg_html_parser_parser_hh_


#include <stddef.h>

#include "dom/core/document.hh"


int html_parse_document(DOM_Document *document, char const *input, size_t input_len);


#endif /* !defined(_queequeg_html_parser_parser_hh_) */

