#include "uncore/cache/simple_cache.hpp"
namespace archXplore{
namespace uncore{
SimpleCache::SimpleCache(sparta::TreeNode * n, const SimpleCacheParameterSet * p):
    sparta::Unit(n),
    m_size(p->size),
    m_num_ways(p->num_ways),
    m_line_size(p->line_size),
    m_num_sets(p->size / (p->line_size * p->num_ways)), 
    m_is_cache_inclusive(p->is_cache_inclusive),
    m_is_first_level_cache(p->is_first_level_cache),
    m_is_last_level_cache(p->is_last_level_cache),
    m_is_data_cache(p->is_data_cache),
    m_is_write_allocate(p->is_write_allocate),
    m_is_write_back(p->is_write_back),
    m_agu(new DefaultAGU(p->size, p->line_size, p->line_size, p->num_ways)),
    m_replacement_policy(new TreePLRU(p->num_ways))
{
    CacheLine default_line(0, 0, p->line_size);
    m_cache.reset(new BasicCacheBlock<CacheLine>(m_num_sets, p->num_ways, default_line, m_agu.get(), m_replacement_policy.get()));
    m_upper_req_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(SimpleCache, receiveMemReq, MemReq));
    m_upper_resp_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(SimpleCache, m_upperRespCreditInCB, uint32_t));
    m_upper_snoop_req_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(SimpleCache, m_upperSnoopReqCreditInCB, uint32_t));
    m_upper_snoop_resp_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(SimpleCache, receiveSnoopResp, SnoopResp));
    m_lower_req_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(SimpleCache, m_lowerReqCreditInCB, uint32_t));
    m_lower_resp_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(SimpleCache, receiveMemResp, MemResp));
    m_lower_snoop_req_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(SimpleCache, receiveSnoopReq, SnoopReq));
    m_lower_snoop_resp_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(SimpleCache, m_snoopRespCreditInCB, uint32_t));
    sparta::StartupEvent(n, CREATE_SPARTA_HANDLER(SimpleCache, m_initCB));
}
auto SimpleCache::sendMemReq(const MemReq& req) -> void{
    // there are many cases to handle, they will be handled outside this function
    m_lower_req_credit --; // consume one credit
    m_lower_req_out.send(req);
}
auto SimpleCache::receiveMemResp(const MemResp& resp) -> void{
    // cache only recieve MemResp for mshr req
    if(resp.isWrite()){ // evict reqs do not need response
        auto it = m_evict_map.find(resp.timestamp);
        sparta_assert(it != m_evict_map.end(), "cannot find evict req with timestamp " );
        m_evict_map.erase(it);
        m_lower_resp_credit_out.send(1);
        return;
    }
    m_mshr_resp = resp;
    m_mshr_resp_valid = true;
    if(!m_is_cache_inclusive && !m_is_first_level_cache){
        // exclusive cache and not first level, forward resp to upper level
        if(m_inflight_resp_valid){
            m_exclusive_forwarding_but_inflight_resp_is_valid = true;
            return;
        }
        m_forwardResp();
        return;
    }
    // inclusive && first level exclusive cache allocate a cacheline
    CacheLine* line = m_allocateLine();
    if(line == nullptr){
        m_allocate_line_failed_in_mshr_resp = true;
        return;
    }
    m_handleMSHRRespAndSend();
}
auto SimpleCache::sendMemResp(const MemResp& resp) -> void{
    // every time a response is sent, a inflight request is done;
    m_upper_resp_credit --; // consume one credit
    m_inflight_req_valid = false; // req is done
    m_inflight_resp_valid = false; // resp is invalid
    m_upper_resp_out.send(resp);
    m_upper_req_credit_out.send(1); // ready for next req
}
auto SimpleCache::receiveMemReq(const MemReq& req) -> void{
    // save it to register
    sparta_assert(m_inflight_req_valid == false, "There is already an inflight request");
    debug_logger_ << "receiveMemReq: " << req << std::endl;
    m_inflight_req = req;
    m_inflight_req_valid = true;
    // exclusive/non-first-level cache write line immediately
    if(!m_is_cache_inclusive && !m_is_first_level_cache && m_inflight_req.isWrite()){
        CacheLine* line = m_allocateLine();
        if(line == nullptr){
            m_allocate_line_failed_in_exclusive_write = true;
            return;
        }
        line->write(m_agu->calcLineOffset(m_inflight_req.pa), m_inflight_req.payload.size, m_inflight_req.payload.data);
        m_handleHit();
        return;
    }
    // check if hit;
    bool hit = false;
    if(m_inflight_req.isRead()){
        hit = m_cache->read(m_inflight_req.pa, m_inflight_req.payload.size, m_inflight_req.payload.data);
    }
    else if(m_inflight_req.isWrite()){
        hit = m_cache->write(m_inflight_req.pa, m_inflight_req.payload.size, m_inflight_req.payload.data);
    }
    // if hit, send response
    if(hit){
        m_handleHit();
    }
    // if miss, send a mshr req
    if(m_mshr_req_valid){ // if there is a mshr req, wait for it to be done
        m_miss_but_inflight_mshr_req_is_valid = true;
        return;
    }
    m_genMSHRReqAndSend();
    return;
}

