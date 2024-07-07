/*
 * Copyright 2024 Adrien Ricciardi
 * This file is part of the queequeg distribution (https://github.com/rshadr/queequeg)
 * See LICENSE for details
 */
#include <memory>
#include <algorithm>

#include "dom/core/node.hh"


DOM_Node::DOM_Node(std::shared_ptr< DOM_Document> node_document,
                   enum dom_node_type node_type)
{
  this->node_document = node_document;
  this->node_type = node_type;
}


void
DOM_Node::insert_node(std::shared_ptr< DOM_Node> node,
                      std::shared_ptr< DOM_Node> child,
                      bool supp_observers_flag)
{
  (void) supp_observers_flag;

  std::shared_ptr< DOM_Node> parent = std::static_pointer_cast< DOM_Node>(this->shared_from_this());

  if (child == nullptr) {
    parent->child_nodes.push_back(node);
  } else {
    auto it =
     std::find(parent->child_nodes.begin(), parent->child_nodes.end(), child);

    parent->child_nodes.insert(it, node);
  }
  node->parent_node = parent;

  /* ... */
}


void
DOM_Node::append_node(std::shared_ptr< DOM_Node> node,
                      bool supp_observers_flag)
{
  this->insert_node(node, nullptr, supp_observers_flag);
}

