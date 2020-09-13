#ifndef _X_VARIANT_H_2018_12_18
#define _X_VARIANT_H_2018_12_18

#include "xmtdef.h"
#include <vector>
#include <map>
#include <assert.h>
#include "base/tstring.h"
#include "base/texception.h"
NAMESPACE_BEGIN

DECLARE_EXCEPTION(TVariantTypeError, TException);
#define byte char
class TVariant {
public:
    enum VALUE_TYPE {
        TYPE_NONE = 0,   // null
        TYPE_BOOL,       // bool
        TYPE_CHAR,       // char
        TYPE_INT = 3,    // int
        TYPE_LONG = 4,   // long
        TYPE_LONG_LONG,  // long long
        TYPE_DOUBLE,     // double
        TYPE_STR,        // tstring
        TYPE_WSTR,       // twstring
        TYPE_CSTR,       // cstring
        TYPE_BIN,        // binary stream
        TYPE_DATETIME,   // datetime
        TYPE_ARRAY,      // array
        TYPE_STRUCT,     // struct
        TYPE_OBJECT      // object

    };

    typedef std::vector<TVariant> typeArray;         // list
    typedef std::map<tstring, TVariant> typeStruct;  // struct

public:  // static
    static const TVariant g_varNULL;

public:
    TVariant(void);
    //! explicit
    TVariant(char val);
    TVariant(bool val);
    TVariant(int val);
    TVariant(double val);
    TVariant(long val);
    TVariant(long long val);
    TVariant(const char *data, size_t nLength);
    TVariant(const tstring &);
    TVariant(const twstring &);
    TVariant(const typeArray &);
    TVariant(const typeStruct &);
    TVariant(VALUE_TYPE type);
    ~TVariant();

    friend ostream &operator<<(ostream &os, TVariant &obj);
    tstring toString();

    size_t size() const;
    TVariant &operator[](const tstring &key);
    TVariant &at(const tstring &key);
    TVariant &operator[](size_t nIndex);
    TVariant &at(size_t nIndex);

public:
    VALUE_TYPE getType() const;
    tstring typeString() const;
    template <class T>
    TVariant &assign(const T &val) {
        return assignVal(val);
    }
    void clear();

    TVariant &append(bool val);
    TVariant &append(char val);
    TVariant &append(int val);
    TVariant &append(double val);
    TVariant &append(long val);
    TVariant &append(const tstring &val);
    TVariant &append(const char *val);
    TVariant &append(const twstring &val);
    TVariant &append(const TVariant &val);

    template <class T, typename... Args>
    TVariant &set(const tstring &key, const T &val, Args... arg) {
        if (m_nType != TYPE_STRUCT)
            throw TVariantTypeError("variant type error. struct needed", TException::XEP_INVALID_ARGS);
        return setKeyVal(key, TVariant(val, arg...));
    }

    bool remove(const tstring &key);

    typeStruct &asStruct();
    typeArray &asArray();
    tstring asString();
    bool &asBool();
    char &asChar();
    int &asInt();
    long &asLong();
    double &asDouble();
    long long &asLongLong();

    tstring asString() const;
    bool asBool() const;
    char asChar() const;
    int asInt() const;
    long asLong() const;
    double asDouble() const;
    long long asLongLong() const;
    const typeArray &asArray() const;
    const typeStruct &asStruct() const;

    vector<tstring> keys() const;
    typeArray values() const;

private:
    TVariant &assignVal(char val);
    TVariant &assignVal(bool val);
    TVariant &assignVal(int val);
    TVariant &assignVal(double val);
    TVariant &assignVal(long val);
    TVariant &assignVal(long long val);
    TVariant &assignVal(const tstring &val);
    TVariant &assignVal(const twstring &val);
    TVariant &assignVal(const typeArray &val);
    TVariant &assignVal(const typeStruct &val);
    TVariant &setKeyVal(const tstring &key, const TVariant &val);

protected:
    union data {
        bool m_bAsBool;
        char m_cAsChar;
        int m_nAsInt;
        double *m_dAsDouble;
        long *m_lAsLong;
        long long *m_lAsLongLong;
        char *m_cAsString;
        tstring *m_ptrAsString;
        twstring *m_ptrAsWString;
        typeArray *m_ptrAsArray;
        typeStruct *m_ptrAsStruct;
    };

    data m_data{};

private:
    static std::vector<tstring> VariantTypeString;

private:
    VALUE_TYPE m_nType;
};

NAMESPACE_END
#endif
