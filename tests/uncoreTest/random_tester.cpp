#include "random_tester.hpp"
#include "python/EmbeddedModule.hpp"
namespace archXplore{
namespace uncore{
RandomTester::RandomTester(sparta::TreeNode * n, const RandomTesterParameterSet * p) :
    sparta::Unit(n),
    m_base_addr(p->base_addr),
    m_size(p->size),
    m_line_size(p->line_size),      
    m_line_mask(p->line_mask),
    m_cpuid(p->cpuid),
    m_num_iters(p->num_iters),
    m_seed(p->seed),
    m_req_stride(p->req_stride),
    m_req_queue_size(p->req_queue_size),
    m_req_gen_prob(p->req_gen_prob)

{
    m_lower_req_credit_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(RandomTester, m_lowerReqCreditInCB, uint32_t));
    m_lower_resp_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(RandomTester, sendMemResp, MemResp));
    sparta::StartupEvent(n, CREATE_SPARTA_HANDLER(RandomTester, m_initCB));
}
auto RandomTester::sendMemReq(const MemReq& req) -> void {
    m_lower_req_out.send(req);
    m_lower_req_credit --;
}
auto RandomTester::receiveMemResp(const MemResp& resp) -> void {
    info_logger_ << "Received resp: " << resp << std::endl;
    // try to send a new req
    bool try_send_req = false;
    if(m_req_queue.size() == m_req_queue_size) // sendReq is stucked by queue full
        try_send_req = true;

    for(auto it = m_req_queue.begin(); it != m_req_queue.end(); it++){
        if(it->timestamp != resp.timestamp) continue;
        it->flag.set(MemReq::INVALID);
    }
    // deque
    for(auto it = m_req_queue.begin(); it != m_req_queue.end();){
        if(it->flag != MemReq::INVALID){
            break;
        }
        auto map_it = m_mem_map.find(it->timestamp);
        m_mem_map.erase(map_it);
        m_req_queue.erase(it); // automatically move back
    }
    if(try_send_req)
        m_sendReqCB(); // try send req if possible
}
auto RandomTester::sendMemResp(const MemResp& resp) -> void {
    assert(0); // Not implemented
}
auto RandomTester::receiveMemReq(const MemReq& req) -> void {
    assert(0); // Not implemented
}
auto RandomTester::sendSnoopReq(const SnoopReq& req) -> void {
    assert(0); // Not implemented
}
auto RandomTester::receiveSnoopResp(const SnoopResp& resp) -> void {
    assert(0); // Not implemented
}
auto RandomTester::sendSnoopResp(const SnoopResp& resp) -> void {
    assert(0); // Not implemented
}
auto RandomTester::receiveSnoopReq(const SnoopReq& req) -> void {
    assert(0); // Not implemented
}
auto RandomTester::m_lowerReqCreditInCB(const uint32_t & credit) -> void{
    bool try_send_req = false;
    if(m_lower_req_credit == 0) // sendReq is stucked by no credit
        try_send_req = true;
    m_lower_req_credit += credit;
    if (try_send_req)
        m_sendReqCB(); 
}
auto RandomTester::m_initCB() -> void{
    m_lower_resp_credit_out.send(1);
}
inline auto RandomTester::m_genRandReq() -> void{
    uint64_t addr = m_base_addr + (uint64_t)((rand() % m_size) & m_line_mask);
    uint32_t size = rand() % m_line_size + 1;
    addr += rand() % (m_line_size - size); // align
    uint64_t timestamp = getClock()->currentCycle();
    m_mem_map[timestamp].reset(new uint8_t[size]);
    m_req_queue.push_back(MemReq{
        m_cpuid,
        0, // threadid
        timestamp,
        addr, 
        addr,
        0, 
        rand() % 2 == 0 ? MemReq::READ : MemReq::WRITE,
        size,
        m_mem_map[timestamp].get()
    });
}
auto RandomTester::m_sendReqCB() -> void{
    if(m_num_iters == 0) return;
    if(m_req_queue.size() < m_req_queue_size && m_lower_req_credit > 0){
        if(rand() % 100 < m_req_gen_prob){
            m_num_iters --;
            m_genRandReq();
            sendMemReq(m_req_queue.front());
            info_logger_ << "Send req: " << m_req_queue.front() << std::endl;
        }
        else{
            m_send_req_event.schedule(rand() % m_req_stride + 1);
        }
    }
    // else schedule again when recieving lower credit
}
REGISTER_SPARTA_UNIT(RandomTester, RandomTesterParameterSet);

} // namespace uncore
} // namespace archXplore
