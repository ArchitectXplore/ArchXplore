#pragma once

#include "iceoryx_hoofs/cxx/vector.hpp"

#include "cpu/ThreadEvent.hpp"

namespace archXplore
{

    namespace iss
    {
        constexpr size_t MAX_HARTS = 128;

        constexpr size_t MESSAGE_VECTOR_SIZE = 16384;

        constexpr size_t MESSAGE_BUFFER_SIZE = 4;

        typedef iox::cxx::vector<cpu::ThreadEvent_t, MESSAGE_VECTOR_SIZE> Message_t;

        constexpr size_t MESSAGE_CHUNK_SIZE = sizeof(Message_t);

    } // namespace iss

} // namespace archXplore
