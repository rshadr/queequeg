/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include "dom/core/element.hh"


DOM_Element::DOM_Element(std::shared_ptr< DOM_Document> node_document)
: DOM_Node(node_document, DOM_NODETYPE_ELEMENT)
{
}

