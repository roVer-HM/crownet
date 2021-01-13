//
// Generated file, do not edit! Created by nedtool 5.6 from rover/aid/AidCommand.msg.
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
#include "AidCommand_m.h"

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

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("rover::AidCommandCode");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("rover::AidCommandCode"));
    e->insert(AID_C_BIND, "AID_C_BIND");
    e->insert(AID_C_APP_REQ, "AID_C_APP_REQ");
    e->insert(AID_C_APP_CAP, "AID_C_APP_CAP");
    e->insert(AID_C_CONNECT, "AID_C_CONNECT");
    e->insert(AID_C_DATA, "AID_C_DATA");
    e->insert(AID_C_CLOSE, "AID_C_CLOSE");
    e->insert(AID_C_DESTROY, "AID_C_DESTROY");
)

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("rover::AidRecipientClass");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("rover::AidRecipientClass"));
    e->insert(ALL, "ALL");
    e->insert(ALL_LOCAL, "ALL_LOCAL");
    e->insert(SOME, "SOME");
    e->insert(SOME_LOCAL, "SOME_LOCAL");
    e->insert(ONE, "ONE");
    e->insert(ONE_LOCAL, "ONE_LOCAL");
)

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("rover::AidStatusInd");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("rover::AidStatusInd"));
    e->insert(AID_I_DATA, "AID_I_DATA");
    e->insert(AID_I_ERROR, "AID_I_ERROR");
    e->insert(AID_I_STATUS, "AID_I_STATUS");
    e->insert(AID_I_CLOSE, "AID_I_CLOSE");
)

Register_Class(AidCommand)

AidCommand::AidCommand() : ::omnetpp::cObject()
{
}

AidCommand::AidCommand(const AidCommand& other) : ::omnetpp::cObject(other)
{
    copy(other);
}

AidCommand::~AidCommand()
{
}

AidCommand& AidCommand::operator=(const AidCommand& other)
{
    if (this == &other) return *this;
    ::omnetpp::cObject::operator=(other);
    copy(other);
    return *this;
}

void AidCommand::copy(const AidCommand& other)
{
    this->userId = other.userId;
}

void AidCommand::parsimPack(omnetpp::cCommBuffer *b) const
{
    doParsimPacking(b,this->userId);
}

void AidCommand::parsimUnpack(omnetpp::cCommBuffer *b)
{
    doParsimUnpacking(b,this->userId);
}

int AidCommand::getUserId() const
{
    return this->userId;
}

void AidCommand::setUserId(int userId)
{
    this->userId = userId;
}

class AidCommandDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_userId,
    };
  public:
    AidCommandDescriptor();
    virtual ~AidCommandDescriptor();

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

Register_ClassDescriptor(AidCommandDescriptor)

AidCommandDescriptor::AidCommandDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::AidCommand)), "omnetpp::cObject")
{
    propertynames = nullptr;
}

AidCommandDescriptor::~AidCommandDescriptor()
{
    delete[] propertynames;
}

bool AidCommandDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AidCommand *>(obj)!=nullptr;
}

const char **AidCommandDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AidCommandDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AidCommandDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount() : 1;
}

unsigned int AidCommandDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_userId
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *AidCommandDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "userId",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int AidCommandDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'u' && strcmp(fieldName, "userId") == 0) return base+0;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AidCommandDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_userId
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **AidCommandDescriptor::getFieldPropertyNames(int field) const
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

const char *AidCommandDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int AidCommandDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AidCommand *pp = (AidCommand *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AidCommandDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidCommand *pp = (AidCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AidCommandDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidCommand *pp = (AidCommand *)object; (void)pp;
    switch (field) {
        case FIELD_userId: return long2string(pp->getUserId());
        default: return "";
    }
}

bool AidCommandDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AidCommand *pp = (AidCommand *)object; (void)pp;
    switch (field) {
        case FIELD_userId: pp->setUserId(string2long(value)); return true;
        default: return false;
    }
}

const char *AidCommandDescriptor::getFieldStructName(int field) const
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

