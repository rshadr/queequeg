/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include "dom/core/element.hh"


namespace DOM {


Element::Element(std::shared_ptr< DOM::Document> node_document,
                         enum InfraNamespace name_space,
                         uint16_t local_name)
: DOM::Node(node_document, DOM_NODETYPE_ELEMENT)
{
  this->name_space = name_space;
  this->local_name = local_name;
}


} /* namespace DOM */

