//
// Generated file, do not edit! Created by nedtool 5.6 from rover/applications/PositionMapPacket.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include "PositionMapPacket_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace {
template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)(static_cast<const omnetpp::cObject *>(t));
}

template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && !std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)dynamic_cast<const void *>(t);
}

template <class T> inline
typename std::enable_if<!std::is_polymorphic<T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)static_cast<const void *>(t);
}

}


// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule to generate operator<< for shared_ptr<T>
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const std::shared_ptr<T>& t) { return out << t.get(); }

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');

    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

Register_Class(PositionMapPacket)

PositionMapPacket::PositionMapPacket() : ::inet::ApplicationPacket()
{
}

PositionMapPacket::PositionMapPacket(const PositionMapPacket& other) : ::inet::ApplicationPacket(other)
{
    copy(other);
}

PositionMapPacket::~PositionMapPacket()
{
    delete [] this->cellX;
    delete [] this->cellY;
    delete [] this->cellCount;
    delete [] this->mTime;
}

PositionMapPacket& PositionMapPacket::operator=(const PositionMapPacket& other)
{
    if (this == &other) return *this;
    ::inet::ApplicationPacket::operator=(other);
    copy(other);
    return *this;
}

void PositionMapPacket::copy(const PositionMapPacket& other)
{
    this->nodeId = other.nodeId;
    for (size_t i = 0; i < 2; i++) {
        this->cellId[i] = other.cellId[i];
    }
    this->numCells = other.numCells;
    delete [] this->cellX;
    this->cellX = (other.cellX_arraysize==0) ? nullptr : new int[other.cellX_arraysize];
    cellX_arraysize = other.cellX_arraysize;
    for (size_t i = 0; i < cellX_arraysize; i++) {
        this->cellX[i] = other.cellX[i];
    }
    delete [] this->cellY;
    this->cellY = (other.cellY_arraysize==0) ? nullptr : new int[other.cellY_arraysize];
    cellY_arraysize = other.cellY_arraysize;
    for (size_t i = 0; i < cellY_arraysize; i++) {
        this->cellY[i] = other.cellY[i];
    }
    delete [] this->cellCount;
    this->cellCount = (other.cellCount_arraysize==0) ? nullptr : new int[other.cellCount_arraysize];
    cellCount_arraysize = other.cellCount_arraysize;
    for (size_t i = 0; i < cellCount_arraysize; i++) {
        this->cellCount[i] = other.cellCount[i];
    }
    delete [] this->mTime;
    this->mTime = (other.mTime_arraysize==0) ? nullptr : new omnetpp::simtime_t[other.mTime_arraysize];
    mTime_arraysize = other.mTime_arraysize;
    for (size_t i = 0; i < mTime_arraysize; i++) {
        this->mTime[i] = other.mTime[i];
    }
}

void PositionMapPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::ApplicationPacket::parsimPack(b);
    doParsimPacking(b,this->nodeId);
    doParsimArrayPacking(b,this->cellId,2);
    doParsimPacking(b,this->numCells);
    b->pack(cellX_arraysize);
    doParsimArrayPacking(b,this->cellX,cellX_arraysize);
    b->pack(cellY_arraysize);
    doParsimArrayPacking(b,this->cellY,cellY_arraysize);
    b->pack(cellCount_arraysize);
    doParsimArrayPacking(b,this->cellCount,cellCount_arraysize);
    b->pack(mTime_arraysize);
    doParsimArrayPacking(b,this->mTime,mTime_arraysize);
}

void PositionMapPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::ApplicationPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->nodeId);
    doParsimArrayUnpacking(b,this->cellId,2);
    doParsimUnpacking(b,this->numCells);
    delete [] this->cellX;
    b->unpack(cellX_arraysize);
    if (cellX_arraysize == 0) {
        this->cellX = nullptr;
    } else {
        this->cellX = new int[cellX_arraysize];
        doParsimArrayUnpacking(b,this->cellX,cellX_arraysize);
    }
    delete [] this->cellY;
    b->unpack(cellY_arraysize);
    if (cellY_arraysize == 0) {
        this->cellY = nullptr;
    } else {
        this->cellY = new int[cellY_arraysize];
        doParsimArrayUnpacking(b,this->cellY,cellY_arraysize);
    }
    delete [] this->cellCount;
    b->unpack(cellCount_arraysize);
    if (cellCount_arraysize == 0) {
        this->cellCount = nullptr;
    } else {
        this->cellCount = new int[cellCount_arraysize];
        doParsimArrayUnpacking(b,this->cellCount,cellCount_arraysize);
    }
    delete [] this->mTime;
    b->unpack(mTime_arraysize);
    if (mTime_arraysize == 0) {
        this->mTime = nullptr;
    } else {
        this->mTime = new omnetpp::simtime_t[mTime_arraysize];
        doParsimArrayUnpacking(b,this->mTime,mTime_arraysize);
    }
}

