#pragma once

#include "sparta/simulation/ClockManager.hpp"
#include "sparta/events/EventSet.hpp"
#include "sparta/events/StartupEvent.hpp"

#include "system/abstractSystem.hpp"
#include "iss/qemu/qemuInterface.hpp"
#include "iss/qemu/qemuISS.hpp"

namespace archXplore
{
    namespace system
    {
        namespace qemu
        {

            class qemuSystem : public abstractSystem
            {
            public:
                qemuSystem() : m_global_scheduler("GlobalScheduler", getSearchScope()),
                               m_clock_manager(&m_global_scheduler),
                               m_global_event_set(this),
                               m_qemu_sync_event(
                                   &m_global_event_set, "qemuSyncEvent",
                                   CREATE_SPARTA_HANDLER(qemuSystem, handleSyncEvent))
                {
                    m_qemu_if = iss::qemu::qemuInterface::getInstance();
                };
                ~qemuSystem()
                {
                };

                auto _cleanUp() -> void override {
                    m_qemu_if->qemu_shutdown();
                };

                auto _build() -> void override
                {
                    // Create Global Clock
                    m_global_clock = m_clock_manager.makeRoot(this, "GlobalClock");
                    setClock(m_global_clock.get());
                    // Setup StartupEvent
                    sparta::StartupEvent(
                        &m_global_event_set, CREATE_SPARTA_HANDLER(qemuSystem, waitFirstSyncEvent));
                    // Create Individual Clock for each CPU
                    for (auto it : m_cpuInfos)
                    {
                        const std::string clock_name = "CPU" + std::to_string(it.first) + "Clock";
                        const sparta::Clock::Frequency freq = it.second.freq;
                        sparta::Clock::Handle clock = m_clock_manager.makeClock(clock_name, m_global_clock, freq);
                        it.second.cpu->getContainer()->setClock(clock.get());
                    };
                    // Build ClockTree
                    m_clock_manager.normalize();
                    // Enter Finalized, Build Resources and Start Connection
                    enterConfiguring();
                    enterFinalized();
                    m_global_scheduler.finalize();
                };

                auto boot(const iss::qemu::qemuArgs_t &args) -> void
                {
                    m_qemu_if->bootQemuThread(args);
                };

                auto _run(sparta::Scheduler::Tick tick) -> void override
                {
                    m_global_scheduler.run(tick, false, false);
                };

                auto waitFirstSyncEvent() -> void {
                    while(!m_qemu_if->pendingSyncEvent()){
                        continue;
                    }
                    handleSyncEvent();
                };

                auto handleSyncEvent() -> void
                {
                    m_qemu_sync_event.schedule(1);
                    if (m_qemu_if->pendingSyncEvent())
                    {
                        auto ev = m_qemu_if->getPendingSyncEvent();
                        if (ev.event_type == iss::systemSyncEventTypeEnum_t::hartInit)
                        {
                            m_qemu_if->removeSyncEvent();
                            m_cpuInfos[ev.hart_id].cpu->powerOn();
                        }
                        else if (ev.event_type == iss::systemSyncEventTypeEnum_t::systemExit)
                        {
                            bool can_exit = true;
                            for (auto cpuInfo : m_cpuInfos) {
                                auto insnQueue = iss::qemu::qemuInterface::getHartInsnQueuePtr(cpuInfo.first);
                                if(insnQueue->isEmpty()) {
                                    cpuInfo.second.cpu->powerOff();
                                } else {
                                    can_exit = false;
                                }
                            }
                            if(can_exit) {
                                m_qemu_sync_event.cancel();
                            }
                        }
                    };
                };

                auto _createISS() -> iss::abstractISS::UniquePtr override {
                    return std::make_unique<iss::qemu::qemuISS>();
                }

            private:
                // Sparta Clock and Scheduler
                sparta::Scheduler m_global_scheduler;
                sparta::ClockManager m_clock_manager;
                sparta::Clock::Handle m_global_clock;
                // Sparta Global Event Set
                sparta::EventSet m_global_event_set;
                // QEMU System Sync Event
                sparta::Event<sparta::SchedulingPhase::Update> m_qemu_sync_event;
                // QEMU Interface Instance
                iss::qemu::qemuInterface::ptrType m_qemu_if;
            };

        } // namespace qemu
    }     // namespace system

} // namespace archXplore
