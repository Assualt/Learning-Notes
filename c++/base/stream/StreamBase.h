#pragma once
#include "base/Exception.h"
#include "base/Format.h"
#include "base/nonecopyable.h"

#include <cassert>
#include <cstdio>
#include <iosfwd>
#include <iostream>
#include <limits>
#include <list>
#include <utility>

namespace muduo::base {

class ReadStream;
class WriteStream;
class StreamBase;

typedef unsigned char tbyte;
//! Exception about the operations of Stream
DECLARE_EXCEPTION(StreamException, Exception)

//! The Status of stream
class StreamStatus : private nonecopyable {
public:
    enum seek_dir {
        seek_begin                = SEEK_SET, //!< begin from the start
        seek_cur [[maybe_unused]] = SEEK_CUR, //!< begin from current position
        seek_end [[maybe_unused]] = SEEK_END  //!< begin from the end
    };
    //! Indicate the state of the stream
    enum typeState {
        STATE_CLOSE = 0x00, // stream is close
        STATE_OPEN  = 0x01, // stream is open and ok
        STATE_ERR   = 0x02, // stream op error
        STATE_EOF   = 0x04, // end of stream
        STATE_MASK  = 0x07,

        Ext_STATE_OFFSET = 4,
        Ext_STATE_MASK   = ~STATE_MASK,
        Ext_STATE_Base   = 0x08,
    };

    //! Indicate the end of the stream
    static const size_t nEOS;
    static const size_t nERRORS;
    static const off_t  nOutOfRange;

    //! Constructor
    StreamStatus()
        : m_nState(STATE_CLOSE) {}
    //! Destructor
    virtual ~StreamStatus() = default;

    //! check the result code is whether ok
    //! return true while nResult is neither nERRORS nor nEOS
    static inline bool resultIsOK(size_t nResult) { return (nResult + 1 > 1); }

    //! check the result code is whether reach the end of stream
    //! return true while nResult is nEOS
    static inline bool resultIsEndofStream(size_t nResult) { return (nResult == nEOS); }

    //! check the result code is whether error stream
    //! return true while nResult is nERRORS
    static inline bool resultIsError(size_t nResult) { return (nResult == nERRORS); }

    //! Return the state of the stream
    [[nodiscard]] virtual typeState getState() const { return static_cast<typeState>(m_nState); }

    //! Return true when the stream is closed.
    [[nodiscard]] bool closed() const { return (getState() & STATE_MASK) == STATE_CLOSE; }
    //! Return true when the stream is opened.
    [[nodiscard]] bool opened() const { return (getState() & STATE_OPEN) != 0; }
    //! Return true when the stream is error.
    [[nodiscard]] bool error() const { return (getState() & STATE_ERR) != 0; }
    //! Return true when the stream is empty.
    [[nodiscard]] bool eof() const { return (getState() & STATE_EOF) != 0; }
    //! Return true when can not read any thing from the stream.include eof,error or close
    [[nodiscard]] bool fail() const { return (getState() & (STATE_ERR | STATE_EOF)) != 0 || closed(); }

protected:
    typedef size_t typeFlags;
    //! Set the state of the stream.
    void setState(typeState nState) { m_nState = (m_nState & Ext_STATE_MASK) | static_cast<typeFlags>(nState); }
    //! Set one of the states of the stream.
    void setStateBit(typeState nState) { m_nState |= static_cast<typeFlags>(nState); }
    //! Clear one of the states of the stream.
    void clearStateBit(typeState nState) { m_nState &= ~(static_cast<typeFlags>(nState)); }
    //! check the bit
    [[nodiscard]] bool checkExtStateBit(long nBits) const {
        nBits <<= Ext_STATE_OFFSET;
        return (static_cast<long>(m_nState) & nBits) == nBits;
    }
    //! set the ext bits
    void setExtStateBit(long nBits) { m_nState |= (nBits << Ext_STATE_OFFSET); }
    //! clear the ext bits
    void clearExtStateBit(long nBits) { m_nState &= ~(nBits << Ext_STATE_OFFSET); }
    //! clear all ext bites
    void clearAllExtStateBit() { m_nState &= STATE_MASK; }

private:
    typeFlags m_nState;
};

//! The class manager the input stream buffer
class ReadStreamBuffer : protected nonecopyable {
public:
    class iterator {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef tbyte                     value_type;
        typedef size_t                    difference_type;
        typedef size_t                    distance_type; // retained
        typedef tbyte                    *pointer;
        typedef const tbyte              &reference;

