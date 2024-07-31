#ifndef _queequeg_dom_event_target_hh_
#define _queequeg_dom_event_target_hh_
/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

#include <memory>


namespace DOM {


class EventTarget : public std::enable_shared_from_this< EventTarget> {
  protected:
    EventTarget(void) = default;
  public:
    virtual ~EventTarget() = default;
};


} /* namespace DOM */


#endif /* !defined(_queequeg_dom_event_target_hh_) */

