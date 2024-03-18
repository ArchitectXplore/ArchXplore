#ifndef __MEMIF_HPP__
#define __MEMIF_HPP__
#include "./util.hpp"
#include "./MemPkt.hpp"
#include <memory>

namespace archXplore{

class MemIf{
public:
    class UncoreIf;
    class CoreIf;

    class CoreIf{
    public:
        virtual void sendMemReq(MemReq::PtrType) = 0;
        virtual void recieveMemResp(MemResp::PtrType) = 0;
        virtual void accessMemNoEffect(MemReq::PtrType, MemResp::PtrType) = 0;

        // TODO: clint
        virtual void get_clint(const reg_t& idx, reg_t& ret) const = 0;
        virtual void set_clint(const reg_t& idx, const reg_t& pc) = 0;
    };
    class UncoreIf{
    public:
        virtual void sendMemResp(MemResp::PtrType) = 0;
        virtual void recieveMemReq(MemReq::PtrType) = 0;
        virtual void accessMemNoEffect(MemReq::PtrType, MemResp::PtrType) = 0;
    };
    
};


}
#endif //__MEMIF_HPP__
