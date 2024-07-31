#include "dom/core/node.hh"


namespace DOM {


TreeNodeDFSIterator::TreeNodeDFSIterator(const TreeNodeDFSIterator& it)
{
  this->chain_ = it.chain_;
  this->root_vec_ = it.root_vec_;

  this->chain_.front() = {

    /*
     * Fix up iterators (they aren't the same anymore)
     */
    (it.chain_.front().cur == it.root_vec_.begin())
      ? this->root_vec_.begin()
      : this->root_vec_.end(),

    this->root_vec_.end()
  };

}


TreeNodeDFSIterator::value_type
TreeNodeDFSIterator::operator*() const
{
  const ChainLink_& last_link = this->chain_.back();

  return *last_link.cur;
}


bool
TreeNodeDFSIterator::operator==(const TreeNodeDFSIterator& other_it) const
{
  ChainLink_ link = this->chain_.back();
  ChainLink_ other_link = other_it.chain_.back();

  /*
   * Comparing iterators instead of nodes to cover the case of an 'end()'
   * iterator.
   */

  return (link.cur == other_link.cur);
}


TreeNodeDFSIterator&
TreeNodeDFSIterator::operator++()
{
  /* XXX: exception if empty/end */
  ChainLink_& last_link = this->chain_.back();

  /*
   * If we have reached the end of the child vector of a node, we now process
   * the node itself.
   */
  if (last_link.cur == last_link.end) {
    this->chain_.pop_back();
    return *this;
  }

  /*
   * Advance where there are things to look at.
   */
  ++last_link.cur;

  /*
   * Drop down along the current branch.
   */
  this->fall_through_branch_();

  return *this;
}


TreeNodeDFSIterator
TreeNodeDFSIterator::operator++(int)
{
  auto tmp = *this;
  ++*this;
  return tmp;
}


/*
 * These two functions initializer the 'root_vec_'; the purpose of it is to
 * have a local iterator that doesn't depend on the parent (possibly
 * non-existing) of the 'root_node'. It is then used both as a means to
 * differentiate between 'end()' and non-'end()', as well as to process
 * the root itself
 */
TreeNodeDFSIterator
TreeNodeDFSIterator::make_begin(std::shared_ptr< DOM::Node> root_node)
{
  TreeNodeDFSIterator it;

  it.root_vec_ = { root_node };

  it.chain_.push_back({
    it.root_vec_.begin(),
    it.root_vec_.end()
  });

  it.fall_through_branch_();

#if 0
  it.chain_.push_back({
    root_node->child_nodes.begin(),
    root_node->child_nodes.end()
  });
#endif

  return it;
}

TreeNodeDFSIterator
TreeNodeDFSIterator::make_end(std::shared_ptr< DOM::Node> root_node)
{
  TreeNodeDFSIterator it;

  it.root_vec_ = { root_node };

  it.chain_.push_back({
    it.root_vec_.end(),
    it.root_vec_.end()
  });

  it.fall_through_branch_();

#if 0
  it.chain_.push_back({
    root_node->child_nodes.end(),
    root_node->child_nodes.end()
  });
#endif

  return it;
}


void
TreeNodeDFSIterator::fall_through_branch_(void)
{
  ChainLink_& last_link = this->chain_.back();

  while (! (*last_link.cur)->child_nodes.empty() ) {
    ChainLink_ new_link = {
      (*last_link.cur)->child_nodes.begin(),
      (*last_link.cur)->child_nodes.end()
    };

    this->chain_.push_back(new_link);
    last_link = this->chain_.back();
  }

}


} /* namespace DOM */

