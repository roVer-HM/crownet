//
// Generated file, do not edit! Created by nedtool 5.6 from rover/applications/udpapp/detour/DetourAppPacket.msg.
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
#include "DetourAppPacket_m.h"

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
    omnetpp::cEnum *e = omnetpp::cEnum::find("rover::DetourPktType");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("rover::DetourPktType"));
    e->insert(INCIDENT, "INCIDENT");
    e->insert(PROPAGATE, "PROPAGATE");
)

Register_Class(DetourAppPacket)

DetourAppPacket::DetourAppPacket() : ::inet::ApplicationPacket()
{
}

DetourAppPacket::DetourAppPacket(const DetourAppPacket& other) : ::inet::ApplicationPacket(other)
{
    copy(other);
}

DetourAppPacket::~DetourAppPacket()
{
    delete [] this->alternativeRoute;
}

DetourAppPacket& DetourAppPacket::operator=(const DetourAppPacket& other)
{
    if (this == &other) return *this;
    ::inet::ApplicationPacket::operator=(other);
    copy(other);
    return *this;
}

void DetourAppPacket::copy(const DetourAppPacket& other)
{
    this->incidentReason = other.incidentReason;
    this->repeatTime = other.repeatTime;
    this->repeateInterval = other.repeateInterval;
    this->closedTarget = other.closedTarget;
    delete [] this->alternativeRoute;
    this->alternativeRoute = (other.alternativeRoute_arraysize==0) ? nullptr : new omnetpp::opp_string[other.alternativeRoute_arraysize];
    alternativeRoute_arraysize = other.alternativeRoute_arraysize;
    for (size_t i = 0; i < alternativeRoute_arraysize; i++) {
        this->alternativeRoute[i] = other.alternativeRoute[i];
    }
    this->firstHopTime = other.firstHopTime;
    this->firstHopId = other.firstHopId;
    this->firstHopOrigin = other.firstHopOrigin;
    this->lastHopTime = other.lastHopTime;
    this->lastHopId = other.lastHopId;
    this->lastHopOrigin = other.lastHopOrigin;
    this->type = other.type;
}

void DetourAppPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::ApplicationPacket::parsimPack(b);
    doParsimPacking(b,this->incidentReason);
    doParsimPacking(b,this->repeatTime);
    doParsimPacking(b,this->repeateInterval);
    doParsimPacking(b,this->closedTarget);
    b->pack(alternativeRoute_arraysize);
    doParsimArrayPacking(b,this->alternativeRoute,alternativeRoute_arraysize);
    doParsimPacking(b,this->firstHopTime);
    doParsimPacking(b,this->firstHopId);
    doParsimPacking(b,this->firstHopOrigin);
    doParsimPacking(b,this->lastHopTime);
    doParsimPacking(b,this->lastHopId);
    doParsimPacking(b,this->lastHopOrigin);
    doParsimPacking(b,this->type);
}

void DetourAppPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::ApplicationPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->incidentReason);
    doParsimUnpacking(b,this->repeatTime);
    doParsimUnpacking(b,this->repeateInterval);
    doParsimUnpacking(b,this->closedTarget);
    delete [] this->alternativeRoute;
    b->unpack(alternativeRoute_arraysize);
    if (alternativeRoute_arraysize == 0) {
        this->alternativeRoute = nullptr;
    } else {
        this->alternativeRoute = new omnetpp::opp_string[alternativeRoute_arraysize];
        doParsimArrayUnpacking(b,this->alternativeRoute,alternativeRoute_arraysize);
    }
    doParsimUnpacking(b,this->firstHopTime);
    doParsimUnpacking(b,this->firstHopId);
    doParsimUnpacking(b,this->firstHopOrigin);
    doParsimUnpacking(b,this->lastHopTime);
    doParsimUnpacking(b,this->lastHopId);
    doParsimUnpacking(b,this->lastHopOrigin);
    doParsimUnpacking(b,this->type);
}

const char * DetourAppPacket::getIncidentReason() const
{
    return this->incidentReason.c_str();
}

void DetourAppPacket::setIncidentReason(const char * incidentReason)
{
    handleChange();
    this->incidentReason = incidentReason;
}

