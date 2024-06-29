#ifndef _queequeg_dom_event_target_hh_
#define _queequeg_dom_event_target_hh_
/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */


class DOM_Event;


class DOM_EventTarget {
  protected:
    typedef DOM_EventTarget *(DOM_EventTarget::*get_parent_cb_t) (DOM_Event *event);
};


#endif /* !defined(_queequeg_dom_event_target_hh_) */

