#ifndef __RANDOM_TESTER_HPP__
#define __RANDOM_TESTER_HPP__
#include "sparta/ports/DataPort.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/simulation/ParameterSet.hpp"

#include "uncore/mem_if.hpp"

#include <queue>
#include <unordered_map>
#include <random>
namespace archXplore{
namespace uncore{
class RandomTesterParameterSet : public sparta::ParameterSet{
public:
    RandomTesterParameterSet(sparta::TreeNode * n) :
        sparta::ParameterSet(n){}
    PARAMETER(uint64_t, base_addr, 0x0, "Base address of the memory region to test")
    PARAMETER(uint64_t, size, 0x1000000, "Size of the memory region to test")
    PARAMETER(uint32_t, line_size, 64, "Line size of the memory region to test")
    PARAMETER(uint32_t, line_mask, 64 - 1, "Line mask of the memory region to test")
    PARAMETER(uint32_t, cpuid, 0, "cpu id")
    PARAMETER(uint32_t, num_iters, 1000, "Number of iterations to test")
    PARAMETER(uint32_t, seed, 114514, "Seed for the random number generator")
    PARAMETER(uint32_t, req_stride, 5, "stride between a generation try")
    PARAMETER(uint32_t, req_gen_prob, 100, "the probability of generating req. in %")
    PARAMETER(uint32_t, req_queue_size, 32, "the size of the request queue")
}; // class RandomTesterParameterSet

class RandomTester : public sparta::Unit, public MemIf{
public:
    RandomTester(sparta::TreeNode * n, const RandomTesterParameterSet * p);
    static constexpr char name[] = "random_tester";
    virtual auto sendMemReq(const MemReq& req) -> void override;
    virtual auto receiveMemResp(const MemResp& resp) -> void override;
    virtual auto sendMemResp(const MemResp& resp) -> void override;
    virtual auto receiveMemReq(const MemReq& req) -> void override;

    virtual auto sendSnoopReq(const SnoopReq& req) -> void override;
    virtual auto receiveSnoopResp(const SnoopResp& resp) -> void override;
    virtual auto sendSnoopResp(const SnoopResp& resp) -> void override;
    virtual auto receiveSnoopReq(const SnoopReq& req) -> void override;
protected:
    uint64_t m_base_addr;
    uint64_t m_size;
    uint32_t m_line_size;
    uint32_t m_line_mask;
    uint32_t m_cpuid;
    uint32_t m_num_iters;
    uint32_t m_seed;
    uint32_t m_req_stride;
    uint32_t m_req_gen_prob;
    uint32_t m_req_queue_size;

    std::deque<MemReq> m_req_queue;
    std::deque<MemReq> m_debug_req_queue;
    std::unordered_map<uint64_t, std::unique_ptr<uint8_t[]>> m_mem_map;
    std::unordered_map<uint64_t, std::unique_ptr<uint8_t[]>> m_debug_mem_map;

    sparta::DataOutPort<MemReq> m_lower_req_out{&unit_port_set_, "lower_req_out", 1};
    sparta::DataInPort<uint32_t> m_lower_req_credit_in{&unit_port_set_, "lower_req_credit_in", 1};
    uint32_t m_lower_req_credit = 0;
    sparta::DataInPort<MemResp> m_lower_resp_in{&unit_port_set_, "lower_resp_in", 1};
    sparta::DataOutPort<uint32_t> m_lower_resp_credit_out{&unit_port_set_, "lower_resp_credit_out", 1};

    sparta::DataOutPort<MemReq> m_debug_mem_req_out{&unit_port_set_, "debug_mem_req_out", 0};

    sparta::UniqueEvent<> m_send_req_event{&unit_event_set_, "enque_req", CREATE_SPARTA_HANDLER(RandomTester, m_sendReqCB)};

    auto m_lowerReqCreditInCB(const uint32_t & credit) -> void;
    auto m_initCB() -> void;
    auto m_sendReqCB() -> void;
    inline auto m_genRandReq() -> MemReq;
    inline auto m_debugCloneReq(const MemReq& req) -> MemReq;


}; // class RandomTester
} // namespace uncore
} // namespace archXplore



#endif // __RANDOM_TESTER_HPP__