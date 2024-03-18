#ifndef __MEMORYPKT__HPP__
#define __MEMORYPKT__HPP__
#include <stdint.h>
#include <memory>
// #include <sparta/kernel/Scheduler.hpp>
#include "./Flags.hpp"
namespace archXplore{

// using Tick = sparta::Scheduler::Tick;
using Tick = uint64_t;
using payload_t = uint64_t; // max payload is 64, for RISCV ld


class MemReq{
public: 
	using PtrType = std::shared_ptr<MemReq>;
	// * use GEM5 flags directly for convinience
	using FlagsType = uint64_t;
	enum: FlagsType{
		ARCH_BITS                   = 0x000000FF,
        /** The request was an instruction fetch. */
        INST_FETCH                  = 0x00000100,
		/** The request is a page table walk */
        PT_WALK                     = 0x20000000,
	};
	using flag_t = Flags<FlagsType>;
	MemReq() = default;

    uint32_t pa = 0; // physical addr
    uint32_t va = 0; // virtual addr
    uint32_t size = 0; // data size
    uint32_t threadId = 0; // thread id
    Tick timestamp = 0; //timestamp
    uint32_t pc = 0; // ! for 32 ISA
	flag_t flag; // memory request type
	payload_t data = 0;
	MemReq(const uint32_t& pa, const uint32_t& va, const uint32_t& size, const uint32_t& threadId, 
		const Tick& timestamp, const uint32_t& pc, const payload_t& data, const FlagsType& flag):
		pa(pa), va(va), size(size), threadId(threadId), timestamp(timestamp),
		pc(pc), flag(flag), data(data)
		{	
		}
	inline static MemReq::PtrType createMemReq(const uint32_t& pa, const uint32_t& va, const uint32_t& size, const uint32_t& threadId, 
		const Tick& timestamp, const uint32_t& pc, const payload_t& data, const FlagsType& flag){
		return std::make_shared<MemReq>(pa, va, size, threadId, timestamp, pc, data, flag);
	}
};


class MemResp{
public:
	using PtrType = std::shared_ptr<MemResp>;
	MemResp() = default;
    Tick timestamp = 0; //timestamp
	payload_t data;
	MemResp(const payload_t& data, const Tick& timestamp): data(data), timestamp(timestamp){}
	inline static MemResp::PtrType createMemResp(const payload_t& data, const Tick& timestamp){
		return std::make_shared<MemResp>(data, timestamp);
	}
};


using MemReqPtr = std::shared_ptr<MemReq>;
using MemRespPtr = std::shared_ptr<MemResp>;



// inline static MemReqPtr createFetchReq(const uint32_t& pa, const uint32_t& pc, const uint32_t& size, const uint32_t& threadId, 
// 		const Tick& timestamp, const payload_t& data)
// {
// 	return std::make_shared<MemReq>(pa, pc, size, threadId, timestamp, pc, INST_FETCH);
// }

}; // namespace archXplore
#endif // __MEMORYPKT__HPP__