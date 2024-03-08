#pragma once

#include <vector>
#include <iostream>
#include <ostream>
#include <iomanip>

#include "Types.hpp"

#include "cpu/StaticInst.hpp"
#include "cpu/PthreadAPI.hpp"
#include "cpu/SyscallAPI.hpp"

namespace archXplore
{
    namespace cpu
    {

        struct ThreadEvent_t
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
                StaticInst_t instruction;
                SyscallAPI_t syscall_api;
                PthreadAPI_t pthread_api;
            };

            const Tag tag = Tag::UNDEFINED;

            EventID_t event_id = 0;

            bool is_last = false;

            ThreadEvent_t(InsnTagType, const EventID_t& event_id, const StaticInst_t& data) noexcept
                : instruction(data), tag{Tag::INSTRUCTION}, event_id{event_id} {}

            ThreadEvent_t(ThreadApiTagType, const EventID_t& event_id, const PthreadAPI_t& data) noexcept
                : pthread_api(data), tag{Tag::THREAD_API}, event_id{event_id}  {}

            ThreadEvent_t(SyscallApiTagType, const EventID_t& event_id, const SyscallAPI_t& data) noexcept
                : syscall_api(data), tag{Tag::SYSCALL_API}, event_id{event_id} {}

            ~ThreadEvent_t(){};

            ThreadEvent_t(const archXplore::cpu::ThreadEvent_t& that) 
                : tag{that.tag}, event_id{that.event_id}, is_last{that.is_last}
            {
                switch(tag)
                {
                    case Tag::INSTRUCTION:
                        new (&instruction) StaticInst_t(that.instruction);
                        break;
                    case Tag::THREAD_API:
                        new (&pthread_api) PthreadAPI_t(that.pthread_api);
                        break;
                    case Tag::SYSCALL_API:
                        new (&syscall_api) SyscallAPI_t(that.syscall_api);
                        break;
                    default:
                        break;
                }
            };


            ThreadEvent_t& operator=(const ThreadEvent_t& that)
            {
                if(this != nullptr && this != &that){
                    this->~ThreadEvent_t();
                };
                new (this) ThreadEvent_t(that);
                return *this;
            };
        };


    } // namespace cpu

} // namespace archXplore
