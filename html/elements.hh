#ifndef _queequeg_html_elements_hh_
#define _queequeg_html_elements_hh_

#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>

/*
 * This enumeration is used to address built-in and custom HTML elements by
 * their local index.
 */

enum html_element_index : uint16_t {
  HTML_ELEMENT_NONE_ = 0,

  /* 4.1 The document element */
  HTML_ELEMENT_HTML,

  /* 4.2 Document metadata */
  HTML_ELEMENT_HEAD,
  HTML_ELEMENT_TITLE,
  HTML_ELEMENT_BASE,
  HTML_ELEMENT_LINK,
  HTML_ELEMENT_META,
  HTML_ELEMENT_STYLE,

  /* 4.3 Sections */
  HTML_ELEMENT_BODY,
  HTML_ELEMENT_ARTICLE,
  HTML_ELEMENT_SECTION,
  HTML_ELEMENT_NAV,
  HTML_ELEMENT_ASIDE,
  HTML_ELEMENT_H1,
  HTML_ELEMENT_H2,
  HTML_ELEMENT_H3,
  HTML_ELEMENT_H4,
  HTML_ELEMENT_H5,
  HTML_ELEMENT_H6,
  HTML_ELEMENT_HGROUP,
  HTML_ELEMENT_HEADER,
  HTML_ELEMENT_FOOTER,
  HTML_ELEMENT_ADDRESS,

  /* 4.4 Grouping content */
  HTML_ELEMENT_P,
  HTML_ELEMENT_HR,
  HTML_ELEMENT_PRE,
  HTML_ELEMENT_BLOCKQUOTE,
  HTML_ELEMENT_OL,
  HTML_ELEMENT_UL,
  HTML_ELEMENT_MENU,
  HTML_ELEMENT_LI,
  HTML_ELEMENT_DL,
  HTML_ELEMENT_DT,
  HTML_ELEMENT_DD,
  HTML_ELEMENT_FIGURE,
  HTML_ELEMENT_FIGCAPTION,
  HTML_ELEMENT_MAIN,
  HTML_ELEMENT_SEARCH,
  HTML_ELEMENT_DIV,

  /* 4.5 Text-level semantics */
  HTML_ELEMENT_A,
  HTML_ELEMENT_EM,
  HTML_ELEMENT_STRONG,
  HTML_ELEMENT_SMALL,
  HTML_ELEMENT_S,
  HTML_ELEMENT_CITE,
  HTML_ELEMENT_Q,
  HTML_ELEMENT_DFN,
  HTML_ELEMENT_ABBR,
  HTML_ELEMENT_RUBY,
  HTML_ELEMENT_RT,
  HTML_ELEMENT_RP,
  HTML_ELEMENT_DATA,
  HTML_ELEMENT_TIME,
  HTML_ELEMENT_CODE,
  HTML_ELEMENT_VAR,
  HTML_ELEMENT_SAMP,
  HTML_ELEMENT_KBD,
  HTML_ELEMENT_SUB,
  HTML_ELEMENT_SUP,
  HTML_ELEMENT_I,
  HTML_ELEMENT_B,
  HTML_ELEMENT_U,
  HTML_ELEMENT_MARK,
  HTML_ELEMENT_BDI,
  HTML_ELEMENT_BDO,
  HTML_ELEMENT_SPAN,
  HTML_ELEMENT_BR,
  HTML_ELEMENT_WBR,

  /* 4.7 Edits */
  HTML_ELEMENT_INS,
  HTML_ELEMENT_DEL,

  /* 4.8 Embedded content */
  HTML_ELEMENT_PICTURE,
  HTML_ELEMENT_SOURCE,
  HTML_ELEMENT_IMG,
  HTML_ELEMENT_IFRAME,
  HTML_ELEMENT_EMBED,
  HTML_ELEMENT_OBJECT,
  HTML_ELEMENT_VIDEO,
  HTML_ELEMENT_AUDIO,
  HTML_ELEMENT_TRACK,
  HTML_ELEMENT_MAP,
  HTML_ELEMENT_AREA,

