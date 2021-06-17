#include "base/noncopyable.h"
#include "base/texception.h"
#include <string.h>
#include <memory>
namespace muduo {
namespace base {

template<class val_type>
class StreamBaseImpl : public noncopyable {
public:
    using pointer = val_type *;
    using reference = val_type&;
    using size_type = size_t;

public:
    StreamBaseImpl() 
        : ptr_(nullptr), size_(0), size_offset(0), size_capcity(16), expand_ratio(0.75) {
        CheckCapcity(0);
    }

    ~StreamBaseImpl() {
        if(ptr_) 
            delete ptr_;
    }

    size_type writeBlock(const pointer val, size_type nBytes) {
        if(nullptr == val)
            return size_type(0);
        CheckCapcity(nBytes);
        memcpy(ptr_+ size_, val, nBytes);
        return nBytes;
    }

    size_type readBlock(pointer buf, size_type nBytes) {
        if(buf == nullptr)
            return size_type(0);
        memcpy(buf, ptr_ + size_offset, nBytes);
        size_offset += nBytes;
        return nBytes;
    }

    void seekPos(size_type nPos) {
        size_offset = nPos;
    }       

protected:
    void CheckCapcity(size_type nbytes) {
        if(size_ == 0) {
            ptr_ = new val_type[size_capcity];
            memset(ptr_, 0, size_capcity;
        } else if (size_ * 1.0 / size_capcity > expand_ratio) {
            size_capcity = size_capcity << 1;
            std::auto_ptr<val_type> _au(new val_type[size_capcity]);
            memcpy(_au.get(), ptr_, size_);
            delete [] ptr_;
            ptr_ = _au.release();
        } else if(nbytes + size_ > size_capcity) {
            size_capcity = (size_ + nbytes ) << 1;
            std::auto_ptr<val_type> _au(new val_type[size_capcity]);
            memcpy(_au.get(), ptr_, size_);
            delete [] ptr_;
            ptr_ = _au.release();
        }
    }    
protected:
    pointer ptr_;
    size_type size_;
    size_type size_offset;
    size_type size_capcity;
    double expand_ratio;
};



using BinaryStream = base::StreamBaseImpl<char>;
using WBinaryStream = base::StreamBaseImpl<wchar_t>;
}
}