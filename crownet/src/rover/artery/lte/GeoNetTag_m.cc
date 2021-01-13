//
// Generated file, do not edit! Created by nedtool 5.6 from rover/artery/lte/GeoNetTag.msg.
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
#include "GeoNetTag_m.h"

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

namespace rover {

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

Register_Class(GeoNetTag)

GeoNetTag::GeoNetTag() : ::inet::TagBase()
{
}

GeoNetTag::GeoNetTag(const GeoNetTag& other) : ::inet::TagBase(other)
{
    copy(other);
}

GeoNetTag::~GeoNetTag()
{
}

GeoNetTag& GeoNetTag::operator=(const GeoNetTag& other)
{
    if (this == &other) return *this;
    ::inet::TagBase::operator=(other);
    copy(other);
    return *this;
}

void GeoNetTag::copy(const GeoNetTag& other)
{
    this->srcAddrIp = other.srcAddrIp;
    this->dstAddrIp = other.dstAddrIp;
    this->srcAddrMac = other.srcAddrMac;
    this->dstAddrMac = other.dstAddrMac;
}

void GeoNetTag::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::TagBase::parsimPack(b);
    doParsimPacking(b,this->srcAddrIp);
    doParsimPacking(b,this->dstAddrIp);
    doParsimPacking(b,this->srcAddrMac);
    doParsimPacking(b,this->dstAddrMac);
}

void GeoNetTag::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::TagBase::parsimUnpack(b);
    doParsimUnpacking(b,this->srcAddrIp);
    doParsimUnpacking(b,this->dstAddrIp);
    doParsimUnpacking(b,this->srcAddrMac);
    doParsimUnpacking(b,this->dstAddrMac);
}

uint32_t GeoNetTag::getSrcAddrIp() const
{
    return this->srcAddrIp;
}

void GeoNetTag::setSrcAddrIp(uint32_t srcAddrIp)
{
    this->srcAddrIp = srcAddrIp;
}

uint32_t GeoNetTag::getDstAddrIp() const
{
    return this->dstAddrIp;
}

void GeoNetTag::setDstAddrIp(uint32_t dstAddrIp)
{
    this->dstAddrIp = dstAddrIp;
}

const inet::MacAddress& GeoNetTag::getSrcAddrMac() const
{
    return this->srcAddrMac;
}

void GeoNetTag::setSrcAddrMac(const inet::MacAddress& srcAddrMac)
{
    this->srcAddrMac = srcAddrMac;
}

const inet::MacAddress& GeoNetTag::getDstAddrMac() const
{
    return this->dstAddrMac;
}

void GeoNetTag::setDstAddrMac(const inet::MacAddress& dstAddrMac)
{
    this->dstAddrMac = dstAddrMac;
}

class GeoNetTagDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_srcAddrIp,
        FIELD_dstAddrIp,
        FIELD_srcAddrMac,
        FIELD_dstAddrMac,
    };
  public:
    GeoNetTagDescriptor();
    virtual ~GeoNetTagDescriptor();

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

Register_ClassDescriptor(GeoNetTagDescriptor)

GeoNetTagDescriptor::GeoNetTagDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::GeoNetTag)), "inet::TagBase")
{
    propertynames = nullptr;
}

GeoNetTagDescriptor::~GeoNetTagDescriptor()
{
    delete[] propertynames;
}

bool GeoNetTagDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<GeoNetTag *>(obj)!=nullptr;
}

const char **GeoNetTagDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *GeoNetTagDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int GeoNetTagDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount() : 4;
}

unsigned int GeoNetTagDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_srcAddrIp
        FD_ISEDITABLE,    // FIELD_dstAddrIp
        0,    // FIELD_srcAddrMac
        0,    // FIELD_dstAddrMac
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *GeoNetTagDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "srcAddrIp",
        "dstAddrIp",
        "srcAddrMac",
        "dstAddrMac",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int GeoNetTagDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 's' && strcmp(fieldName, "srcAddrIp") == 0) return base+0;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dstAddrIp") == 0) return base+1;
    if (fieldName[0] == 's' && strcmp(fieldName, "srcAddrMac") == 0) return base+2;
    if (fieldName[0] == 'd' && strcmp(fieldName, "dstAddrMac") == 0) return base+3;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *GeoNetTagDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "uint32",    // FIELD_srcAddrIp
        "uint32",    // FIELD_dstAddrIp
        "inet::MacAddress",    // FIELD_srcAddrMac
        "inet::MacAddress",    // FIELD_dstAddrMac
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **GeoNetTagDescriptor::getFieldPropertyNames(int field) const
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

const char *GeoNetTagDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int GeoNetTagDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    GeoNetTag *pp = (GeoNetTag *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *GeoNetTagDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    GeoNetTag *pp = (GeoNetTag *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string GeoNetTagDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    GeoNetTag *pp = (GeoNetTag *)object; (void)pp;
    switch (field) {
        case FIELD_srcAddrIp: return ulong2string(pp->getSrcAddrIp());
        case FIELD_dstAddrIp: return ulong2string(pp->getDstAddrIp());
        case FIELD_srcAddrMac: return pp->getSrcAddrMac().str();
        case FIELD_dstAddrMac: return pp->getDstAddrMac().str();
        default: return "";
    }
}

bool GeoNetTagDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    GeoNetTag *pp = (GeoNetTag *)object; (void)pp;
    switch (field) {
        case FIELD_srcAddrIp: pp->setSrcAddrIp(string2ulong(value)); return true;
        case FIELD_dstAddrIp: pp->setDstAddrIp(string2ulong(value)); return true;
        default: return false;
    }
}

const char *GeoNetTagDescriptor::getFieldStructName(int field) const
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

void *GeoNetTagDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    GeoNetTag *pp = (GeoNetTag *)object; (void)pp;
    switch (field) {
        case FIELD_srcAddrMac: return toVoidPtr(&pp->getSrcAddrMac()); break;
        case FIELD_dstAddrMac: return toVoidPtr(&pp->getDstAddrMac()); break;
        default: return nullptr;
    }
}

} // namespace rover

