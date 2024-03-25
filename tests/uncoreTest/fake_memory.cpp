#include "fake_memory.hpp"
#include "python/EmbeddedModule.hpp"
namespace archXplore{
namespace uncore{
FakeMemory::FakeMemory(sparta::TreeNode * n, const FakeMemoryParameterSet * p):
    sparta::Unit(n),
    m_base_addr(p->base_addr),
    m_size(p->size),
    m_data(new uint8_t[p->size])
{
    m_upper_req_in.registerConsumerHandler(CREATE_SPARTA_HANDLER_WITH_DATA(FakeMemory, receiveMemReq, MemReq));
    memset(m_data.get(), 0, p->size);
}
auto FakeMemory::sendMemReq(const MemReq& req) -> void {
    assert(0); // Not implemented
}
auto FakeMemory::receiveMemResp(const MemResp& resp) -> void {
    assert(0); // Not implemented
}
auto FakeMemory::sendMemResp(const MemResp& resp) -> void {
    assert(0); // Not implemented
}
auto FakeMemory::receiveMemReq(const MemReq& req) -> void {
    debug_logger_ << "Received memory request: " << req << std::endl;
    sparta_assert(0 <= req.pa - m_base_addr && req.pa - m_base_addr+ req.payload.size < m_size, "Invalid memory access");
    if(req.isRead()){
        memcpy(req.payload.data, m_data.get() + (req.pa - m_base_addr), req.payload.size);
    }
    else if(req.isWrite()){
        memcpy(m_data.get() + (req.pa - m_base_addr), req.payload.data, req.payload.size);
    }
    return;
}
auto FakeMemory::sendSnoopReq(const SnoopReq& req) -> void {
    assert(0); // Not implemented
}
auto FakeMemory::receiveSnoopResp(const SnoopResp& resp) -> void {
    assert(0); // Not implemented
}
auto FakeMemory::sendSnoopResp(const SnoopResp& resp) -> void {
    assert(0); // Not implemented
}
auto FakeMemory::receiveSnoopReq(const SnoopReq& req) -> void {
    assert(0); // Not implemented
}
REGISTER_SPARTA_UNIT(FakeMemory, FakeMemoryParameterSet);
} // namespace uncore
} // namespace archXplore