omnetpp::simtime_t DetourAppPacket::getRepeatTime() const
{
    return this->repeatTime;
}

void DetourAppPacket::setRepeatTime(omnetpp::simtime_t repeatTime)
{
    handleChange();
    this->repeatTime = repeatTime;
}

omnetpp::simtime_t DetourAppPacket::getRepeateInterval() const
{
    return this->repeateInterval;
}

void DetourAppPacket::setRepeateInterval(omnetpp::simtime_t repeateInterval)
{
    handleChange();
    this->repeateInterval = repeateInterval;
}

const char * DetourAppPacket::getClosedTarget() const
{
    return this->closedTarget.c_str();
}

void DetourAppPacket::setClosedTarget(const char * closedTarget)
{
    handleChange();
    this->closedTarget = closedTarget;
}

size_t DetourAppPacket::getAlternativeRouteArraySize() const
{
    return alternativeRoute_arraysize;
}

const char * DetourAppPacket::getAlternativeRoute(size_t k) const
{
    if (k >= alternativeRoute_arraysize) throw omnetpp::cRuntimeError("Array of size alternativeRoute_arraysize indexed by %lu", (unsigned long)k);
    return this->alternativeRoute[k].c_str();
}

void DetourAppPacket::setAlternativeRouteArraySize(size_t newSize)
{
    handleChange();
    omnetpp::opp_string *alternativeRoute2 = (newSize==0) ? nullptr : new omnetpp::opp_string[newSize];
    size_t minSize = alternativeRoute_arraysize < newSize ? alternativeRoute_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        alternativeRoute2[i] = this->alternativeRoute[i];
    delete [] this->alternativeRoute;
    this->alternativeRoute = alternativeRoute2;
    alternativeRoute_arraysize = newSize;
}

void DetourAppPacket::setAlternativeRoute(size_t k, const char * alternativeRoute)
{
    if (k >= alternativeRoute_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    this->alternativeRoute[k] = alternativeRoute;
}

void DetourAppPacket::insertAlternativeRoute(size_t k, const char * alternativeRoute)
{
    handleChange();
    if (k > alternativeRoute_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = alternativeRoute_arraysize + 1;
    omnetpp::opp_string *alternativeRoute2 = new omnetpp::opp_string[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        alternativeRoute2[i] = this->alternativeRoute[i];
    alternativeRoute2[k] = alternativeRoute;
    for (i = k + 1; i < newSize; i++)
        alternativeRoute2[i] = this->alternativeRoute[i-1];
    delete [] this->alternativeRoute;
    this->alternativeRoute = alternativeRoute2;
    alternativeRoute_arraysize = newSize;
}

void DetourAppPacket::insertAlternativeRoute(const char * alternativeRoute)
{
    insertAlternativeRoute(alternativeRoute_arraysize, alternativeRoute);
}

void DetourAppPacket::eraseAlternativeRoute(size_t k)
{
    if (k >= alternativeRoute_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    handleChange();
    size_t newSize = alternativeRoute_arraysize - 1;
    omnetpp::opp_string *alternativeRoute2 = (newSize == 0) ? nullptr : new omnetpp::opp_string[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        alternativeRoute2[i] = this->alternativeRoute[i];
    for (i = k; i < newSize; i++)
        alternativeRoute2[i] = this->alternativeRoute[i+1];
    delete [] this->alternativeRoute;
    this->alternativeRoute = alternativeRoute2;
    alternativeRoute_arraysize = newSize;
}

omnetpp::simtime_t DetourAppPacket::getFirstHopTime() const
{
    return this->firstHopTime;
}

void DetourAppPacket::setFirstHopTime(omnetpp::simtime_t firstHopTime)
{
    handleChange();
    this->firstHopTime = firstHopTime;
}

uint32_t DetourAppPacket::getFirstHopId() const
{
    return this->firstHopId;
}

void DetourAppPacket::setFirstHopId(uint32_t firstHopId)
{
    handleChange();
    this->firstHopId = firstHopId;
}

const inet::Coord& DetourAppPacket::getFirstHopOrigin() const
{
    return this->firstHopOrigin;
}

void DetourAppPacket::setFirstHopOrigin(const inet::Coord& firstHopOrigin)
{
    handleChange();
    this->firstHopOrigin = firstHopOrigin;
}

omnetpp::simtime_t DetourAppPacket::getLastHopTime() const
{
    return this->lastHopTime;
}

void DetourAppPacket::setLastHopTime(omnetpp::simtime_t lastHopTime)
{
    handleChange();
    this->lastHopTime = lastHopTime;
}

uint32_t DetourAppPacket::getLastHopId() const
{
    return this->lastHopId;
}

void DetourAppPacket::setLastHopId(uint32_t lastHopId)
{
    handleChange();
    this->lastHopId = lastHopId;
}

const inet::Coord& DetourAppPacket::getLastHopOrigin() const
{
    return this->lastHopOrigin;
}

void DetourAppPacket::setLastHopOrigin(const inet::Coord& lastHopOrigin)
{
    handleChange();
    this->lastHopOrigin = lastHopOrigin;
}

rover::DetourPktType DetourAppPacket::getType() const
{
    return this->type;
}

void DetourAppPacket::setType(rover::DetourPktType type)
{
    handleChange();
    this->type = type;
}

class DetourAppPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_incidentReason,
        FIELD_repeatTime,
        FIELD_repeateInterval,
        FIELD_closedTarget,
        FIELD_alternativeRoute,
        FIELD_firstHopTime,
        FIELD_firstHopId,
        FIELD_firstHopOrigin,
        FIELD_lastHopTime,
        FIELD_lastHopId,
        FIELD_lastHopOrigin,
        FIELD_type,
    };
  public:
    DetourAppPacketDescriptor();
    virtual ~DetourAppPacketDescriptor();

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

Register_ClassDescriptor(DetourAppPacketDescriptor)

DetourAppPacketDescriptor::DetourAppPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::DetourAppPacket)), "inet::ApplicationPacket")
{
    propertynames = nullptr;
}

DetourAppPacketDescriptor::~DetourAppPacketDescriptor()
{
    delete[] propertynames;
}

bool DetourAppPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<DetourAppPacket *>(obj)!=nullptr;
}

