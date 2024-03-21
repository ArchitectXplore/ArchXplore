#ifndef __MEMORYPKT__HPP__
#define __MEMORYPKT__HPP__
#include <stdint.h>
#include <memory>
#include <iostream>	
#include "sparta/utils/SpartaAssert.hpp"
#include "utils/common.hpp"
#include "flags.hpp"
#include <cstring>
namespace archXplore{

struct Payload{
uint8_t* data = nullptr;
uint32_t size = 0;
bool operator==(const Payload& other){
	if(other.size == 0 && size == 0)
		return true;
	if(other.size!= size)
		return false;
	sparta_assert(other.data!= nullptr && data!= nullptr, "Payload data should not be null");
	return std::memcmp(data, other.data, size);
}
bool operator!=(const Payload& other){
	return !(*this == other);
}
Payload():data(nullptr), size(0){}
Payload(const uint32_t& size, uint8_t* data, const bool& must_move = false):
    data(data), size(size){
}
Payload(const Payload& other):data(other.data), size(other.size){
}

Payload& operator=(const Payload& other){
	data = other.data;
	size = other.size;
	return *this;
}

};
inline std::ostream& operator<<(std::ostream& os, const Payload& p){
	if(p.size == 0)
		os << "Payload(empty)" << std::endl;
	else {
		sparta_assert(p.data!= nullptr, "Payload data should not be null");
		os << "Payload: size=" << p.size << "data: 0x";
		for(int i = p.size - 1; i >= 0; i--){
			os << HEX2(p.data[i]);
		}
		os << std::endl;
	}
	return os;
}

class MemReq{
public: 
	// * use GEM5 flags directly for convinience
	using FlagsType = uint64_t;
	enum : FlagsType
    {
        /**
         * Architecture specific flags.
         *
         * These bits int the flag field are reserved for
         * architecture-specific code. For example, SPARC uses them to
         * represent ASIs.
         */
        ARCH_BITS                   = 0x000000FF,
		READ 						= 0X00000001,
		WRITE 						= 0X00000002,	
		FETCH 						= 0X00000004,
        /** The request was an instruction fetch. */
        INST_FETCH                  = 0x00000100,
        /** The virtual address is also the physical address. */
        PHYSICAL                    = 0x00000200,
        /**
         * The request is to an uncacheable address.
         *
         * @note Uncacheable accesses may be reordered by CPU models. The
         * STRICT_ORDER flag should be set if such reordering is
         * undesirable.
         */
        UNCACHEABLE                 = 0x00000400,
        /**
         * The request is required to be strictly ordered by <i>CPU
         * models</i> and is non-speculative.
         *
         * A strictly ordered request is guaranteed to never be
         * re-ordered or executed speculatively by a CPU model. The
         * memory system may still reorder requests in caches unless
         * the UNCACHEABLE flag is set as well.
         */
        STRICT_ORDER                = 0x00000800,
        /** This request is made in privileged mode. */
        PRIVILEGED                  = 0x00008000,

        /**
         * This is a write that is targeted and zeroing an entire
         * cache block.  There is no need for a read/modify/write
         */
        CACHE_BLOCK_ZERO            = 0x00010000,

        /** The request should not cause a memory access. */
        NO_ACCESS                   = 0x00080000,
        /**
         * This request will lock or unlock the accessed memory. When
         * used with a load, the access locks the particular chunk of
         * memory. When used with a store, it unlocks. The rule is
         * that locked accesses have to be made up of a locked load,
         * some operation on the data, and then a locked store.
         */
        LOCKED_RMW                  = 0x00100000,
        /** The request is a Load locked/store conditional. */
        LLSC                        = 0x00200000,
        /** This request is for a memory swap. */
        MEM_SWAP                    = 0x00400000,
        MEM_SWAP_COND               = 0x00800000,
        /** This request is a read which will be followed by a write. */
        READ_MODIFY_WRITE           = 0x00020000,