        explicit iterator(ReadStreamBuffer &iBuffer)
            : m_pBuffer(&iBuffer) {}

        explicit iterator(ReadStreamBuffer *pBuffer)
            : m_pBuffer(pBuffer) {}

        iterator(const iterator &it)
            : m_pBuffer(it.m_pBuffer) {}

        ~iterator() = default;

        const tbyte     &operator*() const { return *m_pBuffer->getBegin(); }
        inline iterator &operator++() {
            if (m_pBuffer->moveNext() == 0 && m_pBuffer->getNext() == 0)
                m_pBuffer = nullptr;
            return *this;
        }
        iterator operator++(int) {
            iterator it(*this);
            ++(*this);
            return it;
        }
        inline const tbyte *operator&() const { return m_pBuffer->getBegin(); }
        inline bool         operator==(const iterator &it) const { return (m_pBuffer == it.m_pBuffer); }
        inline bool         operator!=(const iterator &it) const { return !(*this == it); }

    protected:
        ReadStreamBuffer *m_pBuffer;
    };
    typedef iterator const_iterator;

    //! return the begin/end of iterator
    iterator begin() { return iterator(this); }
    iterator end() { return iterator(nullptr); }

    //! default constructor
    ReadStreamBuffer()
        : m_pParent(nullptr)
        , m_nCacheSize(0)
        , m_lpReadBegin(nullptr)
        , m_lpReadEnd(nullptr)
        , m_lpBufferBegin(nullptr) {}
    //! Init with the Input Stream and cache size
    ReadStreamBuffer(ReadStream *pParent, size_t nCacheSize);

    //! Destructor
    ~ReadStreamBuffer() { release(); }

    //! init the buffer
    bool init(ReadStream *pParent, size_t nCacheSize);
    //! get net valid able buffer, return the byte count added into the buffer
    inline size_t getNext(size_t nSize = static_cast<size_t>(-1));
    //! seek the read position
    off_t seekg(off_t nOffset, StreamStatus::seek_dir nSeekFrom = StreamStatus::seek_dir::seek_begin);
    //! return the rest bytes in buffer
    [[nodiscard]] inline size_t getRestCount() const { return m_lpReadEnd - m_lpReadBegin; }

    //! return the buffer begin pos
    [[nodiscard]] inline const tbyte *getBegin() const { return m_lpReadBegin; }
    //! return the buffer end pos
    inline const tbyte *getEnd() const { return m_lpReadEnd; }
    //! return the buffer begin pos
    [[nodiscard]] inline const tbyte *getBuffer() const { return m_lpBufferBegin; }
    //! return the total cache size
    inline size_t getCacheSize() const { return m_nCacheSize; }
    //! reset the buffer begin pos
    inline void setCurBegin(const tbyte *tBegin) { m_lpReadBegin = tBegin; }

    inline int sgetc() {
        if (m_lpReadEnd == m_lpReadBegin && getNext() == 0)
            return EOF;
        int ch = static_cast<int>(*m_lpReadBegin);
        ++m_lpReadBegin;
        return ch;
    }
    //! move the cursor to next, return the rest byte count in buffer
    inline size_t moveNext() {
        ++m_lpReadBegin;
        assert(m_lpReadBegin <= m_lpReadEnd);
        return m_lpReadEnd - m_lpReadBegin;
    }
    inline size_t moveNext(size_t nOffset) {
        m_lpReadBegin += nOffset;
        return m_lpReadEnd - m_lpReadBegin;
    }
    //! move the cursor to pre
    inline void pushBack() {
        --m_lpReadBegin;
        assert(m_lpReadBegin >= m_lpBufferBegin);
    }

    //! swap operator
    void swap(ReadStreamBuffer &buf);

    //! get Stream source
    inline ReadStream *geReadStream() const { return m_pParent; }

