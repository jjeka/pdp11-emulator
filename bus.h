#ifndef BUS_H
#define BUS_H

#include <vector>
#include <stdint.h>

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
        uint16_t start;
        uint16_t size;
        unsigned flags;
    };

private:
    std::vector<AddressRegion> regions_;
    Vcpu* vcpu_;

private:
    AddressRegion* resolveRegion_(uint16_t start, uint16_t size, bool write);

public:
    Bus(Vcpu* vcpu);

    void addAddressRegion(Device* owner, uint16_t start, uint16_t size, unsigned flags);

    void set8(uint16_t address, uint8_t data);
    uint8_t get8(uint16_t address);
    void set16(uint16_t address, uint16_t data);
    uint16_t get16(uint16_t address);


};

#endif // BUS_H
