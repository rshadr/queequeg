/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */

#include <unordered_map>
#include <string>

#include "html/elements.hh"


namespace HTML {

/*
 * This table should in principle never get accessed per se; it is used as a base
 * for any HTML documents own lookup table, which can get expanded dynamically
 * through custom elements, for example.
 */

const std::unordered_map< std::string, uint16_t> k_local_names_table = {
  /* 4.1 The document element */
  { "html", HTML_ELEMENT_HTML },


  /* 4.2 Document metadata */
  { "head", HTML_ELEMENT_HEAD },
  { "title", HTML_ELEMENT_TITLE },
  { "base", HTML_ELEMENT_BASE },
  { "link", HTML_ELEMENT_LINK },
  { "meta", HTML_ELEMENT_META },
  { "style", HTML_ELEMENT_STYLE },

  /* 4.3 Sections */
  { "body", HTML_ELEMENT_BODY },
  { "article", HTML_ELEMENT_ARTICLE },
  { "section", HTML_ELEMENT_SECTION },
  { "nav", HTML_ELEMENT_NAV },
  { "aside", HTML_ELEMENT_ASIDE },
  { "h1", HTML_ELEMENT_H1 },
  { "h2", HTML_ELEMENT_H2 },
  { "h3", HTML_ELEMENT_H3 },
  { "h4", HTML_ELEMENT_H4 },
  { "h5", HTML_ELEMENT_H5 },
  { "h6", HTML_ELEMENT_H6 },
  { "hgroup", HTML_ELEMENT_HGROUP },
  { "header", HTML_ELEMENT_HEADER },
  { "footer", HTML_ELEMENT_FOOTER },
  { "address", HTML_ELEMENT_ADDRESS },

  /* 4.4 Grouping content */
  { "p", HTML_ELEMENT_P },
  { "hr", HTML_ELEMENT_HR },
  { "pre", HTML_ELEMENT_PRE },
  { "blockquote", HTML_ELEMENT_BLOCKQUOTE },
  { "ol", HTML_ELEMENT_OL },
  { "ul", HTML_ELEMENT_UL },
  { "menu", HTML_ELEMENT_MENU },
  { "li", HTML_ELEMENT_LI },
  { "dl", HTML_ELEMENT_DL },
  { "dd", HTML_ELEMENT_DD },
  { "figure", HTML_ELEMENT_FIGURE },
  { "figcaption", HTML_ELEMENT_FIGCAPTION },
  { "main", HTML_ELEMENT_MAIN },
  { "search", HTML_ELEMENT_SEARCH },
  { "div", HTML_ELEMENT_DIV },

  /* 4.5 Text-level semantics */
  { "a", HTML_ELEMENT_A },
  { "em", HTML_ELEMENT_EM },
  { "strong", HTML_ELEMENT_STRONG },
  { "small", HTML_ELEMENT_SMALL },
  { "s", HTML_ELEMENT_S },
  { "cite", HTML_ELEMENT_CITE },
  { "q", HTML_ELEMENT_Q },
  { "dfn", HTML_ELEMENT_DFN },
  { "abbr", HTML_ELEMENT_ABBR },
  { "ruby", HTML_ELEMENT_RUBY },
  { "rt", HTML_ELEMENT_RT },
  { "rp", HTML_ELEMENT_RP },
  { "data", HTML_ELEMENT_DATA },
  { "time", HTML_ELEMENT_TIME },
  { "code", HTML_ELEMENT_CODE },
  { "var", HTML_ELEMENT_VAR },
  { "samp", HTML_ELEMENT_SAMP },
  { "kbd", HTML_ELEMENT_KBD },
  { "sub", HTML_ELEMENT_SUB },
  { "sup", HTML_ELEMENT_SUP },
  { "i", HTML_ELEMENT_I },
  { "b", HTML_ELEMENT_B },
  { "u", HTML_ELEMENT_U },
  { "mark", HTML_ELEMENT_MARK },
  { "bdi", HTML_ELEMENT_BDI },
  { "bdo", HTML_ELEMENT_BDO },
  { "span", HTML_ELEMENT_SPAN },
  { "br", HTML_ELEMENT_BR },
  { "wbr", HTML_ELEMENT_WBR },

  /* 4.7 Edits */
  { "ins", HTML_ELEMENT_INS },
  { "del", HTML_ELEMENT_DEL },

  /* 4.8 Embedded content */
  { "picture", HTML_ELEMENT_PICTURE },
  { "source", HTML_ELEMENT_SOURCE },
  { "img", HTML_ELEMENT_IMG },
  { "iframe", HTML_ELEMENT_IFRAME },
  { "embed", HTML_ELEMENT_EMBED },
  { "object", HTML_ELEMENT_OBJECT },
  { "video", HTML_ELEMENT_VIDEO },
  { "audio", HTML_ELEMENT_AUDIO },
  { "track", HTML_ELEMENT_TRACK },
  { "map", HTML_ELEMENT_MAP },
  { "area", HTML_ELEMENT_AREA },

  /* 4.9 Tabular data */
  { "table", HTML_ELEMENT_TABLE },
  { "caption", HTML_ELEMENT_CAPTION },
  { "colgroup", HTML_ELEMENT_COLGROUP },
  { "col", HTML_ELEMENT_COL },
  { "tbody", HTML_ELEMENT_TBODY },
  { "thead", HTML_ELEMENT_THEAD },
  { "tfoot", HTML_ELEMENT_TFOOT },
  { "tr", HTML_ELEMENT_TR },
  { "td", HTML_ELEMENT_TD },
  { "th", HTML_ELEMENT_TH },

  /* 4.10 Forms */
  { "form", HTML_ELEMENT_FORM },
  { "label", HTML_ELEMENT_LABEL },
  { "input", HTML_ELEMENT_INPUT },
  { "button", HTML_ELEMENT_BUTTON },
  { "select", HTML_ELEMENT_SELECT },
  { "datalist", HTML_ELEMENT_DATALIST },
  { "optgroup", HTML_ELEMENT_OPTGROUP },
  { "option", HTML_ELEMENT_OPTION },
  { "textarea", HTML_ELEMENT_TEXTAREA },
  { "output", HTML_ELEMENT_OUTPUT },
  { "progress", HTML_ELEMENT_PROGRESS },
  { "meter", HTML_ELEMENT_METER },
  { "fieldset", HTML_ELEMENT_FIELDSET },
  { "legend", HTML_ELEMENT_LEGEND },

  /* 4.11 Interactive elements */
  { "details", HTML_ELEMENT_DETAILS },
  { "summary", HTML_ELEMENT_SUMMARY },
  { "dialog", HTML_ELEMENT_DIALOG },

  /* 4.12 Scripting */
  { "script", HTML_ELEMENT_SCRIPT },
  { "noscript", HTML_ELEMENT_NOSCRIPT },
  { "template", HTML_ELEMENT_TEMPLATE },
  { "slot", HTML_ELEMENT_SLOT },
  { "canvas", HTML_ELEMENT_CANVAS },

  /* 16 Obsolete features */
  { "applet", HTML_ELEMENT_APPLET },
  { "acronym", HTML_ELEMENT_ACRONYM },
  { "bgsound", HTML_ELEMENT_BGSOUND },
  { "dir", HTML_ELEMENT_DIR },
  { "frame", HTML_ELEMENT_FRAME },
  { "frameset", HTML_ELEMENT_FRAMESET },
  { "noframes", HTML_ELEMENT_NOFRAMES },
  /* ... */
  { "tt", HTML_ELEMENT_TT },

};


} /* namespace HTML */

