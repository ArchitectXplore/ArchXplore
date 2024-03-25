#include "uncore/memory/dramsim3.hpp"
#include "python/EmbeddedModule.hpp"
namespace archXplore{
namespace uncore{
DRAMSim3::DRAMSim3(sparta::TreeNode * n, const DRAMSim3ParameterSet * p):
    sparta::Unit(n),
    m_read_callback(std::bind(&DRAMSim3::m_readComplete, this, std::placeholders::_1)),
    m_write_callback(std::bind(&DRAMSim3::m_writeComplete, this, std::placeholders::_1)),
    m_num_ports(p->num_ports)
{
    m_wrapper.reset(new DRAMSim3Wrapper(p->config_file, p->trace_file, m_read_callback, m_write_callback));
    m_burst_size = m_wrapper->burstSize();
    m_queue_size = m_wrapper->queueSize();
    // m_upper_req_in_ports.reserve(m_num_ports);
    // m_upper_req_credit_out_ports.reserve(m_num_ports);
    // m_upper_resp_out_ports.reserve(m_num_ports);
    // m_upper_resp_credit_in_ports.reserve(m_num_ports);
    m_upper_resp_credit_vec.resize(m_num_ports, 0);

    for(int i = 0; i < m_num_ports; i++){
        // register ports
        m_upper_req_in_ports.push_back(std::unique_ptr<sparta::DataInPort<MemReq>>(new sparta::DataInPort<MemReq>{&unit_port_set_, "upper_req_in_" + std::to_string(i), 1}));
        m_upper_req_credit_out_ports.push_back(std::unique_ptr<sparta::DataOutPort<uint32_t>>(new sparta::DataOutPort<uint32_t>{&unit_port_set_, "upper_req_credit_out_" + std::to_string(i), 1}));
        m_upper_resp_out_ports.push_back(std::unique_ptr<sparta::DataOutPort<MemResp>>(new sparta::DataOutPort<MemResp>{&unit_port_set_, "upper_resp_out_" + std::to_string(i), 1}));
        m_upper_resp_credit_in_ports.push_back(std::unique_ptr<sparta::DataInPort<uint32_t>>(new sparta::DataInPort<uint32_t>{&unit_port_set_, "upper_resp_credit_in_" + std::to_string(i), 1}));
        m_upper_req_in_ports[i]->registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(DRAMSim3, receiveMemReq, MemReq));
        m_upper_resp_credit_in_ports[i]->registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(DRAMSim3, m_upperRespCreditInCB, uint32_t));
    }
    
    m_lower_resp_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(DRAMSim3, receiveMemResp, MemResp));
    
    sparta::StartupEvent(n, CREATE_SPARTA_HANDLER(DRAMSim3, m_initCB));
}

auto DRAMSim3::sendMemReq(const MemReq& req) -> void{
    // no need to implement
    assert(0);
}
auto DRAMSim3::receiveMemResp(const MemResp& resp) -> void{
    // no need to implement, data will be read to req's payload if it is a read
    // so, no resp will be sent
    assert(0);
}
auto DRAMSim3::sendMemResp(const MemResp& resp) -> void{
    debug_logger_ << "DRAMSim3: Sending response to CPU " << resp << std::endl;
    debug_logger_ << "port info: " <<  m_upper_resp_out_ports[resp.cpuid]->getNumBoundPorts() 
                    << " " << (m_upper_resp_out_ports[resp.cpuid]->isBound() ? "bound" : "unbound")
                    // << " " << m_upper_resp_out_ports[resp.cpuid]->
                    << std::endl;
        m_upper_resp_out_ports[resp.cpuid]->send(resp);
    m_upper_resp_credit_vec[resp.cpuid] --;
}
auto DRAMSim3::receiveMemReq(const MemReq& req) -> void{
    // there are 2 cases when recieving a req
    // 1. the req queue is not full. we can simply push the req to the queue
    // 2. req queue is full. we can push the req in pending queue
    // the second case will happen since we may give extra credit to the upper level to avoid deadlock
    info_logger_ << "DRAMSim3: Received a request from CPU " << req << std::endl;
    info_logger_ << "is full() " << (m_is_empty() ? "true" : "false") << std::endl;
    // sparta_assert(m_wrapper->canAccept(req.pa, req.isWrite()), "DRAMSim3: Request cannot be accepted");
    if(m_is_empty())
        m_tick_event.schedule(1); // wake up dram from idle
    // if can not accept 
    bool can_accept = m_wrapper->canAccept(req.pa, req.isWrite());
    if(!can_accept){ // if req queue is full, push it to pending queue
        m_pending_queue.push(req);
    }
    else{
         // push req into the queue
        if(req.isRead()){
            m_inflight_reads++;
            m_read_queue[req.pa].push(req);
        }
        else if(req.isWrite()){
            m_inflight_writes++;
            m_write_queue[req.pa].push(req);
        }
        // send req to DRAMSim3
        debug_logger_ << "DRAMSim3: Enqueuing request for address " << HEX16(req.pa) << std::endl;
        m_wrapper->enqueue(req.pa, req.isWrite());
    }
   
    // now if req queue is not full, send credits
    if(!m_is_full()){
        m_upper_req_credit_out_ports[req.cpuid]->send(1);
    }
    else{ // pending cpus waiting for credits when the queue is full
        m_cpu_wait_for_credit.push(req.cpuid);
    }
    return;
}
auto DRAMSim3::sendSnoopReq(const SnoopReq& req) -> void{
    assert(0);
}
auto DRAMSim3::receiveSnoopResp(const SnoopResp& resp) -> void{
    assert(0); // Not implemented
}
auto DRAMSim3::sendSnoopResp(const SnoopResp& resp) -> void{
    assert(0); // Not implemented
}
auto DRAMSim3::receiveSnoopReq(const SnoopReq& req) -> void{
    assert(0); // Not implemented
}

