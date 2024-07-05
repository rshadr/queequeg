#ifndef _queequeg_dom_document_hh_
#define _queequeg_dom_document_hh_

#include <memory>

#include <infra/namespace.h>

#include "dom/core/node.hh"


class DOM_DocumentType;
class DOM_Element;


enum dom_document_format {
  DOM_DOCFORMAT_XML,
  DOM_DOCFORMAT_HTML,
};


enum dom_document_quirks_mode {
  DOM_QUIRKSMODE_NO_QUIRKS,
  DOM_QUIRKSMODE_LIMITED_QUIRKS,
  DOM_QUIRKSMODE_QUIRKS,
};


class DOM_Document : public DOM_Node {
  public:
    DOM_Document(enum dom_document_format format);
   // ~DOM_Document();

  public:
    std::shared_ptr< DOM_DocumentType> doctype;

    enum dom_document_format       document_format;
    enum dom_document_quirks_mode  quirks_mode;


    [[nodiscard]] std::shared_ptr< DOM_Element> create_element(uint16_t local_name,
                                                               enum InfraNamespace name_space,
                                                               void *prefix = nullptr,
                                                               void *is = nullptr,
                                                               bool sync_custom_elements = false);
};

#endif /* !defined(_queequeg_dom_document_hh_) */