void *AidCommandDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AidCommand *pp = (AidCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(AidBindCommand)

AidBindCommand::AidBindCommand() : ::rover::AidCommand()
{
}

AidBindCommand::AidBindCommand(const AidBindCommand& other) : ::rover::AidCommand(other)
{
    copy(other);
}

AidBindCommand::~AidBindCommand()
{
}

AidBindCommand& AidBindCommand::operator=(const AidBindCommand& other)
{
    if (this == &other) return *this;
    ::rover::AidCommand::operator=(other);
    copy(other);
    return *this;
}

void AidBindCommand::copy(const AidBindCommand& other)
{
    this->localAddr = other.localAddr;
    this->localPort = other.localPort;
}

void AidBindCommand::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::AidCommand::parsimPack(b);
    doParsimPacking(b,this->localAddr);
    doParsimPacking(b,this->localPort);
}

void AidBindCommand::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::AidCommand::parsimUnpack(b);
    doParsimUnpacking(b,this->localAddr);
    doParsimUnpacking(b,this->localPort);
}

const inet::L3Address& AidBindCommand::getLocalAddr() const
{
    return this->localAddr;
}

void AidBindCommand::setLocalAddr(const inet::L3Address& localAddr)
{
    this->localAddr = localAddr;
}

int AidBindCommand::getLocalPort() const
{
    return this->localPort;
}

void AidBindCommand::setLocalPort(int localPort)
{
    this->localPort = localPort;
}

class AidBindCommandDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_localAddr,
        FIELD_localPort,
    };
  public:
    AidBindCommandDescriptor();
    virtual ~AidBindCommandDescriptor();

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

Register_ClassDescriptor(AidBindCommandDescriptor)

AidBindCommandDescriptor::AidBindCommandDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::AidBindCommand)), "rover::AidCommand")
{
    propertynames = nullptr;
}

AidBindCommandDescriptor::~AidBindCommandDescriptor()
{
    delete[] propertynames;
}

bool AidBindCommandDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AidBindCommand *>(obj)!=nullptr;
}

const char **AidBindCommandDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AidBindCommandDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AidBindCommandDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount() : 2;
}

unsigned int AidBindCommandDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        0,    // FIELD_localAddr
        FD_ISEDITABLE,    // FIELD_localPort
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *AidBindCommandDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "localAddr",
        "localPort",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int AidBindCommandDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'l' && strcmp(fieldName, "localAddr") == 0) return base+0;
    if (fieldName[0] == 'l' && strcmp(fieldName, "localPort") == 0) return base+1;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AidBindCommandDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "inet::L3Address",    // FIELD_localAddr
        "int",    // FIELD_localPort
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **AidBindCommandDescriptor::getFieldPropertyNames(int field) const
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

const char *AidBindCommandDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int AidBindCommandDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AidBindCommand *pp = (AidBindCommand *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AidBindCommandDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidBindCommand *pp = (AidBindCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AidBindCommandDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidBindCommand *pp = (AidBindCommand *)object; (void)pp;
    switch (field) {
        case FIELD_localAddr: return pp->getLocalAddr().str();
        case FIELD_localPort: return long2string(pp->getLocalPort());
        default: return "";
    }
}

bool AidBindCommandDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AidBindCommand *pp = (AidBindCommand *)object; (void)pp;
    switch (field) {
        case FIELD_localPort: pp->setLocalPort(string2long(value)); return true;
        default: return false;
    }
}

const char *AidBindCommandDescriptor::getFieldStructName(int field) const
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

void *AidBindCommandDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AidBindCommand *pp = (AidBindCommand *)object; (void)pp;
    switch (field) {
        case FIELD_localAddr: return toVoidPtr(&pp->getLocalAddr()); break;
        default: return nullptr;
    }
}

Register_Class(AidAppReqCommand)

AidAppReqCommand::AidAppReqCommand() : ::rover::AidCommand()
{
}

AidAppReqCommand::AidAppReqCommand(const AidAppReqCommand& other) : ::rover::AidCommand(other)
{
    copy(other);
}

AidAppReqCommand::~AidAppReqCommand()
{
}

AidAppReqCommand& AidAppReqCommand::operator=(const AidAppReqCommand& other)
{
    if (this == &other) return *this;
    ::rover::AidCommand::operator=(other);
    copy(other);
    return *this;
}

