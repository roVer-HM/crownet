//
// Generated file, do not edit! Created by nedtool 5.6 from rover/common/ItsPdu.msg.
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
#include "ItsPdu_m.h"

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
    omnetpp::cEnum *e = omnetpp::cEnum::find("rover::ItsStationType");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("rover::ItsStationType"));
    e->insert(UNKNOWN, "UNKNOWN");
    e->insert(PEDSTRIAN, "PEDSTRIAN");
    e->insert(CYCLIST, "CYCLIST");
    e->insert(MOPED, "MOPED");
    e->insert(MOTORCYCLE, "MOTORCYCLE");
    e->insert(PASSENGER_CAR, "PASSENGER_CAR");
    e->insert(BUS, "BUS");
    e->insert(LIGHT_TRUCK, "LIGHT_TRUCK");
    e->insert(HEAVY_TRUCK, "HEAVY_TRUCK");
    e->insert(TRAILER, "TRAILER");
    e->insert(SPECIAL_VEHICLES, "SPECIAL_VEHICLES");
    e->insert(TRAM, "TRAM");
    e->insert(ROAD_SIDE_UNIT, "ROAD_SIDE_UNIT");
)

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("rover::ItsVehicleRole");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("rover::ItsVehicleRole"));
    e->insert(DEFAULT, "DEFAULT");
    e->insert(PUBLIC_TRANSPORT, "PUBLIC_TRANSPORT");
    e->insert(SPECIAL_TRANSPORT, "SPECIAL_TRANSPORT");
    e->insert(DANGEROUS_GOODS, "DANGEROUS_GOODS");
    e->insert(ROAD_WORK, "ROAD_WORK");
    e->insert(RESCUE, "RESCUE");
    e->insert(EMERGENCY, "EMERGENCY");
    e->insert(SAFTEY_CAR, "SAFTEY_CAR");
    e->insert(AGRICULTURE, "AGRICULTURE");
    e->insert(COMMERCIAL, "COMMERCIAL");
    e->insert(MILITARY, "MILITARY");
    e->insert(ROEAD_OPERATOR, "ROEAD_OPERATOR");
    e->insert(TAXI, "TAXI");
    e->insert(RESERVERD1, "RESERVERD1");
    e->insert(RESERVERD2, "RESERVERD2");
    e->insert(RESERVERD3, "RESERVERD3");
)

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("rover::ItsVruProfile");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("rover::ItsVruProfile"));
    e->insert(PEDESTRIAN, "PEDESTRIAN");
    e->insert(BICYCLIST, "BICYCLIST");
    e->insert(MOTORCYCLIST, "MOTORCYCLIST");
    e->insert(ANIMALS, "ANIMALS");
)

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("rover::ItsVruType");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("rover::ItsVruType"));
    e->insert(ADULT, "ADULT");
    e->insert(CHILD, "CHILD");
    e->insert(INFANT, "INFANT");
    e->insert(ANIMAL, "ANIMAL");
)

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("rover::ItsVruDeviceType");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("rover::ItsVruDeviceType"));
    e->insert(VRU_TX, "VRU_TX");
    e->insert(VRU_RX, "VRU_RX");
    e->insert(VRU_ST, "VRU_ST");
)

EXECUTE_ON_STARTUP(
    omnetpp::cEnum *e = omnetpp::cEnum::find("rover::ItsMessageId");
    if (!e) omnetpp::enums.getInstance()->add(e = new omnetpp::cEnum("rover::ItsMessageId"));
    e->insert(DENM, "DENM");
    e->insert(CAM, "CAM");
    e->insert(POI, "POI");
    e->insert(SPAT, "SPAT");
    e->insert(MAP, "MAP");
    e->insert(IVI, "IVI");
    e->insert(EV_RSR, "EV_RSR");
    e->insert(CPM, "CPM");
    e->insert(MCM, "MCM");
    e->insert(VAM, "VAM");
)

Register_Class(ItsBase)

ItsBase::ItsBase() : ::omnetpp::cObject()
{
}

ItsBase::ItsBase(const ItsBase& other) : ::omnetpp::cObject(other)
{
    copy(other);
}

ItsBase::~ItsBase()
{
}

ItsBase& ItsBase::operator=(const ItsBase& other)
{
    if (this == &other) return *this;
    ::omnetpp::cObject::operator=(other);
    copy(other);
    return *this;
}

void ItsBase::copy(const ItsBase& other)
{
}

void ItsBase::parsimPack(omnetpp::cCommBuffer *b) const
{
}

void ItsBase::parsimUnpack(omnetpp::cCommBuffer *b)
{
}

class ItsBaseDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsBaseDescriptor();
    virtual ~ItsBaseDescriptor();

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

Register_ClassDescriptor(ItsBaseDescriptor)

ItsBaseDescriptor::ItsBaseDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsBase)), "omnetpp::cObject")
{
    propertynames = nullptr;
}

ItsBaseDescriptor::~ItsBaseDescriptor()
{
    delete[] propertynames;
}

bool ItsBaseDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsBase *>(obj)!=nullptr;
}

const char **ItsBaseDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsBaseDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsBaseDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsBaseDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsBaseDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsBaseDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsBaseDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsBaseDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsBaseDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsBaseDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsBase *pp = (ItsBase *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsBaseDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsBase *pp = (ItsBase *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsBaseDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsBase *pp = (ItsBase *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsBaseDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsBase *pp = (ItsBase *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsBaseDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsBaseDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsBase *pp = (ItsBase *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ReferencePosition)

ReferencePosition::ReferencePosition() : ::rover::ItsBase()
{
}

ReferencePosition::ReferencePosition(const ReferencePosition& other) : ::rover::ItsBase(other)
{
    copy(other);
}

ReferencePosition::~ReferencePosition()
{
}

ReferencePosition& ReferencePosition::operator=(const ReferencePosition& other)
{
    if (this == &other) return *this;
    ::rover::ItsBase::operator=(other);
    copy(other);
    return *this;
}

void ReferencePosition::copy(const ReferencePosition& other)
{
    this->referencePosition = other.referencePosition;
    this->semiMajorConf = other.semiMajorConf;
    this->semiMinorConf = other.semiMinorConf;
    this->headingValue = other.headingValue;
}

void ReferencePosition::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsBase::parsimPack(b);
    doParsimPacking(b,this->referencePosition);
    doParsimPacking(b,this->semiMajorConf);
    doParsimPacking(b,this->semiMinorConf);
    doParsimPacking(b,this->headingValue);
}

void ReferencePosition::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsBase::parsimUnpack(b);
    doParsimUnpacking(b,this->referencePosition);
    doParsimUnpacking(b,this->semiMajorConf);
    doParsimUnpacking(b,this->semiMinorConf);
    doParsimUnpacking(b,this->headingValue);
}

const inet::Coord& ReferencePosition::getReferencePosition() const
{
    return this->referencePosition;
}

void ReferencePosition::setReferencePosition(const inet::Coord& referencePosition)
{
    this->referencePosition = referencePosition;
}

double ReferencePosition::getSemiMajorConf() const
{
    return this->semiMajorConf;
}

void ReferencePosition::setSemiMajorConf(double semiMajorConf)
{
    this->semiMajorConf = semiMajorConf;
}

double ReferencePosition::getSemiMinorConf() const
{
    return this->semiMinorConf;
}

void ReferencePosition::setSemiMinorConf(double semiMinorConf)
{
    this->semiMinorConf = semiMinorConf;
}

const inet::Coord& ReferencePosition::getHeadingValue() const
{
    return this->headingValue;
}

void ReferencePosition::setHeadingValue(const inet::Coord& headingValue)
{
    this->headingValue = headingValue;
}

class ReferencePositionDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_referencePosition,
        FIELD_semiMajorConf,
        FIELD_semiMinorConf,
        FIELD_headingValue,
    };
  public:
    ReferencePositionDescriptor();
    virtual ~ReferencePositionDescriptor();

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

Register_ClassDescriptor(ReferencePositionDescriptor)

ReferencePositionDescriptor::ReferencePositionDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ReferencePosition)), "rover::ItsBase")
{
    propertynames = nullptr;
}

ReferencePositionDescriptor::~ReferencePositionDescriptor()
{
    delete[] propertynames;
}

bool ReferencePositionDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ReferencePosition *>(obj)!=nullptr;
}

const char **ReferencePositionDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ReferencePositionDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ReferencePositionDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount() : 4;
}

unsigned int ReferencePositionDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND,    // FIELD_referencePosition
        FD_ISEDITABLE,    // FIELD_semiMajorConf
        FD_ISEDITABLE,    // FIELD_semiMinorConf
        FD_ISCOMPOUND,    // FIELD_headingValue
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *ReferencePositionDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "referencePosition",
        "semiMajorConf",
        "semiMinorConf",
        "headingValue",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int ReferencePositionDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'r' && strcmp(fieldName, "referencePosition") == 0) return base+0;
    if (fieldName[0] == 's' && strcmp(fieldName, "semiMajorConf") == 0) return base+1;
    if (fieldName[0] == 's' && strcmp(fieldName, "semiMinorConf") == 0) return base+2;
    if (fieldName[0] == 'h' && strcmp(fieldName, "headingValue") == 0) return base+3;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ReferencePositionDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "inet::Coord",    // FIELD_referencePosition
        "double",    // FIELD_semiMajorConf
        "double",    // FIELD_semiMinorConf
        "inet::Coord",    // FIELD_headingValue
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **ReferencePositionDescriptor::getFieldPropertyNames(int field) const
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

const char *ReferencePositionDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ReferencePositionDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ReferencePosition *pp = (ReferencePosition *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ReferencePositionDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ReferencePosition *pp = (ReferencePosition *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ReferencePositionDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ReferencePosition *pp = (ReferencePosition *)object; (void)pp;
    switch (field) {
        case FIELD_referencePosition: {std::stringstream out; out << pp->getReferencePosition(); return out.str();}
        case FIELD_semiMajorConf: return double2string(pp->getSemiMajorConf());
        case FIELD_semiMinorConf: return double2string(pp->getSemiMinorConf());
        case FIELD_headingValue: {std::stringstream out; out << pp->getHeadingValue(); return out.str();}
        default: return "";
    }
}

bool ReferencePositionDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ReferencePosition *pp = (ReferencePosition *)object; (void)pp;
    switch (field) {
        case FIELD_semiMajorConf: pp->setSemiMajorConf(string2double(value)); return true;
        case FIELD_semiMinorConf: pp->setSemiMinorConf(string2double(value)); return true;
        default: return false;
    }
}

const char *ReferencePositionDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_referencePosition: return omnetpp::opp_typename(typeid(inet::Coord));
        case FIELD_headingValue: return omnetpp::opp_typename(typeid(inet::Coord));
        default: return nullptr;
    };
}

void *ReferencePositionDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ReferencePosition *pp = (ReferencePosition *)object; (void)pp;
    switch (field) {
        case FIELD_referencePosition: return toVoidPtr(&pp->getReferencePosition()); break;
        case FIELD_headingValue: return toVoidPtr(&pp->getHeadingValue()); break;
        default: return nullptr;
    }
}

Register_Class(ItsContainer)

