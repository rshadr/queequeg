#ifndef _queequeg_dom_comment_hh_
#define _queequeg_dom_comment_hh_


#include <string>
#include <memory>


#include "dom/core/character_data.hh"


namespace DOM {


class Document;


class Comment : public DOM::CharacterData {
  public:
    Comment(std::shared_ptr< DOM::Document> node_document,
            std::string data = "")
  : DOM::CharacterData(node_document, DOM_NODETYPE_COMMENT, data) { }
    virtual ~Comment() = default;
};


} /* namespace DOM */


#endif /* !defined(_queequeg_dom_comment_hh_) */

