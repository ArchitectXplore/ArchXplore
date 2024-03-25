#ifndef __DRAMSIM3_HPP__
#define __DRAMSIM3_HPP__
#include "dramsim3_wrapper.hpp"
#include "sparta/ports/DataPort.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "uncore/mem_if.hpp"
#include <unordered_map>
#include <queue>
namespace archXplore{
namespace uncore{
class DRAMSim3ParameterSet : public sparta::ParameterSet{
public:
    DRAMSim3ParameterSet(sparta::TreeNode * n):
        sparta::ParameterSet(n){}
    // TODO: change to relative path
    PARAMETER(std::string, config_file, "/root/ArchXplore/ext/DRAMsim3/configs/DDR3_4Gb_x4_1600.ini", "DRAMSim3 config file")
    PARAMETER(std::string, trace_file, "/root/ArchXplore/tests/uncoreTest", "DRAMSim3 trace file")
    PARAMETER(uint32_t, num_ports, 64, "DRAMSim3 ports number")
}; // class DRAMSim3ParameterSet

class DRAMSim3: public sparta::Unit, public MemIf{
public:
    DRAMSim3(sparta::TreeNode * n, const DRAMSim3ParameterSet * p);

    static constexpr char name[] = "dramsim3";
    virtual auto sendMemReq(const MemReq& req) -> void override;
    virtual auto receiveMemResp(const MemResp& resp) -> void override;
    virtual auto sendMemResp(const MemResp& resp) -> void override;
    virtual auto receiveMemReq(const MemReq& req) -> void override;
    virtual auto sendSnoopReq(const SnoopReq& req) -> void override;
    virtual auto receiveSnoopResp(const SnoopResp& resp) -> void override;
    virtual auto sendSnoopResp(const SnoopResp& resp) -> void override;
    virtual auto receiveSnoopReq(const SnoopReq& req) -> void override;
protected:
    std::unique_ptr<DRAMSim3Wrapper> m_wrapper;
    uint32_t m_queue_size;
    uint32_t m_burst_size;
    uint32_t m_num_ports;
    // dramsim3 callbacks
    std::function<void(uint64_t)> m_read_callback;
    std::function<void(uint64_t)> m_write_callback;
    auto m_readComplete(uint64_t addr) -> void;
    auto m_writeComplete(uint64_t addr) -> void;

    std::queue<MemReq> m_pending_queue;
    std::queue<uint32_t> m_cpu_wait_for_credit;
    std::unordered_map<uint64_t, std::queue<MemReq>> m_read_queue;
    uint32_t m_inflight_reads = 0;
    std::unordered_map<uint64_t, std::queue<MemReq>> m_write_queue;
    uint32_t m_inflight_writes = 0;
    // ports
    // upper
    std::vector<std::unique_ptr<sparta::DataInPort<MemReq>>> m_upper_req_in_ports;
    std::vector<std::unique_ptr<sparta::DataOutPort<uint32_t>>> m_upper_req_credit_out_ports;
    std::vector<std::unique_ptr<sparta::DataOutPort<MemResp>>> m_upper_resp_out_ports;
    std::vector<std::unique_ptr<sparta::DataInPort<uint32_t>>> m_upper_resp_credit_in_ports;
    std::vector<uint32_t> m_upper_resp_credit_vec;

    // lower 
    // latency is 0 since it connect to immediate response data memory. it will not influence data response to upper level.
    sparta::DataOutPort<MemReq> m_lower_req_out{&unit_port_set_, "lower_req_out", 0};
    sparta::DataInPort<MemResp> m_lower_resp_in{&unit_port_set_, "lower_resp_in", 0};

    sparta::UniqueEvent<> m_tick_event{&unit_event_set_, "m_tick_event", CREATE_SPARTA_HANDLER(DRAMSim3, m_tick)};

    auto m_is_full() -> bool;
    auto m_is_empty() -> bool;
    auto m_upperRespCreditInCB(const uint32_t& credit) -> void;
    auto m_initCB() -> void;
    inline auto m_handlePendingQueue() -> void;
    inline auto m_handleCPUWaitForCredit() -> void;
    auto m_tick() -> void;
}; // class DRAMSim3
} // namespace uncore
} // namespace archXplore 
#endif // __DRAMSIM3_HPP__