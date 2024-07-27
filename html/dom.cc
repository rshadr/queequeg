#include "dom/html/html_element.hh"

#include "dom/html/html_html_element.hh"
#include "dom/html/html_head_element.hh"
#include "dom/html/html_script_element.hh"

#include "html/elements.hh"


namespace HTML {


DOM_HTMLElement *
new_element_with_index(std::shared_ptr< DOM_Document> document,
                       uint16_t local_name)
{
  (void) document;

  if (local_name > 0 && local_name < NUM_HTML_BUILTIN_ELEMENTS)
  {
    switch (local_name)
    {
      case HTML_ELEMENT_APPLET:  case HTML_ELEMENT_BGSOUND: case HTML_ELEMENT_BLINK:
      case HTML_ELEMENT_ISINDEX: case HTML_ELEMENT_KEYGEN:  case HTML_ELEMENT_MULTICOL:
      case HTML_ELEMENT_NEXTID:  case HTML_ELEMENT_SPACER: {
        /* XXX */
        return nullptr;
      }


      case HTML_ELEMENT_ACRONYM:  case HTML_ELEMENT_BASEFONT:  case HTML_ELEMENT_BIG:
      case HTML_ELEMENT_CENTER:   case HTML_ELEMENT_NOBR:      case HTML_ELEMENT_NOEMBED:
      case HTML_ELEMENT_NOFRAMES: case HTML_ELEMENT_PLAINTEXT: case HTML_ELEMENT_RB:
      case HTML_ELEMENT_RTC:      case HTML_ELEMENT_STRIKE:    case HTML_ELEMENT_TT: {
        return new DOM_HTMLElement(document, INFRA_NAMESPACE_HTML, local_name);
      }


      case HTML_ELEMENT_LISTING: case HTML_ELEMENT_XMP: {
        /* XXX */
        return nullptr;
      }


      /*
       * HTML Elements with their interfaces explicitly stated in their standards
       * section.
       */
      case HTML_ELEMENT_HTML: {
        return new DOM_HTMLHtmlElement(document, INFRA_NAMESPACE_HTML, local_name);
      }


      case HTML_ELEMENT_HEAD: {
        return new DOM_HTMLHeadElement(document, INFRA_NAMESPACE_HTML, local_name);
      }


      /* ... */

      case HTML_ELEMENT_SCRIPT: {
        return new DOM_HTMLScriptElement(document, INFRA_NAMESPACE_HTML, local_name);
      }


      /* ... */


      default:
        return new DOM_HTMLElement(document, INFRA_NAMESPACE_HTML, local_name);
    }
  }

  return nullptr;
}


} /* namespace HTML */

