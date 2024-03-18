#include "SpikeAbstractProcessor.hpp"
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <algorithm> 
#include "../riscv/decode.h"
#include "../riscv/decode_macros.h"
#include "../fesvr/byteorder.h"
#include "../softfloat/softfloat.h"
namespace archXplore{
std::unordered_map<reg_t, SpikeInstr::PtrType> SpikeInstr::_instrMap 
    = std::unordered_map<reg_t, SpikeInstr::PtrType>();
reg_t SpikeInstr::getXRegVal(const reg_t& reg){
    reg_t ret;
    _processor->getXpr(reg, ret);
    return ret;
}
freg_t SpikeInstr::getFRegVal(const reg_t& reg){
    freg_t ret;
    _processor->getFpr(reg, ret);
    return ret;
}
void SpikeInstr::setXRegVal(const reg_t& reg, const reg_t& val){_processor->setXpr(reg, val);}
void SpikeInstr::setFRegVal(const reg_t& reg, const freg_t& val){_processor->setFpr(reg, val);}



// * mem if
void SpikeAbstractProcessor::sendMemReq(MemReq::PtrType req) {
    throw "unimpl!";
}
void SpikeAbstractProcessor::recieveMemResp(MemResp::PtrType resp) {
    throw "unimpl!";
}
void SpikeAbstractProcessor::accessMemNoEffect(MemReq::PtrType req, MemResp::PtrType resp) {
}

void SpikeAbstractProcessor::get_clint(const reg_t& idx, reg_t& ret) const {
}
void SpikeAbstractProcessor::set_clint(const reg_t& idx, const reg_t& pc) {
}

// * state if
void SpikeAbstractProcessor::reset_state() const {
}

void SpikeAbstractProcessor::getPc(reg_t& ret) const {
    ret = _processor->state.pc;
}
void SpikeAbstractProcessor::setPc(const reg_t& pc) {
    _processor->state.pc = pc;
}

void SpikeAbstractProcessor::getXpr(const size_t& idx, reg_t& ret) const {
    ret = _xprf->get(idx);
    // std::cout << "\tget xpr " << idx << ": " << ret << std::endl;
}
void SpikeAbstractProcessor::setXpr(const size_t& idx, const reg_t& val) {
    // std::cout << "\twrite xpr " << idx << ": " << val << std::endl;
    if(trace_enable)
        printf("%016llx -> x%d", val, idx);
    _xprf->write(idx, val);
}

void SpikeAbstractProcessor::getFpr(const size_t& idx, freg_t& ret) const {
    ret = _fprf->get(idx);
    // std::cout << "\tget fpr " << idx << ": " << ret << std::endl;
}
void SpikeAbstractProcessor::setFpr(const size_t& idx, const freg_t& val) {
    if(trace_enable)
        printf("%016llf -> f%d", val, idx);
    _fprf->write(idx, val);
    // std::cout << "\twrite fpr " << idx << ": " << val << std::endl;
}

void SpikeAbstractProcessor::getCsr(const reg_t& idx, reg_t& ret, InstrBase::PtrType instr) const {
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    ret = _processor->get_csr(idx, ptr->instr, false, false);
}
void SpikeAbstractProcessor::peekCsr(const reg_t& idx, reg_t& ret) const noexcept {
    ret = _processor->get_csr(idx, insn_t(0), false, true);
}
void SpikeAbstractProcessor::setCsr(const reg_t& idx, const reg_t& val, InstrBase::PtrType instr) {
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    auto search = _processor->state.csrmap.find(idx);
    if(search == _processor->state.csrmap.end())
        throw "undefined csr idx";
    search->second->verify_permissions(ptr->instr, true);
    search->second->write(val);
}
void SpikeAbstractProcessor::touchCsr(const reg_t& idx, reg_t& val) noexcept {
    _processor->state.csrmap[idx]->write(val);
}
void SpikeAbstractProcessor::backdoorWriteMIP(const uint64_t& mask, const uint64_t& val){
    _processor->state.mip->backdoor_write_with_mask(mask, val);
}
void SpikeAbstractProcessor::syncTimer(const uint64_t& ticks){
     _processor->state.time->sync(ticks);
}
bool SpikeAbstractProcessor::isWaitingForInterrupt() const noexcept{
    return _processor->in_wfi;
}
void SpikeAbstractProcessor::cleanWaitingForInterrupt() noexcept{
    _processor->in_wfi = false;
}



// * exe if
inline void setException(SpikeInstr::PtrType ptr, trap_t& t){
    ptr->evalid = true;
    ptr->trap = t.clone();
    ptr->ecause = t.cause();
}

void SpikeAbstractProcessor::run(const reg_t& n) {
    for (size_t i = 0, steps = 0; i < n; i += steps)
  {
    steps = std::min(n - i, INTERLEAVE - _current_step);
    int count = steps;
    while(count -- > 0 && step()){
    }

    _current_step += steps;
    if (_current_step == INTERLEAVE)
    {
      _current_step = 0;
      _processor->get_mmu()->yield_load_reservation();
    reg_t rtc_ticks = INTERLEAVE / INSNS_PER_RTC_TICK;
    for (auto &dev : _devices) dev->tick(rtc_ticks);
    }
  }
}
bool SpikeAbstractProcessor::step() {
    // * gen instr
    reg_t pc;
    getPc(pc);
    auto instrPtr = createInstr(pc);

    // * handle interruption
    handleInterrupts(instrPtr);
    // * wfi 
    if(unlikely(_processor->in_wfi)){
        auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instrPtr);
        SpikeInstr::commitInstr(ptr);
        return false;
    }