const char **DetourAppPacketDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *DetourAppPacketDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int DetourAppPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 12+basedesc->getFieldCount() : 12;
}

unsigned int DetourAppPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_incidentReason
        0,    // FIELD_repeatTime
        0,    // FIELD_repeateInterval
        FD_ISEDITABLE,    // FIELD_closedTarget
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_alternativeRoute
        0,    // FIELD_firstHopTime
        FD_ISEDITABLE,    // FIELD_firstHopId
        FD_ISCOMPOUND,    // FIELD_firstHopOrigin
        0,    // FIELD_lastHopTime
        FD_ISEDITABLE,    // FIELD_lastHopId
        FD_ISCOMPOUND,    // FIELD_lastHopOrigin
        0,    // FIELD_type
    };
    return (field >= 0 && field < 12) ? fieldTypeFlags[field] : 0;
}

const char *DetourAppPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "incidentReason",
        "repeatTime",
        "repeateInterval",
        "closedTarget",
        "alternativeRoute",
        "firstHopTime",
        "firstHopId",
        "firstHopOrigin",
        "lastHopTime",
        "lastHopId",
        "lastHopOrigin",
        "type",
    };
    return (field >= 0 && field < 12) ? fieldNames[field] : nullptr;
}

int DetourAppPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'i' && strcmp(fieldName, "incidentReason") == 0) return base+0;
    if (fieldName[0] == 'r' && strcmp(fieldName, "repeatTime") == 0) return base+1;
    if (fieldName[0] == 'r' && strcmp(fieldName, "repeateInterval") == 0) return base+2;
    if (fieldName[0] == 'c' && strcmp(fieldName, "closedTarget") == 0) return base+3;
    if (fieldName[0] == 'a' && strcmp(fieldName, "alternativeRoute") == 0) return base+4;
    if (fieldName[0] == 'f' && strcmp(fieldName, "firstHopTime") == 0) return base+5;
    if (fieldName[0] == 'f' && strcmp(fieldName, "firstHopId") == 0) return base+6;
    if (fieldName[0] == 'f' && strcmp(fieldName, "firstHopOrigin") == 0) return base+7;
    if (fieldName[0] == 'l' && strcmp(fieldName, "lastHopTime") == 0) return base+8;
    if (fieldName[0] == 'l' && strcmp(fieldName, "lastHopId") == 0) return base+9;
    if (fieldName[0] == 'l' && strcmp(fieldName, "lastHopOrigin") == 0) return base+10;
    if (fieldName[0] == 't' && strcmp(fieldName, "type") == 0) return base+11;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *DetourAppPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "string",    // FIELD_incidentReason
        "omnetpp::simtime_t",    // FIELD_repeatTime
        "omnetpp::simtime_t",    // FIELD_repeateInterval
        "string",    // FIELD_closedTarget
        "string",    // FIELD_alternativeRoute
        "omnetpp::simtime_t",    // FIELD_firstHopTime
        "uint32_t",    // FIELD_firstHopId
        "inet::Coord",    // FIELD_firstHopOrigin
        "omnetpp::simtime_t",    // FIELD_lastHopTime
        "uint32_t",    // FIELD_lastHopId
        "inet::Coord",    // FIELD_lastHopOrigin
        "rover::DetourPktType",    // FIELD_type
    };
    return (field >= 0 && field < 12) ? fieldTypeStrings[field] : nullptr;
}