        /** The request is a prefetch. */
        PREFETCH                    = 0x01000000,
        /** The request should be prefetched into the exclusive state. */
        PF_EXCLUSIVE                = 0x02000000,
        /** The request should be marked as LRU. */
        EVICT_NEXT                  = 0x04000000,
        /** The request should be marked with ACQUIRE. */
        ACQUIRE                     = 0x00020000,
        /** The request should be marked with RELEASE. */
        RELEASE                     = 0x00040000,

        /** The request is an atomic that returns data. */
        ATOMIC_RETURN_OP            = 0x40000000,
        /** The request is an atomic that does not return data. */
        ATOMIC_NO_RETURN_OP         = 0x80000000,

        /** The request should be marked with KERNEL.
          * Used to indicate the synchronization associated with a GPU kernel
          * launch or completion.
          */
        KERNEL                      = 0x00001000,

        /** The request targets the secure memory space. */
        SECURE                      = 0x10000000,
        /** The request is a page table walk */
        PT_WALK                     = 0x20000000,

        /** The request invalidates a memory location */
        INVALIDATE                  = 0x0000000100000000,
        /** The request cleans a memory location */
        CLEAN                       = 0x0000000200000000,

        /** The request targets the point of unification */
        DST_POU                     = 0x0000001000000000,

        /** The request targets the point of coherence */
        DST_POC                     = 0x0000002000000000,

        /** Bits to define the destination of a request */
        DST_BITS                    = 0x0000003000000000,

        /** hardware transactional memory **/

        /** The request starts a HTM transaction */
        HTM_START                   = 0x0000010000000000,

        /** The request commits a HTM transaction */
        HTM_COMMIT                  = 0x0000020000000000,

        /** The request cancels a HTM transaction */
        HTM_CANCEL                  = 0x0000040000000000,

        /** The request aborts a HTM transaction */
        HTM_ABORT                   = 0x0000080000000000,

        // What is the different between HTM cancel and abort?
        //
        // HTM_CANCEL will originate from a user instruction, e.g.
        // Arm's TCANCEL or x86's XABORT. This is an explicit request
        // to end a transaction and restore from the last checkpoint.
        //
        // HTM_ABORT is an internally generated request used to synchronize
        // a transaction's failure between the core and memory subsystem.
        // If a transaction fails in the core, e.g. because an instruction
        // within the transaction generates an exception, the core will prepare
        // itself to stop fetching/executing more instructions and send an
        // HTM_ABORT to the memory subsystem before restoring the checkpoint.
        // Similarly, the transaction could fail in the memory subsystem and
        // this will be communicated to the core via the Packet object.
        // Once the core notices, it will do the same as the above and send
        // a HTM_ABORT to the memory subsystem.
        // A HTM_CANCEL sent to the memory subsystem will ultimately return
        // to the core which in turn will send a HTM_ABORT.
        //
        // This separation is necessary to ensure the disjoint components
        // of the system work correctly together.

        /** The Request is a TLB shootdown */
        TLBI                        = 0x0000100000000000,

        /** The Request is a TLB shootdown sync */
        TLBI_SYNC                   = 0x0000200000000000,

        /** The Request tells the CPU model that a
            remote TLB Sync has been requested */
        TLBI_EXT_SYNC               = 0x0000400000000000,

        /** The Request tells the interconnect that a
            remote TLB Sync request has completed */
        TLBI_EXT_SYNC_COMP          = 0x0000800000000000,

        /**
         * These flags are *not* cleared when a Request object is
         * reused (assigned a new address).
         */
        STICKY_FLAGS = INST_FETCH
    };
	using flag_t = Flags<FlagsType>;
	MemReq() = default;

