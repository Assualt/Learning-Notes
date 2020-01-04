#ifndef _X_REF_H_2018_12_18
#define _X_REF_H_2018_12_18

#include "xmtdef.h"
#include <exception>

NAMESPACE_BEGIN

template <class Ty>
class auto_ptr_ary;

template <class Ty>
struct auto_ptr_ary_ref {
    //! construct from compatible auto_ptr
    auto_ptr_ary_ref(auto_ptr_ary<Ty>& Right);
    //! reference to constructor argument
    auto_ptr_ary<Ty>& Ref;
};

//! wrap an object pointer to ensure destruction
template <class _Ty>
class auto_ptr_ary {
public:
    //! construct from object pointer
    explicit auto_ptr_ary(_Ty* ptr) throw() : m_ptr(ptr) {}
    //! construct by assuming pointer from Right auto_ptr_ref
    auto_ptr_ary(auto_ptr_ary_ref<_Ty> Right) throw();

    //! construct by assuming pointer from Right
    template <class Other>
    auto_ptr_ary(auto_ptr_ary<Other>& Right) throw();

private:
    _Ty* m_ptr;
};

NAMESPACE_END
#endif  //