void AidAppReqCommand::copy(const AidAppReqCommand& other)
{
    this->minRate = other.minRate;
    this->maxRate = other.maxRate;
    this->recipientClass = other.recipientClass;
    this->remoteAddr = other.remoteAddr;
    this->remotePort = other.remotePort;
}

void AidAppReqCommand::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::AidCommand::parsimPack(b);
    doParsimPacking(b,this->minRate);
    doParsimPacking(b,this->maxRate);
    doParsimPacking(b,this->recipientClass);
    doParsimPacking(b,this->remoteAddr);
    doParsimPacking(b,this->remotePort);
}

void AidAppReqCommand::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::AidCommand::parsimUnpack(b);
    doParsimUnpacking(b,this->minRate);
    doParsimUnpacking(b,this->maxRate);
    doParsimUnpacking(b,this->recipientClass);
    doParsimUnpacking(b,this->remoteAddr);
    doParsimUnpacking(b,this->remotePort);
}

double AidAppReqCommand::getMinRate() const
{
    return this->minRate;
}

void AidAppReqCommand::setMinRate(double minRate)
{
    this->minRate = minRate;
}

double AidAppReqCommand::getMaxRate() const
{
    return this->maxRate;
}

void AidAppReqCommand::setMaxRate(double maxRate)
{
    this->maxRate = maxRate;
}

rover::AidRecipientClass AidAppReqCommand::getRecipientClass() const
{
    return this->recipientClass;
}

void AidAppReqCommand::setRecipientClass(rover::AidRecipientClass recipientClass)
{
    this->recipientClass = recipientClass;
}

const inet::L3Address& AidAppReqCommand::getRemoteAddr() const
{
    return this->remoteAddr;
}

void AidAppReqCommand::setRemoteAddr(const inet::L3Address& remoteAddr)
{
    this->remoteAddr = remoteAddr;
}

int AidAppReqCommand::getRemotePort() const
{
    return this->remotePort;
}

void AidAppReqCommand::setRemotePort(int remotePort)
{
    this->remotePort = remotePort;
}

class AidAppReqCommandDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_minRate,
        FIELD_maxRate,
        FIELD_recipientClass,
        FIELD_remoteAddr,
        FIELD_remotePort,
    };
  public:
    AidAppReqCommandDescriptor();
    virtual ~AidAppReqCommandDescriptor();

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

Register_ClassDescriptor(AidAppReqCommandDescriptor)

AidAppReqCommandDescriptor::AidAppReqCommandDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::AidAppReqCommand)), "rover::AidCommand")
{
    propertynames = nullptr;
}

AidAppReqCommandDescriptor::~AidAppReqCommandDescriptor()
{
    delete[] propertynames;
}

bool AidAppReqCommandDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AidAppReqCommand *>(obj)!=nullptr;
}

const char **AidAppReqCommandDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AidAppReqCommandDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AidAppReqCommandDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int AidAppReqCommandDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_minRate
        FD_ISEDITABLE,    // FIELD_maxRate
        0,    // FIELD_recipientClass
        0,    // FIELD_remoteAddr
        FD_ISEDITABLE,    // FIELD_remotePort
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *AidAppReqCommandDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "minRate",
        "maxRate",
        "recipientClass",
        "remoteAddr",
        "remotePort",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int AidAppReqCommandDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'm' && strcmp(fieldName, "minRate") == 0) return base+0;
    if (fieldName[0] == 'm' && strcmp(fieldName, "maxRate") == 0) return base+1;
    if (fieldName[0] == 'r' && strcmp(fieldName, "recipientClass") == 0) return base+2;
    if (fieldName[0] == 'r' && strcmp(fieldName, "remoteAddr") == 0) return base+3;
    if (fieldName[0] == 'r' && strcmp(fieldName, "remotePort") == 0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AidAppReqCommandDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "double",    // FIELD_minRate
        "double",    // FIELD_maxRate
        "rover::AidRecipientClass",    // FIELD_recipientClass
        "inet::L3Address",    // FIELD_remoteAddr
        "int",    // FIELD_remotePort
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **AidAppReqCommandDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_recipientClass: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *AidAppReqCommandDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_recipientClass:
            if (!strcmp(propertyname, "enum")) return "rover::AidRecipientClass";
            return nullptr;
        default: return nullptr;
    }
}

int AidAppReqCommandDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AidAppReqCommand *pp = (AidAppReqCommand *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AidAppReqCommandDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidAppReqCommand *pp = (AidAppReqCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AidAppReqCommandDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidAppReqCommand *pp = (AidAppReqCommand *)object; (void)pp;
    switch (field) {
        case FIELD_minRate: return double2string(pp->getMinRate());
        case FIELD_maxRate: return double2string(pp->getMaxRate());
        case FIELD_recipientClass: return enum2string(pp->getRecipientClass(), "rover::AidRecipientClass");
        case FIELD_remoteAddr: return pp->getRemoteAddr().str();
        case FIELD_remotePort: return long2string(pp->getRemotePort());
        default: return "";
    }
}

bool AidAppReqCommandDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AidAppReqCommand *pp = (AidAppReqCommand *)object; (void)pp;
    switch (field) {
        case FIELD_minRate: pp->setMinRate(string2double(value)); return true;
        case FIELD_maxRate: pp->setMaxRate(string2double(value)); return true;
        case FIELD_remotePort: pp->setRemotePort(string2long(value)); return true;
        default: return false;
    }
}

const char *AidAppReqCommandDescriptor::getFieldStructName(int field) const
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

void *AidAppReqCommandDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AidAppReqCommand *pp = (AidAppReqCommand *)object; (void)pp;
    switch (field) {
        case FIELD_remoteAddr: return toVoidPtr(&pp->getRemoteAddr()); break;
        default: return nullptr;
    }
}

Register_Class(AidAppCapCommand)

AidAppCapCommand::AidAppCapCommand() : ::rover::AidCommand()
{
}

AidAppCapCommand::AidAppCapCommand(const AidAppCapCommand& other) : ::rover::AidCommand(other)
{
    copy(other);
}

AidAppCapCommand::~AidAppCapCommand()
{
}

AidAppCapCommand& AidAppCapCommand::operator=(const AidAppCapCommand& other)
{
    if (this == &other) return *this;
    ::rover::AidCommand::operator=(other);
    copy(other);
    return *this;
}

void AidAppCapCommand::copy(const AidAppCapCommand& other)
{
}

void AidAppCapCommand::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::AidCommand::parsimPack(b);
}

void AidAppCapCommand::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::AidCommand::parsimUnpack(b);
}

class AidAppCapCommandDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    AidAppCapCommandDescriptor();
    virtual ~AidAppCapCommandDescriptor();

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

Register_ClassDescriptor(AidAppCapCommandDescriptor)

AidAppCapCommandDescriptor::AidAppCapCommandDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::AidAppCapCommand)), "rover::AidCommand")
{
    propertynames = nullptr;
}

AidAppCapCommandDescriptor::~AidAppCapCommandDescriptor()
{
    delete[] propertynames;
}

bool AidAppCapCommandDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AidAppCapCommand *>(obj)!=nullptr;
}

const char **AidAppCapCommandDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AidAppCapCommandDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AidAppCapCommandDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int AidAppCapCommandDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *AidAppCapCommandDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int AidAppCapCommandDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AidAppCapCommandDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **AidAppCapCommandDescriptor::getFieldPropertyNames(int field) const
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

const char *AidAppCapCommandDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int AidAppCapCommandDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AidAppCapCommand *pp = (AidAppCapCommand *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AidAppCapCommandDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidAppCapCommand *pp = (AidAppCapCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AidAppCapCommandDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidAppCapCommand *pp = (AidAppCapCommand *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool AidAppCapCommandDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AidAppCapCommand *pp = (AidAppCapCommand *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *AidAppCapCommandDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *AidAppCapCommandDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AidAppCapCommand *pp = (AidAppCapCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(AidConnectCommand)

AidConnectCommand::AidConnectCommand() : ::rover::AidCommand()
{
}

AidConnectCommand::AidConnectCommand(const AidConnectCommand& other) : ::rover::AidCommand(other)
{
    copy(other);
}

AidConnectCommand::~AidConnectCommand()
{
}

AidConnectCommand& AidConnectCommand::operator=(const AidConnectCommand& other)
{
    if (this == &other) return *this;
    ::rover::AidCommand::operator=(other);
    copy(other);
    return *this;
}

void AidConnectCommand::copy(const AidConnectCommand& other)
{
}

void AidConnectCommand::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::AidCommand::parsimPack(b);
}

void AidConnectCommand::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::AidCommand::parsimUnpack(b);
}

class AidConnectCommandDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    AidConnectCommandDescriptor();
    virtual ~AidConnectCommandDescriptor();

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

Register_ClassDescriptor(AidConnectCommandDescriptor)

AidConnectCommandDescriptor::AidConnectCommandDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::AidConnectCommand)), "rover::AidCommand")
{
    propertynames = nullptr;
}