const char **DetourAppPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_type: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *DetourAppPacketDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_type:
            if (!strcmp(propertyname, "enum")) return "rover::DetourPktType";
            return nullptr;
        default: return nullptr;
    }
}

int DetourAppPacketDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    DetourAppPacket *pp = (DetourAppPacket *)object; (void)pp;
    switch (field) {
        case FIELD_alternativeRoute: return pp->getAlternativeRouteArraySize();
        default: return 0;
    }
}

const char *DetourAppPacketDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    DetourAppPacket *pp = (DetourAppPacket *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string DetourAppPacketDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    DetourAppPacket *pp = (DetourAppPacket *)object; (void)pp;
    switch (field) {
        case FIELD_incidentReason: return oppstring2string(pp->getIncidentReason());
        case FIELD_repeatTime: return simtime2string(pp->getRepeatTime());
        case FIELD_repeateInterval: return simtime2string(pp->getRepeateInterval());
        case FIELD_closedTarget: return oppstring2string(pp->getClosedTarget());
        case FIELD_alternativeRoute: return oppstring2string(pp->getAlternativeRoute(i));
        case FIELD_firstHopTime: return simtime2string(pp->getFirstHopTime());
        case FIELD_firstHopId: return ulong2string(pp->getFirstHopId());
        case FIELD_firstHopOrigin: {std::stringstream out; out << pp->getFirstHopOrigin(); return out.str();}
        case FIELD_lastHopTime: return simtime2string(pp->getLastHopTime());
        case FIELD_lastHopId: return ulong2string(pp->getLastHopId());
        case FIELD_lastHopOrigin: {std::stringstream out; out << pp->getLastHopOrigin(); return out.str();}
        case FIELD_type: return enum2string(pp->getType(), "rover::DetourPktType");
        default: return "";
    }
}

bool DetourAppPacketDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    DetourAppPacket *pp = (DetourAppPacket *)object; (void)pp;
    switch (field) {
        case FIELD_incidentReason: pp->setIncidentReason((value)); return true;
        case FIELD_closedTarget: pp->setClosedTarget((value)); return true;
        case FIELD_alternativeRoute: pp->setAlternativeRoute(i,(value)); return true;
        case FIELD_firstHopId: pp->setFirstHopId(string2ulong(value)); return true;
        case FIELD_lastHopId: pp->setLastHopId(string2ulong(value)); return true;
        default: return false;
    }
}

const char *DetourAppPacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_firstHopOrigin: return omnetpp::opp_typename(typeid(inet::Coord));
        case FIELD_lastHopOrigin: return omnetpp::opp_typename(typeid(inet::Coord));
        default: return nullptr;
    };
}

void *DetourAppPacketDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    DetourAppPacket *pp = (DetourAppPacket *)object; (void)pp;
    switch (field) {
        case FIELD_firstHopOrigin: return toVoidPtr(&pp->getFirstHopOrigin()); break;
        case FIELD_lastHopOrigin: return toVoidPtr(&pp->getLastHopOrigin()); break;
        default: return nullptr;
    }
}

} // namespace rover

