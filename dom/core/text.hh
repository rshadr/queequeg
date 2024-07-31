#ifndef _queequeg_dom_text_hh_
#define _queequeg_dom_text_hh_


#include <string>
#include <memory>


#include "dom/core/character_data.hh"


namespace DOM {


class Document;


class Text : public DOM::CharacterData {
  public:
    Text(std::shared_ptr< DOM::Document> document,
         std::string data = "")
  : DOM::CharacterData(document, DOM_NODETYPE_TEXT, data) { }
    virtual ~Text() = default;
};


} /* namespace DOM */


#endif /* !defined(_queequeg_dom_text_hh_) */

