#pragma once
#include "Exception.h"
#include "noncopyable.h"
#include <assert.h>
namespace muduo {
namespace base {

using tbyte = unsigned char;

DECLARE_EXCEPTION(StreamException, Exception);

class ReadStream;
class WriteStream;

class StreamStatus : public noncopyable {

public:
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

public:
    //! Constructor
    StreamStatus(void);
    //! Destructor
    virtual ~StreamStatus(void) = default;

public:
    //! Indicate the end of the stream
    static const size_t nEOS;
    static const size_t nERRORS;
    static const off_t  nOutOfRange;

    //! check the result code is whethere ok
    //! return true while nResult is neither nERRORS nor nEOS
    static inline bool resultIsOK(size_t nResult) {
        return (nResult + 1 > 1);
    }

    //! check the result code is whether reach the end of stream
    //! return true while nResult is nEOS
    static inline bool resultIsEndofStream(size_t nResult) {
        return (nResult == nEOS);
    }

    //! check the result code is whether error stream
    //! return true while nResult is nERRORS
    static inline bool resultIsError(size_t nResult) {
        return (nResult == nERRORS);
    }

    //! Return the state of the stream
    virtual typeState getState(void) const {
        return static_cast<typeState>(m_nState);
    }

    //! Return true when the stream is closed.
    bool closed(void) const {
        return (getState() & STATE_MASK) == STATE_CLOSE;
    }
    //! Return true when the stream is opened.
    bool opened(void) const {
        return (getState() & STATE_OPEN) != 0;
    }
    //! Return true when the stream is error.
    bool error(void) const {
        return (getState() & STATE_ERR) != 0;
    }
    //! Return true when the stream is empty.
    bool eof(void) const {
        return (getState() & STATE_EOF) != 0;
    }
    //! Return true when can not read any thing from the stream.include eof,error or close
    bool fail(void) const {
        return (getState() & (STATE_ERR | STATE_EOF)) != 0 || closed();
    }

protected:
    typedef size_t typeFlags;
    //! Set the state of the stream.
    void setState(typeState nState) {
        m_nState = (m_nState & Ext_STATE_MASK) | static_cast<typeFlags>(nState);
    }
    //! Set one of the states of the stream.
    void setStateBit(typeState nState) {
        m_nState |= static_cast<typeFlags>(nState);
    }
    //! Clear one of the states of the stream.
    void clearStateBit(typeState nState) {
        m_nState &= ~(static_cast<typeFlags>(nState));
    }
    //! check the bit
    bool checkExtStateBit(long nBits) const {
        nBits <<= Ext_STATE_OFFSET;
        return (static_cast<long>(m_nState) & nBits) == nBits;
    }
    //! set the ext bits
    void setExtStateBit(long nBits) {
        m_nState |= (nBits << Ext_STATE_OFFSET);
    }
    //! clear the ext bits
    void clearExtStateBit(long nBits) {
        m_nState &= ~(nBits << Ext_STATE_OFFSET);
    }
    //! clear all ext bites
    void clearAllExtStateBit(void) {
        m_nState &= STATE_MASK;
    }

protected:
    typeFlags m_nState;
};

class ReadStreamBuffer : protected noncopyable {
public:
    class iterator {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef tbyte                     value_type;
        typedef size_t                    difference_type;
        typedef size_t                    distance_type; // retained
        typedef tbyte *                   pointer;
        typedef const tbyte &             reference;

        iterator(ReadStreamBuffer &iBuffer)
            : m_pBuffer(&iBuffer) {
        }
        iterator(ReadStreamBuffer *pBuffer)
            : m_pBuffer(pBuffer) {
        }
        iterator(const iterator &it)
            : m_pBuffer(it.m_pBuffer) {
        }

        ~iterator(void) {
        }

        const tbyte &operator*(void) const {
            return *m_pBuffer->getBegin();
        }
        inline iterator &operator++(void) {
            if (m_pBuffer->moveNext() == 0 && m_pBuffer->getNext() == 0)
                m_pBuffer = NULL;
            return *this;
        }
        iterator operator++(int) {
            iterator it(*this);
            ++(*this);
            return it;
        }
        inline const tbyte *operator&(void) const {
            return m_pBuffer->getBegin();
        }
        inline bool operator==(const iterator &it) const {
            return (m_pBuffer == it.m_pBuffer);
        }
        inline bool operator!=(const iterator &it) const {
            return !(*this == it);
        }

    protected:
        ReadStreamBuffer *m_pBuffer;
    };
    typedef iterator const_iterator;

    //! return the begin/end of iterator
    iterator begin(void) {
        return iterator(this);
    }
    iterator end(void) {
        return iterator(NULL);
    }

    //! default constructor
    ReadStreamBuffer(void)
        : m_pParent(NULL)
        , m_nCacheSize(0)
        , m_lpReadBegin(NULL)
        , m_lpReadEnd(NULL)
        , m_lpBufferBegin(NULL) {
    }
    //! Init with the Input Stream and cache size
    ReadStreamBuffer(ReadStream *pParent, size_t nCacheSize);

    //! Destructor
    ~ReadStreamBuffer(void) {
        release();
    }

    //! init the buffer
    bool init(ReadStream *pParent, size_t nCacheSize);
    //! get net valid able buffer, return the byte count added into the buffer
    inline size_t getNext(size_t nSize = INFINITE_VALUE);
    //! seek the read position
    off_t seekg(off_t nOffset, int seek_begin);
    //! return the rest bytes in buffer
    inline size_t getRestCount(void) const {
        return m_lpReadEnd - m_lpReadBegin;
    }

    //! return the buffer begin pos
    inline const tbyte *getBegin(void) const {
        return m_lpReadBegin;
    }
    //! return the buffer end pos
    inline const tbyte *getEnd(void) const {
        return m_lpReadEnd;
    }
    //! return the buffer begin pos
    inline const tbyte *getBuffer(void) const {
        return m_lpBufferBegin;
    }
    //! return the total cache size
    inline size_t getCacheSize(void) const {
        return m_nCacheSize;
    }
    //! reset the buffer begin pos
    inline void setCurBegin(const tbyte *tBegin) {
        m_lpReadBegin = tBegin;
    }

    inline int sgetc(void) {
        if (m_lpReadEnd == m_lpReadBegin && getNext() == 0)
            return EOF;
        int ch = static_cast<int>(*m_lpReadBegin);
        ++m_lpReadBegin;
        return ch;
    }
    //! move the cursor to next, return the rest byte count in buffer
    inline size_t moveNext(void) {
        ++m_lpReadBegin;
        assert(m_lpReadBegin <= m_lpReadEnd);
        return m_lpReadEnd - m_lpReadBegin;
    }
    inline size_t moveNext(size_t nOffset) {
        m_lpReadBegin += nOffset;
        return m_lpReadEnd - m_lpReadBegin;
    }
    //! move the cursor to pre
    inline void pushBack(void) {
        --m_lpReadBegin;
        assert(m_lpReadBegin >= m_lpBufferBegin);
    }

    //! swap operator
    void swap(ReadStreamBuffer &buf);

    //! get Stream source
    inline ReadStream *geReadStream(void) const {
        return m_pParent;
    }

    //! released all
    void release(void);

protected:
    ReadStream *m_pParent;
    size_t       m_nCacheSize;
    const tbyte *m_lpReadBegin;
    const tbyte *m_lpReadEnd;
    const tbyte *m_lpBufferBegin;

    friend class ReadStream;
};
class WriteStreamBuffer : protected noncopyable {};
class ReadStream : protected noncopyable {};
class WriteStream : protected noncopyable {};

} // namespace base
} // namespace muduo