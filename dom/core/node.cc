#include <memory>

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
  std::shared_ptr< DOM_Node> parent = std::static_pointer_cast< DOM_Node>(this->shared_from_this());

  if (child == nullptr) {
    parent->child_nodes.push_back(node);
  } else {
    /* ... */
  }
  node->parent_node = parent;

  /* ... */
}