    //! released all
    void release();

protected:
    ReadStream  *m_pParent;
    size_t       m_nCacheSize;
    const tbyte *m_lpReadBegin;
    const tbyte *m_lpReadEnd;
    const tbyte *m_lpBufferBegin;

    friend class ReadStream;
};

//! The class manager the output stream buffer
class WriteStreamBuffer : protected nonecopyable {
public:
    class [[maybe_unused]] write_iterator {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef tbyte                     value_type;
        typedef tbyte                    *pointer;
        typedef const tbyte              &reference;

        explicit write_iterator(WriteStreamBuffer &iBuffer)
            : m_pBuffer(&iBuffer) {}
        explicit write_iterator(WriteStreamBuffer *pBuffer)
            : m_pBuffer(pBuffer) {}
        write_iterator(const write_iterator &it)
            : m_pBuffer(it.m_pBuffer) {}
        ~write_iterator() {}

        tbyte &operator*() { return *m_pBuffer->getBegin(); }
        tbyte  operator*() const { return *m_pBuffer->getBegin(); }

        write_iterator &operator++() {
            if (m_pBuffer->moveNext() == 0) {
                m_pBuffer->getNext();
                if (m_pBuffer->getCacheSize() == 0)
                    m_pBuffer = nullptr;
            } // if (m_pBuffer->moveNext() == 0)
            return *this;
        }
        write_iterator operator++(int) {
            write_iterator it(*this);
            ++(*this);
            return it;
        }
        bool operator==(const write_iterator &it) const { return (m_pBuffer == it.m_pBuffer); }
        bool operator!=(const write_iterator &it) const { return !(*this == it); }

    protected:
        WriteStreamBuffer *m_pBuffer;
    };

    //! Default constructor
    WriteStreamBuffer()
        : m_pParent(nullptr)
        , m_lpWriteBegin(nullptr)
        , m_lpWriteEnd(nullptr)
        , m_lpBufferBegin(nullptr) {}
    //! constructor with output Stream and cache size
    WriteStreamBuffer(WriteStream *pParent, size_t nCacheSize);
    //! destructor
    ~WriteStreamBuffer() {
        try {
            release();
        } catch (...) {
        }
    }

    //! init the buffer
    bool init(WriteStream *pParent, size_t nCacheSize);
    //! get net valid able buffer, return the bytes count be flushed.
    inline size_t getNext();
    //! seek the new write position
    size_t seekp(off_t nOffset, StreamStatus::seek_dir nSeekFrom = StreamStatus::seek_dir::seek_begin);

    //! return the buffer begin pos
    inline tbyte *getBegin() const { return m_lpWriteBegin; }
    //! return the buffer end pos
    inline tbyte *getEnd() const { return m_lpWriteEnd; }
    //! reset the buffer begin pos
    inline void setCurBegin(tbyte *tBegin) { m_lpWriteBegin = tBegin; }
    //! return the buffer begin pos
    inline tbyte *getBuffer() const { return m_lpBufferBegin; }
    //! return the total cache size
    inline size_t getCacheSize() const { return m_lpWriteEnd - m_lpBufferBegin; }

    //! return the free buffer size can be writen
    inline size_t getFreeSize() const { return m_lpWriteEnd - m_lpWriteBegin; }
    //! return the data count in the buffer
    inline size_t getDataInBuffer() const { return m_lpWriteBegin - m_lpBufferBegin; }

    //! move the curset to next
    inline size_t moveNext() {
        ++m_lpWriteBegin;
        assert(m_lpWriteBegin <= m_lpWriteEnd);
        return m_lpWriteEnd - m_lpWriteBegin;
    }
    inline size_t moveNext(size_t nOffset) {
        m_lpWriteBegin += nOffset;
        assert(m_lpWriteBegin <= m_lpWriteEnd);
        return m_lpWriteEnd - m_lpWriteBegin;
    }

    inline void sputc(int ch) {
        if (++m_lpWriteBegin >= m_lpWriteEnd)
            getNext();
        *m_lpWriteBegin = static_cast<tbyte>(ch);
    }
    //! swap operator
    void swap(WriteStreamBuffer &buf);

