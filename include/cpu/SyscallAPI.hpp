#pragma once
#include "Types.hpp"

namespace archXplore
{
    namespace cpu
    {
        struct SyscallAPI_t
        {
            enum APIType_t
            {
                UNKNOWN_API
            };

            APIType_t api_type;

        };
    } // namespace cpu
} // namespace archXplore