ItsContainer::ItsContainer() : ::rover::ItsBase()
{
}

ItsContainer::ItsContainer(const ItsContainer& other) : ::rover::ItsBase(other)
{
    copy(other);
}

ItsContainer::~ItsContainer()
{
}

ItsContainer& ItsContainer::operator=(const ItsContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsBase::operator=(other);
    copy(other);
    return *this;
}

void ItsContainer::copy(const ItsContainer& other)
{
}

void ItsContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsBase::parsimPack(b);
}

void ItsContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsBase::parsimUnpack(b);
}

class ItsContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsContainerDescriptor();
    virtual ~ItsContainerDescriptor();

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

Register_ClassDescriptor(ItsContainerDescriptor)

ItsContainerDescriptor::ItsContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsContainer)), "rover::ItsBase")
{
    propertynames = nullptr;
}

ItsContainerDescriptor::~ItsContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsContainer *>(obj)!=nullptr;
}

const char **ItsContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsContainer *pp = (ItsContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsContainer *pp = (ItsContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsContainer *pp = (ItsContainer *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsContainer *pp = (ItsContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsContainer *pp = (ItsContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsBasicContainer)

ItsBasicContainer::ItsBasicContainer() : ::rover::ItsContainer()
{
}

ItsBasicContainer::ItsBasicContainer(const ItsBasicContainer& other) : ::rover::ItsContainer(other)
{
    copy(other);
}

ItsBasicContainer::~ItsBasicContainer()
{
}

ItsBasicContainer& ItsBasicContainer::operator=(const ItsBasicContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsBasicContainer::copy(const ItsBasicContainer& other)
{
    this->stationType = other.stationType;
    this->referencePosition = other.referencePosition;
    this->role = other.role;
    for (size_t i = 0; i < 40; i++) {
        this->pathHistory[i] = other.pathHistory[i];
    }
}

void ItsBasicContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsContainer::parsimPack(b);
    doParsimPacking(b,this->stationType);
    doParsimPacking(b,this->referencePosition);
    doParsimPacking(b,this->role);
    doParsimArrayPacking(b,this->pathHistory,40);
}

void ItsBasicContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsContainer::parsimUnpack(b);
    doParsimUnpacking(b,this->stationType);
    doParsimUnpacking(b,this->referencePosition);
    doParsimUnpacking(b,this->role);
    doParsimArrayUnpacking(b,this->pathHistory,40);
}

rover::ItsStationType ItsBasicContainer::getStationType() const
{
    return this->stationType;
}

void ItsBasicContainer::setStationType(rover::ItsStationType stationType)
{
    this->stationType = stationType;
}

const ReferencePosition& ItsBasicContainer::getReferencePosition() const
{
    return this->referencePosition;
}

void ItsBasicContainer::setReferencePosition(const ReferencePosition& referencePosition)
{
    this->referencePosition = referencePosition;
}

rover::ItsVehicleRole ItsBasicContainer::getRole() const
{
    return this->role;
}

void ItsBasicContainer::setRole(rover::ItsVehicleRole role)
{
    this->role = role;
}

size_t ItsBasicContainer::getPathHistoryArraySize() const
{
    return 40;
}

const PathPoint& ItsBasicContainer::getPathHistory(size_t k) const
{
    if (k >= 40) throw omnetpp::cRuntimeError("Array of size 40 indexed by %lu", (unsigned long)k);
    return this->pathHistory[k];
}

void ItsBasicContainer::setPathHistory(size_t k, const PathPoint& pathHistory)
{
    if (k >= 40) throw omnetpp::cRuntimeError("Array of size 40 indexed by %lu", (unsigned long)k);
    this->pathHistory[k] = pathHistory;
}

class ItsBasicContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_stationType,
        FIELD_referencePosition,
        FIELD_role,
        FIELD_pathHistory,
    };
  public:
    ItsBasicContainerDescriptor();
    virtual ~ItsBasicContainerDescriptor();

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

Register_ClassDescriptor(ItsBasicContainerDescriptor)

ItsBasicContainerDescriptor::ItsBasicContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsBasicContainer)), "rover::ItsContainer")
{
    propertynames = nullptr;
}

ItsBasicContainerDescriptor::~ItsBasicContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsBasicContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsBasicContainer *>(obj)!=nullptr;
}

const char **ItsBasicContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsBasicContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsBasicContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount() : 4;
}

unsigned int ItsBasicContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        0,    // FIELD_stationType
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_referencePosition
        0,    // FIELD_role
        FD_ISARRAY | FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_pathHistory
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *ItsBasicContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "stationType",
        "referencePosition",
        "role",
        "pathHistory",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int ItsBasicContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 's' && strcmp(fieldName, "stationType") == 0) return base+0;
    if (fieldName[0] == 'r' && strcmp(fieldName, "referencePosition") == 0) return base+1;
    if (fieldName[0] == 'r' && strcmp(fieldName, "role") == 0) return base+2;
    if (fieldName[0] == 'p' && strcmp(fieldName, "pathHistory") == 0) return base+3;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsBasicContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "rover::ItsStationType",    // FIELD_stationType
        "rover::ReferencePosition",    // FIELD_referencePosition
        "rover::ItsVehicleRole",    // FIELD_role
        "rover::PathPoint",    // FIELD_pathHistory
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **ItsBasicContainerDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_stationType: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        case FIELD_role: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *ItsBasicContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_stationType:
            if (!strcmp(propertyname, "enum")) return "rover::ItsStationType";
            return nullptr;
        case FIELD_role:
            if (!strcmp(propertyname, "enum")) return "rover::ItsVehicleRole";
            return nullptr;
        default: return nullptr;
    }
}

int ItsBasicContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsBasicContainer *pp = (ItsBasicContainer *)object; (void)pp;
    switch (field) {
        case FIELD_pathHistory: return 40;
        default: return 0;
    }
}

const char *ItsBasicContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsBasicContainer *pp = (ItsBasicContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsBasicContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsBasicContainer *pp = (ItsBasicContainer *)object; (void)pp;
    switch (field) {
        case FIELD_stationType: return enum2string(pp->getStationType(), "rover::ItsStationType");
        case FIELD_referencePosition: {std::stringstream out; out << pp->getReferencePosition(); return out.str();}
        case FIELD_role: return enum2string(pp->getRole(), "rover::ItsVehicleRole");
        case FIELD_pathHistory: {std::stringstream out; out << pp->getPathHistory(i); return out.str();}
        default: return "";
    }
}

bool ItsBasicContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsBasicContainer *pp = (ItsBasicContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsBasicContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_referencePosition: return omnetpp::opp_typename(typeid(ReferencePosition));
        case FIELD_pathHistory: return omnetpp::opp_typename(typeid(PathPoint));
        default: return nullptr;
    };
}

void *ItsBasicContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsBasicContainer *pp = (ItsBasicContainer *)object; (void)pp;
    switch (field) {
        case FIELD_referencePosition: return toVoidPtr(&pp->getReferencePosition()); break;
        case FIELD_pathHistory: return toVoidPtr(&pp->getPathHistory(i)); break;
        default: return nullptr;
    }
}

Register_Class(ItsBasicVehicleLfContainer)

ItsBasicVehicleLfContainer::ItsBasicVehicleLfContainer() : ::rover::ItsBasicContainer()
{
}

ItsBasicVehicleLfContainer::ItsBasicVehicleLfContainer(const ItsBasicVehicleLfContainer& other) : ::rover::ItsBasicContainer(other)
{
    copy(other);
}

ItsBasicVehicleLfContainer::~ItsBasicVehicleLfContainer()
{
}

ItsBasicVehicleLfContainer& ItsBasicVehicleLfContainer::operator=(const ItsBasicVehicleLfContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsBasicContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsBasicVehicleLfContainer::copy(const ItsBasicVehicleLfContainer& other)
{
    this->exterioLights = other.exterioLights;
    for (size_t i = 0; i < 40; i++) {
        this->pathHistory[i] = other.pathHistory[i];
    }
}

void ItsBasicVehicleLfContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsBasicContainer::parsimPack(b);
    doParsimPacking(b,this->exterioLights);
    doParsimArrayPacking(b,this->pathHistory,40);
}

void ItsBasicVehicleLfContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsBasicContainer::parsimUnpack(b);
    doParsimUnpacking(b,this->exterioLights);
    doParsimArrayUnpacking(b,this->pathHistory,40);
}

short ItsBasicVehicleLfContainer::getExterioLights() const
{
    return this->exterioLights;
}

void ItsBasicVehicleLfContainer::setExterioLights(short exterioLights)
{
    this->exterioLights = exterioLights;
}

size_t ItsBasicVehicleLfContainer::getPathHistoryArraySize() const
{
    return 40;
}

const PathPoint& ItsBasicVehicleLfContainer::getPathHistory(size_t k) const
{
    if (k >= 40) throw omnetpp::cRuntimeError("Array of size 40 indexed by %lu", (unsigned long)k);
    return this->pathHistory[k];
}

void ItsBasicVehicleLfContainer::setPathHistory(size_t k, const PathPoint& pathHistory)
{
    if (k >= 40) throw omnetpp::cRuntimeError("Array of size 40 indexed by %lu", (unsigned long)k);
    this->pathHistory[k] = pathHistory;
}

class ItsBasicVehicleLfContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_exterioLights,
        FIELD_pathHistory,
    };
  public:
    ItsBasicVehicleLfContainerDescriptor();
    virtual ~ItsBasicVehicleLfContainerDescriptor();

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

Register_ClassDescriptor(ItsBasicVehicleLfContainerDescriptor)

ItsBasicVehicleLfContainerDescriptor::ItsBasicVehicleLfContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsBasicVehicleLfContainer)), "rover::ItsBasicContainer")
{
    propertynames = nullptr;
}

ItsBasicVehicleLfContainerDescriptor::~ItsBasicVehicleLfContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsBasicVehicleLfContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsBasicVehicleLfContainer *>(obj)!=nullptr;
}

const char **ItsBasicVehicleLfContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsBasicVehicleLfContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsBasicVehicleLfContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount() : 2;
}

unsigned int ItsBasicVehicleLfContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_exterioLights
        FD_ISARRAY | FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_pathHistory
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *ItsBasicVehicleLfContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "exterioLights",
        "pathHistory",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int ItsBasicVehicleLfContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'e' && strcmp(fieldName, "exterioLights") == 0) return base+0;
    if (fieldName[0] == 'p' && strcmp(fieldName, "pathHistory") == 0) return base+1;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsBasicVehicleLfContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "short",    // FIELD_exterioLights
        "rover::PathPoint",    // FIELD_pathHistory
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **ItsBasicVehicleLfContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsBasicVehicleLfContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsBasicVehicleLfContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsBasicVehicleLfContainer *pp = (ItsBasicVehicleLfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_pathHistory: return 40;
        default: return 0;
    }
}

const char *ItsBasicVehicleLfContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsBasicVehicleLfContainer *pp = (ItsBasicVehicleLfContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsBasicVehicleLfContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsBasicVehicleLfContainer *pp = (ItsBasicVehicleLfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_exterioLights: return long2string(pp->getExterioLights());
        case FIELD_pathHistory: {std::stringstream out; out << pp->getPathHistory(i); return out.str();}
        default: return "";
    }
}

bool ItsBasicVehicleLfContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsBasicVehicleLfContainer *pp = (ItsBasicVehicleLfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_exterioLights: pp->setExterioLights(string2long(value)); return true;
        default: return false;
    }
}

const char *ItsBasicVehicleLfContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_pathHistory: return omnetpp::opp_typename(typeid(PathPoint));
        default: return nullptr;
    };
}

void *ItsBasicVehicleLfContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsBasicVehicleLfContainer *pp = (ItsBasicVehicleLfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_pathHistory: return toVoidPtr(&pp->getPathHistory(i)); break;
        default: return nullptr;
    }
}

Register_Class(ItsVamBasicContainer)

ItsVamBasicContainer::ItsVamBasicContainer() : ::rover::ItsBasicContainer()
{
}

ItsVamBasicContainer::ItsVamBasicContainer(const ItsVamBasicContainer& other) : ::rover::ItsBasicContainer(other)
{
    copy(other);
}

ItsVamBasicContainer::~ItsVamBasicContainer()
{
}

ItsVamBasicContainer& ItsVamBasicContainer::operator=(const ItsVamBasicContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsBasicContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsVamBasicContainer::copy(const ItsVamBasicContainer& other)
{
    this->vruProfile = other.vruProfile;
    this->vruType = other.vruType;
    this->vruDeviceType = other.vruDeviceType;
}

void ItsVamBasicContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsBasicContainer::parsimPack(b);
    doParsimPacking(b,this->vruProfile);
    doParsimPacking(b,this->vruType);
    doParsimPacking(b,this->vruDeviceType);
}

void ItsVamBasicContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsBasicContainer::parsimUnpack(b);
    doParsimUnpacking(b,this->vruProfile);
    doParsimUnpacking(b,this->vruType);
    doParsimUnpacking(b,this->vruDeviceType);
}

rover::ItsVruProfile ItsVamBasicContainer::getVruProfile() const
{
    return this->vruProfile;
}

void ItsVamBasicContainer::setVruProfile(rover::ItsVruProfile vruProfile)
{
    this->vruProfile = vruProfile;
}

rover::ItsVruType ItsVamBasicContainer::getVruType() const
{
    return this->vruType;
}

void ItsVamBasicContainer::setVruType(rover::ItsVruType vruType)
{
    this->vruType = vruType;
}

rover::ItsVruDeviceType ItsVamBasicContainer::getVruDeviceType() const
{
    return this->vruDeviceType;
}

void ItsVamBasicContainer::setVruDeviceType(rover::ItsVruDeviceType vruDeviceType)
{
    this->vruDeviceType = vruDeviceType;
}

class ItsVamBasicContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_vruProfile,
        FIELD_vruType,
        FIELD_vruDeviceType,
    };
  public:
    ItsVamBasicContainerDescriptor();
    virtual ~ItsVamBasicContainerDescriptor();

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

Register_ClassDescriptor(ItsVamBasicContainerDescriptor)

ItsVamBasicContainerDescriptor::ItsVamBasicContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsVamBasicContainer)), "rover::ItsBasicContainer")
{
    propertynames = nullptr;
}

ItsVamBasicContainerDescriptor::~ItsVamBasicContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsVamBasicContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsVamBasicContainer *>(obj)!=nullptr;
}

const char **ItsVamBasicContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsVamBasicContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsVamBasicContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount() : 3;
}

unsigned int ItsVamBasicContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        0,    // FIELD_vruProfile
        0,    // FIELD_vruType
        0,    // FIELD_vruDeviceType
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *ItsVamBasicContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "vruProfile",
        "vruType",
        "vruDeviceType",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int ItsVamBasicContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'v' && strcmp(fieldName, "vruProfile") == 0) return base+0;
    if (fieldName[0] == 'v' && strcmp(fieldName, "vruType") == 0) return base+1;
    if (fieldName[0] == 'v' && strcmp(fieldName, "vruDeviceType") == 0) return base+2;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsVamBasicContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "rover::ItsVruProfile",    // FIELD_vruProfile
        "rover::ItsVruType",    // FIELD_vruType
        "rover::ItsVruDeviceType",    // FIELD_vruDeviceType
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **ItsVamBasicContainerDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_vruProfile: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        case FIELD_vruType: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        case FIELD_vruDeviceType: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *ItsVamBasicContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_vruProfile:
            if (!strcmp(propertyname, "enum")) return "rover::ItsVruProfile";
            return nullptr;
        case FIELD_vruType:
            if (!strcmp(propertyname, "enum")) return "rover::ItsVruType";
            return nullptr;
        case FIELD_vruDeviceType:
            if (!strcmp(propertyname, "enum")) return "rover::ItsVruDeviceType";
            return nullptr;
        default: return nullptr;
    }
}

int ItsVamBasicContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsVamBasicContainer *pp = (ItsVamBasicContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsVamBasicContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsVamBasicContainer *pp = (ItsVamBasicContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsVamBasicContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsVamBasicContainer *pp = (ItsVamBasicContainer *)object; (void)pp;
    switch (field) {
        case FIELD_vruProfile: return enum2string(pp->getVruProfile(), "rover::ItsVruProfile");
        case FIELD_vruType: return enum2string(pp->getVruType(), "rover::ItsVruType");
        case FIELD_vruDeviceType: return enum2string(pp->getVruDeviceType(), "rover::ItsVruDeviceType");
        default: return "";
    }
}

bool ItsVamBasicContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsVamBasicContainer *pp = (ItsVamBasicContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsVamBasicContainerDescriptor::getFieldStructName(int field) const
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

void *ItsVamBasicContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsVamBasicContainer *pp = (ItsVamBasicContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsHfContainer)

ItsHfContainer::ItsHfContainer() : ::rover::ItsContainer()
{
}

ItsHfContainer::ItsHfContainer(const ItsHfContainer& other) : ::rover::ItsContainer(other)
{
    copy(other);
}

ItsHfContainer::~ItsHfContainer()
{
}

ItsHfContainer& ItsHfContainer::operator=(const ItsHfContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsHfContainer::copy(const ItsHfContainer& other)
{
}

void ItsHfContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsContainer::parsimPack(b);
}

void ItsHfContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsContainer::parsimUnpack(b);
}

class ItsHfContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsHfContainerDescriptor();
    virtual ~ItsHfContainerDescriptor();

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

Register_ClassDescriptor(ItsHfContainerDescriptor)

ItsHfContainerDescriptor::ItsHfContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsHfContainer)), "rover::ItsContainer")
{
    propertynames = nullptr;
}

ItsHfContainerDescriptor::~ItsHfContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsHfContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsHfContainer *>(obj)!=nullptr;
}

const char **ItsHfContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsHfContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsHfContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsHfContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsHfContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsHfContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsHfContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsHfContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsHfContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsHfContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsHfContainer *pp = (ItsHfContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsHfContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsHfContainer *pp = (ItsHfContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsHfContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsHfContainer *pp = (ItsHfContainer *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsHfContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsHfContainer *pp = (ItsHfContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsHfContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsHfContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsHfContainer *pp = (ItsHfContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsBasicVehicleHfContainer)

ItsBasicVehicleHfContainer::ItsBasicVehicleHfContainer() : ::rover::ItsHfContainer()
{
}

ItsBasicVehicleHfContainer::ItsBasicVehicleHfContainer(const ItsBasicVehicleHfContainer& other) : ::rover::ItsHfContainer(other)
{
    copy(other);
}

ItsBasicVehicleHfContainer::~ItsBasicVehicleHfContainer()
{
}

ItsBasicVehicleHfContainer& ItsBasicVehicleHfContainer::operator=(const ItsBasicVehicleHfContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsHfContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsBasicVehicleHfContainer::copy(const ItsBasicVehicleHfContainer& other)
{
    this->heading = other.heading;
    this->speed = other.speed;
    this->vehicleLength = other.vehicleLength;
    this->vehicleWidth = other.vehicleWidth;
}

void ItsBasicVehicleHfContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsHfContainer::parsimPack(b);
    doParsimPacking(b,this->heading);
    doParsimPacking(b,this->speed);
    doParsimPacking(b,this->vehicleLength);
    doParsimPacking(b,this->vehicleWidth);
}

void ItsBasicVehicleHfContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsHfContainer::parsimUnpack(b);
    doParsimUnpacking(b,this->heading);
    doParsimUnpacking(b,this->speed);
    doParsimUnpacking(b,this->vehicleLength);
    doParsimUnpacking(b,this->vehicleWidth);
}

const inet::Coord& ItsBasicVehicleHfContainer::getHeading() const
{
    return this->heading;
}

void ItsBasicVehicleHfContainer::setHeading(const inet::Coord& heading)
{
    this->heading = heading;
}

double ItsBasicVehicleHfContainer::getSpeed() const
{
    return this->speed;
}

void ItsBasicVehicleHfContainer::setSpeed(double speed)
{
    this->speed = speed;
}

double ItsBasicVehicleHfContainer::getVehicleLength() const
{
    return this->vehicleLength;
}

void ItsBasicVehicleHfContainer::setVehicleLength(double vehicleLength)
{
    this->vehicleLength = vehicleLength;
}

double ItsBasicVehicleHfContainer::getVehicleWidth() const
{
    return this->vehicleWidth;
}

void ItsBasicVehicleHfContainer::setVehicleWidth(double vehicleWidth)
{
    this->vehicleWidth = vehicleWidth;
}

class ItsBasicVehicleHfContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_heading,
        FIELD_speed,
        FIELD_vehicleLength,
        FIELD_vehicleWidth,
    };
  public:
    ItsBasicVehicleHfContainerDescriptor();
    virtual ~ItsBasicVehicleHfContainerDescriptor();

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

Register_ClassDescriptor(ItsBasicVehicleHfContainerDescriptor)

ItsBasicVehicleHfContainerDescriptor::ItsBasicVehicleHfContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsBasicVehicleHfContainer)), "rover::ItsHfContainer")
{
    propertynames = nullptr;
}

ItsBasicVehicleHfContainerDescriptor::~ItsBasicVehicleHfContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsBasicVehicleHfContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsBasicVehicleHfContainer *>(obj)!=nullptr;
}

const char **ItsBasicVehicleHfContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsBasicVehicleHfContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsBasicVehicleHfContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount() : 4;
}

unsigned int ItsBasicVehicleHfContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND,    // FIELD_heading
        FD_ISEDITABLE,    // FIELD_speed
        FD_ISEDITABLE,    // FIELD_vehicleLength
        FD_ISEDITABLE,    // FIELD_vehicleWidth
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *ItsBasicVehicleHfContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "heading",
        "speed",
        "vehicleLength",
        "vehicleWidth",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int ItsBasicVehicleHfContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'h' && strcmp(fieldName, "heading") == 0) return base+0;
    if (fieldName[0] == 's' && strcmp(fieldName, "speed") == 0) return base+1;
    if (fieldName[0] == 'v' && strcmp(fieldName, "vehicleLength") == 0) return base+2;
    if (fieldName[0] == 'v' && strcmp(fieldName, "vehicleWidth") == 0) return base+3;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsBasicVehicleHfContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "inet::Coord",    // FIELD_heading
        "double",    // FIELD_speed
        "double",    // FIELD_vehicleLength
        "double",    // FIELD_vehicleWidth
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **ItsBasicVehicleHfContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsBasicVehicleHfContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsBasicVehicleHfContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsBasicVehicleHfContainer *pp = (ItsBasicVehicleHfContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsBasicVehicleHfContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsBasicVehicleHfContainer *pp = (ItsBasicVehicleHfContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsBasicVehicleHfContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsBasicVehicleHfContainer *pp = (ItsBasicVehicleHfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_heading: {std::stringstream out; out << pp->getHeading(); return out.str();}
        case FIELD_speed: return double2string(pp->getSpeed());
        case FIELD_vehicleLength: return double2string(pp->getVehicleLength());
        case FIELD_vehicleWidth: return double2string(pp->getVehicleWidth());
        default: return "";
    }
}

bool ItsBasicVehicleHfContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsBasicVehicleHfContainer *pp = (ItsBasicVehicleHfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_speed: pp->setSpeed(string2double(value)); return true;
        case FIELD_vehicleLength: pp->setVehicleLength(string2double(value)); return true;
        case FIELD_vehicleWidth: pp->setVehicleWidth(string2double(value)); return true;
        default: return false;
    }
}

const char *ItsBasicVehicleHfContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_heading: return omnetpp::opp_typename(typeid(inet::Coord));
        default: return nullptr;
    };
}

void *ItsBasicVehicleHfContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsBasicVehicleHfContainer *pp = (ItsBasicVehicleHfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_heading: return toVoidPtr(&pp->getHeading()); break;
        default: return nullptr;
    }
}

Register_Class(ItsVamHfContainer)

ItsVamHfContainer::ItsVamHfContainer() : ::rover::ItsHfContainer()
{
}

ItsVamHfContainer::ItsVamHfContainer(const ItsVamHfContainer& other) : ::rover::ItsHfContainer(other)
{
    copy(other);
}

ItsVamHfContainer::~ItsVamHfContainer()
{
}

ItsVamHfContainer& ItsVamHfContainer::operator=(const ItsVamHfContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsHfContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsVamHfContainer::copy(const ItsVamHfContainer& other)
{
    this->speed = other.speed;
    this->direction = other.direction;
    this->orientation = other.orientation;
    for (size_t i = 0; i < 10; i++) {
        this->predictedTrajectory[i] = other.predictedTrajectory[i];
    }
    for (size_t i = 0; i < 10; i++) {
        this->predictedVelocity[i] = other.predictedVelocity[i];
    }
}

void ItsVamHfContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsHfContainer::parsimPack(b);
    doParsimPacking(b,this->speed);
    doParsimPacking(b,this->direction);
    doParsimPacking(b,this->orientation);
    doParsimArrayPacking(b,this->predictedTrajectory,10);
    doParsimArrayPacking(b,this->predictedVelocity,10);
}

void ItsVamHfContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsHfContainer::parsimUnpack(b);
    doParsimUnpacking(b,this->speed);
    doParsimUnpacking(b,this->direction);
    doParsimUnpacking(b,this->orientation);
    doParsimArrayUnpacking(b,this->predictedTrajectory,10);
    doParsimArrayUnpacking(b,this->predictedVelocity,10);
}

double ItsVamHfContainer::getSpeed() const
{
    return this->speed;
}

void ItsVamHfContainer::setSpeed(double speed)
{
    this->speed = speed;
}

const inet::Coord& ItsVamHfContainer::getDirection() const
{
    return this->direction;
}

void ItsVamHfContainer::setDirection(const inet::Coord& direction)
{
    this->direction = direction;
}

const inet::Coord& ItsVamHfContainer::getOrientation() const
{
    return this->orientation;
}

void ItsVamHfContainer::setOrientation(const inet::Coord& orientation)
{
    this->orientation = orientation;
}

size_t ItsVamHfContainer::getPredictedTrajectoryArraySize() const
{
    return 10;
}

const PathPoint& ItsVamHfContainer::getPredictedTrajectory(size_t k) const
{
    if (k >= 10) throw omnetpp::cRuntimeError("Array of size 10 indexed by %lu", (unsigned long)k);
    return this->predictedTrajectory[k];
}

void ItsVamHfContainer::setPredictedTrajectory(size_t k, const PathPoint& predictedTrajectory)
{
    if (k >= 10) throw omnetpp::cRuntimeError("Array of size 10 indexed by %lu", (unsigned long)k);
    this->predictedTrajectory[k] = predictedTrajectory;
}

size_t ItsVamHfContainer::getPredictedVelocityArraySize() const
{
    return 10;
}

const PathPoint& ItsVamHfContainer::getPredictedVelocity(size_t k) const
{
    if (k >= 10) throw omnetpp::cRuntimeError("Array of size 10 indexed by %lu", (unsigned long)k);
    return this->predictedVelocity[k];
}

void ItsVamHfContainer::setPredictedVelocity(size_t k, const PathPoint& predictedVelocity)
{
    if (k >= 10) throw omnetpp::cRuntimeError("Array of size 10 indexed by %lu", (unsigned long)k);
    this->predictedVelocity[k] = predictedVelocity;
}

class ItsVamHfContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_speed,
        FIELD_direction,
        FIELD_orientation,
        FIELD_predictedTrajectory,
        FIELD_predictedVelocity,
    };
  public:
    ItsVamHfContainerDescriptor();
    virtual ~ItsVamHfContainerDescriptor();

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

Register_ClassDescriptor(ItsVamHfContainerDescriptor)

ItsVamHfContainerDescriptor::ItsVamHfContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsVamHfContainer)), "rover::ItsHfContainer")
{
    propertynames = nullptr;
}

ItsVamHfContainerDescriptor::~ItsVamHfContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsVamHfContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsVamHfContainer *>(obj)!=nullptr;
}

const char **ItsVamHfContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsVamHfContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsVamHfContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int ItsVamHfContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_speed
        FD_ISCOMPOUND,    // FIELD_direction
        FD_ISCOMPOUND,    // FIELD_orientation
        FD_ISARRAY | FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_predictedTrajectory
        FD_ISARRAY | FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_predictedVelocity
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *ItsVamHfContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "speed",
        "direction",
        "orientation",
        "predictedTrajectory",
        "predictedVelocity",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int ItsVamHfContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 's' && strcmp(fieldName, "speed") == 0) return base+0;
    if (fieldName[0] == 'd' && strcmp(fieldName, "direction") == 0) return base+1;
    if (fieldName[0] == 'o' && strcmp(fieldName, "orientation") == 0) return base+2;
    if (fieldName[0] == 'p' && strcmp(fieldName, "predictedTrajectory") == 0) return base+3;
    if (fieldName[0] == 'p' && strcmp(fieldName, "predictedVelocity") == 0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsVamHfContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "double",    // FIELD_speed
        "inet::Coord",    // FIELD_direction
        "inet::Coord",    // FIELD_orientation
        "rover::PathPoint",    // FIELD_predictedTrajectory
        "rover::PathPoint",    // FIELD_predictedVelocity
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **ItsVamHfContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsVamHfContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsVamHfContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsVamHfContainer *pp = (ItsVamHfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_predictedTrajectory: return 10;
        case FIELD_predictedVelocity: return 10;
        default: return 0;
    }
}

const char *ItsVamHfContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsVamHfContainer *pp = (ItsVamHfContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsVamHfContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsVamHfContainer *pp = (ItsVamHfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_speed: return double2string(pp->getSpeed());
        case FIELD_direction: {std::stringstream out; out << pp->getDirection(); return out.str();}
        case FIELD_orientation: {std::stringstream out; out << pp->getOrientation(); return out.str();}
        case FIELD_predictedTrajectory: {std::stringstream out; out << pp->getPredictedTrajectory(i); return out.str();}
        case FIELD_predictedVelocity: {std::stringstream out; out << pp->getPredictedVelocity(i); return out.str();}
        default: return "";
    }
}

bool ItsVamHfContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsVamHfContainer *pp = (ItsVamHfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_speed: pp->setSpeed(string2double(value)); return true;
        default: return false;
    }
}

const char *ItsVamHfContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_direction: return omnetpp::opp_typename(typeid(inet::Coord));
        case FIELD_orientation: return omnetpp::opp_typename(typeid(inet::Coord));
        case FIELD_predictedTrajectory: return omnetpp::opp_typename(typeid(PathPoint));
        case FIELD_predictedVelocity: return omnetpp::opp_typename(typeid(PathPoint));
        default: return nullptr;
    };
}

void *ItsVamHfContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsVamHfContainer *pp = (ItsVamHfContainer *)object; (void)pp;
    switch (field) {
        case FIELD_direction: return toVoidPtr(&pp->getDirection()); break;
        case FIELD_orientation: return toVoidPtr(&pp->getOrientation()); break;
        case FIELD_predictedTrajectory: return toVoidPtr(&pp->getPredictedTrajectory(i)); break;
        case FIELD_predictedVelocity: return toVoidPtr(&pp->getPredictedVelocity(i)); break;
        default: return nullptr;
    }
}

Register_Class(ItsLfContainer)

ItsLfContainer::ItsLfContainer() : ::rover::ItsContainer()
{
}

ItsLfContainer::ItsLfContainer(const ItsLfContainer& other) : ::rover::ItsContainer(other)
{
    copy(other);
}

ItsLfContainer::~ItsLfContainer()
{
}

ItsLfContainer& ItsLfContainer::operator=(const ItsLfContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsLfContainer::copy(const ItsLfContainer& other)
{
}

void ItsLfContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsContainer::parsimPack(b);
}

void ItsLfContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsContainer::parsimUnpack(b);
}

class ItsLfContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsLfContainerDescriptor();
    virtual ~ItsLfContainerDescriptor();

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

Register_ClassDescriptor(ItsLfContainerDescriptor)

ItsLfContainerDescriptor::ItsLfContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsLfContainer)), "rover::ItsContainer")
{
    propertynames = nullptr;
}

ItsLfContainerDescriptor::~ItsLfContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsLfContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsLfContainer *>(obj)!=nullptr;
}

