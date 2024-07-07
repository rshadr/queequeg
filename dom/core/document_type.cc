/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include <stdio.h>

#include "dom/core/document_type.hh"


DOM_DocumentType::DOM_DocumentType(std::shared_ptr< DOM_Document> node_document)
: DOM_Node(node_document, DOM_NODETYPE_DOCUMENT_TYPE)
{
}


DOM_DocumentType::~DOM_DocumentType()
{
  printf("Destroying doctype!\n");
}

