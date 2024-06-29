#ifndef _queequeg_dom_document_hh_
#define _queequeg_dom_document_hh_

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


class DOM_Document : DOM_Node {
  public:
    DOM_DocumentType *doctype;

    enum dom_document_format       format;
    enum dom_document_quirks_mode  quirks_mode;

};

#endif /* !defined(_queequeg_dom_document_hh_) */