    //! get Stream source
    inline WriteStream *geWriteStream() const { return m_pParent; }
    void                release();
    size_t              flush();

protected:
    WriteStream *m_pParent;
    tbyte       *m_lpWriteBegin;
    tbyte       *m_lpWriteEnd;
    tbyte       *m_lpBufferBegin;
    friend class WriteStream;
};

//! The base class of Readable stream
class ReadStream : virtual public StreamStatus {
public:
    //! Constructor
    ReadStream(StreamStatus::typeState nState = STATE_CLOSE)
        : StreamStatus() {
        setState(nState);
    }
    //! Destructor
    virtual ~ReadStream() {}

    //! try to read nSize bytes into lpBuf, until reach the end of the stream of error occur
    inline size_t readBlock(void *lpBuf, size_t nSize) {
        size_t nReadNum = read(lpBuf, nSize);
        if (nReadNum == nSize || !StreamStatus::resultIsOK(nReadNum))
            return nReadNum;
        return loopRead(lpBuf, nSize, nReadNum);
    }

    //! try to read nSize bytes into lpBuf, until reach the end of the stream of error occur,
    //! if read not finish, will throw Exception
    inline void readBlockException(void *lpBuf, size_t nSize) {
        if (readBlock(lpBuf, nSize) != nSize)
            throw StreamException(FmtString("read data (%)byte(s) error!").arg(nSize).str());
    }

    //! Virtual Read function. Returns the number of bytes read, which may be less than nSize if there are fewer than
    //! count bytes left.
    virtual size_t read(void *lpBuf, size_t nSize) = 0;
    //! Clear the input buffer of the stream, and return the count of bytes which is clear
    virtual size_t clearInput() { return 0; }

    //! Virtual Seek function. Seek the read position, return the current postion if succeed, else return ERRORS
    virtual off_t seekg(off_t nOffset, StreamStatus::seek_dir nSeekFrom = seek_begin);
    //! Virtual Tell function, return the current read postion if succeed, else return ERRORS
    virtual off_t tellg();

protected:
    inline void getReadBufferImp(const ReadStreamBuffer &buf, const tbyte *&pBuffer, size_t &nSize,
                                 const tbyte *&pReadBegin, const tbyte *&pReadEnd) {
        pBuffer    = buf.m_lpBufferBegin;
        nSize      = buf.m_nCacheSize;
        pReadBegin = buf.m_lpReadBegin;
        pReadEnd   = buf.m_lpReadEnd;
    }
    inline void setReadBufferImp(ReadStreamBuffer &buf, const tbyte *pBuffer, size_t nSize, const tbyte *pReadBegin,
                                 const tbyte *pReadEnd) {
        buf.m_lpBufferBegin = pBuffer;
        buf.m_nCacheSize    = nSize;
        buf.m_lpReadBegin   = pReadBegin;
        buf.m_lpReadEnd     = pReadEnd;
    }

    //! \param nSuggestSize	suggest at least nSuggestSize bytes in buffer.
    virtual void   allocReadBuffer(ReadStreamBuffer &buf, size_t nSuggestSize);
    virtual void   releaseReadBuffer(ReadStreamBuffer &buf);
    virtual size_t fillReadBuffer(ReadStreamBuffer &buf, size_t nSuggestSize);

    friend class ReadStreamBuffer;

private:
    size_t loopRead(void *lpBuf, size_t nSize, size_t nHasReadSize);
};

//! The base class of Writable stream
class WriteStream : virtual public StreamStatus {
public:
    //! Constructor
    WriteStream(StreamStatus::typeState nState = STATE_CLOSE)
        : StreamStatus() {
        setState(nState);
    }

    //! Destructor
    virtual ~WriteStream() {}

    struct TWriteDataBlocks {
        const void *data;
        size_t      size;
    };

    //! try to write nSize bytes into lpBuf, until reach the end of the stream of error occur
    inline size_t writeBlock(const void *lpBuf, size_t nSize) {
        size_t nWriteNum = write(lpBuf, nSize);
        if (nWriteNum == nSize || !StreamStatus::resultIsOK(nWriteNum))
            return nWriteNum;
        return loopWrite(lpBuf, nSize, nWriteNum);
    }
    virtual size_t writeBlocks(const TWriteDataBlocks *pDatas, size_t n, size_t nOffset);