  /* 4.9 Tabular data */
  HTML_ELEMENT_TABLE,
  HTML_ELEMENT_CAPTION,
  HTML_ELEMENT_COLGROUP,
  HTML_ELEMENT_COL,
  HTML_ELEMENT_TBODY,
  HTML_ELEMENT_THEAD,
  HTML_ELEMENT_TFOOT,
  HTML_ELEMENT_TR,
  HTML_ELEMENT_TD,
  HTML_ELEMENT_TH,

  /* 4.10 Forms */
  HTML_ELEMENT_FORM,
  HTML_ELEMENT_LABEL,
  HTML_ELEMENT_INPUT,
  HTML_ELEMENT_BUTTON,
  HTML_ELEMENT_SELECT,
  HTML_ELEMENT_DATALIST,
  HTML_ELEMENT_OPTGROUP,
  HTML_ELEMENT_OPTION,
  HTML_ELEMENT_TEXTAREA,
  HTML_ELEMENT_OUTPUT,
  HTML_ELEMENT_PROGRESS,
  HTML_ELEMENT_METER,
  HTML_ELEMENT_FIELDSET,
  HTML_ELEMENT_LEGEND,

  /* 4.11 Interactive elements */
  HTML_ELEMENT_DETAILS,
  HTML_ELEMENT_SUMMARY,
  HTML_ELEMENT_DIALOG,

  /* 4.12 Scripting */
  HTML_ELEMENT_SCRIPT,
  HTML_ELEMENT_NOSCRIPT,
  HTML_ELEMENT_TEMPLATE,
  HTML_ELEMENT_SLOT,
  HTML_ELEMENT_CANVAS,

  /* 16 Obsolete features */
  HTML_ELEMENT_APPLET,
  HTML_ELEMENT_ACRONYM,
  HTML_ELEMENT_BGSOUND,
  HTML_ELEMENT_DIR,
  HTML_ELEMENT_FRAME,
  HTML_ELEMENT_FRAMESET,
  HTML_ELEMENT_NOFRAMES,
  HTML_ELEMENT_ISINDEX,
  HTML_ELEMENT_KEYGEN,
  HTML_ELEMENT_LISTING,
  HTML_ELEMENT_MENUITEM,
  HTML_ELEMENT_NEXTID,
  HTML_ELEMENT_NOEMBED,
  HTML_ELEMENT_PARAM,
  HTML_ELEMENT_PLAINTEXT,
  HTML_ELEMENT_RB,
  HTML_ELEMENT_RTC,
  HTML_ELEMENT_STRIKE,
  HTML_ELEMENT_XMP,
  HTML_ELEMENT_BASEFONT,
  HTML_ELEMENT_BIG,
  HTML_ELEMENT_BLINK,
  HTML_ELEMENT_CENTER,
  HTML_ELEMENT_FONT,
  HTML_ELEMENT_MARQUEE,
  HTML_ELEMENT_MULTICOL,
  HTML_ELEMENT_NOBR,
  HTML_ELEMENT_SPACER,
  HTML_ELEMENT_TT,

  NUM_HTML_BUILTIN_ELEMENTS,

  HTML_ELEMENT_CUSTOM_RANGE_BEGIN_ = 256,
  HTML_ELEMENT_CUSTOM_RANGE_END_   = 65535,
};


namespace DOM {


class Document;
class HTMLElement;


};


namespace HTML {


/*
 * The main purpose of this table is to translate tag names to element indices during
 * parsing.
 * XXX: 2-way map for later
 */
DOM::HTMLElement *new_element_with_index(std::shared_ptr< DOM::Document> document,
                                         uint16_t local_name);

extern const std::unordered_map< std::string, uint16_t> k_local_names_table;


} /* namespace HTML */


#endif /* !defined(_queequeg_html_elements_hh_) */

