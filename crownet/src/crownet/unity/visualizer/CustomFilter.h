namespace inet {
namespace utils {
namespace filters {

class INET_API PersonIdFilter : public cObjectResultFilter
{
public:
    virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t, cObject *object, cObject *details) override;
};

}
}

}
