#ifndef _queequeg_dom_character_data_hh_
#define _queequeg_dom_character_data_hh_

#include <string>
#include <memory>


#include "dom/core/node.hh"


namespace DOM {


class Document;


class CharacterData : public DOM::Node {
  protected:
    CharacterData(std::shared_ptr< DOM::Document> node_document,
                  enum dom_node_type node_type,
                  std::string data = "")
  : DOM::Node(node_document, node_type)
  {
    this->data = data;
  }
  public:
    virtual ~CharacterData() = default;

  public:
    std::string data;
};


} /* namespace DOM */


#endif /* !defined(_queequeg_dom_character_data_hh_) */