const char **ItsLfContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsLfContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsLfContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsLfContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsLfContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsLfContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsLfContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsLfContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsLfContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsLfContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsLfContainer *pp = (ItsLfContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsLfContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsLfContainer *pp = (ItsLfContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsLfContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsLfContainer *pp = (ItsLfContainer *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsLfContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsLfContainer *pp = (ItsLfContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsLfContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsLfContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsLfContainer *pp = (ItsLfContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsSpecialVehicleContainer)

ItsSpecialVehicleContainer::ItsSpecialVehicleContainer() : ::rover::ItsContainer()
{
}

ItsSpecialVehicleContainer::ItsSpecialVehicleContainer(const ItsSpecialVehicleContainer& other) : ::rover::ItsContainer(other)
{
    copy(other);
}

ItsSpecialVehicleContainer::~ItsSpecialVehicleContainer()
{
}

ItsSpecialVehicleContainer& ItsSpecialVehicleContainer::operator=(const ItsSpecialVehicleContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsSpecialVehicleContainer::copy(const ItsSpecialVehicleContainer& other)
{
}

void ItsSpecialVehicleContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsContainer::parsimPack(b);
}

void ItsSpecialVehicleContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsContainer::parsimUnpack(b);
}

class ItsSpecialVehicleContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsSpecialVehicleContainerDescriptor();
    virtual ~ItsSpecialVehicleContainerDescriptor();

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

Register_ClassDescriptor(ItsSpecialVehicleContainerDescriptor)

ItsSpecialVehicleContainerDescriptor::ItsSpecialVehicleContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsSpecialVehicleContainer)), "rover::ItsContainer")
{
    propertynames = nullptr;
}

ItsSpecialVehicleContainerDescriptor::~ItsSpecialVehicleContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsSpecialVehicleContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsSpecialVehicleContainer *>(obj)!=nullptr;
}

const char **ItsSpecialVehicleContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsSpecialVehicleContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsSpecialVehicleContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsSpecialVehicleContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsSpecialVehicleContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsSpecialVehicleContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsSpecialVehicleContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsSpecialVehicleContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsSpecialVehicleContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsSpecialVehicleContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsSpecialVehicleContainer *pp = (ItsSpecialVehicleContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsSpecialVehicleContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsSpecialVehicleContainer *pp = (ItsSpecialVehicleContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsSpecialVehicleContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsSpecialVehicleContainer *pp = (ItsSpecialVehicleContainer *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsSpecialVehicleContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsSpecialVehicleContainer *pp = (ItsSpecialVehicleContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsSpecialVehicleContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsSpecialVehicleContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsSpecialVehicleContainer *pp = (ItsSpecialVehicleContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsVamClusterContainer)

ItsVamClusterContainer::ItsVamClusterContainer() : ::rover::ItsSpecialVehicleContainer()
{
}

ItsVamClusterContainer::ItsVamClusterContainer(const ItsVamClusterContainer& other) : ::rover::ItsSpecialVehicleContainer(other)
{
    copy(other);
}

ItsVamClusterContainer::~ItsVamClusterContainer()
{
}

ItsVamClusterContainer& ItsVamClusterContainer::operator=(const ItsVamClusterContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsSpecialVehicleContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsVamClusterContainer::copy(const ItsVamClusterContainer& other)
{
    this->clusterIdentifier = other.clusterIdentifier;
    this->clusterPosition = other.clusterPosition;
    this->clusterDimension = other.clusterDimension;
    this->clusterSize = other.clusterSize;
}

void ItsVamClusterContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsSpecialVehicleContainer::parsimPack(b);
    doParsimPacking(b,this->clusterIdentifier);
    doParsimPacking(b,this->clusterPosition);
    doParsimPacking(b,this->clusterDimension);
    doParsimPacking(b,this->clusterSize);
}

void ItsVamClusterContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsSpecialVehicleContainer::parsimUnpack(b);
    doParsimUnpacking(b,this->clusterIdentifier);
    doParsimUnpacking(b,this->clusterPosition);
    doParsimUnpacking(b,this->clusterDimension);
    doParsimUnpacking(b,this->clusterSize);
}

int ItsVamClusterContainer::getClusterIdentifier() const
{
    return this->clusterIdentifier;
}

void ItsVamClusterContainer::setClusterIdentifier(int clusterIdentifier)
{
    this->clusterIdentifier = clusterIdentifier;
}

const inet::Coord& ItsVamClusterContainer::getClusterPosition() const
{
    return this->clusterPosition;
}

void ItsVamClusterContainer::setClusterPosition(const inet::Coord& clusterPosition)
{
    this->clusterPosition = clusterPosition;
}

const inet::Coord& ItsVamClusterContainer::getClusterDimension() const
{
    return this->clusterDimension;
}

void ItsVamClusterContainer::setClusterDimension(const inet::Coord& clusterDimension)
{
    this->clusterDimension = clusterDimension;
}

int ItsVamClusterContainer::getClusterSize() const
{
    return this->clusterSize;
}

void ItsVamClusterContainer::setClusterSize(int clusterSize)
{
    this->clusterSize = clusterSize;
}

class ItsVamClusterContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_clusterIdentifier,
        FIELD_clusterPosition,
        FIELD_clusterDimension,
        FIELD_clusterSize,
    };
  public:
    ItsVamClusterContainerDescriptor();
    virtual ~ItsVamClusterContainerDescriptor();

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

Register_ClassDescriptor(ItsVamClusterContainerDescriptor)

ItsVamClusterContainerDescriptor::ItsVamClusterContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsVamClusterContainer)), "rover::ItsSpecialVehicleContainer")
{
    propertynames = nullptr;
}

ItsVamClusterContainerDescriptor::~ItsVamClusterContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsVamClusterContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsVamClusterContainer *>(obj)!=nullptr;
}

const char **ItsVamClusterContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsVamClusterContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsVamClusterContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount() : 4;
}

unsigned int ItsVamClusterContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_clusterIdentifier
        FD_ISCOMPOUND,    // FIELD_clusterPosition
        FD_ISCOMPOUND,    // FIELD_clusterDimension
        FD_ISEDITABLE,    // FIELD_clusterSize
    };
    return (field >= 0 && field < 4) ? fieldTypeFlags[field] : 0;
}

const char *ItsVamClusterContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "clusterIdentifier",
        "clusterPosition",
        "clusterDimension",
        "clusterSize",
    };
    return (field >= 0 && field < 4) ? fieldNames[field] : nullptr;
}

int ItsVamClusterContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'c' && strcmp(fieldName, "clusterIdentifier") == 0) return base+0;
    if (fieldName[0] == 'c' && strcmp(fieldName, "clusterPosition") == 0) return base+1;
    if (fieldName[0] == 'c' && strcmp(fieldName, "clusterDimension") == 0) return base+2;
    if (fieldName[0] == 'c' && strcmp(fieldName, "clusterSize") == 0) return base+3;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsVamClusterContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_clusterIdentifier
        "inet::Coord",    // FIELD_clusterPosition
        "inet::Coord",    // FIELD_clusterDimension
        "int",    // FIELD_clusterSize
    };
    return (field >= 0 && field < 4) ? fieldTypeStrings[field] : nullptr;
}

const char **ItsVamClusterContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsVamClusterContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsVamClusterContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsVamClusterContainer *pp = (ItsVamClusterContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsVamClusterContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsVamClusterContainer *pp = (ItsVamClusterContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsVamClusterContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsVamClusterContainer *pp = (ItsVamClusterContainer *)object; (void)pp;
    switch (field) {
        case FIELD_clusterIdentifier: return long2string(pp->getClusterIdentifier());
        case FIELD_clusterPosition: {std::stringstream out; out << pp->getClusterPosition(); return out.str();}
        case FIELD_clusterDimension: {std::stringstream out; out << pp->getClusterDimension(); return out.str();}
        case FIELD_clusterSize: return long2string(pp->getClusterSize());
        default: return "";
    }
}

bool ItsVamClusterContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsVamClusterContainer *pp = (ItsVamClusterContainer *)object; (void)pp;
    switch (field) {
        case FIELD_clusterIdentifier: pp->setClusterIdentifier(string2long(value)); return true;
        case FIELD_clusterSize: pp->setClusterSize(string2long(value)); return true;
        default: return false;
    }
}

const char *ItsVamClusterContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_clusterPosition: return omnetpp::opp_typename(typeid(inet::Coord));
        case FIELD_clusterDimension: return omnetpp::opp_typename(typeid(inet::Coord));
        default: return nullptr;
    };
}

void *ItsVamClusterContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsVamClusterContainer *pp = (ItsVamClusterContainer *)object; (void)pp;
    switch (field) {
        case FIELD_clusterPosition: return toVoidPtr(&pp->getClusterPosition()); break;
        case FIELD_clusterDimension: return toVoidPtr(&pp->getClusterDimension()); break;
        default: return nullptr;
    }
}

Register_Class(ItsPublicTransportContainer)

ItsPublicTransportContainer::ItsPublicTransportContainer() : ::rover::ItsSpecialVehicleContainer()
{
}

ItsPublicTransportContainer::ItsPublicTransportContainer(const ItsPublicTransportContainer& other) : ::rover::ItsSpecialVehicleContainer(other)
{
    copy(other);
}

ItsPublicTransportContainer::~ItsPublicTransportContainer()
{
}

ItsPublicTransportContainer& ItsPublicTransportContainer::operator=(const ItsPublicTransportContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsSpecialVehicleContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsPublicTransportContainer::copy(const ItsPublicTransportContainer& other)
{
}

void ItsPublicTransportContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsSpecialVehicleContainer::parsimPack(b);
}

void ItsPublicTransportContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsSpecialVehicleContainer::parsimUnpack(b);
}

class ItsPublicTransportContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsPublicTransportContainerDescriptor();
    virtual ~ItsPublicTransportContainerDescriptor();

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

Register_ClassDescriptor(ItsPublicTransportContainerDescriptor)

ItsPublicTransportContainerDescriptor::ItsPublicTransportContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsPublicTransportContainer)), "rover::ItsSpecialVehicleContainer")
{
    propertynames = nullptr;
}

ItsPublicTransportContainerDescriptor::~ItsPublicTransportContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsPublicTransportContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsPublicTransportContainer *>(obj)!=nullptr;
}

const char **ItsPublicTransportContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsPublicTransportContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsPublicTransportContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsPublicTransportContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsPublicTransportContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsPublicTransportContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsPublicTransportContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsPublicTransportContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsPublicTransportContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsPublicTransportContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsPublicTransportContainer *pp = (ItsPublicTransportContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsPublicTransportContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsPublicTransportContainer *pp = (ItsPublicTransportContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsPublicTransportContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsPublicTransportContainer *pp = (ItsPublicTransportContainer *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsPublicTransportContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsPublicTransportContainer *pp = (ItsPublicTransportContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsPublicTransportContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsPublicTransportContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsPublicTransportContainer *pp = (ItsPublicTransportContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsSpecialTransportContainer)

ItsSpecialTransportContainer::ItsSpecialTransportContainer() : ::rover::ItsSpecialVehicleContainer()
{
}

ItsSpecialTransportContainer::ItsSpecialTransportContainer(const ItsSpecialTransportContainer& other) : ::rover::ItsSpecialVehicleContainer(other)
{
    copy(other);
}

ItsSpecialTransportContainer::~ItsSpecialTransportContainer()
{
}

ItsSpecialTransportContainer& ItsSpecialTransportContainer::operator=(const ItsSpecialTransportContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsSpecialVehicleContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsSpecialTransportContainer::copy(const ItsSpecialTransportContainer& other)
{
}

void ItsSpecialTransportContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsSpecialVehicleContainer::parsimPack(b);
}

void ItsSpecialTransportContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsSpecialVehicleContainer::parsimUnpack(b);
}

class ItsSpecialTransportContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsSpecialTransportContainerDescriptor();
    virtual ~ItsSpecialTransportContainerDescriptor();

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

Register_ClassDescriptor(ItsSpecialTransportContainerDescriptor)

ItsSpecialTransportContainerDescriptor::ItsSpecialTransportContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsSpecialTransportContainer)), "rover::ItsSpecialVehicleContainer")
{
    propertynames = nullptr;
}