auto SimpleCache::sendSnoopReq(const SnoopReq& req) -> void{

}
auto SimpleCache::receiveSnoopResp(const SnoopResp& resp) -> void{

}
auto SimpleCache::sendSnoopResp(const SnoopResp& resp) -> void{

}
auto SimpleCache::receiveSnoopReq(const SnoopReq& req) -> void{

}
auto SimpleCache::m_upperRespCreditInCB(const uint32_t& credit) -> void{
    m_upper_resp_credit += credit;
    if(m_upper_resp_credit > 0){
        // priority: forwarding > normal
        if(m_inflight_resp_valid){ // send inflight resp first
            sendMemResp(m_inflight_resp);
        }
        else if(m_exclusive_forwarding_but_inflight_resp_is_valid){
            m_exclusive_forwarding_but_inflight_resp_is_valid = false;
            m_forwardResp();
        }
        else if(m_hit_but_inflight_resp_is_valid){
            m_hit_but_inflight_resp_is_valid = false;
            m_genMemRespAndSend();
        } 
    }
    
}
auto SimpleCache::m_upperSnoopReqCreditInCB(const uint32_t& credit) -> void{

}
auto SimpleCache::m_lowerReqCreditInCB(const uint32_t& credit) -> void{
    // priority: mshr req > evict req
    m_lower_req_credit += credit;
    if(m_lower_req_credit > 0){
        if(m_evict_req_valid){
            sendMemReq(m_evict_req);
            m_evict_req_valid = false;
        }
        else if(m_allocate_line_failed_in_exclusive_write){
            m_allocate_line_failed_in_exclusive_write = false;
            CacheLine* line = m_allocateLine();
            line->write(m_agu->calcLineOffset(m_inflight_req.pa), m_inflight_req.payload.size, m_inflight_req.payload.data);
            m_handleHit();
        }
        else if(m_allocate_line_failed_in_mshr_resp){
            m_allocate_line_failed_in_mshr_resp = false;
            CacheLine* line = m_allocateLine();
            m_handleMSHRRespAndSend();
        }
        else if(m_mshr_req_valid){
            sendMemReq(m_mshr_req);
        }
        else if(m_miss_but_inflight_mshr_req_is_valid){
            m_miss_but_inflight_mshr_req_is_valid = false;
            m_genMSHRReqAndSend();
        }
    }
}
auto SimpleCache::m_snoopRespCreditInCB(const uint32_t& credit) -> void{

}
auto SimpleCache::m_initCB() -> void{
    m_upper_req_credit_out.send(1);
    m_upper_snoop_resp_credit_out.send(1);
    m_lower_resp_credit_out.send(1);
    m_lower_snoop_req_credit_out.send(1);
}