    //! try to write nSize bytes into lpBuf, until reach the end of the stream of error occur,
    //! if write not finish, will throw Exception
    inline void writeBlockException(const void *lpBuf, size_t nSize) {
        if (writeBlock(lpBuf, nSize) != nSize)
            throw StreamException(FmtString("write data (%)byte(s) error!").arg(nSize).str());
    }
    void writeBlocksException(const TWriteDataBlocks *pDatas, size_t n, size_t nOffset);

    //! Virtual Write function. Returns the number of bytes actually written.
    virtual size_t write(const void *lpBuf, size_t nSize) = 0;
    virtual size_t write(const TWriteDataBlocks *pDatas, size_t n, size_t nOffset);

    //! Flush the output buffer of the stream, and return the count of bytes which is flush to stream
    virtual size_t flush() { return 0; }

    //! Virtual Seek function. Seek the write position, return the current postion if succeed, else return ERRORS
    virtual off_t seekp(off_t nOffset, seek_dir nSeekFrom = seek_begin);
    //! Virtual Tell function, return the current write postion if succeed, else return ERRORS
    virtual off_t tellp();

    //! Truncate, reset the size of the stream
    virtual bool truncate(off_t nSize) { return false; }

protected:
    inline void getWriteBufferImp(const WriteStreamBuffer &buf, tbyte *&pBuffer, tbyte *&pBufferEnd,
                                  tbyte *&pWritePos) {
        pBuffer    = buf.m_lpBufferBegin;
        pWritePos  = buf.m_lpWriteBegin;
        pBufferEnd = buf.m_lpWriteEnd;
    }
    inline void setWriteBufferImp(WriteStreamBuffer &buf, tbyte *pBuffer, tbyte *pBufferEnd, tbyte *pWritePos) {
        buf.m_lpBufferBegin = pBuffer;
        buf.m_lpWriteBegin  = pWritePos;
        buf.m_lpWriteEnd    = pBufferEnd;
    }

    virtual void   allocWriteBuffer(WriteStreamBuffer &buf, size_t nSuggestSize);
    virtual void   releaseWriteBuffer(WriteStreamBuffer &buf);
    virtual size_t flushWriteBuffer(WriteStreamBuffer &buf, bool boJustFlush);

    friend class WriteStreamBuffer;

private:
    size_t loopWrite(const void *lpBuf, size_t nSize, size_t nHasWrite);
};

//! The base class of stream
class StreamBase : public ReadStream, public WriteStream {
public:
    //! Destructor
    ~StreamBase() override = default;

    //! Virtual Seek function. Seek the write position, return the current postion if succeed, else return ERRORS
    off_t seekp(off_t nOffset, seek_dir nSeekFrom = seek_begin) override { return seekg(nOffset, nSeekFrom); }
    //! Virtual Tell function, return the current write postion if succeed, else return ERRORS
    off_t tellp() override { return tellg(); }

    static size_t copyStreamToStream(ReadStream *pSrc, WriteStream *pTar, size_t nCopyData,
                                     size_t nCachSize = 4 * 1024);
    static size_t copyStreamToStream(ReadStreamBuffer &buf, WriteStream *pTar, size_t nCopyData);
    static size_t copyStreamToStream(ReadStream *pSrc, WriteStreamBuffer &buf, size_t nCopyData);

    static size_t copyStreamToStreamBuffer(ReadStream *pSrc, WriteStream *pTar, size_t nCopyData,
                                           size_t nCachSize = 4 * 1024);

protected:
    //! Constructor, used by the derived class
    explicit StreamBase(StreamStatus::typeState nState = STATE_CLOSE)
        : ReadStream(nState)
        , WriteStream(nState) {}
};

//////////////////////////////////////////////////////////////////////////

inline size_t ReadStreamBuffer::getNext(size_t nSize) {
    assert(m_pParent != nullptr && m_lpBufferBegin != nullptr);
    return m_pParent->fillReadBuffer(*this, nSize);
}

inline size_t WriteStreamBuffer::getNext() {
    assert(m_pParent != nullptr && m_lpBufferBegin != nullptr);
    return m_pParent->flushWriteBuffer(*this, false);
}

} // namespace muduo::base