ItsSpecialTransportContainerDescriptor::~ItsSpecialTransportContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsSpecialTransportContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsSpecialTransportContainer *>(obj)!=nullptr;
}

const char **ItsSpecialTransportContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsSpecialTransportContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsSpecialTransportContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsSpecialTransportContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsSpecialTransportContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsSpecialTransportContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsSpecialTransportContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsSpecialTransportContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsSpecialTransportContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsSpecialTransportContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsSpecialTransportContainer *pp = (ItsSpecialTransportContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsSpecialTransportContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsSpecialTransportContainer *pp = (ItsSpecialTransportContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsSpecialTransportContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsSpecialTransportContainer *pp = (ItsSpecialTransportContainer *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsSpecialTransportContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsSpecialTransportContainer *pp = (ItsSpecialTransportContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsSpecialTransportContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsSpecialTransportContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsSpecialTransportContainer *pp = (ItsSpecialTransportContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsDangerousGoodsContainer)

ItsDangerousGoodsContainer::ItsDangerousGoodsContainer() : ::rover::ItsSpecialVehicleContainer()
{
}

ItsDangerousGoodsContainer::ItsDangerousGoodsContainer(const ItsDangerousGoodsContainer& other) : ::rover::ItsSpecialVehicleContainer(other)
{
    copy(other);
}

ItsDangerousGoodsContainer::~ItsDangerousGoodsContainer()
{
}

ItsDangerousGoodsContainer& ItsDangerousGoodsContainer::operator=(const ItsDangerousGoodsContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsSpecialVehicleContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsDangerousGoodsContainer::copy(const ItsDangerousGoodsContainer& other)
{
}

void ItsDangerousGoodsContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsSpecialVehicleContainer::parsimPack(b);
}

void ItsDangerousGoodsContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsSpecialVehicleContainer::parsimUnpack(b);
}

class ItsDangerousGoodsContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsDangerousGoodsContainerDescriptor();
    virtual ~ItsDangerousGoodsContainerDescriptor();

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

Register_ClassDescriptor(ItsDangerousGoodsContainerDescriptor)

ItsDangerousGoodsContainerDescriptor::ItsDangerousGoodsContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsDangerousGoodsContainer)), "rover::ItsSpecialVehicleContainer")
{
    propertynames = nullptr;
}

ItsDangerousGoodsContainerDescriptor::~ItsDangerousGoodsContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsDangerousGoodsContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsDangerousGoodsContainer *>(obj)!=nullptr;
}

const char **ItsDangerousGoodsContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsDangerousGoodsContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsDangerousGoodsContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsDangerousGoodsContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsDangerousGoodsContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsDangerousGoodsContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsDangerousGoodsContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsDangerousGoodsContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsDangerousGoodsContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsDangerousGoodsContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsDangerousGoodsContainer *pp = (ItsDangerousGoodsContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsDangerousGoodsContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsDangerousGoodsContainer *pp = (ItsDangerousGoodsContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsDangerousGoodsContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsDangerousGoodsContainer *pp = (ItsDangerousGoodsContainer *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsDangerousGoodsContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsDangerousGoodsContainer *pp = (ItsDangerousGoodsContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsDangerousGoodsContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsDangerousGoodsContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsDangerousGoodsContainer *pp = (ItsDangerousGoodsContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsRoadWorksContainerBasic)

ItsRoadWorksContainerBasic::ItsRoadWorksContainerBasic() : ::rover::ItsSpecialVehicleContainer()
{
}

ItsRoadWorksContainerBasic::ItsRoadWorksContainerBasic(const ItsRoadWorksContainerBasic& other) : ::rover::ItsSpecialVehicleContainer(other)
{
    copy(other);
}

ItsRoadWorksContainerBasic::~ItsRoadWorksContainerBasic()
{
}

ItsRoadWorksContainerBasic& ItsRoadWorksContainerBasic::operator=(const ItsRoadWorksContainerBasic& other)
{
    if (this == &other) return *this;
    ::rover::ItsSpecialVehicleContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsRoadWorksContainerBasic::copy(const ItsRoadWorksContainerBasic& other)
{
}

void ItsRoadWorksContainerBasic::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsSpecialVehicleContainer::parsimPack(b);
}

void ItsRoadWorksContainerBasic::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsSpecialVehicleContainer::parsimUnpack(b);
}

class ItsRoadWorksContainerBasicDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsRoadWorksContainerBasicDescriptor();
    virtual ~ItsRoadWorksContainerBasicDescriptor();

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

Register_ClassDescriptor(ItsRoadWorksContainerBasicDescriptor)

ItsRoadWorksContainerBasicDescriptor::ItsRoadWorksContainerBasicDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsRoadWorksContainerBasic)), "rover::ItsSpecialVehicleContainer")
{
    propertynames = nullptr;
}

ItsRoadWorksContainerBasicDescriptor::~ItsRoadWorksContainerBasicDescriptor()
{
    delete[] propertynames;
}

bool ItsRoadWorksContainerBasicDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsRoadWorksContainerBasic *>(obj)!=nullptr;
}

const char **ItsRoadWorksContainerBasicDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsRoadWorksContainerBasicDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsRoadWorksContainerBasicDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsRoadWorksContainerBasicDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsRoadWorksContainerBasicDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsRoadWorksContainerBasicDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsRoadWorksContainerBasicDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsRoadWorksContainerBasicDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsRoadWorksContainerBasicDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsRoadWorksContainerBasicDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsRoadWorksContainerBasic *pp = (ItsRoadWorksContainerBasic *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsRoadWorksContainerBasicDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsRoadWorksContainerBasic *pp = (ItsRoadWorksContainerBasic *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsRoadWorksContainerBasicDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsRoadWorksContainerBasic *pp = (ItsRoadWorksContainerBasic *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsRoadWorksContainerBasicDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsRoadWorksContainerBasic *pp = (ItsRoadWorksContainerBasic *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsRoadWorksContainerBasicDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsRoadWorksContainerBasicDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsRoadWorksContainerBasic *pp = (ItsRoadWorksContainerBasic *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsRescueContainer)

ItsRescueContainer::ItsRescueContainer() : ::rover::ItsSpecialVehicleContainer()
{
}

ItsRescueContainer::ItsRescueContainer(const ItsRescueContainer& other) : ::rover::ItsSpecialVehicleContainer(other)
{
    copy(other);
}

ItsRescueContainer::~ItsRescueContainer()
{
}

ItsRescueContainer& ItsRescueContainer::operator=(const ItsRescueContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsSpecialVehicleContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsRescueContainer::copy(const ItsRescueContainer& other)
{
}

void ItsRescueContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsSpecialVehicleContainer::parsimPack(b);
}

void ItsRescueContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsSpecialVehicleContainer::parsimUnpack(b);
}

class ItsRescueContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsRescueContainerDescriptor();
    virtual ~ItsRescueContainerDescriptor();

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

Register_ClassDescriptor(ItsRescueContainerDescriptor)

ItsRescueContainerDescriptor::ItsRescueContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsRescueContainer)), "rover::ItsSpecialVehicleContainer")
{
    propertynames = nullptr;
}

ItsRescueContainerDescriptor::~ItsRescueContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsRescueContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsRescueContainer *>(obj)!=nullptr;
}

const char **ItsRescueContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsRescueContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsRescueContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsRescueContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsRescueContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsRescueContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsRescueContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsRescueContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsRescueContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsRescueContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsRescueContainer *pp = (ItsRescueContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsRescueContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsRescueContainer *pp = (ItsRescueContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsRescueContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsRescueContainer *pp = (ItsRescueContainer *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsRescueContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsRescueContainer *pp = (ItsRescueContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsRescueContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsRescueContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsRescueContainer *pp = (ItsRescueContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsEmergencyContainer)

ItsEmergencyContainer::ItsEmergencyContainer() : ::rover::ItsSpecialVehicleContainer()
{
}

ItsEmergencyContainer::ItsEmergencyContainer(const ItsEmergencyContainer& other) : ::rover::ItsSpecialVehicleContainer(other)
{
    copy(other);
}

ItsEmergencyContainer::~ItsEmergencyContainer()
{
}

ItsEmergencyContainer& ItsEmergencyContainer::operator=(const ItsEmergencyContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsSpecialVehicleContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsEmergencyContainer::copy(const ItsEmergencyContainer& other)
{
}

void ItsEmergencyContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsSpecialVehicleContainer::parsimPack(b);
}

void ItsEmergencyContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsSpecialVehicleContainer::parsimUnpack(b);
}

class ItsEmergencyContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsEmergencyContainerDescriptor();
    virtual ~ItsEmergencyContainerDescriptor();

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

Register_ClassDescriptor(ItsEmergencyContainerDescriptor)

ItsEmergencyContainerDescriptor::ItsEmergencyContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsEmergencyContainer)), "rover::ItsSpecialVehicleContainer")
{
    propertynames = nullptr;
}

ItsEmergencyContainerDescriptor::~ItsEmergencyContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsEmergencyContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsEmergencyContainer *>(obj)!=nullptr;
}

const char **ItsEmergencyContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsEmergencyContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsEmergencyContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsEmergencyContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsEmergencyContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsEmergencyContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsEmergencyContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsEmergencyContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsEmergencyContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsEmergencyContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsEmergencyContainer *pp = (ItsEmergencyContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsEmergencyContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsEmergencyContainer *pp = (ItsEmergencyContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsEmergencyContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsEmergencyContainer *pp = (ItsEmergencyContainer *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsEmergencyContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsEmergencyContainer *pp = (ItsEmergencyContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsEmergencyContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsEmergencyContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsEmergencyContainer *pp = (ItsEmergencyContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsSafetyCarContainer)

ItsSafetyCarContainer::ItsSafetyCarContainer() : ::rover::ItsSpecialVehicleContainer()
{
}

ItsSafetyCarContainer::ItsSafetyCarContainer(const ItsSafetyCarContainer& other) : ::rover::ItsSpecialVehicleContainer(other)
{
    copy(other);
}

ItsSafetyCarContainer::~ItsSafetyCarContainer()
{
}

ItsSafetyCarContainer& ItsSafetyCarContainer::operator=(const ItsSafetyCarContainer& other)
{
    if (this == &other) return *this;
    ::rover::ItsSpecialVehicleContainer::operator=(other);
    copy(other);
    return *this;
}

void ItsSafetyCarContainer::copy(const ItsSafetyCarContainer& other)
{
}

void ItsSafetyCarContainer::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsSpecialVehicleContainer::parsimPack(b);
}

void ItsSafetyCarContainer::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsSpecialVehicleContainer::parsimUnpack(b);
}

class ItsSafetyCarContainerDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
    };
  public:
    ItsSafetyCarContainerDescriptor();
    virtual ~ItsSafetyCarContainerDescriptor();

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

Register_ClassDescriptor(ItsSafetyCarContainerDescriptor)

ItsSafetyCarContainerDescriptor::ItsSafetyCarContainerDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsSafetyCarContainer)), "rover::ItsSpecialVehicleContainer")
{
    propertynames = nullptr;
}

