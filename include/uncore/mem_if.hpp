#ifndef __MEM_IF_HPP__
#define __MEM_IF_HPP__
#include "mem_pkt.hpp"
namespace archXplore{
class MemIf{
public:
    virtual auto sendMemReq(const MemReq& req) -> void = 0;
    virtual auto receiveMemResp(const MemResp& resp) -> void = 0;
    virtual auto sendMemResp(const MemResp& resp) -> void = 0;
    virtual auto receiveMemReq(const MemReq& req) -> void = 0;

    virtual auto sendSnoopReq(const SnoopReq& req) -> void = 0;
    virtual auto receiveSnoopResp(const SnoopResp& resp) -> void = 0;
    virtual auto sendSnoopResp(const SnoopResp& resp) -> void = 0;
    virtual auto receiveSnoopReq(const SnoopReq& req) -> void = 0;
}; // class MemIf

} // namespace archXplore
#endif // __MEM_IF_HPP__