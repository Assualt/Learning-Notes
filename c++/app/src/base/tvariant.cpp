#include "base/tvariant.h"

NAMESPACE_BEGIN

std::vector<tstring> TVariant::VariantTypeString = {
        // NO LIMIT
        "[NONE]",
        "[BOOL]",
        "[CHAR]",
        "[INT]",
        "[LONG]",
        "[ULON]",
        "[DOUB]",
        "[STR]",
        "[WSTR]",
        "[CSTR]",
        "[BIN]",
        "[DAT]",
        "[ARY]",
        "[STU]",
        "[OBJ]",
};

TVariant::TVariant() : m_nType(TYPE_NONE) {}

TVariant::TVariant(bool val) : m_nType(TYPE_BOOL) {
    m_data.m_bAsBool = val;
}

TVariant::TVariant(char val) : m_nType(TYPE_CHAR) {
    m_data.m_cAsChar = val;
}

TVariant::TVariant(int val) : m_nType(TYPE_INT) {
    m_data.m_nAsInt = val;
}

TVariant::TVariant(double val) : m_nType(TYPE_DOUBLE) {
    m_data.m_dAsDouble = new double(val);
}

TVariant::TVariant(long val) : m_nType(TYPE_LONG) {
    m_data.m_lAsLong = new long(val);
}
TVariant::TVariant(long long val) : m_nType(TYPE_LONG_LONG) {
    m_data.m_lAsLongLong = new long long(val);
}

TVariant::TVariant(const tstring &res) : m_nType(TYPE_STR) {
    m_data.m_ptrAsString = new tstring;
    m_data.m_ptrAsString->assign(res);
}

TVariant::TVariant(const twstring &res) : m_nType(TYPE_WSTR) {
    m_data.m_ptrAsWString = new twstring(res);
}

TVariant::TVariant(const char *data, size_t nLength) : m_nType(TYPE_CSTR) {
    if (nLength == static_cast<size_t>(-1))
        nLength = strlen(data);
    m_data.m_cAsString = new char[nLength];
    memset(m_data.m_cAsString, 0, nLength);
    strncpy(m_data.m_cAsString, data, nLength);
    m_data.m_cAsString[nLength] = '\0';
}

TVariant::TVariant(const typeArray &array) {
    m_data.m_ptrAsArray = new typeArray(array);
}

TVariant::TVariant(const typeStruct &structData) {
    m_data.m_ptrAsStruct = new typeStruct(structData);
}

void TVariant::clear() {
    switch (m_nType) {
    case TYPE_CSTR:
        delete m_data.m_cAsString;
        break;
    case TYPE_STR:
        delete m_data.m_ptrAsString;
        break;
    case TYPE_LONG:
        delete m_data.m_lAsLong;
        break;
    case TYPE_LONG_LONG:
        delete m_data.m_lAsLongLong;
        break;
    case TYPE_WSTR:
        delete m_data.m_ptrAsWString;
        break;
    case TYPE_ARRAY:
        delete m_data.m_ptrAsArray;
        break;
    case TYPE_STRUCT:
        delete m_data.m_ptrAsStruct;
        break;
    default:
        break;
    }
}

TVariant::~TVariant() {
    clear();
}

TVariant::TVariant(TVariant::VALUE_TYPE type) : m_nType(type) {
    switch (m_nType) {
    case TYPE_DOUBLE:
        m_data.m_dAsDouble = new double(0);
        break;
    case TYPE_LONG:
        m_data.m_lAsLong = new long(0);
        break;
    case TYPE_CSTR:
        m_data.m_cAsString = new char;
        break;
    case TYPE_STR:
        m_data.m_ptrAsString = new tstring;
        break;
    case TYPE_WSTR:
        m_data.m_ptrAsWString = new twstring;
        break;
    case TYPE_ARRAY:
        m_data.m_ptrAsArray = new typeArray;
        break;
    case TYPE_STRUCT:
        m_data.m_ptrAsStruct = new typeStruct;
        break;
    default:
        break;
    }
}

TVariant::VALUE_TYPE TVariant::getType() const {
    return m_nType;
}

tstring TVariant::typeString() const {
    return VariantTypeString[static_cast<int>(getType())];
}

TVariant &TVariant::assignVal(char val) {
    if (getType() != TYPE_CHAR)
        throw TVariantTypeError("variant type error");
    this->m_data.m_cAsChar = val;
    return *this;
}

TVariant &TVariant::assignVal(bool val) {
    if (getType() != TYPE_BOOL)
        throw TVariantTypeError("variant type error");
    this->m_data.m_bAsBool = val;
    return *this;
}

TVariant &TVariant::assignVal(int val) {
    if (getType() != TYPE_INT)
        throw TVariantTypeError("variant type error");
    this->m_data.m_nAsInt = val;
    return *this;
}

TVariant &TVariant::assignVal(double val) {
    if (getType() != TYPE_DOUBLE)
        throw TVariantTypeError("variant type error");
    this->m_data.m_dAsDouble = new double(val);
    return *this;
}