AidConnectCommandDescriptor::~AidConnectCommandDescriptor()
{
    delete[] propertynames;
}

bool AidConnectCommandDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AidConnectCommand *>(obj)!=nullptr;
}

const char **AidConnectCommandDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AidConnectCommandDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AidConnectCommandDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int AidConnectCommandDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *AidConnectCommandDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int AidConnectCommandDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AidConnectCommandDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **AidConnectCommandDescriptor::getFieldPropertyNames(int field) const
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

const char *AidConnectCommandDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int AidConnectCommandDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AidConnectCommand *pp = (AidConnectCommand *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AidConnectCommandDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidConnectCommand *pp = (AidConnectCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AidConnectCommandDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidConnectCommand *pp = (AidConnectCommand *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool AidConnectCommandDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AidConnectCommand *pp = (AidConnectCommand *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *AidConnectCommandDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *AidConnectCommandDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AidConnectCommand *pp = (AidConnectCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(AidCloseCommand)

AidCloseCommand::AidCloseCommand() : ::rover::AidCommand()
{
}

AidCloseCommand::AidCloseCommand(const AidCloseCommand& other) : ::rover::AidCommand(other)
{
    copy(other);
}

AidCloseCommand::~AidCloseCommand()
{
}

AidCloseCommand& AidCloseCommand::operator=(const AidCloseCommand& other)
{
    if (this == &other) return *this;
    ::rover::AidCommand::operator=(other);
    copy(other);
    return *this;
}

void AidCloseCommand::copy(const AidCloseCommand& other)
{
}

void AidCloseCommand::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::AidCommand::parsimPack(b);
}

void AidCloseCommand::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::AidCommand::parsimUnpack(b);
}

class AidCloseCommandDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    AidCloseCommandDescriptor();
    virtual ~AidCloseCommandDescriptor();

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

Register_ClassDescriptor(AidCloseCommandDescriptor)

AidCloseCommandDescriptor::AidCloseCommandDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::AidCloseCommand)), "rover::AidCommand")
{
    propertynames = nullptr;
}

AidCloseCommandDescriptor::~AidCloseCommandDescriptor()
{
    delete[] propertynames;
}

bool AidCloseCommandDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AidCloseCommand *>(obj)!=nullptr;
}

const char **AidCloseCommandDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AidCloseCommandDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AidCloseCommandDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int AidCloseCommandDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *AidCloseCommandDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int AidCloseCommandDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AidCloseCommandDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **AidCloseCommandDescriptor::getFieldPropertyNames(int field) const
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

const char *AidCloseCommandDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int AidCloseCommandDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AidCloseCommand *pp = (AidCloseCommand *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AidCloseCommandDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidCloseCommand *pp = (AidCloseCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AidCloseCommandDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidCloseCommand *pp = (AidCloseCommand *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool AidCloseCommandDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AidCloseCommand *pp = (AidCloseCommand *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *AidCloseCommandDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *AidCloseCommandDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AidCloseCommand *pp = (AidCloseCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(AidDestroyCommand)

AidDestroyCommand::AidDestroyCommand() : ::rover::AidCommand()
{
}

AidDestroyCommand::AidDestroyCommand(const AidDestroyCommand& other) : ::rover::AidCommand(other)
{
    copy(other);
}

AidDestroyCommand::~AidDestroyCommand()
{
}

AidDestroyCommand& AidDestroyCommand::operator=(const AidDestroyCommand& other)
{
    if (this == &other) return *this;
    ::rover::AidCommand::operator=(other);
    copy(other);
    return *this;
}

void AidDestroyCommand::copy(const AidDestroyCommand& other)
{
}

void AidDestroyCommand::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::AidCommand::parsimPack(b);
}

void AidDestroyCommand::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::AidCommand::parsimUnpack(b);
}

class AidDestroyCommandDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    AidDestroyCommandDescriptor();
    virtual ~AidDestroyCommandDescriptor();

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

Register_ClassDescriptor(AidDestroyCommandDescriptor)

AidDestroyCommandDescriptor::AidDestroyCommandDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::AidDestroyCommand)), "rover::AidCommand")
{
    propertynames = nullptr;
}