auto DRAMSim3::m_readComplete(uint64_t addr) -> void{
    // generate a response from the front of the queue
    debug_logger_ << "DRAMSim3: Read complete for address " << HEX16(addr) << std::endl;
    auto it = m_read_queue.find(addr);
    MemReq req = it->second.front();
    MemResp resp = MemResp(req); 
    it->second.pop();
    if(it->second.empty())
        m_read_queue.erase(it);
    m_inflight_reads--;
    m_lower_req_out.send(req); // send req to lower level for data if needed
    // send response to upper level
    // TODO: do not handle credit == 0 case
    sendMemResp(resp);

    // handle pending requests first
    if(m_pending_queue.size() != 0){
        m_handlePendingQueue();
        return;
    }
    // handle waiting cpus
    if(m_cpu_wait_for_credit.size() != 0){
        m_handleCPUWaitForCredit();
    }
}
auto DRAMSim3::m_writeComplete(uint64_t addr) -> void{
    // generate a response from the front of the queue
    debug_logger_ << "DRAMSim3: Write complete for address " << HEX16(addr) << std::endl;
    auto it = m_write_queue.find(addr);
    MemReq req = it->second.front();
    MemResp resp = MemResp(req); 
    it->second.pop();
    if(it->second.empty())
        m_write_queue.erase(it); // ? may be a burden. costly
    m_inflight_writes--;
    m_lower_req_out.send(req); // send req to lower level for data if needed
    // send response to upper level
    // TODO: do not handle credit == 0 case
    sendMemResp(resp);

    // handle pending requests first
    if(m_pending_queue.size() != 0){
        m_handlePendingQueue();
        return;
    }
    // handle waiting cpus
    if(m_cpu_wait_for_credit.size() != 0){
        m_handleCPUWaitForCredit();
    }
}
auto DRAMSim3::m_is_full() -> bool{
    return (m_queue_size - m_inflight_reads - m_inflight_writes) == 0;
} 
auto DRAMSim3::m_is_empty() -> bool{
    return m_inflight_reads == 0 && m_inflight_writes == 0;
}
inline auto DRAMSim3::m_handlePendingQueue() -> void{
    // if can not accept, will be add to queue end 
    receiveMemReq(m_pending_queue.front());
    m_pending_queue.pop();
}
inline auto DRAMSim3::m_handleCPUWaitForCredit() -> void{
    m_upper_req_credit_out_ports[m_cpu_wait_for_credit.front()]->send(1);
    m_cpu_wait_for_credit.pop();
}
auto DRAMSim3::m_initCB() -> void{
    for(int i = 0; i < m_num_ports; i++){
        m_upper_req_credit_out_ports[i]->send(1);
    }
}
auto DRAMSim3::m_upperRespCreditInCB(const uint32_t& credit) -> void{
    // TODO: do not handle credit == 0 case
}
auto DRAMSim3::m_tick() -> void{
    debug_logger_ << "DRAMSim3: tick" << std::endl;
    m_wrapper->tick();
    if(!m_is_empty())
        m_tick_event.schedule(1);
}

REGISTER_SPARTA_UNIT(DRAMSim3, DRAMSim3ParameterSet);
} // namespace uncore
} // namespace archXplore