	uint32_t cpuid = 0; // cpu id
    uint32_t threadId = 0; // thread id
    uint64_t timestamp = 0; //timestamp
    uint32_t pa = 0; // physical addr
    uint32_t va = 0; // virtual addr
    uint32_t pc = 0; // ! for 32 ISA
	flag_t flag; // memory request type
	Payload payload;
	MemReq(const uint32_t cpuid, const uint32_t& threadId, const uint64_t& timestamp, const uint32_t& pa, const uint32_t& va, 
		const uint32_t& pc, const FlagsType& flag, const Payload& data):
		cpuid(cpuid), pa(pa), va(va), threadId(threadId), timestamp(timestamp),
		pc(pc), flag(flag), payload(data)
		{	
		}
	MemReq(const uint32_t cpuid, const uint32_t& threadId, const uint64_t& timestamp, const uint32_t& pa, const uint32_t& va, 
		const uint32_t& pc, const FlagsType& flag, const uint32_t& size, uint8_t* data_ptr):
		cpuid(cpuid), pa(pa), va(va), threadId(threadId), timestamp(timestamp),
		pc(pc), flag(flag), payload(Payload{size, data_ptr})
		{	
		}
	auto isRead() const -> bool{return flag.isSet(READ);}
    auto isWrite() const -> bool{return flag.isSet(WRITE);}
    auto isFetch() const -> bool{return flag.isSet(FETCH);}
};
inline std::ostream& operator<<(std::ostream& os, const Flags<uint64_t>& p){
	os << HEX16(p.get());
	return os;
}
inline std::ostream& operator<<(std::ostream& os, const MemReq& p){
	os << "MemReq: cpuid=" << p.cpuid 
		<< " threadId=" << p.threadId 
		<< " timestamp=" << p.timestamp 
		<< " pa=0x" << HEX16(p.pa) 
		<< " va=0x" << HEX16(p.va) 
		<< " pc=0x" << HEX16(p.pc) 
		<< " flag=" << p.flag 
		<< " data=" << p.payload 
		<< std::endl;
	return os;
}

class MemResp{
public:
	using FlagsType = uint64_t;
	enum : FlagsType
    {
        /**
         * Architecture specific flags.
         *
         * These bits int the flag field are reserved for
         * architecture-specific code. For example, SPARC uses them to
         * represent ASIs.
         */
        ARCH_BITS                   = 0x000000FF,
		READ 						= 0X00000001,
		WRITE 						= 0X00000002,	
		FETCH 						= 0X00000004,
        /** The request was an instruction fetch. */
        INST_FETCH                  = 0x00000100,
        /** The virtual address is also the physical address. */
        PHYSICAL                    = 0x00000200,
        /**
         * The request is to an uncacheable address.
         *
         * @note Uncacheable accesses may be reordered by CPU models. The
         * STRICT_ORDER flag should be set if such reordering is
         * undesirable.
         */
        UNCACHEABLE                 = 0x00000400,
        /**
         * The request is required to be strictly ordered by <i>CPU
         * models</i> and is non-speculative.
         *
         * A strictly ordered request is guaranteed to never be
         * re-ordered or executed speculatively by a CPU model. The
         * memory system may still reorder requests in caches unless
         * the UNCACHEABLE flag is set as well.
         */
        STRICT_ORDER                = 0x00000800,
        /** This request is made in privileged mode. */
        PRIVILEGED                  = 0x00008000,

        /**
         * This is a write that is targeted and zeroing an entire
         * cache block.  There is no need for a read/modify/write
         */
        CACHE_BLOCK_ZERO            = 0x00010000,

        /** The request should not cause a memory access. */
        NO_ACCESS                   = 0x00080000,
        /**
         * This request will lock or unlock the accessed memory. When
         * used with a load, the access locks the particular chunk of
         * memory. When used with a store, it unlocks. The rule is
         * that locked accesses have to be made up of a locked load,
         * some operation on the data, and then a locked store.
         */
        LOCKED_RMW                  = 0x00100000,
        /** The request is a Load locked/store conditional. */
        LLSC                        = 0x00200000,
        /** This request is for a memory swap. */
        MEM_SWAP                    = 0x00400000,
        MEM_SWAP_COND               = 0x00800000,
        /** This request is a read which will be followed by a write. */
        READ_MODIFY_WRITE           = 0x00020000,

        /** The request is a prefetch. */
        PREFETCH                    = 0x01000000,
        /** The request should be prefetched into the exclusive state. */
        PF_EXCLUSIVE                = 0x02000000,
        /** The request should be marked as LRU. */
        EVICT_NEXT                  = 0x04000000,
        /** The request should be marked with ACQUIRE. */
        ACQUIRE                     = 0x00020000,
        /** The request should be marked with RELEASE. */
        RELEASE                     = 0x00040000,

