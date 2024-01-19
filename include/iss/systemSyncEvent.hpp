#pragma once

#include "iss/type.hpp"

namespace archXplore {
namespace iss {

enum systemSyncEventTypeEnum_t  {
    systemInit,
    hartInit,
    systemExit
};

struct systemSyncEvent_t {
    eventId_t event_id;
    systemSyncEventTypeEnum_t event_type;
    hartId_t hart_id;
};


}
}