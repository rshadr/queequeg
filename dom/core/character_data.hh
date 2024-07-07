#ifndef _queequeg_dom_character_data_hh_
#define _queequeg_dom_character_data_hh_

#include <string>

#include "dom/core/node.hh"


class DOM_CharacterData : public DOM_Node {
#if 0
  protected:
    DOM_CharacterData();
  public:
    virtual ~DOM_CharacterData();
#endif

  public:
    std::string data;
};


#endif /* !defined(_queequeg_dom_character_data_hh_) */