    // * fetch 
    if(likely(!instrPtr->evalid)){
        ptw(instrPtr);
    }
	if(likely(!instrPtr->evalid)){
        pmpCheck(instrPtr);
    }
    // actually this stage contian decode
    if(likely(!instrPtr->evalid)){
        fetch(instrPtr);
    }
    
    // * decode 
    // actually this stage is useless in spike abstract core since fetch can carry out everything
    // still add this stage just for tese
    if(likely(!instrPtr->evalid)){
        decode(instrPtr);
    }

    // * rename
    // this is also a fake rename
    if(likely(!instrPtr->evalid)){
        instrPtr->setAllPrgeAsAreg();
        updateRename(instrPtr);
    }

    // * exe 
    if(likely(!instrPtr->evalid)){
		if(instrPtr->isLd() || instrPtr->isSt() || 
            instrPtr->isFLd() || instrPtr->isFSt() ||
            instrPtr->isAmo()
            ){
            if(likely(!instrPtr->evalid)){
                checkPermission(instrPtr);
            }
    		if(likely(!instrPtr->evalid))
				addrGenForMem(instrPtr);

			if(likely(!instrPtr->evalid) && (instrPtr->isSt() || instrPtr->isFSt() || instrPtr->isAmo()))
				getStoreData(instrPtr);

    		if(likely(!instrPtr->evalid))
				ptw(instrPtr);

    		if(likely(!instrPtr->evalid))
				pmpCheck(instrPtr);

    		if(likely(!instrPtr->evalid) && (instrPtr->isSt() || instrPtr->isFSt()))
				store(instrPtr);
    		else if(likely(!instrPtr->evalid) && (instrPtr->isLd() || instrPtr->isFLd()))
				load(instrPtr);
            else if(likely(!instrPtr->evalid) && (instrPtr->isAmo())){
                amo(instrPtr);
            }

    		if(likely(!instrPtr->evalid))
				instrPtr->npc = instrPtr->pc + 4; // ! not for c
		}
		else 
            if(likely(!instrPtr->evalid))
        	    execute(instrPtr);
    }

    // * commit 
	if(likely(!instrPtr->evalid)){
    	commit(instrPtr);
    }

    // * handle wfi
    if(_processor->in_wfi){
        if(trace_enable)
            printf("\n\tenter wfi");
    }

