#ifndef __SIMPLE_CACHE_HPP__
#define __SIMPLE_CACHE_HPP__
#include <unordered_map>
#include "sparta/ports/DataPort.hpp"
#include "sparta/events/UniqueEvent.hpp"
#include "sparta/simulation/Unit.hpp"
#include "sparta/simulation/TreeNode.hpp"
#include "sparta/simulation/ParameterSet.hpp"
#include "basic_cache_block.hpp"
#include "default_agu.hpp"
#include "treeplru.hpp"
#include "uncore/mem_if.hpp"
#include "uncore/mem_pkt.hpp"

namespace archXplore{
namespace uncore{
class CacheLine: public BasicCacheLine{
public:
    CacheLine(const CacheLine& other){
        m_tag = other.m_tag;
        m_addr = other.m_addr;
        m_state = other.m_state;
        m_data.reset(new uint8_t[other.m_size]);
    }
    CacheLine& operator=(const CacheLine& other){
        m_tag = other.m_tag;
        m_addr = other.m_addr;
        m_state = other.m_state;
        m_size = other.m_size;
        m_data.reset(new uint8_t[other.m_size]);
        memcpy(m_data.get(), other.m_data.get(), other.m_size);
        return *this;
    }
    CacheLine(const uint64_t& tag, const uint64_t& addr, const uint32_t& size, const uint8_t* data){
        m_tag = tag;
        m_addr = addr;
        m_state = BasicCacheLineState::Modified;
        m_size = size;
        m_data.reset(new uint8_t[size]);
        memcpy(m_data.get(), data, size);
    }
    CacheLine(const uint64_t& tag, const uint64_t& addr, const uint32_t& size){
        m_tag = tag;
        m_addr = addr;
        m_state = BasicCacheLineState::Modified;
        m_size = size;
        m_data.reset(new uint8_t[size]);
        memset(m_data.get(), 0, size);
    }
    virtual auto read(const uint64_t& offset, const uint32_t& size, uint8_t* buff) const -> bool{
        memcpy(buff, m_data.get() + offset, size);
        return true; 
    }
    virtual auto write(const uint64_t& offset, const uint32_t& size, const uint8_t* buff) -> bool {
        memcpy(m_data.get() + offset, buff, size);
        return true;
    }
protected:
    using BasicCacheLine::m_tag;
    using BasicCacheLine::m_addr;
    using BasicCacheLine::m_state;
    uint32_t m_size;
    std::unique_ptr<uint8_t[]> m_data;
}; // class CacheLine

class SimpleCacheParameterSet: public sparta::ParameterSet{
public:
    SimpleCacheParameterSet(sparta::TreeNode * n):
        sparta::ParameterSet(n)
    {}
    PARAMETER(uint32_t, size, 1024, "Cache size in bytes")
    PARAMETER(uint32_t, num_ways, 4, "Number of cache ways")
    PARAMETER(uint32_t, line_size, 64, "Cache line size in bytes")
    PARAMETER(bool, is_cache_inclusive, true, "Is cache inclusive")
    PARAMETER(bool, is_first_level_cache, false, "Is first level cache")
    PARAMETER(bool, is_last_level_cache, false, "Is last level cache")
    PARAMETER(bool, is_data_cache, true, "Is data cache")
    PARAMETER(bool, is_write_allocate, true, "Is write allocate cache")
    PARAMETER(bool, is_write_back, true, "Is write back cache")
};
class SimpleCache: public sparta::Unit, public MemIf{
public:
    SimpleCache(sparta::TreeNode * n, const SimpleCacheParameterSet * p);
    static constexpr char name[] = "simple_cache";
    virtual auto sendMemReq(const MemReq& req) -> void override;
    virtual auto receiveMemResp(const MemResp& resp) -> void override;
    virtual auto sendMemResp(const MemResp& resp) -> void override;
    virtual auto receiveMemReq(const MemReq& req) -> void override;

    virtual auto sendSnoopReq(const SnoopReq& req) -> void override;
    virtual auto receiveSnoopResp(const SnoopResp& resp) -> void override;
    virtual auto sendSnoopResp(const SnoopResp& resp) -> void override;
    virtual auto receiveSnoopReq(const SnoopReq& req) -> void override;
protected:
    uint32_t m_size;
    uint32_t m_num_sets;
    uint32_t m_num_ways;
    uint32_t m_line_size;
    bool m_is_cache_inclusive;
    bool m_is_first_level_cache;
    bool m_is_last_level_cache;
    bool m_is_data_cache;
    bool m_is_write_allocate;
    bool m_is_write_back;
    std::unique_ptr<BasicCacheBlock<CacheLine>> m_cache;
    std::unique_ptr<AGUIf> m_agu;
    std::unique_ptr<ReplacementIf> m_replacement_policy;

