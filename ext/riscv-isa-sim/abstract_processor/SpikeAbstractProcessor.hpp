#ifndef __SPIKE_ABSTRACT_PROCESSOR_HPP__
#define __SPIKE_ABSTRACT_PROCESSOR_HPP__
#include "AbstractProcessorBase.hpp"
#include "fake_regfile.hpp"
// #include "spike_instr.hpp"
#include "../riscv/simif.h"
#include "../riscv/processor.h"
#include "../riscv/mmu.h"
#include "../riscv/devices.h"
#include "spike_cfg.hpp"
#include <memory>
#include <cmath>
#include "../riscv/decode_macros.h"
#include <functional>
namespace archXplore{
static const size_t INTERLEAVE = 5000;
static const size_t INSNS_PER_RTC_TICK = 100; // 10 MHz clock for 1 BIPS core
class SpikeAbstractProcessor;
class SpikeInstr: public RVInstrBase{
public:
    friend class SpikeAbstractProcessor;
    using PtrType = std::shared_ptr<SpikeInstr>;
    // exception 
    using InstrBase::evalid;
    using InstrBase::ecause;
    using TrapPtr = std::unique_ptr<trap_t>;
    TrapPtr trap;
    // exec result
    using InstrBase::npc;

    insn_t instr;   // spike insn. for mapping
    insn_func_t instr_func; // spike insn function. For execution

    // fetch
    using InstrBase::pc; // pc
    reg_t pc_page; // pc page
    reg_t pc_paddr; // pc paddr
    reg_t fetch_len = 32/8; 
    bool isFetched = false;

    // load/store
    reg_t mem_addr = 0; //memory operation vaddr
    reg_t mem_page = 0; //memory operation paddr
    reg_t mem_paddr = 0;
    uint8_t mem_data[8];


    // cfg
    // reg_t page_size = 0;

    SpikeInstr(const reg_t& pc, SpikeAbstractProcessor* ptr){
        _processor = ptr;
        this->pc = pc;
        instr = insn_t(0);
        memset(&mem_data, 0, sizeof(mem_data));
    }
    void setAllPrgeAsAreg(){
        _prd = rd();
        _prs1 = rs1();
        _prs2 = rs2();
        _prs3 = rs3();
    }
    virtual uint64_t prd() { return _prd; }
    virtual uint64_t prs1() { return _prs1; }
    virtual uint64_t prs2() { return _prs2; }
    virtual uint64_t prs3() { return _prs3; }
    virtual void setPrd(const uint64_t& prd)   {_prd = prd; }
    virtual void setPrs1(const uint64_t& prs1) {_prs1 = prs1; }
    virtual void setPrs2(const uint64_t& prs2) {_prs2 = prs2; }
    virtual void setPrs3(const uint64_t& prs3) {_prs3 = prs3; }
    virtual reg_t getXRegVal(const reg_t& reg);
    virtual freg_t getFRegVal(const reg_t& reg);
    virtual void setXRegVal(const reg_t& reg, const reg_t& val);
    virtual void setFRegVal(const reg_t& reg, const freg_t& val);


    static SpikeInstr::PtrType getSpikeInstrFromInsn(const insn_t& ins){
        auto it = _instrMap.find((reg_t)&ins);
        if(it == _instrMap.end())
            throw "getSpikeInstrFromInsn:ins not in the map";
        return it->second;
    }
    static void commitInstr(SpikeInstr::PtrType ptr){
        _delete_insn_from_map(ptr->instr);
    }
    
private:
    SpikeAbstractProcessor* _processor;
    reg_t _prd = 0;
    reg_t _prs1 = 0;
    reg_t _prs2 = 0;
    reg_t _prs3 = 0;

    static void _add_map(const insn_t& insn, SpikeInstr::PtrType timingPtr){
        if(_instrMap.find((reg_t)&insn) != _instrMap.end())
            throw "already exists in map";
        _instrMap[(reg_t)&insn] = timingPtr;
    }
    static void _delete_insn_from_map(const insn_t& insn){
        auto it = _instrMap.find((reg_t)&insn);
        if(it == _instrMap.end())
            throw "delete do not find the insn in map";
        _instrMap.erase(it);
    }
    static std::unordered_map<reg_t, SpikeInstr::PtrType> _instrMap;
};


class SpikeAbstractProcessor: public AbstractProcessorBase, public simif_t{
private:
    std::shared_ptr<processor_t> _processor;
    std::map<size_t, processor_t*> _harts;
    using DevicePtr = std::shared_ptr<abstract_device_t>;
    std::vector<DevicePtr> _devices;
    reg_t _current_step = 0;
    std::shared_ptr<FakeRegFile<reg_t>> _xprf;
    std::shared_ptr<FakeRegFile<freg_t>> _fprf;
public:
    using AbstractProcessorBase::UncorePtr;
    reg_t page_size;
    reg_t page_shift;
    bool trace_enable;
    SpikeAbstractProcessor(const isa_parser_t *isa, const SpikeCfg *cfg,
                            uint32_t id, bool halt_on_reset,
                         FILE* log_file, std::ostream& sout_,
                         UncorePtr uncore,
                         std::shared_ptr<FakeRegFile<reg_t>> xprf,
                         std::shared_ptr<FakeRegFile<freg_t>> fprf,
                         bool trace_on
                         ):
                         _processor(std::make_shared<processor_t>(isa, cfg, this, id, halt_on_reset, log_file, sout_)),
                         AbstractProcessorBase(uncore),
                         _xprf(xprf),
                         _fprf(fprf),
                         page_size(cfg->page_size),
                         trace_enable(trace_on)
    {
        _harts[0] = _processor.get();
        debug_mmu = new mmu_t(this, cfg->endianness, nullptr);
        _devices.push_back(std::make_shared<clint_t>(
            this, // simif_t
            0, // freq_hz not important when realtime is false
            false // realtime
        ));
        page_shift = (reg_t) std::log2(page_size);
        
    }
    ~SpikeAbstractProcessor(){
        delete debug_mmu;
    }
    void setSerialized(const bool& flag){_processor->state.serialized = flag;}
    void advancePc(archXplore::SpikeInstr::PtrType ptr){
        if (unlikely(ptr->npc & 1 == 1)) { 
            // std::cout << "problem npc: " << ptr->npc << std::endl;
            switch (ptr->npc) { 
                case 3: _processor->state.serialized = true;break; // PC_SERIALIZE_BEFORE
                case 5: break;  //PC_SERIALIZE_AFTER
                default: abort(); 
            } 
            ptr->npc = _processor->state.pc;
            // break; 
        } else { 
            _processor->state.pc = ptr->npc; 
        }
    }
    void tick(reg_t time){
        for(auto device : _devices)
            device->tick(time);
    }
    bool step();
    void run( const reg_t& interval);
    std::shared_ptr<SpikeInstr> createInstr(const reg_t & pc){
        auto ptr = std::make_shared<SpikeInstr>(pc, this);
        SpikeInstr::_add_map(ptr->instr, ptr);
        if(trace_enable)
            printf("pc:%016llx\t", pc);
        return ptr;
    }
    void retireInstr(InstrBase::PtrType instr){
        auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
        SpikeInstr::commitInstr(ptr);
        if(trace_enable)
            printf("\n");
    }
    // * from AbstractProcessorBase
    virtual void sendMemReq(MemReq::PtrType)  override final;
    virtual void recieveMemResp(MemResp::PtrType)  override final;
    virtual void accessMemNoEffect(MemReq::PtrType, MemResp::PtrType)  override final;