int PositionMapPacket::getNodeId() const
{
    return this->nodeId;
}

void PositionMapPacket::setNodeId(int nodeId)
{
    handleChange();
    this->nodeId = nodeId;
}

size_t PositionMapPacket::getCellIdArraySize() const
{
    return 2;
}

int PositionMapPacket::getCellId(size_t k) const
{
    if (k >= 2) throw omnetpp::cRuntimeError("Array of size 2 indexed by %lu", (unsigned long)k);
    return this->cellId[k];
}

void PositionMapPacket::setCellId(size_t k, int cellId)
{
    if (k >= 2) throw omnetpp::cRuntimeError("Array of size 2 indexed by %lu", (unsigned long)k);
    handleChange();
    this->cellId[k] = cellId;
}

int PositionMapPacket::getNumCells() const
{
    return this->numCells;
}

void PositionMapPacket::setNumCells(int numCells)
{
    handleChange();
    this->numCells = numCells;
}

size_t PositionMapPacket::getCellXArraySize() const
{
    return cellX_arraysize;
}

int PositionMapPacket::getCellX(size_t k) const
{
    if (k >= cellX_arraysize) throw omnetpp::cRuntimeError("Array of size cellX_arraysize indexed by %lu", (unsigned long)k);
    return this->cellX[k];
}

void PositionMapPacket::setCellXArraySize(size_t newSize)
{
    handleChange();
    int *cellX2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = cellX_arraysize < newSize ? cellX_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        cellX2[i] = this->cellX[i];
    for (size_t i = minSize; i < newSize; i++)
        cellX2[i] = 0;
    delete [] this->cellX;
    this->cellX = cellX2;
    cellX_arraysize = newSize;
}

