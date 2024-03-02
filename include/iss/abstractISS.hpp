#pragma once

#include <vector>

#include "sparta/sparta.hpp"
#include "sparta/utils/SpartaAssert.hpp"

#include "types.hpp"
#include "cpu/threadEvent.hpp"

// Forward Declaration
namespace archXplore::cpu
{
    class abstractCPU;
}

namespace archXplore
{
    namespace iss
    {
        class abstractISS
        {
        public:
            friend class cpu::abstractCPU;

            /**
             * @brief Initialize the CPU state
             * 
             * This function should be called when the CPU is initialized.
             */
            virtual inline auto initCPUState() -> void = 0;

            /**
             * @brief Generate a fetch request
             * 
             * This function should be called when the CPU needs to fetch instructions.
             *
             * 
             * @param cur_pc Current program counter
             * @param fetch_width Fetch width in bytes
             */
            virtual inline auto generateFetchRequest(const addr_t& cur_pc, const size_t& fetch_width) -> void = 0;

            /**
             * @brief Process a fetch response
             * 
             * This function should be called when the CPU receives a fetch response from the memory system.
             *
             * @param data Pointer to the fetched data
             * @param length Length of the fetched data in bytes
             */
            virtual inline auto processFetchResponse(const uint8_t* data) -> std::vector<cpu::instruction_t> = 0;

            // Delete copy-construct function
            abstractISS(const abstractISS &that) = delete;
            abstractISS &operator=(const abstractISS &that) = delete;

            /**
             * @brief Constructor
            */
            abstractISS();

            /**
             * @brief Destructor
            */
            ~abstractISS();


        protected:

            /**
             * @brief Initialize the ISS
             * 
             * This function should be called after the CPU is initialized.
             */
            inline virtual auto initialize() -> void = 0;

            /**
             * @brief Monitor wakeup event of the CPU
             * 
             * This function should be called when the CPU tries to wake up from a sleep state.
             */
            virtual inline auto wakeUpMonitor() -> void = 0;

            /**
             * @brief Set the CPU pointer
             * @param cpu CPU pointer
             */
            auto setCPU(cpu::abstractCPU *cpu) -> void;

        protected:
            // CPU pointer
            cpu::abstractCPU *m_cpu;
        };

    } // namespace iss
} // namespace archXplore
