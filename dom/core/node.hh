#ifndef _queequeg_dom_node_hh_
#define _queequeg_dom_node_hh_

#include <cstddef>
#include <vector>
#include <memory>

#include "dom/events/event_target.hh"

#include "qglib/iterator.hh"




enum dom_node_type {
  DOM_NODETYPE_ELEMENT                = 1,
  DOM_NODETYPE_ATTRIBUTE              = 2,
  DOM_NODETYPE_TEXT                   = 3,
  DOM_NODETYPE_CDATA_SECTION          = 4,
  DOM_NODETYPE_ENTITY_REFERENCE       = 5, /* legacy */
  DOM_NODETYPE_ENTITY                 = 6, /* legacy */
  DOM_NODETYPE_PROCESSING_INSTRUCTION = 7,
  DOM_NODETYPE_COMMENT                = 8,
  DOM_NODETYPE_DOCUMENT               = 9,
  DOM_NODETYPE_DOCUMENT_TYPE          = 10,
  DOM_NODETYPE_DOCUMENT_FRAGMENT      = 11,
  DOM_NODETYPE_NOTATION               = 12, /* legacy */
};


namespace DOM {


class Document;
class TreeNodeDFSIterator;


class Node : public DOM::EventTarget {
  protected:
    Node(std::shared_ptr< Document> node_document,
         enum dom_node_type node_type);
  public:
    virtual ~Node() = default;

  public:
    std::weak_ptr< Document> node_document;
    enum dom_node_type node_type;

    std::weak_ptr< Node> parent_node;
    std::vector< std::shared_ptr< Node>> child_nodes;

    inline bool is_element(void) const;
    inline bool is_text(void) const;
    inline bool is_document(void) const;


    std::shared_ptr< Node> get_previous_sibling(void);

    void insert_node(std::shared_ptr< Node> node,
                     std::shared_ptr< Node> child,
                     bool supp_observers = false);

    void append_node(std::shared_ptr< Node> node, bool supp_observers = false);
};


inline bool
Node::is_element(void) const
{
  return (this->node_type == DOM_NODETYPE_ELEMENT);
}

inline bool
Node::is_text(void) const
{
  return (this->node_type == DOM_NODETYPE_TEXT);
}

inline bool
Node::is_document(void) const
{
  return (this->node_type == DOM_NODETYPE_DOCUMENT);
}



struct TreeNodeDFSIterator {
  using difference_type = std::ptrdiff_t;
  using value_type = std::shared_ptr< DOM::Node>;


  TreeNodeDFSIterator(void) = default;
  TreeNodeDFSIterator(const TreeNodeDFSIterator& it);

  ~TreeNodeDFSIterator() = default;


  value_type operator*() const;

  bool operator==(const TreeNodeDFSIterator& other_it) const;

  TreeNodeDFSIterator& operator++();
  TreeNodeDFSIterator operator++(int);


  static TreeNodeDFSIterator make_begin(std::shared_ptr< DOM::Node> root_node);
  static TreeNodeDFSIterator make_end(std::shared_ptr< DOM::Node> root_node);


  private:
    void fall_through_branch_(void);

    using ChainLink_ = struct {
      std::vector< std::shared_ptr< DOM::Node>>::iterator cur;
      std::vector< std::shared_ptr< DOM::Node>>::iterator end;
    };

    std::vector< std::shared_ptr< DOM::Node>> root_vec_;
    std::vector< ChainLink_> chain_ = { };
};

static_assert(std::forward_iterator<TreeNodeDFSIterator>);


} /* namespace DOM */


#endif /* !defined(_queequeg_dom_node_hh_) */