void PositionMapPacket::setCellX(size_t k, int cellX)
{
    if (k >= cellX_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    this->cellX[k] = cellX;
}

void PositionMapPacket::insertCellX(size_t k, int cellX)
{
    handleChange();
    if (k > cellX_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = cellX_arraysize + 1;
    int *cellX2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        cellX2[i] = this->cellX[i];
    cellX2[k] = cellX;
    for (i = k + 1; i < newSize; i++)
        cellX2[i] = this->cellX[i-1];
    delete [] this->cellX;
    this->cellX = cellX2;
    cellX_arraysize = newSize;
}

void PositionMapPacket::insertCellX(int cellX)
{
    insertCellX(cellX_arraysize, cellX);
}

void PositionMapPacket::eraseCellX(size_t k)
{
    if (k >= cellX_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    size_t newSize = cellX_arraysize - 1;
    int *cellX2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        cellX2[i] = this->cellX[i];
    for (i = k; i < newSize; i++)
        cellX2[i] = this->cellX[i+1];
    delete [] this->cellX;
    this->cellX = cellX2;
    cellX_arraysize = newSize;
}

size_t PositionMapPacket::getCellYArraySize() const
{
    return cellY_arraysize;
}

int PositionMapPacket::getCellY(size_t k) const
{
    if (k >= cellY_arraysize) throw omnetpp::cRuntimeError("Array of size cellY_arraysize indexed by %lu", (unsigned long)k);
    return this->cellY[k];
}

void PositionMapPacket::setCellYArraySize(size_t newSize)
{
    handleChange();
    int *cellY2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = cellY_arraysize < newSize ? cellY_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        cellY2[i] = this->cellY[i];
    for (size_t i = minSize; i < newSize; i++)
        cellY2[i] = 0;
    delete [] this->cellY;
    this->cellY = cellY2;
    cellY_arraysize = newSize;
}

void PositionMapPacket::setCellY(size_t k, int cellY)
{
    if (k >= cellY_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    this->cellY[k] = cellY;
}

void PositionMapPacket::insertCellY(size_t k, int cellY)
{
    handleChange();
    if (k > cellY_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = cellY_arraysize + 1;
    int *cellY2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        cellY2[i] = this->cellY[i];
    cellY2[k] = cellY;
    for (i = k + 1; i < newSize; i++)
        cellY2[i] = this->cellY[i-1];
    delete [] this->cellY;
    this->cellY = cellY2;
    cellY_arraysize = newSize;
}

void PositionMapPacket::insertCellY(int cellY)
{
    insertCellY(cellY_arraysize, cellY);
}

void PositionMapPacket::eraseCellY(size_t k)
{
    if (k >= cellY_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    size_t newSize = cellY_arraysize - 1;
    int *cellY2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        cellY2[i] = this->cellY[i];
    for (i = k; i < newSize; i++)
        cellY2[i] = this->cellY[i+1];
    delete [] this->cellY;
    this->cellY = cellY2;
    cellY_arraysize = newSize;
}

size_t PositionMapPacket::getCellCountArraySize() const
{
    return cellCount_arraysize;
}

int PositionMapPacket::getCellCount(size_t k) const
{
    if (k >= cellCount_arraysize) throw omnetpp::cRuntimeError("Array of size cellCount_arraysize indexed by %lu", (unsigned long)k);
    return this->cellCount[k];
}

void PositionMapPacket::setCellCountArraySize(size_t newSize)
{
    handleChange();
    int *cellCount2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = cellCount_arraysize < newSize ? cellCount_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        cellCount2[i] = this->cellCount[i];
    for (size_t i = minSize; i < newSize; i++)
        cellCount2[i] = 0;
    delete [] this->cellCount;
    this->cellCount = cellCount2;
    cellCount_arraysize = newSize;
}

void PositionMapPacket::setCellCount(size_t k, int cellCount)
{
    if (k >= cellCount_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    this->cellCount[k] = cellCount;
}

void PositionMapPacket::insertCellCount(size_t k, int cellCount)
{
    handleChange();
    if (k > cellCount_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = cellCount_arraysize + 1;
    int *cellCount2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        cellCount2[i] = this->cellCount[i];
    cellCount2[k] = cellCount;
    for (i = k + 1; i < newSize; i++)
        cellCount2[i] = this->cellCount[i-1];
    delete [] this->cellCount;
    this->cellCount = cellCount2;
    cellCount_arraysize = newSize;
}

void PositionMapPacket::insertCellCount(int cellCount)
{
    insertCellCount(cellCount_arraysize, cellCount);
}

void PositionMapPacket::eraseCellCount(size_t k)
{
    if (k >= cellCount_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    size_t newSize = cellCount_arraysize - 1;
    int *cellCount2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        cellCount2[i] = this->cellCount[i];
    for (i = k; i < newSize; i++)
        cellCount2[i] = this->cellCount[i+1];
    delete [] this->cellCount;
    this->cellCount = cellCount2;
    cellCount_arraysize = newSize;
}

size_t PositionMapPacket::getMTimeArraySize() const
{
    return mTime_arraysize;
}

omnetpp::simtime_t PositionMapPacket::getMTime(size_t k) const
{
    if (k >= mTime_arraysize) throw omnetpp::cRuntimeError("Array of size mTime_arraysize indexed by %lu", (unsigned long)k);
    return this->mTime[k];
}

void PositionMapPacket::setMTimeArraySize(size_t newSize)
{
    handleChange();
    omnetpp::simtime_t *mTime2 = (newSize==0) ? nullptr : new omnetpp::simtime_t[newSize];
    size_t minSize = mTime_arraysize < newSize ? mTime_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        mTime2[i] = this->mTime[i];
    for (size_t i = minSize; i < newSize; i++)
        mTime2[i] = SIMTIME_ZERO;
    delete [] this->mTime;
    this->mTime = mTime2;
    mTime_arraysize = newSize;
}

void PositionMapPacket::setMTime(size_t k, omnetpp::simtime_t mTime)
{
    if (k >= mTime_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    this->mTime[k] = mTime;
}

void PositionMapPacket::insertMTime(size_t k, omnetpp::simtime_t mTime)
{
    handleChange();
    if (k > mTime_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = mTime_arraysize + 1;
    omnetpp::simtime_t *mTime2 = new omnetpp::simtime_t[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        mTime2[i] = this->mTime[i];
    mTime2[k] = mTime;
    for (i = k + 1; i < newSize; i++)
        mTime2[i] = this->mTime[i-1];
    delete [] this->mTime;
    this->mTime = mTime2;
    mTime_arraysize = newSize;
}

void PositionMapPacket::insertMTime(omnetpp::simtime_t mTime)
{
    insertMTime(mTime_arraysize, mTime);
}

void PositionMapPacket::eraseMTime(size_t k)
{
    if (k >= mTime_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    size_t newSize = mTime_arraysize - 1;
    omnetpp::simtime_t *mTime2 = (newSize == 0) ? nullptr : new omnetpp::simtime_t[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        mTime2[i] = this->mTime[i];
    for (i = k; i < newSize; i++)
        mTime2[i] = this->mTime[i+1];
    delete [] this->mTime;
    this->mTime = mTime2;
    mTime_arraysize = newSize;
}

class PositionMapPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_nodeId,
        FIELD_cellId,
        FIELD_numCells,
        FIELD_cellX,
        FIELD_cellY,
        FIELD_cellCount,
        FIELD_mTime,
    };
  public:
    PositionMapPacketDescriptor();
    virtual ~PositionMapPacketDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(PositionMapPacketDescriptor)

PositionMapPacketDescriptor::PositionMapPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(PositionMapPacket)), "inet::ApplicationPacket")
{
    propertynames = nullptr;
}

PositionMapPacketDescriptor::~PositionMapPacketDescriptor()
{
    delete[] propertynames;
}

bool PositionMapPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<PositionMapPacket *>(obj)!=nullptr;
}

const char **PositionMapPacketDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *PositionMapPacketDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int PositionMapPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 7+basedesc->getFieldCount() : 7;
}

unsigned int PositionMapPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_nodeId
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_cellId
        FD_ISEDITABLE,    // FIELD_numCells
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_cellX
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_cellY
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_cellCount
        FD_ISARRAY,    // FIELD_mTime
    };
    return (field >= 0 && field < 7) ? fieldTypeFlags[field] : 0;
}

const char *PositionMapPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "nodeId",
        "cellId",
        "numCells",
        "cellX",
        "cellY",
        "cellCount",
        "mTime",
    };
    return (field >= 0 && field < 7) ? fieldNames[field] : nullptr;
}

int PositionMapPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'n' && strcmp(fieldName, "nodeId") == 0) return base+0;
    if (fieldName[0] == 'c' && strcmp(fieldName, "cellId") == 0) return base+1;
    if (fieldName[0] == 'n' && strcmp(fieldName, "numCells") == 0) return base+2;
    if (fieldName[0] == 'c' && strcmp(fieldName, "cellX") == 0) return base+3;
    if (fieldName[0] == 'c' && strcmp(fieldName, "cellY") == 0) return base+4;
    if (fieldName[0] == 'c' && strcmp(fieldName, "cellCount") == 0) return base+5;
    if (fieldName[0] == 'm' && strcmp(fieldName, "mTime") == 0) return base+6;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *PositionMapPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_nodeId
        "int",    // FIELD_cellId
        "int",    // FIELD_numCells
        "int",    // FIELD_cellX
        "int",    // FIELD_cellY
        "int",    // FIELD_cellCount
        "omnetpp::simtime_t",    // FIELD_mTime
    };
    return (field >= 0 && field < 7) ? fieldTypeStrings[field] : nullptr;
}

const char **PositionMapPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *PositionMapPacketDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int PositionMapPacketDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    PositionMapPacket *pp = (PositionMapPacket *)object; (void)pp;
    switch (field) {
        case FIELD_cellId: return 2;
        case FIELD_cellX: return pp->getCellXArraySize();
        case FIELD_cellY: return pp->getCellYArraySize();
        case FIELD_cellCount: return pp->getCellCountArraySize();
        case FIELD_mTime: return pp->getMTimeArraySize();
        default: return 0;
    }
}

const char *PositionMapPacketDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    PositionMapPacket *pp = (PositionMapPacket *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string PositionMapPacketDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    PositionMapPacket *pp = (PositionMapPacket *)object; (void)pp;
    switch (field) {
        case FIELD_nodeId: return long2string(pp->getNodeId());
        case FIELD_cellId: return long2string(pp->getCellId(i));
        case FIELD_numCells: return long2string(pp->getNumCells());
        case FIELD_cellX: return long2string(pp->getCellX(i));
        case FIELD_cellY: return long2string(pp->getCellY(i));
        case FIELD_cellCount: return long2string(pp->getCellCount(i));
        case FIELD_mTime: return simtime2string(pp->getMTime(i));
        default: return "";
    }
}

bool PositionMapPacketDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    PositionMapPacket *pp = (PositionMapPacket *)object; (void)pp;
    switch (field) {
        case FIELD_nodeId: pp->setNodeId(string2long(value)); return true;
        case FIELD_cellId: pp->setCellId(i,string2long(value)); return true;
        case FIELD_numCells: pp->setNumCells(string2long(value)); return true;
        case FIELD_cellX: pp->setCellX(i,string2long(value)); return true;
        case FIELD_cellY: pp->setCellY(i,string2long(value)); return true;
        case FIELD_cellCount: pp->setCellCount(i,string2long(value)); return true;
        default: return false;
    }
}

const char *PositionMapPacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *PositionMapPacketDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    PositionMapPacket *pp = (PositionMapPacket *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

