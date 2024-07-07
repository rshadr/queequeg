#ifndef _queequeg_dom_event_target_hh_
#define _queequeg_dom_event_target_hh_
/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

#include <memory>


class DOM_Event;


class DOM_EventTarget : public std::enable_shared_from_this< DOM_EventTarget> {
  protected:
    DOM_EventTarget(void) { }
  public:
    virtual ~DOM_EventTarget() { }
};


#endif /* !defined(_queequeg_dom_event_target_hh_) */

