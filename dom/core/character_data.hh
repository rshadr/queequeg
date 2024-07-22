#ifndef _queequeg_dom_character_data_hh_
#define _queequeg_dom_character_data_hh_

#include <string>

#include "dom/core/node.hh"


class DOM_CharacterData : public DOM_Node {
  protected:
    DOM_CharacterData(std::string data = "");
  public:
    virtual ~DOM_CharacterData();

  public:
    std::string data;
};


#endif /* !defined(_queequeg_dom_character_data_hh_) */

