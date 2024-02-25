#pragma once

#include <vector>
#include <iostream>
#include <ostream>
#include <iomanip>

#include "types.hpp"

namespace archXplore
{
    namespace cpu
    {
        struct instruction_t
        {
            enum dependency_t{
                NO_DEP, // No dependency
                RAW, // RAW dependency
                WAW // WAW dependency  
            };

            struct dependency_info_t
            {
                eventId_t uid;
                dependency_t type;
            };

            struct memAccess_t
            {
                addr_t vaddr;
                // iss::addr_t paddr;
                uint8_t len;
                bool is_store;
            };

            struct branchInfo_t
            {
                uint64_t target_pc;
            };

            // Last instruction flag
            bool is_last;
            // Unique instruction id
            eventId_t uid;
            // instruction information
            addr_t pc;
            // iss::addr_t pc_paddr;
            opcode_t opcode;
            uint8_t len;
            // target pc
            branchInfo_t br_info;
            // Memory Access
            std::vector<memAccess_t> mem;
            // Data dependency
            std::vector<dependency_info_t> data_dep;
            // Memory dependency
            std::vector<dependency_info_t> mem_dep;

            inline void clear()
            {
                mem.clear(), data_dep.clear(), mem_dep.clear();
            };

            inline std::string stringize() const
            {
                std::stringstream ss;
                ss << std::dec << uid << " -> " << std::hex << pc;
                return ss.str();
            };
        };

        struct syscallApi_t
        {
            /* data */
        };

        struct threadApi_t
        {
            /**
             * Address of the critical variable used in Pthread calls, for e.g. the
             * mutex lock address, barrier variable address, conditional variable
             * address, or address of the input variable that holds the thread
             * information when creating a new thread
             */
            addr_t pthAddr;

            /**
             * Mutex lock address used in conjunction with conditional variable
             * address set in pthAddr
             */
            addr_t mutexLockAddr;

            /**
             * The type within the THREAD_API class of events
             *
             * INVALID_EVENT    Initialization value
             *
             * MUTEX_LOCK       Mutex lock event simulating lock acquire
             *
             * MUTEX_UNLOCK     Mutex unlock event simulating lock release
             *
             * THREAD_CREATE    New thread creation event
             *
             * THREAD_JOIN      Thread join
             *
             * BARRIER_WAIT     Synchronisation barrier
             *
             * COND_WAIT        Pthread condition wait
             *
             * COND_SG          Pthread condition signal
             *
             * SPIN_LOCK        Pthread spin lock
             *
             * SPIN_UNLOCK      Pthread spin unlock
             *
             * SEM_INIT         Initialise a semaphore
             *
             * SEM_WAIT         Block on a semaphore count
             *
             * SEM_POST         Increment a semaphore
             *
             */
            enum class eventType_t
            {
                INVALID_EVENT = 0,
                MUTEX_LOCK = 1,
                MUTEX_UNLOCK = 2,
                THREAD_CREATE = 3,
                THREAD_JOIN = 4,
                BARRIER_WAIT = 5,
                COND_WAIT = 6,
                COND_SG = 7,
                COND_BR = 8,
                SPIN_LOCK = 9,
                SPIN_UNLOCK = 10,
                SEM_INIT = 11,
                SEM_WAIT = 12,
                SEM_POST = 13,
                SEM_GETV = 14,
                SEM_DEST = 15,
                NUM_TYPES,
            };

            eventType_t eventType;
        };

        struct threadEvent_t
        {
            /**
             * Tag for event.
             * Events have a broad classification.
             */
            enum class Tag : uint8_t
            {
                UNDEFINED, // Initial value.

                /* Events */
                INSTRUCTION, // Instructions events

                THREAD_API, // Calls to thread library

                SYSCALL_API // Calls to syscall api

            };

            /**
             * Constants for tagged dispatch constructors.
             */
            using InsnTagType = std::integral_constant<Tag, Tag::INSTRUCTION>;
            static constexpr auto InsnTag = InsnTagType{};

            using ThreadApiTagType = std::integral_constant<Tag, Tag::THREAD_API>;
            static constexpr auto ThreadApiTag = ThreadApiTagType{};

            using SyscallApiTagType = std::integral_constant<Tag, Tag::SYSCALL_API>;
            static constexpr auto SyscallApiTag = SyscallApiTagType{};

            /**
             * The actual event.
             * An event stream is a sequence of any of the union'd types.
             *
             * impl: these are expected to be set only during construction,
             * hence they are const.
             */
            union
            {
                instruction_t instruction;
                syscallApi_t syscall_api;
                threadApi_t thread_api;
            };

            const Tag tag = Tag::UNDEFINED;
            eventId_t event_id = 0;

            threadEvent_t(InsnTagType, const eventId_t id, const instruction_t data) noexcept
                : instruction(data), tag{Tag::INSTRUCTION}, event_id(id) {}

            threadEvent_t(ThreadApiTagType, const eventId_t id, const threadApi_t data) noexcept
                : thread_api(data), tag{Tag::THREAD_API}, event_id(id) {}

            threadEvent_t(SyscallApiTagType, const eventId_t id, const syscallApi_t data) noexcept
                : syscall_api(data), tag{Tag::SYSCALL_API}, event_id(id) {}

            ~threadEvent_t(){};
        };


    } // namespace cpu

} // namespace archXplore