    // registers
    MemReq m_inflight_req;
    bool m_inflight_req_valid = false;
    // mshr
    MemReq m_mshr_req;
    bool m_mshr_req_valid = false;
    std::unique_ptr<uint8_t[]> m_mshr_data;
    //evict reg
    MemReq m_evict_req;
    bool m_evict_req_valid = false;
    CacheLine* m_evict_line = nullptr;
    // this map is used to store evicted lines' data part.
    std::unordered_map<uint64_t, std::unique_ptr<uint8_t[]>> m_evict_map;

    MemResp m_inflight_resp;
    bool m_inflight_resp_valid = false;
    MemResp m_mshr_resp;
    bool m_mshr_resp_valid = false;
    SnoopReq m_inflight_snoop_req;
    bool m_inflight_snoop_req_valid = false;
    SnoopResp m_inflight_snoop_resp;
    bool m_inflight_snoop_resp_valid = false;

    bool m_exclusive_forwarding_but_inflight_resp_is_valid = false;
    bool m_hit_but_inflight_resp_is_valid = false;

    bool m_miss_but_inflight_mshr_req_is_valid = false;
    bool m_allocate_line_failed_in_mshr_resp = false;
    bool m_allocate_line_failed_in_exclusive_write = false;

    // <> upper cache
    // mem
    sparta::DataInPort<MemReq> m_upper_req_in{&unit_port_set_, "upper_req_in", 1};
    sparta::DataOutPort<uint32_t> m_upper_req_credit_out{&unit_port_set_, "upper_req_credit_out", 1};
    sparta::DataOutPort<MemResp> m_upper_resp_out{&unit_port_set_, "upper_resp_out", 1};
    sparta::DataInPort<uint32_t> m_upper_resp_credit_in{&unit_port_set_, "upper_resp_credit_in", 1};
    uint32_t m_upper_resp_credit;
    // snoop
    sparta::DataOutPort<SnoopReq> m_upper_snoop_req_out{&unit_port_set_, "upper_snoop_req_out", 1};
    sparta::DataInPort<uint32_t> m_upper_snoop_req_credit_in{&unit_port_set_, "upper_snoop_req_credit_in", 1};
    uint32_t m_upper_snoop_req_credit;
    sparta::DataInPort<SnoopResp> m_upper_snoop_resp_in{&unit_port_set_, "upper_snoop_resp_in", 1};
    sparta::DataOutPort<uint32_t> m_upper_snoop_resp_credit_out{&unit_port_set_, "upper_snoop_resp_credit_out", 1};

    // <> lower cache
    // mem
    sparta::DataOutPort<MemReq> m_lower_req_out{&unit_port_set_, "lower_req_out", 1};
    sparta::DataInPort<uint32_t> m_lower_req_credit_in{&unit_port_set_, "lower_req_credit_in", 1};
    uint32_t m_lower_req_credit;
    sparta::DataInPort<MemResp> m_lower_resp_in{&unit_port_set_, "lower_resp_in", 1};
    sparta::DataOutPort<uint32_t> m_lower_resp_credit_out{&unit_port_set_, "lower_resp_credit_out", 1};
    // snoop
    sparta::DataInPort<SnoopReq> m_lower_snoop_req_in{&unit_port_set_, "snoop_req_in", 1};
    sparta::DataOutPort<uint32_t> m_lower_snoop_req_credit_out{&unit_port_set_, "snoop_req_credit_out", 1};
    sparta::DataOutPort<SnoopResp> m_lower_snoop_resp_out{&unit_port_set_, "snoop_resp_out", 1};
    sparta::DataInPort<uint32_t> m_lower_snoop_resp_credit_in{&unit_port_set_, "snoop_resp_credit_in", 1};
    uint32_t m_lower_snoop_resp_credit;

    auto m_upperRespCreditInCB(const uint32_t& credit) -> void;
    auto m_upperSnoopReqCreditInCB(const uint32_t& credit) -> void;
    auto m_lowerReqCreditInCB(const uint32_t& credit) -> void;
    auto m_snoopRespCreditInCB(const uint32_t& credit) -> void;
    auto m_initCB() -> void;

    inline auto m_missReplacement() -> CacheLine*;
    auto m_handleHit() -> void;
    auto m_genMemRespAndSend() -> void;
    auto m_genMSHRReqAndSend() -> void;
    auto m_genEvictReqAndSend() -> void;
    auto m_allocateLine() -> CacheLine*;
    auto m_forwardResp() -> void;
    auto m_handleMSHRRespAndSend() -> void;
}; // class SimpleCache

} // namespace uncore
} // namespace archXplore
#endif // __SIMPLE_CACHE_HPP__