AidDestroyCommandDescriptor::~AidDestroyCommandDescriptor()
{
    delete[] propertynames;
}

bool AidDestroyCommandDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AidDestroyCommand *>(obj)!=nullptr;
}

const char **AidDestroyCommandDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AidDestroyCommandDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AidDestroyCommandDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int AidDestroyCommandDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *AidDestroyCommandDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int AidDestroyCommandDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AidDestroyCommandDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **AidDestroyCommandDescriptor::getFieldPropertyNames(int field) const
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

const char *AidDestroyCommandDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int AidDestroyCommandDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AidDestroyCommand *pp = (AidDestroyCommand *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AidDestroyCommandDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidDestroyCommand *pp = (AidDestroyCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AidDestroyCommandDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidDestroyCommand *pp = (AidDestroyCommand *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool AidDestroyCommandDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AidDestroyCommand *pp = (AidDestroyCommand *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *AidDestroyCommandDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *AidDestroyCommandDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AidDestroyCommand *pp = (AidDestroyCommand *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(AidControlInfo)

AidControlInfo::AidControlInfo() : ::rover::AidCommand()
{
}

AidControlInfo::AidControlInfo(const AidControlInfo& other) : ::rover::AidCommand(other)
{
    copy(other);
}

AidControlInfo::~AidControlInfo()
{
}

AidControlInfo& AidControlInfo::operator=(const AidControlInfo& other)
{
    if (this == &other) return *this;
    ::rover::AidCommand::operator=(other);
    copy(other);
    return *this;
}

void AidControlInfo::copy(const AidControlInfo& other)
{
}

void AidControlInfo::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::AidCommand::parsimPack(b);
}

void AidControlInfo::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::AidCommand::parsimUnpack(b);
}

class AidControlInfoDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    AidControlInfoDescriptor();
    virtual ~AidControlInfoDescriptor();

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

Register_ClassDescriptor(AidControlInfoDescriptor)

AidControlInfoDescriptor::AidControlInfoDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::AidControlInfo)), "rover::AidCommand")
{
    propertynames = nullptr;
}

AidControlInfoDescriptor::~AidControlInfoDescriptor()
{
    delete[] propertynames;
}

bool AidControlInfoDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AidControlInfo *>(obj)!=nullptr;
}

const char **AidControlInfoDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AidControlInfoDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AidControlInfoDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int AidControlInfoDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *AidControlInfoDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int AidControlInfoDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AidControlInfoDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **AidControlInfoDescriptor::getFieldPropertyNames(int field) const
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

const char *AidControlInfoDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int AidControlInfoDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AidControlInfo *pp = (AidControlInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AidControlInfoDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidControlInfo *pp = (AidControlInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AidControlInfoDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidControlInfo *pp = (AidControlInfo *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool AidControlInfoDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AidControlInfo *pp = (AidControlInfo *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *AidControlInfoDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *AidControlInfoDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AidControlInfo *pp = (AidControlInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(AidErrorInfo)

AidErrorInfo::AidErrorInfo() : ::rover::AidControlInfo()
{
}

AidErrorInfo::AidErrorInfo(const AidErrorInfo& other) : ::rover::AidControlInfo(other)
{
    copy(other);
}

AidErrorInfo::~AidErrorInfo()
{
}

AidErrorInfo& AidErrorInfo::operator=(const AidErrorInfo& other)
{
    if (this == &other) return *this;
    ::rover::AidControlInfo::operator=(other);
    copy(other);
    return *this;
}

void AidErrorInfo::copy(const AidErrorInfo& other)
{
}

void AidErrorInfo::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::AidControlInfo::parsimPack(b);
}

void AidErrorInfo::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::AidControlInfo::parsimUnpack(b);
}

class AidErrorInfoDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    AidErrorInfoDescriptor();
    virtual ~AidErrorInfoDescriptor();

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

Register_ClassDescriptor(AidErrorInfoDescriptor)

AidErrorInfoDescriptor::AidErrorInfoDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::AidErrorInfo)), "rover::AidControlInfo")
{
    propertynames = nullptr;
}

