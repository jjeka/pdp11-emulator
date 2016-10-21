#ifndef BUS_H
#define BUS_H

#include <vector>
#include <stdint.h>
#include <string>

class Vcpu;
class Device;

#define BUS_ADDRESSREGION_READ      (1 << 0)
#define BUS_ADDRESSREGION_WRITE     (1 << 1)
#define BUS_ADDRESSREGION_REGISTER  (1 << 2)

class Bus
{

public:
    struct AddressRegion
    {
        Device* owner;
        unsigned start;
        unsigned size;
        unsigned flags;
        std::string name;
    };

private:
    std::vector<AddressRegion> regions_;
    Vcpu* vcpu_;

private:
    AddressRegion* resolveRegion_(unsigned start, unsigned size, bool write);

public:
    Bus(Vcpu* vcpu);

    void addAddressRegion(Device* owner, unsigned start, unsigned size, unsigned flags, const std::string& name);

    void set8(unsigned address, uint8_t data);
    uint8_t get8(unsigned address);
    void set16(unsigned address, uint16_t data);
    uint16_t get16(unsigned address);


};

#endif // BUS_H