        /** The request is an atomic that returns data. */
        ATOMIC_RETURN_OP            = 0x40000000,
        /** The request is an atomic that does not return data. */
        ATOMIC_NO_RETURN_OP         = 0x80000000,

        /** The request should be marked with KERNEL.
          * Used to indicate the synchronization associated with a GPU kernel
          * launch or completion.
          */
        KERNEL                      = 0x00001000,

        /** The request targets the secure memory space. */
        SECURE                      = 0x10000000,
        /** The request is a page table walk */
        PT_WALK                     = 0x20000000,

        /** The request invalidates a memory location */
        INVALIDATE                  = 0x0000000100000000,
        /** The request cleans a memory location */
        CLEAN                       = 0x0000000200000000,

        /** The request targets the point of unification */
        DST_POU                     = 0x0000001000000000,

        /** The request targets the point of coherence */
        DST_POC                     = 0x0000002000000000,

        /** Bits to define the destination of a request */
        DST_BITS                    = 0x0000003000000000,

        /** hardware transactional memory **/

        /** The request starts a HTM transaction */
        HTM_START                   = 0x0000010000000000,

        /** The request commits a HTM transaction */
        HTM_COMMIT                  = 0x0000020000000000,

        /** The request cancels a HTM transaction */
        HTM_CANCEL                  = 0x0000040000000000,

        /** The request aborts a HTM transaction */
        HTM_ABORT                   = 0x0000080000000000,

        // What is the different between HTM cancel and abort?
        //
        // HTM_CANCEL will originate from a user instruction, e.g.
        // Arm's TCANCEL or x86's XABORT. This is an explicit request
        // to end a transaction and restore from the last checkpoint.
        //
        // HTM_ABORT is an internally generated request used to synchronize
        // a transaction's failure between the core and memory subsystem.
        // If a transaction fails in the core, e.g. because an instruction
        // within the transaction generates an exception, the core will prepare
        // itself to stop fetching/executing more instructions and send an
        // HTM_ABORT to the memory subsystem before restoring the checkpoint.
        // Similarly, the transaction could fail in the memory subsystem and
        // this will be communicated to the core via the Packet object.
        // Once the core notices, it will do the same as the above and send
        // a HTM_ABORT to the memory subsystem.
        // A HTM_CANCEL sent to the memory subsystem will ultimately return
        // to the core which in turn will send a HTM_ABORT.
        //
        // This separation is necessary to ensure the disjoint components
        // of the system work correctly together.

        /** The Request is a TLB shootdown */
        TLBI                        = 0x0000100000000000,

        /** The Request is a TLB shootdown sync */
        TLBI_SYNC                   = 0x0000200000000000,

        /** The Request tells the CPU model that a
            remote TLB Sync has been requested */
        TLBI_EXT_SYNC               = 0x0000400000000000,

        /** The Request tells the interconnect that a
            remote TLB Sync request has completed */
        TLBI_EXT_SYNC_COMP          = 0x0000800000000000,