    // * handle exception
    // exception should be handeld after interrupt since interrupt will set up a trap
    if(!_processor->in_wfi && instrPtr->evalid){
        handleExceptions(instrPtr);
    }
    // * get next pc
    if(!_processor->in_wfi && !instrPtr->evalid){
        advancePc(instrPtr);
    }
	retireInstr(instrPtr);
    return !_processor->in_wfi;
}

void SpikeAbstractProcessor::reset(){
    _processor->reset();
}



void SpikeAbstractProcessor::checkInterrupt(InstrBase::PtrType instr){
    if(_processor->state.mip->read() & _processor->state.mie->read())
        instr->ivalid = true;
}
void SpikeAbstractProcessor::decode(InstrBase::PtrType instr) {
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    try{
        ptr->instr_func = _processor->decode_insn(ptr->getRaw());

        
        return;
    }
    catch(trap_t& t){
        setException(ptr, t);
    }
    catch (wait_for_interrupt_t &t)
    {
      _processor->in_wfi = true;
    }
    return;
}  
void SpikeAbstractProcessor::updateRename(InstrBase::PtrType instr) {
    
}  
void SpikeAbstractProcessor::execute(InstrBase::PtrType instr) {
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    try{
		ptr->npc = ptr->instr_func(_processor.get(), ptr->instr, ptr->pc);
        return;
    }
    catch(trap_t& t){
        setException(ptr, t);
    }
    catch (wait_for_interrupt_t &t)
    {
      _processor->in_wfi = true;
    }
    return; 

}
void SpikeAbstractProcessor::getStoreData(InstrBase::PtrType instr){
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    if(ptr->isFSt()){
        freg_t val;
		getFpr(ptr->rs2(), val);
		memcpy(&ptr->mem_data, (char*)&val.v[0], sizeof(ptr->mem_data));
    }
	else if(ptr->isSt() || ptr->isHsv() || ptr->isAmo()){
		reg_t val;
		getXpr(ptr->rs2(), val);
		memcpy(&ptr->mem_data, (char*)&val, sizeof(ptr->mem_data));
	}
}
void SpikeAbstractProcessor::checkPermission(InstrBase::PtrType instr){
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    try{
        processor_t* p = _processor.get();
        insn_t insn = ptr->instr;
        reg_t pc = ptr->pc;
        if(ptr->isF()){
            if(ptr->isSingle()){
                require_extension('F');
            }
            else if(ptr->isDouble()){
                require_extension('D');
            }
            require_fp;
        }
        else if(ptr->isAmo()){
            require_extension('A');
        }
        if(ptr->memLen() == 8){
            // require_rv64;
        }
    }
    catch(trap_t& t){
        setException(ptr, t);
    }
    catch (wait_for_interrupt_t &t)
    {
      _processor->in_wfi = true;
    }
}
void SpikeAbstractProcessor::addrGenForMem(InstrBase::PtrType instr){
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    reg_t tmp;
    if(ptr->isLd()  || ptr->isFLd()){
        getXpr(ptr->rs1(), tmp);
        ptr->mem_addr = tmp + ptr->i_imm();
    }
	else if(ptr->isSt() || ptr->isFSt()){
		getXpr(ptr->rs1(), tmp);
        ptr->mem_addr = tmp + ptr->s_imm();
	}
    else if(ptr->isAmo() || ptr->isHsv() || ptr->isHlv()){
        getXpr(ptr->rs1(), tmp);
        ptr->mem_addr = tmp;
    }
    else{
        throw "unexpected case in addrGenForMem";
    }
}
void SpikeAbstractProcessor::ptw(InstrBase::PtrType instr){
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    try{
        if(!ptr->isFetched){ // do fetch
            auto fetch_access_info = _processor->mmu->generate_access_info(ptr->pc, FETCH, 
                {false, false, false});
            ptr->pc_page = _processor->mmu->walk(fetch_access_info);
            ptr->pc_paddr = ptr->pc_page | (ptr->pc & (page_size - 1));
        }
        else{
            if(ptr->isLoad()){// load and lr
                auto mem_access_info = _processor->mmu->generate_access_info(ptr->mem_addr, LOAD, 
                    {ptr->isHlv(), ptr->isHlvx(), ptr->isLR()});
                ptr->mem_page = _processor->mmu->walk(mem_access_info);
                ptr->mem_paddr = ptr->mem_page | (ptr->mem_addr & (page_size - 1));
            }
            else if(ptr->isStore()){ //store and other amo
                auto mem_access_info = _processor->mmu->generate_access_info(ptr->mem_addr, STORE, 
                        {ptr->isHsv(), false, false});
                ptr->mem_page = _processor->mmu->walk(mem_access_info);
                ptr->mem_paddr = ptr->mem_page | (ptr->mem_addr & (page_size - 1));
            }
            else{
                throw "unexcepted case in decode";
            }
            
        }
    }
    catch(trap_t& t){
        setException(ptr, t);
    }
    catch (wait_for_interrupt_t &t)
    {
      _processor->in_wfi = true;
    }
    return;
}
void SpikeAbstractProcessor::pmpCheck(InstrBase::PtrType instr) {
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    if(!ptr->isFetched){ // fetch
		auto fetch_access_info = _processor->mmu->generate_access_info(ptr->pc, FETCH, 
                {false, false, false});
        if(!_processor->mmu->pmp_ok(ptr->pc_paddr,
                ptr->fetch_len, FETCH, (reg_t)fetch_access_info.effective_virt)){
            trap_t t = trap_instruction_access_fault(_processor->state.v, ptr->pc, 0, 0);
            setException(ptr, t);
        }
    }
    else{
        if(ptr->isLoad()){
			auto mem_access_info = _processor->mmu->generate_access_info(ptr->mem_addr, LOAD, 
                    {ptr->isHlv(), ptr->isHlvx(), ptr->isLR()});
            if(!_processor->mmu->pmp_ok(ptr->mem_paddr,
                    ptr->memLen(), LOAD, (reg_t)mem_access_info.effective_virt)){
                trap_t t = trap_load_access_fault(_processor->state.v, ptr->mem_addr, 0, 0);
                setException(ptr, t);
            }
        }
        else if(ptr->isStore()){
			auto mem_access_info = _processor->mmu->generate_access_info(ptr->mem_addr, STORE, 
                        {ptr->isHsv(), false, false});
            if(!_processor->mmu->pmp_ok(ptr->mem_paddr,
                    ptr->memLen(), STORE, (reg_t)mem_access_info.effective_virt)){
                trap_t t = trap_store_access_fault(_processor->state.v, ptr->mem_addr, 0, 0);
                setException(ptr, t);
            }
        }
        else{
            throw "unexpected case in pmp ls check";
        }
    } 
}
void SpikeAbstractProcessor::fetch(InstrBase::PtrType instr){
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    try{
        auto fetch_access_info = _processor->mmu->generate_access_info(ptr->pc, FETCH, 
                {false, false, false});
        reg_t vpn = ptr->pc >> page_shift;
        tlb_entry_t tlb_entry;
        if (auto host_addr = this->addr_to_mem(ptr->pc_paddr)) {
            tlb_entry = _processor->mmu->refill_tlb(ptr->pc, ptr->pc_paddr, host_addr, FETCH);
        } else {
            uint16_t tmp;
            if (!mmio_fetch(ptr->pc_paddr,sizeof(tmp) , (uint8_t*)&tmp)){
                trap_t t = trap_instruction_access_fault(_processor->state.v, ptr->pc, 0, 0);
                setException(ptr, t);
            }
            tlb_entry = {(char*)&tmp - ptr->pc_paddr, ptr->pc_paddr - ptr->pc};
        }

        insn_bits_t insn = from_le(*(uint16_t*)(tlb_entry.host_offset + ptr->pc));
        int length = insn_length(insn);

        if (likely(length == 4)) {
            insn |= (insn_bits_t)from_le(*(const uint16_t*)_processor->mmu->translate_insn_addr_to_host(ptr->pc + 2)) << 16;
        } else if (length == 2) {
        // entire instruction already fetched
        } else if (length == 6) {
            insn |= (insn_bits_t)from_le(*(const uint16_t*)_processor->mmu->translate_insn_addr_to_host(ptr->pc + 2)) << 16;
            insn |= (insn_bits_t)from_le(*(const uint16_t*)_processor->mmu->translate_insn_addr_to_host(ptr->pc + 4)) << 32;
        } else {
            static_assert(sizeof(insn_bits_t) == 8, "insn_bits_t must be uint64_t");
            insn |= (insn_bits_t)from_le(*(const uint16_t*)_processor->mmu->translate_insn_addr_to_host(ptr->pc + 2)) << 16;
            insn |= (insn_bits_t)from_le(*(const uint16_t*)_processor->mmu->translate_insn_addr_to_host(ptr->pc + 4)) << 32;
            insn |= (insn_bits_t)from_le(*(const uint16_t*)_processor->mmu->translate_insn_addr_to_host(ptr->pc + 6)) << 48;
        }

        ptr->setRaw(insn);
        ptr->instr = insn_t(insn);
        ptr->isFetched = true;
        if(trace_enable)
            printf("%08x\t", ptr->getRaw());
        return;
    }
    catch(trap_t& t){
        setException(ptr, t);
    }
    catch (wait_for_interrupt_t &t)
    {
      _processor->in_wfi = true;
    }
    return; 
}
void SpikeAbstractProcessor::amo(InstrBase::PtrType instr, amoFunc funct){
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    reg_t rs2 = *((reg_t*)&ptr->mem_data);
    memset((char*)&ptr->mem_data, 0, sizeof(ptr->mem_data));
    load(instr);
    reg_t old_data = *((reg_t*)&ptr->mem_data);
    reg_t ret = funct(old_data, rs2);
    memset((char*)&ptr->mem_data, 0, sizeof(ptr->mem_data));
    memcpy((char*)&ptr->mem_data, (char*)&ret, ptr->memLen());
    store(instr);
    memset((char*)&ptr->mem_data, 0, sizeof(ptr->mem_data));
    memcpy((char*)&ptr->mem_data, (char*)&old_data, ptr->memLen());
}
void SpikeAbstractProcessor::amo(InstrBase::PtrType instr) {
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    if(ptr->isAmoAdd()){
        amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
            return old_data + rs2;
        });
    }
    else if(ptr->isAmoSwap()){
        amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
            return rs2;
        });
    }
    else if(ptr->isAmoAnd()){
        amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
            return old_data & rs2;
        });
    }
    else if(ptr->isAmoOr()){
        amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
            return old_data | rs2;
        });
    }
    else if(ptr->isAmoXor()){
        amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
            return old_data ^ rs2;
        });
    } 
    else if(ptr->isAmoMin()){
        if(ptr->memLen() == 8){
            amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
                return std::min((int64_t)old_data, (int64_t)rs2);
            });
        }
        else{
            amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
                return std::min((int32_t)old_data, (int32_t)rs2);
            });
        }
        
    }
    else if(ptr->isAmoMinU()){
        amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
            return std::min(old_data, rs2);
        });
    }
    else if(ptr->isAmoMax()){
        if(ptr->memLen() == 8){
            amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
                return std::max((int64_t)old_data, (int64_t)rs2);
            });
        }
        else{
            amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
                return std::max((int32_t)old_data, (int32_t)rs2);
            });
        }
    }
    else if(ptr->isAmoMaxU()){
        amo(instr, [](const reg_t& old_data, const reg_t& rs2) -> reg_t{
            return std::max(old_data, rs2);
        });
    }
    // TODO: amocas
    else if(ptr->isLR()){
        load(instr);
    }
    else if(ptr->isSC()){
        bool have_reservation = _processor->mmu->check_load_reservation(ptr->mem_addr, 
            ptr->memLen());
        if(have_reservation)
            store(instr);
        _processor->mmu->yield_load_reservation();
        memset((char*)&ptr->mem_data, 0, sizeof(ptr->mem_data));
        ptr->mem_data[0] = (uint8_t)!have_reservation;
    }

}
void SpikeAbstractProcessor::load(InstrBase::PtrType instr) {
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    auto mem_access_info = _processor->mmu->generate_access_info(ptr->mem_addr, LOAD, 
                    {ptr->isHlv(), ptr->isHlvx(), ptr->isLR()});
    if (mem_access_info.flags.lr && !reservable(ptr->mem_paddr)) {
        trap_t t = trap_load_access_fault(mem_access_info.effective_virt, ptr->mem_addr, 0, 0);
        setException(ptr, t);
    }
    if (auto host_addr = addr_to_mem(ptr->mem_paddr)) {
		memcpy((char*)&ptr->mem_data, host_addr, ptr->memLen());
        if (_processor->mmu->tracer.interested_in_range(ptr->mem_paddr, 
                ptr->mem_paddr + page_size, LOAD))
            _processor->mmu->tracer.trace(ptr->mem_paddr, ptr->memLen(), LOAD);
        else if (!mem_access_info.flags.is_special_access())
            _processor->mmu->refill_tlb(ptr->mem_addr, ptr->mem_paddr, host_addr, LOAD);
    } else if (!mmio_load(ptr->mem_paddr, ptr->memLen(), (uint8_t*)&ptr->mem_data)) {
        trap_t t = trap_load_access_fault(mem_access_info.effective_virt, ptr->mem_paddr, 0, 0);
        setException(ptr, t);
    }

    if (mem_access_info.flags.lr) {
        _processor->mmu->load_reservation_address = ptr->mem_paddr;
    }
    if(trace_enable)
        printf("load mem %016llx(p:%016llx) : %016llx\t", ptr->mem_addr, ptr->mem_paddr, *((uint64_t*)(&ptr->mem_data)));
}
void SpikeAbstractProcessor::store(InstrBase::PtrType instr){
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    auto mem_access_info = _processor->mmu->generate_access_info(ptr->mem_addr, STORE, 
            {ptr->isHsv(), false, false});
    if (auto host_addr = addr_to_mem(ptr->mem_paddr)) {
        memcpy(host_addr, (char*)&ptr->mem_data, ptr->memLen());
        if (_processor->mmu->tracer.interested_in_range(ptr->mem_paddr, 
                ptr->mem_paddr + page_size, STORE))
            _processor->mmu->tracer.trace(ptr->mem_paddr, ptr->memLen(), STORE);
        else if (!mem_access_info.flags.is_special_access())
            _processor->mmu->refill_tlb(ptr->mem_addr, ptr->mem_paddr, host_addr, STORE);
    } else if (!mmio_store(ptr->mem_paddr, ptr->memLen(), (uint8_t*)&ptr->mem_data)) {
        trap_t t = trap_store_access_fault(mem_access_info.effective_virt, ptr->mem_addr, 0, 0);
        setException(ptr, t);
    }
    if(trace_enable)
        printf("store mem %016llx(p:%016llx) : %016llx\t", ptr->mem_addr, ptr->mem_paddr, *((uint64_t*)(&ptr->mem_data)));
}
void SpikeAbstractProcessor::handleInterrupts(InstrBase::PtrType instr){
    try{
        _processor->take_interrupt(_processor->state.mip->read() & _processor->state.mie->read());
    }
    catch(trap_t& t){
        auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
        setException(ptr, t);
    }
}
void SpikeAbstractProcessor::handleExceptions(InstrBase::PtrType instr){
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
    if(!ptr->evalid) return;
    if(trace_enable)
        printf("\n\tget exception: %x", ptr->ecause);
    _processor->take_trap(*(ptr->trap), ptr->pc);
    auto match = _processor->TM.detect_trap_match(*(ptr->trap));
    if (match.has_value())
        _processor->take_trigger_action(match->action, 0, _processor->state.pc, 0);
    return;
}
void SpikeAbstractProcessor::commit(InstrBase::PtrType instr){
    auto ptr = std::dynamic_pointer_cast<SpikeInstr>(instr);
	if(ptr->isLd()){
		reg_t res = *((uint64_t*)&ptr->mem_data);
        if(!ptr->isLoadUnsigned()){ // not unsigned
            if(ptr->memLen() == 1){ // lb
                res &= RV64_BYTE_MASK;
                if((res & RV64_BYTE_HIGHEST) == RV64_BYTE_HIGHEST){
                    res |= RV64_BYTE_MASK_R;
                }
            }
            else if(ptr->memLen() == 2){// lh
                res &= RV64_HALF_MASK;
                if((res & RV64_HALF_HIGHEST) == RV64_HALF_HIGHEST){
                    res |= RV64_HALF_MASK_R;
                }
            }
            else if(ptr->memLen() == 4){ // lw
                res &= RV64_WORD_MASK;
                if((res & RV64_WORD_HIGHEST) == RV64_WORD_HIGHEST){
                    res |= RV64_WORD_MASK_R;
                }
            }
        }
		setXpr(ptr->rd(), res);
	}
	else if(ptr->isFLd()){
        if (ptr->memLen() == 2){
		    setFpr(ptr->rd(), freg(f16(*((uint16_t*)&ptr->mem_data))));
        }
        else if (ptr->memLen() == 4){
		    setFpr(ptr->rd(), freg(f32(*((uint32_t*)&ptr->mem_data))));
        }
        else if (ptr->memLen() == 8){
		    setFpr(ptr->rd(), freg(f64(*((uint64_t*)&ptr->mem_data))));
        }
        else{
            throw "unexcepted case in fload commit";
        }
	}
    else if(ptr->isAmo()){
        reg_t res = *((uint64_t*)&ptr->mem_data);
        if(ptr->memLen() == 4){ // amo.w
            res &= RV64_WORD_MASK;
                if((res & RV64_WORD_HIGHEST) == RV64_WORD_HIGHEST){
                    res |= RV64_WORD_MASK_R;
                }
        }
		setXpr(ptr->rd(), res);
    }
}


