#ifndef _queequeg_qglib_iterator_hh_
#define _queequeg_qglib_iterator_hh_


#include <iterator>


namespace QueequegLib {


template< typename Class,
          typename Iterator,
          typename Generator>
concept generates_thin_iterator_point = true;


template< typename Class,
          typename Iterator,
          generates_thin_iterator_point<Class, Iterator> MakeBegin,
          generates_thin_iterator_point<Class, Iterator> MakeEnd>
class ThinIterator final {
  friend Class;

  protected:
    ThinIterator(Class *pobj) : pobj_(pobj) { }
    ~ThinIterator() = default;

  public:
    Iterator begin(void)
    {
      return MakeBegin(this->pobj_);
    }

    Iterator end(void) final
    {
      return MakeEnd(this->pobj_);
    }

  private:
    Class *pobj_;
};


}; /* namespace QueequegLib */


#endif /* !defined(_queequeg_qglib_iterator_hh_) */