TVariant &TVariant::assignVal(long val) {
    if (getType() != TYPE_LONG)
        throw TVariantTypeError("variant type error");
    this->m_data.m_lAsLong = new long(val);
    return *this;
}
TVariant &TVariant::assignVal(long long val) {
    if (getType() != TYPE_LONG_LONG)
        throw TVariantTypeError("variant type error");
    this->m_data.m_lAsLongLong = new long long(val);
    return *this;
}

TVariant &TVariant::assignVal(const tstring &val) {
    if (getType() != TYPE_STR)
        throw TVariantTypeError("variant type error");
    *(this->m_data.m_ptrAsString) = val;
    return *this;
}

TVariant &TVariant::assignVal(const twstring &val) {
    if (getType() != TYPE_WSTR)
        throw TVariantTypeError("variant type error");
    *(this->m_data.m_ptrAsWString) = val;
    return *this;
}

TVariant &TVariant::assignVal(const typeArray &val) {
    if (getType() != TYPE_ARRAY)
        throw TVariantTypeError("variant type error");
    *(this->m_data.m_ptrAsArray) = val;
    return *this;
}
TVariant &TVariant::assignVal(const typeStruct &val) {
    if (getType() != TYPE_ARRAY)
        throw TVariantTypeError("variant type error");
    *(this->m_data.m_ptrAsStruct) = val;
    return *this;
}

tstring TVariant::toString() {
    std::stringstream ss;
    switch (m_nType) {
    case TYPE_BOOL:
        ss << typeString() << ":" << m_data.m_bAsBool << "[END]";
        break;
    case TYPE_CHAR:
        ss << typeString() << ":" << m_data.m_cAsChar << "[END]";
        break;
    case TYPE_INT:
        ss << typeString() << ":" << m_data.m_nAsInt << "[END]";
        break;
    case TYPE_DOUBLE:
        ss << typeString() << ":" << *m_data.m_dAsDouble << "[END]";
        break;
    case TYPE_LONG:
        ss << typeString() << ":" << *m_data.m_lAsLong << "[END]";
        break;
    case TYPE_CSTR:
        ss << typeString() << ":" << m_data.m_cAsString << "[END]";
        break;
    case TYPE_STR:
        ss << typeString() << ":" << m_data.m_ptrAsString->c_str() << "[END]";
        break;
    case TYPE_WSTR:
        ss << typeString() << ":"
           << "[END]";
        break;
    case TYPE_ARRAY: {
        ss << typeString() << ":{";
        size_t nSize = size();
        for (size_t i = 0; i < nSize; ++i) {
            auto item = m_data.m_ptrAsArray->operator[](i);
            ss << item.toString();
            if (i != nSize - 1)
                ss << ",";
        }
        ss << "}";
    } break;
    case TYPE_STRUCT: {
        ss << typeString() << ":{";
        size_t nIndex = 0, nSize = size();
        for (auto item : *(m_data.m_ptrAsStruct)) {
            ss << "\"" << item.first << "\":\"" << item.second.toString() << "\"";
            if (nIndex++ != nSize - 1)
                ss << ",";
        }
        ss << "}";
    } break;
    default:
        break;
    }
    return ss.str();
}

TVariant &TVariant::append(bool val) {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error. array needed", TException::XEP_INVALID_ARGS);
    m_data.m_ptrAsArray->push_back(TVariant(val));
    return *this;
}

TVariant &TVariant::append(char val) {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error. array needed", TException::XEP_INVALID_ARGS);
    m_data.m_ptrAsArray->push_back(TVariant(val));
    return *this;
}

TVariant &TVariant::append(int val) {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error. array needed", TException::XEP_INVALID_ARGS);
    m_data.m_ptrAsArray->push_back(TVariant(val));
    return *this;
}

TVariant &TVariant::append(double val) {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error. array needed", TException::XEP_INVALID_ARGS);
    m_data.m_ptrAsArray->push_back(TVariant(val));
    return *this;
}

TVariant &TVariant::append(long val) {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error. array needed", TException::XEP_INVALID_ARGS);
    m_data.m_ptrAsArray->push_back(TVariant(val));
    return *this;
}

TVariant &TVariant::append(const char *val) {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error. array needed", TException::XEP_INVALID_ARGS);
    m_data.m_ptrAsArray->push_back(TVariant(val, strlen(val)));
    return *this;
}

TVariant &TVariant::append(const tstring &val) {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error. array needed", TException::XEP_INVALID_ARGS);
    m_data.m_ptrAsArray->push_back(TVariant(val));
    return *this;
}

TVariant &TVariant::append(const twstring &val) {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error. array needed", TException::XEP_INVALID_ARGS);
    m_data.m_ptrAsArray->push_back(TVariant(val));
    return *this;
}

TVariant &TVariant::append(const TVariant &val) {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error. array needed", TException::XEP_INVALID_ARGS);
    m_data.m_ptrAsArray->push_back(val);
    return *this;
}

TVariant &TVariant::setKeyVal(const tstring &key, const TVariant &val) {
    m_data.m_ptrAsStruct->insert({key, val});
    return *this;
}
bool TVariant::remove(const tstring &key) {
    if (m_nType != TYPE_STRUCT)
        throw TVariantTypeError("variant type error", TException::XEP_INVALID_ARGS);
    if (m_data.m_ptrAsStruct->erase(key))
        return true;
    return false;
}

