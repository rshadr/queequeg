#ifndef _queequeg_dom_comment_hh_
#define _queequeg_dom_comment_hh_


#include <string>
#include <memory>


#include "dom/core/character_data.hh"


class DOM_Document;


class DOM_Comment : public DOM_CharacterData {
  public:
    DOM_Comment(std::shared_ptr< DOM_Document> document,
                std::string data = "")
  : DOM_CharacterData(document, DOM_NODETYPE_COMMENT, data) { }
    virtual ~DOM_Comment() { }
};


#endif /* !defined(_queequeg_dom_comment_hh_) */