        /**
         * These flags are *not* cleared when a Request object is
         * reused (assigned a new address).
         */
        STICKY_FLAGS = INST_FETCH
    };
	using flag_t = Flags<FlagsType>;
	MemResp() = default;
	uint32_t cpuid = 0; // cpu id
    uint32_t threadId = 0; // thread id
    uint64_t timestamp = 0; //timestamp
	flag_t flag; // memory request type
	Payload payload;
	MemResp(const uint32_t cpuid, const uint32_t& threadId, const uint64_t& timestamp, const FlagsType& flag, const Payload& data):
		cpuid(cpuid), threadId(threadId), timestamp(timestamp), flag(flag), payload(data)
		{}
	MemResp(const uint32_t cpuid, const uint32_t& threadId, const uint64_t& timestamp, const FlagsType& flag, const uint32_t& size, uint8_t* data_ptr):
		cpuid(cpuid), threadId(threadId), timestamp(timestamp), flag(flag), payload(Payload{size, data_ptr}){}
    MemResp(const MemReq& req):
        MemResp(req.cpuid, req.threadId, req.timestamp, req.flag.get(), req.payload)
    {}
    auto isRead() const -> bool{return flag.isSet(READ);}
    auto isWrite() const -> bool{return flag.isSet(WRITE);}
    auto isFetch() const -> bool{return flag.isSet(FETCH);}
};

inline std::ostream& operator<<(std::ostream& os, const MemResp& p){
	os << "MemResp: cpuid=" << p.cpuid 
		<< " threadId=" << p.threadId 
		<< " timestamp=" << p.timestamp 
		<< " flag=" << p.flag 
		<< " data=" << p.payload 
		<< std::endl;
	return os;	
}

class SnoopReq{
public:
	using FlagsType = uint64_t;
	enum : FlagsType{
        ARCH_BITS                   = 0x000000FF,
		EVICT                      	= 0x00000001,
	}; // enum : FlagsType
	using flag_t = Flags<FlagsType>;
	uint32_t cpuid = 0; // cpu id
    uint32_t threadId = 0; // thread id
    uint64_t timestamp = 0; //timestamp
	flag_t flag; // memory request type
	uint64_t va;
	uint64_t pa;
	Payload payload;
	SnoopReq() = default;
	SnoopReq(const uint32_t cpuid, const uint32_t& threadId, const uint64_t& timestamp, const uint32_t& pa, const uint32_t& va, 
		const FlagsType& flag, const Payload& data):
		cpuid(cpuid), pa(pa), va(va), threadId(threadId), timestamp(timestamp),
		flag(flag), payload(data)
		{	
		}
	SnoopReq(const uint32_t cpuid, const uint32_t& threadId, const uint64_t& timestamp, const uint32_t& pa, const uint32_t& va, 
		const FlagsType& flag, const uint32_t& size, uint8_t* data_ptr):
		cpuid(cpuid), pa(pa), va(va), threadId(threadId), timestamp(timestamp),
		flag(flag), payload(Payload{size, data_ptr})
		{	
		}
}; // class SnoopReq
inline std::ostream& operator<<(std::ostream& os, const SnoopReq& p){
	os << "SnoopReq: cpuid=" << p.cpuid 
		<< " threadId=" << p.threadId 
		<< " timestamp=" << p.timestamp 
		<< " pa=0x" << HEX16(p.pa) 
		<< " va=0x" << HEX16(p.va) 
		<< " flag=" << p.flag 
		<< " data=" << p.payload 
		<< std::endl;	
	return os;
}
class SnoopResp{
public: 
	using FlagsType = uint64_t;
	enum : FlagsType{
		ARCH_BITS                   = 0x000000FF,
		EVICT                      	= 0x00000001,
	}; // enum : FlagsType
	using flag_t = Flags<FlagsType>;
	uint32_t cpuid = 0; // cpu id
    uint32_t threadId = 0; // thread id
    uint64_t timestamp = 0; //timestamp
	flag_t flag; // memory request type
	Payload payload;
	SnoopResp() = default;
	SnoopResp(const uint32_t cpuid, const uint32_t& threadId, const uint64_t& timestamp, const FlagsType& flag, const Payload& data):
		cpuid(cpuid), threadId(threadId), timestamp(timestamp),flag(flag), payload(data)
		{	}
	SnoopResp(const uint32_t cpuid, const uint32_t& threadId, const uint64_t& timestamp, const FlagsType& flag, const uint32_t& size, uint8_t* data_ptr):
		cpuid(cpuid), threadId(threadId), timestamp(timestamp), flag(flag), payload(Payload{size, data_ptr})
		{	}
}; // class SnoopResp
inline std::ostream& operator<<(std::ostream& os, const SnoopResp& p){
	os << "SnoopResp: cpuid=" << p.cpuid 
		<< " threadId=" << p.threadId 
		<< " timestamp=" << p.timestamp 
		<< " flag=" << p.flag 
		<< " data=" << p.payload 
		<< std::endl;
	return os;	
}
}; // namespace archXplore
#endif // __MEMORYPKT__HPP__