inline auto SimpleCache::m_missReplacement() -> CacheLine*{
    CacheLine* line = m_cache->getLRULine(m_inflight_req.pa);
    return line;
}
auto SimpleCache::m_handleHit()->void{
    if(m_inflight_resp_valid){
        m_hit_but_inflight_resp_is_valid = true;
        return;
    }
    m_genMemRespAndSend();
    return;
}
auto SimpleCache::m_genMemRespAndSend() -> void{
    // gen response
    m_inflight_resp_valid = true;
    m_inflight_resp = MemResp(m_inflight_req);
    // send response if credit is available
    if(m_upper_resp_credit > 0){
        sendMemResp(m_inflight_resp);
    }
}
auto SimpleCache::m_genMSHRReqAndSend() -> void{
    m_mshr_req_valid = true;
    // gen mshr req
    m_mshr_data.reset(new uint8_t[m_line_size]);
    m_mshr_req = MemReq(
        m_inflight_req.cpuid,
        m_inflight_req.threadId,
        getClock()->currentCycle(),
        m_inflight_req.pa,
        m_inflight_req.va,
        m_inflight_req.pc,
        MemReq::READ,
        Payload{m_line_size, m_mshr_data.get()}
    );
    // send mshr req
    if(m_lower_req_credit > 0){
        sendMemReq(m_mshr_req);
    }
    return;
}
auto SimpleCache::m_genEvictReqAndSend() -> void{
    m_evict_req_valid = true;
    uint64_t timestamp = getClock()->currentCycle();
    auto pair = m_evict_map.insert(std::make_pair(timestamp, std::unique_ptr<uint8_t[]>(new uint8_t[m_line_size])));
    m_evict_line->read(0, m_line_size, pair.first->second.get());
    m_evict_req = MemReq(
        m_inflight_req.cpuid,
        m_inflight_req.threadId,
        getClock()->currentCycle(),
        m_evict_line->getAddr(),
        m_evict_line->getAddr(),
        0,
        MemReq::WRITE,
        Payload{m_line_size, pair.first->second.get()}
    );
    if(m_lower_req_credit > 0){
        sendMemReq(m_evict_req);
        m_evict_req_valid = false;
    }
    return;
}

auto SimpleCache::m_allocateLine() -> CacheLine*{
    m_evict_line = m_missReplacement();
    if(m_evict_line->isDirty()){ // line is dirty, write back
        if(m_evict_req_valid){ // evict req is valid, wait for it to be done
            return nullptr;
        }
        m_genEvictReqAndSend();
    }
    // set tag, addr, and data
    m_evict_line->setTag(m_agu->calcTag(m_inflight_req.pa));
    m_evict_line->setAddr(m_agu->calcLineAddr(m_inflight_req.pa));
    m_evict_line->setModified();
    return m_evict_line;
}
auto SimpleCache::m_forwardResp() -> void{
    m_inflight_resp = MemResp(m_inflight_req); // allocate the same payload buffer as inflight req
    memcpy(m_inflight_resp.payload.data, m_mshr_resp.payload.data, m_line_size);
    m_mshr_req_valid = false; // mshr req is done after passing the data        
    m_mshr_resp_valid = false;
    m_lower_resp_credit_out.send(1);
    m_inflight_resp_valid = true;
    if(m_upper_resp_credit > 0){
        sendMemResp(m_inflight_resp);
    }
    return;
}
auto SimpleCache::m_handleMSHRRespAndSend() -> void{
    m_evict_line->write(0, m_line_size, m_mshr_resp.payload.data);
    m_mshr_req_valid = false; 
    m_mshr_resp_valid = false;
    m_lower_resp_credit_out.send(1);// mshr req is done
    if(m_inflight_req.isRead()){
        m_evict_line->read(m_agu->calcLineOffset(m_inflight_req.pa), m_inflight_req.payload.size, m_inflight_req.payload.data);
    }
    else if(m_inflight_req.isWrite()){
        m_evict_line->write(m_agu->calcLineOffset(m_inflight_req.pa), m_inflight_req.payload.size, m_inflight_req.payload.data);
    }
    m_handleHit();
}

} // namespace uncore
} // namespace archXplore