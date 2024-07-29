#ifndef _queequeg_dom_document_hh_
#define _queequeg_dom_document_hh_

/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

#include <memory>

#include <infra/namespace.h>

#include "dom/core/node.hh"


class DOM_DocumentType;
class DOM_Element;


enum dom_document_format {
  DOM_DOCUMENT_FORMAT_XML,
  DOM_DOCUMENT_FORMAT_HTML,
};


enum dom_document_quirks_mode {
  DOM_QUIRKSMODE_NO_QUIRKS,
  DOM_QUIRKSMODE_LIMITED_QUIRKS,
  DOM_QUIRKSMODE_QUIRKS,
};


/*
 * Queequeg convenience; this exists so we don't need to coordinate from document to parser.
 * The parser (typically the HTML parser) just updates this over time, but the document
 * isn't supposed to know anything else about the parser.
 */
enum dom_document_parser_status {
  DOM_DOCUMENT_PARSER_STATUS_UNAVAILABLE,
  DOM_DOCUMENT_PARSER_STATUS_RUNNING,
  DOM_DOCUMENT_PARSER_STATUS_DONE,
  DOM_DOCUMENT_PARSER_STATUS_PAUSED,
  DOM_DOCUMENT_PARSER_STATUS_STOPPED,
  DOM_DOCUMENT_PARSER_STATUS_ABORTED,
};


class DOM_Document : public DOM_Node {
  public:
    DOM_Document(enum dom_document_format format = DOM_DOCUMENT_FORMAT_HTML);
    virtual ~DOM_Document();

  public:
    std::shared_ptr< DOM_DocumentType> doctype = nullptr;

    enum dom_document_parser_status parser_status = DOM_DOCUMENT_PARSER_STATUS_UNAVAILABLE;

    enum dom_document_format       document_format;
    enum dom_document_quirks_mode  quirks_mode;


    [[nodiscard]] std::shared_ptr< DOM_Element> create_element(uint16_t local_name,
                                                               enum InfraNamespace name_space,
                                                               void *prefix = nullptr,
                                                               void *is = nullptr,
                                                               bool sync_custom_elements = false);
};

#endif /* !defined(_queequeg_dom_document_hh_) */