    virtual void get_clint(const reg_t& idx, reg_t& ret) const  override final;
    virtual void set_clint(const reg_t& idx, const reg_t& val)  override final;


    // state if
    virtual void reset_state() const override final;

    virtual void getPc(reg_t& ret) const override final;
    virtual void setPc(const reg_t& pc) override final;
    
    virtual void getXpr(const size_t& idx, reg_t& ret) const  override final;
    virtual void setXpr(const size_t& idx, const reg_t& val)  override final;

    virtual void getFpr(const size_t& idx, freg_t& ret) const  override final;
    virtual void setFpr(const size_t& idx, const freg_t& val)  override final;

    virtual void getCsr(const reg_t& idx, reg_t& ret, InstrBase::PtrType instr) const  override final;
    virtual void peekCsr(const reg_t& idx, reg_t& ret) const noexcept  override final;
    virtual void setCsr(const reg_t& idx, const reg_t& val, InstrBase::PtrType instr)  override final;
    virtual void touchCsr(const reg_t& idx, reg_t& val) noexcept  override final;

    virtual void backdoorWriteMIP(const uint64_t& mask, const uint64_t& val) override final;
    virtual void syncTimer(const uint64_t& ticks) override final;
    virtual bool isWaitingForInterrupt() const noexcept override final;
    virtual void cleanWaitingForInterrupt() noexcept override final;

    // exe if
    virtual void reset() override final;
    virtual void checkInterrupt(InstrBase::PtrType instr) override final;  
    virtual void decode(InstrBase::PtrType instr) override final;  
    virtual void updateRename(InstrBase::PtrType instr) override final;  
    virtual void execute(InstrBase::PtrType instr)  override final; 
    virtual void checkPermission(InstrBase::PtrType instr) override final; 
    virtual void addrGenForMem(InstrBase::PtrType instr) override final;
    virtual void getStoreData(InstrBase::PtrType instr) override final;
    virtual void ptw(InstrBase::PtrType instr) override final;
    virtual void pmpCheck(InstrBase::PtrType instr) override final;
    // TODO: actually there are pmo checks in ptw, it is implicitly
    virtual void fetch(InstrBase::PtrType instr) override final;
    using amoFunc = std::function<uint64_t(const uint64_t&, const uint64_t&)>;
    void amo(InstrBase::PtrType instr, amoFunc funct);
    virtual void amo(InstrBase::PtrType instr) override final;
    virtual void load(InstrBase::PtrType instr) override final;
    virtual void store(InstrBase::PtrType instr) override final;
    virtual void handleInterrupts(InstrBase::PtrType instr) override final;
    virtual void handleExceptions(InstrBase::PtrType instr) override final;
    virtual void commit(InstrBase::PtrType instr) override final;

    // simif.h
    virtual char* addr_to_mem(reg_t paddr) override final;
    virtual bool reservable(reg_t paddr) { return addr_to_mem(paddr); }
    // used for MMIO addresses
    virtual bool mmio_fetch(reg_t paddr, size_t len, uint8_t* bytes) { return mmio_load(paddr, len, bytes); }
    virtual bool mmio_load(reg_t paddr, size_t len, uint8_t* bytes) override final;
    virtual bool mmio_store(reg_t paddr, size_t len, const uint8_t* bytes) override final;
    // Callback for processors to let the simulation know they were reset.
    virtual void proc_reset(unsigned id) override final;

    virtual const cfg_t &get_cfg() const override final;
    virtual const std::map<size_t, processor_t*>& get_harts() const override final;

    virtual const char* get_symbol(uint64_t paddr) override final;         
protected:
    using AbstractProcessorBase::_uncore;
};
};
#endif // __SPIKE_ABSTRACT_PROCESSOR_HPP__