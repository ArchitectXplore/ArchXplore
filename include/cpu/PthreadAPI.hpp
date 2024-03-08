#include "Types.hpp"

namespace archXplore
{

    namespace cpu
    {

        struct PthreadAPI_t
        {
            /**
             * Address of the critical variable used in Pthread calls, for e.g. the
             * mutex lock address, barrier variable address, conditional variable
             * address, or address of the input variable that holds the thread
             * information when creating a new thread
             */
            Addr_t pth_addr;

            /**
             * Mutex lock address used in conjunction with conditional variable
             * address set in pthAddr
             */
            Addr_t mutex_lock_addr;

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
            enum class APIType_t
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

            APIType_t api_type;
        };

    } // namespace cpu
} // namespace archXplore