ItsSafetyCarContainerDescriptor::~ItsSafetyCarContainerDescriptor()
{
    delete[] propertynames;
}

bool ItsSafetyCarContainerDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsSafetyCarContainer *>(obj)!=nullptr;
}

const char **ItsSafetyCarContainerDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsSafetyCarContainerDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsSafetyCarContainerDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 0+basedesc->getFieldCount() : 0;
}

unsigned int ItsSafetyCarContainerDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    return 0;
}

const char *ItsSafetyCarContainerDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

int ItsSafetyCarContainerDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsSafetyCarContainerDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

const char **ItsSafetyCarContainerDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsSafetyCarContainerDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsSafetyCarContainerDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsSafetyCarContainer *pp = (ItsSafetyCarContainer *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsSafetyCarContainerDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsSafetyCarContainer *pp = (ItsSafetyCarContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsSafetyCarContainerDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsSafetyCarContainer *pp = (ItsSafetyCarContainer *)object; (void)pp;
    switch (field) {
        default: return "";
    }
}

bool ItsSafetyCarContainerDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsSafetyCarContainer *pp = (ItsSafetyCarContainer *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsSafetyCarContainerDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    return nullptr;
}

void *ItsSafetyCarContainerDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsSafetyCarContainer *pp = (ItsSafetyCarContainer *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsPduHeader)

ItsPduHeader::ItsPduHeader() : ::rover::ItsBase()
{
}

ItsPduHeader::ItsPduHeader(const ItsPduHeader& other) : ::rover::ItsBase(other)
{
    copy(other);
}

ItsPduHeader::~ItsPduHeader()
{
}

ItsPduHeader& ItsPduHeader::operator=(const ItsPduHeader& other)
{
    if (this == &other) return *this;
    ::rover::ItsBase::operator=(other);
    copy(other);
    return *this;
}

void ItsPduHeader::copy(const ItsPduHeader& other)
{
    this->protocolVersion = other.protocolVersion;
    this->messageId = other.messageId;
    this->stationId = other.stationId;
}

void ItsPduHeader::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::rover::ItsBase::parsimPack(b);
    doParsimPacking(b,this->protocolVersion);
    doParsimPacking(b,this->messageId);
    doParsimPacking(b,this->stationId);
}

void ItsPduHeader::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::rover::ItsBase::parsimUnpack(b);
    doParsimUnpacking(b,this->protocolVersion);
    doParsimUnpacking(b,this->messageId);
    doParsimUnpacking(b,this->stationId);
}

int ItsPduHeader::getProtocolVersion() const
{
    return this->protocolVersion;
}

void ItsPduHeader::setProtocolVersion(int protocolVersion)
{
    this->protocolVersion = protocolVersion;
}

rover::ItsMessageId ItsPduHeader::getMessageId() const
{
    return this->messageId;
}

void ItsPduHeader::setMessageId(rover::ItsMessageId messageId)
{
    this->messageId = messageId;
}

int ItsPduHeader::getStationId() const
{
    return this->stationId;
}

void ItsPduHeader::setStationId(int stationId)
{
    this->stationId = stationId;
}

class ItsPduHeaderDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_protocolVersion,
        FIELD_messageId,
        FIELD_stationId,
    };
  public:
    ItsPduHeaderDescriptor();
    virtual ~ItsPduHeaderDescriptor();

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

Register_ClassDescriptor(ItsPduHeaderDescriptor)

ItsPduHeaderDescriptor::ItsPduHeaderDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsPduHeader)), "rover::ItsBase")
{
    propertynames = nullptr;
}

ItsPduHeaderDescriptor::~ItsPduHeaderDescriptor()
{
    delete[] propertynames;
}

bool ItsPduHeaderDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsPduHeader *>(obj)!=nullptr;
}

const char **ItsPduHeaderDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsPduHeaderDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsPduHeaderDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 3+basedesc->getFieldCount() : 3;
}

unsigned int ItsPduHeaderDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_protocolVersion
        0,    // FIELD_messageId
        FD_ISEDITABLE,    // FIELD_stationId
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *ItsPduHeaderDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "protocolVersion",
        "messageId",
        "stationId",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int ItsPduHeaderDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'p' && strcmp(fieldName, "protocolVersion") == 0) return base+0;
    if (fieldName[0] == 'm' && strcmp(fieldName, "messageId") == 0) return base+1;
    if (fieldName[0] == 's' && strcmp(fieldName, "stationId") == 0) return base+2;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsPduHeaderDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_protocolVersion
        "rover::ItsMessageId",    // FIELD_messageId
        "int",    // FIELD_stationId
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **ItsPduHeaderDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_messageId: {
            static const char *names[] = { "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *ItsPduHeaderDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_messageId:
            if (!strcmp(propertyname, "enum")) return "rover::ItsMessageId";
            return nullptr;
        default: return nullptr;
    }
}

int ItsPduHeaderDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsPduHeader *pp = (ItsPduHeader *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsPduHeaderDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsPduHeader *pp = (ItsPduHeader *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsPduHeaderDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsPduHeader *pp = (ItsPduHeader *)object; (void)pp;
    switch (field) {
        case FIELD_protocolVersion: return long2string(pp->getProtocolVersion());
        case FIELD_messageId: return enum2string(pp->getMessageId(), "rover::ItsMessageId");
        case FIELD_stationId: return long2string(pp->getStationId());
        default: return "";
    }
}

bool ItsPduHeaderDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsPduHeader *pp = (ItsPduHeader *)object; (void)pp;
    switch (field) {
        case FIELD_protocolVersion: pp->setProtocolVersion(string2long(value)); return true;
        case FIELD_stationId: pp->setStationId(string2long(value)); return true;
        default: return false;
    }
}

const char *ItsPduHeaderDescriptor::getFieldStructName(int field) const
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

void *ItsPduHeaderDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsPduHeader *pp = (ItsPduHeader *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

Register_Class(ItsCam)

ItsCam::ItsCam() : ::inet::ApplicationPacket()
{
}

ItsCam::ItsCam(const ItsCam& other) : ::inet::ApplicationPacket(other)
{
    copy(other);
}

ItsCam::~ItsCam()
{
}

ItsCam& ItsCam::operator=(const ItsCam& other)
{
    if (this == &other) return *this;
    ::inet::ApplicationPacket::operator=(other);
    copy(other);
    return *this;
}

void ItsCam::copy(const ItsCam& other)
{
    this->itsHeader = other.itsHeader;
    this->generationDeltaTime = other.generationDeltaTime;
    this->basicContainer = other.basicContainer;
    this->hfContainer = other.hfContainer;
    this->lfContainer = other.lfContainer;
    this->specialContainer = other.specialContainer;
}

void ItsCam::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::ApplicationPacket::parsimPack(b);
    doParsimPacking(b,this->itsHeader);
    doParsimPacking(b,this->generationDeltaTime);
    doParsimPacking(b,this->basicContainer);
    doParsimPacking(b,this->hfContainer);
    doParsimPacking(b,this->lfContainer);
    doParsimPacking(b,this->specialContainer);
}

void ItsCam::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::ApplicationPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->itsHeader);
    doParsimUnpacking(b,this->generationDeltaTime);
    doParsimUnpacking(b,this->basicContainer);
    doParsimUnpacking(b,this->hfContainer);
    doParsimUnpacking(b,this->lfContainer);
    doParsimUnpacking(b,this->specialContainer);
}

const ItsPduHeader& ItsCam::getItsHeader() const
{
    return this->itsHeader;
}

void ItsCam::setItsHeader(const ItsPduHeader& itsHeader)
{
    handleChange();
    this->itsHeader = itsHeader;
}

omnetpp::simtime_t ItsCam::getGenerationDeltaTime() const
{
    return this->generationDeltaTime;
}

void ItsCam::setGenerationDeltaTime(omnetpp::simtime_t generationDeltaTime)
{
    handleChange();
    this->generationDeltaTime = generationDeltaTime;
}

const ItsBasicContainer& ItsCam::getBasicContainer() const
{
    return this->basicContainer;
}

void ItsCam::setBasicContainer(const ItsBasicContainer& basicContainer)
{
    handleChange();
    this->basicContainer = basicContainer;
}

const ItsHfContainer& ItsCam::getHfContainer() const
{
    return this->hfContainer;
}

void ItsCam::setHfContainer(const ItsHfContainer& hfContainer)
{
    handleChange();
    this->hfContainer = hfContainer;
}

const ItsLfContainer& ItsCam::getLfContainer() const
{
    return this->lfContainer;
}

void ItsCam::setLfContainer(const ItsLfContainer& lfContainer)
{
    handleChange();
    this->lfContainer = lfContainer;
}

const ItsSpecialVehicleContainer& ItsCam::getSpecialContainer() const
{
    return this->specialContainer;
}

void ItsCam::setSpecialContainer(const ItsSpecialVehicleContainer& specialContainer)
{
    handleChange();
    this->specialContainer = specialContainer;
}

class ItsCamDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_itsHeader,
        FIELD_generationDeltaTime,
        FIELD_basicContainer,
        FIELD_hfContainer,
        FIELD_lfContainer,
        FIELD_specialContainer,
    };
  public:
    ItsCamDescriptor();
    virtual ~ItsCamDescriptor();

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

Register_ClassDescriptor(ItsCamDescriptor)

ItsCamDescriptor::ItsCamDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsCam)), "inet::ApplicationPacket")
{
    propertynames = nullptr;
}

ItsCamDescriptor::~ItsCamDescriptor()
{
    delete[] propertynames;
}

bool ItsCamDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsCam *>(obj)!=nullptr;
}

const char **ItsCamDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsCamDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsCamDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 6+basedesc->getFieldCount() : 6;
}

unsigned int ItsCamDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_itsHeader
        0,    // FIELD_generationDeltaTime
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_basicContainer
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_hfContainer
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_lfContainer
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_specialContainer
    };
    return (field >= 0 && field < 6) ? fieldTypeFlags[field] : 0;
}

const char *ItsCamDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "itsHeader",
        "generationDeltaTime",
        "basicContainer",
        "hfContainer",
        "lfContainer",
        "specialContainer",
    };
    return (field >= 0 && field < 6) ? fieldNames[field] : nullptr;
}

int ItsCamDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'i' && strcmp(fieldName, "itsHeader") == 0) return base+0;
    if (fieldName[0] == 'g' && strcmp(fieldName, "generationDeltaTime") == 0) return base+1;
    if (fieldName[0] == 'b' && strcmp(fieldName, "basicContainer") == 0) return base+2;
    if (fieldName[0] == 'h' && strcmp(fieldName, "hfContainer") == 0) return base+3;
    if (fieldName[0] == 'l' && strcmp(fieldName, "lfContainer") == 0) return base+4;
    if (fieldName[0] == 's' && strcmp(fieldName, "specialContainer") == 0) return base+5;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsCamDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "rover::ItsPduHeader",    // FIELD_itsHeader
        "omnetpp::simtime_t",    // FIELD_generationDeltaTime
        "rover::ItsBasicContainer",    // FIELD_basicContainer
        "rover::ItsHfContainer",    // FIELD_hfContainer
        "rover::ItsLfContainer",    // FIELD_lfContainer
        "rover::ItsSpecialVehicleContainer",    // FIELD_specialContainer
    };
    return (field >= 0 && field < 6) ? fieldTypeStrings[field] : nullptr;
}

const char **ItsCamDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsCamDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsCamDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsCam *pp = (ItsCam *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsCamDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsCam *pp = (ItsCam *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsCamDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsCam *pp = (ItsCam *)object; (void)pp;
    switch (field) {
        case FIELD_itsHeader: {std::stringstream out; out << pp->getItsHeader(); return out.str();}
        case FIELD_generationDeltaTime: return simtime2string(pp->getGenerationDeltaTime());
        case FIELD_basicContainer: {std::stringstream out; out << pp->getBasicContainer(); return out.str();}
        case FIELD_hfContainer: {std::stringstream out; out << pp->getHfContainer(); return out.str();}
        case FIELD_lfContainer: {std::stringstream out; out << pp->getLfContainer(); return out.str();}
        case FIELD_specialContainer: {std::stringstream out; out << pp->getSpecialContainer(); return out.str();}
        default: return "";
    }
}

bool ItsCamDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsCam *pp = (ItsCam *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsCamDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_itsHeader: return omnetpp::opp_typename(typeid(ItsPduHeader));
        case FIELD_basicContainer: return omnetpp::opp_typename(typeid(ItsBasicContainer));
        case FIELD_hfContainer: return omnetpp::opp_typename(typeid(ItsHfContainer));
        case FIELD_lfContainer: return omnetpp::opp_typename(typeid(ItsLfContainer));
        case FIELD_specialContainer: return omnetpp::opp_typename(typeid(ItsSpecialVehicleContainer));
        default: return nullptr;
    };
}

void *ItsCamDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsCam *pp = (ItsCam *)object; (void)pp;
    switch (field) {
        case FIELD_itsHeader: return toVoidPtr(&pp->getItsHeader()); break;
        case FIELD_basicContainer: return toVoidPtr(&pp->getBasicContainer()); break;
        case FIELD_hfContainer: return toVoidPtr(&pp->getHfContainer()); break;
        case FIELD_lfContainer: return toVoidPtr(&pp->getLfContainer()); break;
        case FIELD_specialContainer: return toVoidPtr(&pp->getSpecialContainer()); break;
        default: return nullptr;
    }
}

Register_Class(ItsVam)

ItsVam::ItsVam() : ::inet::ApplicationPacket()
{
}

ItsVam::ItsVam(const ItsVam& other) : ::inet::ApplicationPacket(other)
{
    copy(other);
}

ItsVam::~ItsVam()
{
}

ItsVam& ItsVam::operator=(const ItsVam& other)
{
    if (this == &other) return *this;
    ::inet::ApplicationPacket::operator=(other);
    copy(other);
    return *this;
}

void ItsVam::copy(const ItsVam& other)
{
    this->itsHeader = other.itsHeader;
    this->generationDeltaTime = other.generationDeltaTime;
    this->basicContainer = other.basicContainer;
    this->hfContainer = other.hfContainer;
    this->lfContainer = other.lfContainer;
    this->clusterContainer = other.clusterContainer;
}

void ItsVam::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::inet::ApplicationPacket::parsimPack(b);
    doParsimPacking(b,this->itsHeader);
    doParsimPacking(b,this->generationDeltaTime);
    doParsimPacking(b,this->basicContainer);
    doParsimPacking(b,this->hfContainer);
    doParsimPacking(b,this->lfContainer);
    doParsimPacking(b,this->clusterContainer);
}

void ItsVam::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::inet::ApplicationPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->itsHeader);
    doParsimUnpacking(b,this->generationDeltaTime);
    doParsimUnpacking(b,this->basicContainer);
    doParsimUnpacking(b,this->hfContainer);
    doParsimUnpacking(b,this->lfContainer);
    doParsimUnpacking(b,this->clusterContainer);
}

const ItsPduHeader& ItsVam::getItsHeader() const
{
    return this->itsHeader;
}

void ItsVam::setItsHeader(const ItsPduHeader& itsHeader)
{
    handleChange();
    this->itsHeader = itsHeader;
}

omnetpp::simtime_t ItsVam::getGenerationDeltaTime() const
{
    return this->generationDeltaTime;
}

void ItsVam::setGenerationDeltaTime(omnetpp::simtime_t generationDeltaTime)
{
    handleChange();
    this->generationDeltaTime = generationDeltaTime;
}

const ItsVamBasicContainer& ItsVam::getBasicContainer() const
{
    return this->basicContainer;
}

void ItsVam::setBasicContainer(const ItsVamBasicContainer& basicContainer)
{
    handleChange();
    this->basicContainer = basicContainer;
}

const ItsVamHfContainer& ItsVam::getHfContainer() const
{
    return this->hfContainer;
}

void ItsVam::setHfContainer(const ItsVamHfContainer& hfContainer)
{
    handleChange();
    this->hfContainer = hfContainer;
}

const ItsLfContainer& ItsVam::getLfContainer() const
{
    return this->lfContainer;
}

void ItsVam::setLfContainer(const ItsLfContainer& lfContainer)
{
    handleChange();
    this->lfContainer = lfContainer;
}

const ItsVamClusterContainer& ItsVam::getClusterContainer() const
{
    return this->clusterContainer;
}

void ItsVam::setClusterContainer(const ItsVamClusterContainer& clusterContainer)
{
    handleChange();
    this->clusterContainer = clusterContainer;
}

class ItsVamDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_itsHeader,
        FIELD_generationDeltaTime,
        FIELD_basicContainer,
        FIELD_hfContainer,
        FIELD_lfContainer,
        FIELD_clusterContainer,
    };
  public:
    ItsVamDescriptor();
    virtual ~ItsVamDescriptor();

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

Register_ClassDescriptor(ItsVamDescriptor)

ItsVamDescriptor::ItsVamDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(rover::ItsVam)), "inet::ApplicationPacket")
{
    propertynames = nullptr;
}

ItsVamDescriptor::~ItsVamDescriptor()
{
    delete[] propertynames;
}

bool ItsVamDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<ItsVam *>(obj)!=nullptr;
}

const char **ItsVamDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *ItsVamDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int ItsVamDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 6+basedesc->getFieldCount() : 6;
}

unsigned int ItsVamDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_itsHeader
        0,    // FIELD_generationDeltaTime
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_basicContainer
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_hfContainer
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_lfContainer
        FD_ISCOMPOUND | FD_ISCOBJECT,    // FIELD_clusterContainer
    };
    return (field >= 0 && field < 6) ? fieldTypeFlags[field] : 0;
}

const char *ItsVamDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "itsHeader",
        "generationDeltaTime",
        "basicContainer",
        "hfContainer",
        "lfContainer",
        "clusterContainer",
    };
    return (field >= 0 && field < 6) ? fieldNames[field] : nullptr;
}

int ItsVamDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'i' && strcmp(fieldName, "itsHeader") == 0) return base+0;
    if (fieldName[0] == 'g' && strcmp(fieldName, "generationDeltaTime") == 0) return base+1;
    if (fieldName[0] == 'b' && strcmp(fieldName, "basicContainer") == 0) return base+2;
    if (fieldName[0] == 'h' && strcmp(fieldName, "hfContainer") == 0) return base+3;
    if (fieldName[0] == 'l' && strcmp(fieldName, "lfContainer") == 0) return base+4;
    if (fieldName[0] == 'c' && strcmp(fieldName, "clusterContainer") == 0) return base+5;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *ItsVamDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "rover::ItsPduHeader",    // FIELD_itsHeader
        "omnetpp::simtime_t",    // FIELD_generationDeltaTime
        "rover::ItsVamBasicContainer",    // FIELD_basicContainer
        "rover::ItsVamHfContainer",    // FIELD_hfContainer
        "rover::ItsLfContainer",    // FIELD_lfContainer
        "rover::ItsVamClusterContainer",    // FIELD_clusterContainer
    };
    return (field >= 0 && field < 6) ? fieldTypeStrings[field] : nullptr;
}

const char **ItsVamDescriptor::getFieldPropertyNames(int field) const
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

const char *ItsVamDescriptor::getFieldProperty(int field, const char *propertyname) const
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

int ItsVamDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    ItsVam *pp = (ItsVam *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *ItsVamDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsVam *pp = (ItsVam *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string ItsVamDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    ItsVam *pp = (ItsVam *)object; (void)pp;
    switch (field) {
        case FIELD_itsHeader: {std::stringstream out; out << pp->getItsHeader(); return out.str();}
        case FIELD_generationDeltaTime: return simtime2string(pp->getGenerationDeltaTime());
        case FIELD_basicContainer: {std::stringstream out; out << pp->getBasicContainer(); return out.str();}
        case FIELD_hfContainer: {std::stringstream out; out << pp->getHfContainer(); return out.str();}
        case FIELD_lfContainer: {std::stringstream out; out << pp->getLfContainer(); return out.str();}
        case FIELD_clusterContainer: {std::stringstream out; out << pp->getClusterContainer(); return out.str();}
        default: return "";
    }
}

bool ItsVamDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    ItsVam *pp = (ItsVam *)object; (void)pp;
    switch (field) {
        default: return false;
    }
}

const char *ItsVamDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case FIELD_itsHeader: return omnetpp::opp_typename(typeid(ItsPduHeader));
        case FIELD_basicContainer: return omnetpp::opp_typename(typeid(ItsVamBasicContainer));
        case FIELD_hfContainer: return omnetpp::opp_typename(typeid(ItsVamHfContainer));
        case FIELD_lfContainer: return omnetpp::opp_typename(typeid(ItsLfContainer));
        case FIELD_clusterContainer: return omnetpp::opp_typename(typeid(ItsVamClusterContainer));
        default: return nullptr;
    };
}

void *ItsVamDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    ItsVam *pp = (ItsVam *)object; (void)pp;
    switch (field) {
        case FIELD_itsHeader: return toVoidPtr(&pp->getItsHeader()); break;
        case FIELD_basicContainer: return toVoidPtr(&pp->getBasicContainer()); break;
        case FIELD_hfContainer: return toVoidPtr(&pp->getHfContainer()); break;
        case FIELD_lfContainer: return toVoidPtr(&pp->getLfContainer()); break;
        case FIELD_clusterContainer: return toVoidPtr(&pp->getClusterContainer()); break;
        default: return nullptr;
    }
}

} // namespace rover

