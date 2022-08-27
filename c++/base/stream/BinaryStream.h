#pragma once
#include "StreamBase.h"
#include <stdio.h>

namespace muduo {
namespace base {

enum TAutoDeleteFlag { ADF_NOT_AUTO_DELETE = 0x00, ADF_AUTO_DELETE = 0x01 };

class BinaryStream;

//! Binary Stream, a class in order to store the binary data,
//! and provide a stream interface for access
class BinaryStream : public StreamBase {
public:
    //! Default Constructor
    //! nAutoGroupUpSize == -1 , will add the buffer when need, and the group up size equ the need
    //! nAutoGroupUpSize == 0 , will not group up
    BinaryStream(int nAutoGroupUpSize = -1);
    //! Construct a BinaryStream, attach any data, and it will not auto-delete the lpszBuffer
    BinaryStream(void *lpszBuffer, size_t nSize);
    //! Destructor
    ~BinaryStream(void);

    //! not the virtual function return the current get position
    off_t tellpos(void) const { return m_nCurPos; }

    //! Virtual function of the base class, return the current access position of data.
    virtual off_t tellg(void);
    //! Virtual function of the base class, Set the access position.
    virtual off_t seekg(off_t nPos, StreamStatus::seek_dir nSeekFrom = seek_begin);

    //! Virtual function of the base class, Read some data from storage.
    virtual size_t read(void *buf, size_t nbyte);
    //! Virtual function of the base class, Write some data from storage.
    virtual size_t write(const void *buf, size_t nbyte);
    //! Virtual function of the base class, Resize the whole size of the storage.
    virtual bool truncate(off_t nSize);

    //! Clear all data in the storage
    void clear(void);
    //! Return true when the data is attach in this storage
    bool isAttach(void) const { return !checkExtStateBit(BUF_TYPE_AUTODELETE); }
    //! Attach any data.
    //! the storage will not auto-delete the data, if boReference == true
    void attach(void *lpBuffer, size_t nLength, TAutoDeleteFlag boAutoDel);
    //! Detach the data controlled by the storage
    void *detach(void);

    //! Clear the data, and reserve the space
    void clearAndReserve(size_t nSize);
    //! Reserve the space, will not clear the data.
    void reserve(size_t nSize);
    //! Reset the auto group up size. return the old value
    int resetAutoGroupUpSize(int nNewVal);
    //! Return the size of whole data
    size_t size(void) const { return m_nBufLen; }
    //! reset the size of whole data
    void resetSize(size_t nSize, tbyte bFillter = 0);
    //! Return the size of data can be read
    size_t rest_size(void) const { return m_nBufLen - m_nCurPos; }
    //! Return the capacity space of the storage
    size_t capacity(void) const { return m_nBufMaxLen - m_nBufLen; }
    size_t capacity2(void) const { return m_nBufMaxLen; }
    //! Return the whole space of the storage
    size_t max_size(void) const { return m_nBufMaxLen; }
    //! Return the data address from the begining of the storage
    void *data(void) const { return m_lpBuffer; }
    //! Return the data address from the current access position of the storage
    void *current_data(void) const { return m_lpBuffer + m_nCurPos; }

    //!	return the byte indicated by the index
    tbyte &operator[](size_t index);
    //!	return the byte indicated by the index
    tbyte operator[](size_t index) const;

    //! reset the size of whole data and not fill the buffer
    void resetSizeWithoutFill(size_t nSize);

protected:
    virtual void   allocReadBuffer(ReadStreamBuffer &buf, size_t nSuggestSize);
    virtual void   releaseReadBuffer(ReadStreamBuffer &buf);
    virtual size_t fillReadBuffer(ReadStreamBuffer &buf, size_t nSuggestSize);

    virtual void   allocWriteBuffer(WriteStreamBuffer &buf, size_t nSuggestSize);
    virtual void   releaseWriteBuffer(WriteStreamBuffer &buf);
    virtual size_t flushWriteBuffer(WriteStreamBuffer &buf, bool boJustFlush);

    void setGoodState(void);

    enum tBufferType {
        BUF_TYPE_AUTODELETE = 0x01,
    };

    tbyte *m_lpBuffer;
    size_t m_nBufLen;
    size_t m_nBufMaxLen;
    off_t  m_nCurPos;
    int    m_nAutoGroupUp;
};

} // namespace base
} // namespace muduo