AidErrorInfoDescriptor::~AidErrorInfoDescriptor()
{
    delete[] propertynames;
}

bool AidErrorInfoDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AidErrorInfo *>(obj)!=nullptr;
}

const char **AidErrorInfoDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AidErrorInfoDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AidErrorInfoDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int AidErrorInfoDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *AidErrorInfoDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int AidErrorInfoDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AidErrorInfoDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **AidErrorInfoDescriptor::getFieldPropertyNames(int field) const
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

const char *AidErrorInfoDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int AidErrorInfoDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AidErrorInfo *pp = (AidErrorInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AidErrorInfoDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidErrorInfo *pp = (AidErrorInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AidErrorInfoDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidErrorInfo *pp = (AidErrorInfo *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool AidErrorInfoDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AidErrorInfo *pp = (AidErrorInfo *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *AidErrorInfoDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *AidErrorInfoDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AidErrorInfo *pp = (AidErrorInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(AidStatusInfo)

AidStatusInfo::AidStatusInfo() : ::rover::AidControlInfo()
{
}

AidStatusInfo::AidStatusInfo(const AidStatusInfo& other) : ::rover::AidControlInfo(other)
{
    copy(other);
}

AidStatusInfo::~AidStatusInfo()
{
}

AidStatusInfo& AidStatusInfo::operator=(const AidStatusInfo& other)
{
    if (this == &other) return *this;
    ::rover::AidControlInfo::operator=(other);
    copy(other);
    return *this;
}

void AidStatusInfo::copy(const AidStatusInfo& other)
{
    this->statusCode = other.statusCode;
}

void AidStatusInfo::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::AidControlInfo::parsimPack(b);
    doParsimPacking(b,this->statusCode);
}

void AidStatusInfo::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::AidControlInfo::parsimUnpack(b);
    doParsimUnpacking(b,this->statusCode);
}

int AidStatusInfo::getStatusCode() const
{
    return this->statusCode;
}

void AidStatusInfo::setStatusCode(int statusCode)
{
    this->statusCode = statusCode;
}

class AidStatusInfoDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_statusCode,
    };
  public:
    AidStatusInfoDescriptor();
    virtual ~AidStatusInfoDescriptor();

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

Register_ClassDescriptor(AidStatusInfoDescriptor)

AidStatusInfoDescriptor::AidStatusInfoDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::AidStatusInfo)), "rover::AidControlInfo")
{
    propertynames = nullptr;
}

AidStatusInfoDescriptor::~AidStatusInfoDescriptor()
{
    delete[] propertynames;
}

bool AidStatusInfoDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AidStatusInfo *>(obj)!=nullptr;
}

const char **AidStatusInfoDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *AidStatusInfoDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int AidStatusInfoDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 1+basedesc->getFieldCount() : 1;
}

unsigned int AidStatusInfoDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_statusCode
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *AidStatusInfoDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "statusCode",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int AidStatusInfoDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 's' && strcmp(fieldName, "statusCode") == 0) return base+0;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *AidStatusInfoDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_statusCode
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **AidStatusInfoDescriptor::getFieldPropertyNames(int field) const
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

const char *AidStatusInfoDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int AidStatusInfoDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    AidStatusInfo *pp = (AidStatusInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *AidStatusInfoDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidStatusInfo *pp = (AidStatusInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AidStatusInfoDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    AidStatusInfo *pp = (AidStatusInfo *)object; (void)pp;
    switch (field) {
        case FIELD_statusCode: return long2string(pp->getStatusCode());
        default: return "";
    }
}

bool AidStatusInfoDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    AidStatusInfo *pp = (AidStatusInfo *)object; (void)pp;
    switch (field) {
        case FIELD_statusCode: pp->setStatusCode(string2long(value)); return true;
        default: return false;
    }
}

const char *AidStatusInfoDescriptor::getFieldStructName(int field) const
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

void *AidStatusInfoDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    AidStatusInfo *pp = (AidStatusInfo *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

} // namespace rover

