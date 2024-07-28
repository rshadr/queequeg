#ifndef _queequeg_dom_character_data_hh_
#define _queequeg_dom_character_data_hh_

#include <string>
#include <memory>


#include "dom/core/node.hh"


class DOM_Document;


class DOM_CharacterData : public DOM_Node {
  protected:
    DOM_CharacterData(std::shared_ptr< DOM_Document> document,
                      enum dom_node_type node_type,
                      std::string data = "")
  : DOM_Node(document, node_type)
  {
    this->data = data;
  }
  public:
    virtual ~DOM_CharacterData() { };

  public:
    std::string data;
};


#endif /* !defined(_queequeg_dom_character_data_hh_) */

