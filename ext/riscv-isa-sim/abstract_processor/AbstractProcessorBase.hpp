#ifndef __ABSTRACT_PROCESSOR_BASE__
#define __ABSTRACT_PROCESSOR_BASE__
#include "./util.hpp"
#include <memory>

#include "./memif.hpp"
#include "./InstrBase.hpp"
// #include <stdfloat>

namespace archXplore{
class AbstractProcessorBase: public MemIf::CoreIf{
public:
    using UncorePtr = std::shared_ptr<MemIf::UncoreIf>;
    AbstractProcessorBase(UncorePtr p): _uncore(p){}
    // mem if
    virtual void sendMemReq(MemReq::PtrType) = 0;
    virtual void recieveMemResp(MemResp::PtrType) = 0;
    virtual void accessMemNoEffect(MemReq::PtrType, MemResp::PtrType) = 0;



    // state if
    virtual void reset_state() const = 0;

    virtual void getPc(reg_t& ret) const = 0;
    virtual void setPc(const reg_t& pc) = 0;
    
    virtual void getXpr(const size_t& idx, reg_t& ret) const = 0;
    virtual void setXpr(const size_t& idx, const reg_t& val) = 0;

    virtual void getFpr(const size_t& idx, freg_t& ret) const = 0;
    virtual void setFpr(const size_t& idx, const freg_t& val) = 0;

    virtual void getCsr(const reg_t& idx, reg_t& ret, InstrBase::PtrType instr) const = 0;
    virtual void peekCsr(const reg_t& idx, reg_t& ret) const noexcept = 0;
    virtual void setCsr(const reg_t& idx, const reg_t& val, InstrBase::PtrType instr) = 0;
    virtual void touchCsr(const reg_t& idx, reg_t& val) noexcept = 0;

    virtual void backdoorWriteMIP(const uint64_t& mask, const uint64_t& val) = 0;
    virtual void syncTimer(const uint64_t& ticks) = 0;
    virtual bool isWaitingForInterrupt() const noexcept = 0;
    virtual void cleanWaitingForInterrupt() noexcept = 0;


    // exe if
    virtual void reset()  = 0;
    virtual void checkInterrupt(InstrBase::PtrType instr)  = 0;
    virtual void decode(InstrBase::PtrType instr)  = 0;  
    virtual void updateRename(InstrBase::PtrType instr)  = 0;  
    virtual void execute(InstrBase::PtrType instr)  = 0;
    virtual void fetch(InstrBase::PtrType instr)  = 0;
    virtual void ptw(InstrBase::PtrType instr)  = 0;
    virtual void pmpCheck(InstrBase::PtrType instr)  = 0;
    virtual void checkPermission(InstrBase::PtrType instr)  = 0;
    virtual void addrGenForMem(InstrBase::PtrType instr)  = 0;
    virtual void getStoreData(InstrBase::PtrType instr)  = 0;
    virtual void amo(InstrBase::PtrType instr)  = 0;
    virtual void load(InstrBase::PtrType instr)  = 0;
    virtual void store(InstrBase::PtrType instr)  = 0;
    virtual void handleInterrupts(InstrBase::PtrType instr)  = 0;
    virtual void handleExceptions(InstrBase::PtrType instr)  = 0;
    virtual void commit(InstrBase::PtrType instr)  = 0;
protected:
    UncorePtr _uncore;
};
}


#endif // __ABSTRACT_PROCESSOR_BASE__