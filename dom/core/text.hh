#ifndef _queequeg_dom_text_hh_
#define _queequeg_dom_text_hh_


#include <string>
#include <memory>


#include "dom/core/character_data.hh"


class DOM_Document;


class DOM_Text : public DOM_CharacterData {
  public:
    DOM_Text(std::shared_ptr< DOM_Document> document,
             std::string data = "")
  : DOM_CharacterData(document, DOM_NODETYPE_TEXT, data) { }
    virtual ~DOM_Text() { }
};


#endif /* !defined(_queequeg_dom_text_hh_) */

