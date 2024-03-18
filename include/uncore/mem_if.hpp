#ifndef __MEM_IF_HPP__
#define __MEM_IF_HPP__
#include "mem_pkt.hpp"
namespace archXplore{
namespace cache{

class MemIf{
public:
    virtual auto sendMemReq(MemReqPtr req) -> void = 0;
    virtual auto receiveMemResp(MemRespPtr resp) -> void = 0;
    
    virtual auto sendMemResp(MemRespPtr resp) -> void = 0;
    virtual auto receiveMemReq(MemReqPtr req) -> void = 0;
}; // class MemIf

} // namespace cache
} // namespace archXplore
#endif // __MEM_IF_HPP__