// * simif
#define MAX_PADDR_BITS 56 // imposed by Sv39 / Sv48
inline static bool paddr_ok(reg_t addr)
{
  return (addr >> MAX_PADDR_BITS) == 0;
}
char* SpikeAbstractProcessor::addr_to_mem(reg_t paddr){
    if(!paddr_ok(paddr))
        return nullptr;
    // ! size is fixed to 8(64 bit), can be dangerous
    auto mem_req = MemReq::createMemReq(paddr, 0, 8, 0, 0, 0, 0, 0); 
    auto mem_resp = MemResp::createMemResp(0, 0);
    _uncore->accessMemNoEffect(mem_req, mem_resp);
    // uint64 addr to char*
    return  reinterpret_cast<char*>(mem_resp->data);
}
// used for MMIO addresses
bool SpikeAbstractProcessor::mmio_load(reg_t paddr, size_t len, uint8_t* bytes){
    if (paddr + len < paddr || !paddr_ok(paddr + len - 1))
        return false;
    char* mem = addr_to_mem(paddr);
    memcpy(bytes, mem, len);
    return true;
}
bool SpikeAbstractProcessor::mmio_store(reg_t paddr, size_t len, const uint8_t* bytes){
    if (paddr + len < paddr || !paddr_ok(paddr + len - 1))
        return false;
    char* mem = addr_to_mem(paddr);
    memcpy(mem, bytes, len);
    return true;
}
// Callback for processors to let the simulation know they were reset.
void SpikeAbstractProcessor::proc_reset(unsigned id){
}

const cfg_t& SpikeAbstractProcessor::get_cfg() const{
    return _processor->get_cfg();
}
const std::map<size_t, processor_t*>& SpikeAbstractProcessor::get_harts() const{
    return _harts;
}

const char* SpikeAbstractProcessor::get_symbol(uint64_t paddr){
    return nullptr;
}   
}