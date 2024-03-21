#ifndef __FAKE_MEMORY_HPP__
#define __FAKE_MEMORY_HPP__
#include "sparta/ports/DataPort.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "sparta/utils/SpartaAssert.hpp"

#include "uncore/mem_if.hpp"

namespace archXplore{
namespace uncore{

class FakeMemoryParameterSet : public sparta::ParameterSet{
public:
    FakeMemoryParameterSet(sparta::TreeNode * n):
        sparta::ParameterSet(n){}
    PARAMETER(uint64_t, base_addr, 0x0, "Base address of the memory region to test")
    PARAMETER(uint64_t, size, 0x1000000, "Size of the memory region to test")
}; // class FakeMemoryParameterSet
class FakeMemory : public sparta::Unit, public MemIf{
public:
    // Defined name
    static constexpr char name[] = "fake_memory";
    FakeMemory(sparta::TreeNode * n, const FakeMemoryParameterSet * p);
    virtual auto sendMemReq(const MemReq& req) -> void override;
    virtual auto receiveMemResp(const MemResp& resp) -> void override;
    virtual auto sendMemResp(const MemResp& resp) -> void override;
    virtual auto receiveMemReq(const MemReq& req) -> void override;

    virtual auto sendSnoopReq(const SnoopReq& req) -> void override;
    virtual auto receiveSnoopResp(const SnoopResp& resp) -> void override;
    virtual auto sendSnoopResp(const SnoopResp& resp) -> void override;
    virtual auto receiveSnoopReq(const SnoopReq& req) -> void override;
protected:
    uint64_t m_base_addr = 0;
    uint64_t m_size = 0;
    std::unique_ptr<uint8_t[]> m_data;

    sparta::DataInPort<MemReq> m_upper_req_in{&unit_port_set_, "upper_req_in", 1};
    sparta::DataOutPort<MemResp> m_upper_resp_out{&unit_port_set_, "upper_resp_out", 1};
}; // class FakeMemory
} // namespace uncore
} // namespace archXplore
#endif // __FAKE_MEMORY_HPP__