#pragma once

#include <memory>

namespace archXplore {
namespace iss {


typedef uint64_t eventId_t;
typedef uint16_t hartId_t;

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