ostream &operator<<(ostream &os, TVariant &obj) {
    os << obj.toString();
    return os;
}

size_t TVariant::size() const {
    if (m_nType == TYPE_ARRAY) {
        return m_data.m_ptrAsArray->size();
    } else if (m_nType == TYPE_STRUCT) {
        return m_data.m_ptrAsStruct->size();
    }
    return 0;
}

TVariant &TVariant::operator[](const tstring &key) {
    if (m_nType != TYPE_STRUCT)
        throw TVariantTypeError("variant type error. struct needed", TException::XEP_INVALID_ARGS);
    return m_data.m_ptrAsStruct->operator[](key);
}
TVariant &TVariant::at(const tstring &key) {
    return m_data.m_ptrAsStruct->at(key);
}

TVariant &TVariant::operator[](size_t nIndex) {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error. array needed", TException::XEP_INVALID_ARGS);
    else if (nIndex > m_data.m_ptrAsArray->size() || nIndex < 0)
        throw std::out_of_range("range error");
    return m_data.m_ptrAsArray->at(nIndex);
}

TVariant &TVariant::at(size_t nIndex) {
    return this->operator[](nIndex);
}

TVariant::typeStruct &TVariant::asStruct() {
    if (m_nType != TYPE_STRUCT)
        throw TVariantTypeError("variant type error");
    return *m_data.m_ptrAsStruct;
}

const TVariant::typeStruct &TVariant::asStruct() const {
    if (m_nType != TYPE_STRUCT)
        throw TVariantTypeError("variant type error");
    return *m_data.m_ptrAsStruct;
}

TVariant::typeArray &TVariant::asArray() {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error");
    return *m_data.m_ptrAsArray;
}

const TVariant::typeArray &TVariant::asArray() const {
    if (m_nType != TYPE_ARRAY)
        throw TVariantTypeError("variant type error");
    return *m_data.m_ptrAsArray;
}

tstring TVariant::asString() const {
    if (m_nType == TYPE_CSTR)
        return m_data.m_cAsString;
    else if (m_nType == TYPE_STR)
        return *m_data.m_ptrAsString;
    return "";
}
bool TVariant::asBool() const {
    if (m_nType != TYPE_BOOL)
        throw TVariantTypeError("variant type error");
    return m_data.m_bAsBool;
}
char TVariant::asChar() const {
    if (m_nType != TYPE_CHAR)
        throw TVariantTypeError("variant type error");
    return m_data.m_cAsChar;
}
int TVariant::asInt() const {
    if (m_nType != TYPE_INT)
        throw TVariantTypeError("variant type error");
    return m_data.m_nAsInt;
}
long TVariant::asLong() const {
    if (m_nType != TYPE_LONG)
        throw TVariantTypeError("variant type error");
    return *m_data.m_lAsLong;
}
double TVariant::asDouble() const {
    if (m_nType != TYPE_DOUBLE)
        throw TVariantTypeError("variant type error");
    return *m_data.m_dAsDouble;
}
long long TVariant::asLongLong() const {
    if (m_nType != TYPE_LONG_LONG)
        throw TVariantTypeError("variant type error");
    return *m_data.m_lAsLongLong;
}
tstring TVariant::asString() {
    return *m_data.m_ptrAsString;
}
bool &TVariant::asBool() {
    if (m_nType != TYPE_BOOL)
        throw TVariantTypeError("variant type error");
    return m_data.m_bAsBool;
}
char &TVariant::asChar() {
    if (m_nType != TYPE_CHAR)
        throw TVariantTypeError("variant type error");
    return m_data.m_cAsChar;
}
int &TVariant::asInt() {
    if (m_nType != TYPE_INT)
        throw TVariantTypeError("variant type error");
    return m_data.m_nAsInt;
}
long &TVariant::asLong() {
    if (m_nType != TYPE_LONG)
        throw TVariantTypeError("variant type error");
    return *m_data.m_lAsLong;
}
double &TVariant::asDouble() {
    if (m_nType != TYPE_DOUBLE)
        throw TVariantTypeError("variant type error");
    return *m_data.m_dAsDouble;
}
long long &TVariant::asLongLong() {
    if (m_nType != TYPE_LONG_LONG)
        throw TVariantTypeError("variant type error");
    return *m_data.m_lAsLongLong;
}

std::vector<tstring> TVariant::keys() const {
    if (m_nType != TYPE_STRUCT)
        throw TVariantTypeError("variant type error. struct needed", TException::XEP_INVALID_ARGS);
    std::vector<tstring> result;
    for (auto it : asStruct()) {
        result.push_back(it.first);
    }
    return result;
}
TVariant::typeArray TVariant::values() const {
    if (m_nType != TYPE_STRUCT)
        throw TVariantTypeError("variant type error. struct needed", TException::XEP_INVALID_ARGS);
    typeArray result;
    for (auto it : asStruct()) {
        result.push_back(it.second);
    }
    return result;
}

NAMESPACE_END
