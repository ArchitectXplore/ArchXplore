#pragma once

#include "sparta/simulation/ClockManager.hpp"
#include "sparta/events/EventSet.hpp"
#include "sparta/events/StartupEvent.hpp"
#include "sparta/events/PayloadEvent.hpp"

#include "system/abstractSystem.hpp"
#include "iss/qemu/qemuInterface.hpp"
#include "iss/qemu/qemuISS.hpp"

namespace archXplore
{
    namespace system
    {
        namespace qemu
        {

            class qemuParallelSystem : public abstractSystem
            {
            public:
                qemuParallelSystem() : m_global_scheduler("GlobalScheduler", getSearchScope()),
                                       m_clock_manager(&m_global_scheduler),
                                       m_global_event_set(this)
                {
                    m_qemu_if = iss::qemu::qemuInterface::getInstance();
                };
                ~qemuParallelSystem()
                {
                    m_qemu_if->qemu_shutdown();
                };

                auto threadTickStartHandler(const hartId_t &tid) -> void
                {
                    m_thread_local.emplace_back(
                        [this, tid]
                        {
                            auto tick_diff = this->m_thread_global_clocks[tid]->currentTick() - this->m_thread_local_clocks[tid]->currentTick();
                            this->m_thread_local_schedulers[tid]->run(tick_diff);
                        });
                    if (!this->m_thread_local_schedulers[tid]->isFinished())
                    {
                        m_thread_tick_events[tid]->preparePayload(tid)->schedule(m_tick_quantum);
                    }
                };

                auto threadTickEndHandler() -> void
                {
                    // Wait for all threads to complete their local schedulers
                    for (auto &t : m_thread_local)
                    {
                        if (t.joinable())
                        {
                            t.join();
                        }
                    }
                    m_thread_local.clear();
                    if (m_global_scheduler.isFinished())
                    {
                        m_global_tick_end_event->schedule(m_tick_quantum);
                    }
                };

                auto _build() -> void override
                {
                    // Create Global Clock
                    m_global_clock = m_clock_manager.makeRoot(this, "GlobalClock");
                    setClock(m_global_clock.get());
                    // Create Individual Clock and Scheduler for each CPU
                    m_thread_global_clocks.resize(m_cpu_infos.size());
                    m_thread_local_schedulers.resize(m_cpu_infos.size());
                    m_thread_local_clocks.resize(m_cpu_infos.size());
                    m_thread_tick_events.resize(m_cpu_infos.size());
                    for (auto it : m_cpu_infos)
                    {
                        const std::string local_thread_name = "CPU" + std::to_string(it.first);
                        const sparta::Clock::Frequency freq = it.second.freq;
                        // Create global clock for each CPU
                        sparta::Clock::Handle thread_global_clock =
                            m_thread_global_clocks[it.first] = m_clock_manager.makeClock(local_thread_name + "Clock", m_global_clock, freq);
                        m_thread_tick_events[it.first] = std::make_unique<sparta::PayloadEvent<hartId_t>>(&m_global_event_set, local_thread_name + "TickEvent",
                                                                                                          CREATE_SPARTA_HANDLER_WITH_DATA(qemuParallelSystem, threadTickStartHandler, hartId_t));
                        m_thread_tick_events[it.first]->setClock(m_thread_global_clocks[it.first].get());
                        // Create Local Clock and Scheduler
                        m_thread_local_schedulers[it.first] = std::make_unique<sparta::Scheduler>(local_thread_name + "Scheduler");
                        m_thread_local_clocks[it.first] = std::make_shared<sparta::Clock>(local_thread_name + "Clock", m_thread_local_schedulers[it.first].get());
                        it.second.cpu->getContainer()->setClock(m_thread_local_clocks[it.first].get());
                        // Create global tick end event to finalize tick quantum
                        m_global_tick_end_event = std::make_unique<sparta::Event<sparta::SchedulingPhase::Tick>>(&m_global_event_set, "GlobalTickEndEvent",
                                                                                                                 CREATE_SPARTA_HANDLER(qemuParallelSystem, threadTickEndHandler));
                        m_global_tick_end_event->setClock(m_global_clock.get());
                        // Construct dependencies between events
                        for (auto &thread_tick_start_event : m_thread_tick_events)
                        {
                            *thread_tick_start_event >> *m_global_tick_end_event;
                        }
                    };
                    // Build global clock tree
                    m_clock_manager.normalize();
                    // Enter Finalized, Build Resources and Start Connection
                    enterConfiguring();
                    enterFinalized();
                    m_global_scheduler.finalize();
                    for (auto &it : m_thread_local_schedulers)
                    {
                        it->finalize();
                    }
                };

                auto boot() -> void
                {
                    // Plugin library path
                    std::string plugin_path = executablePath() + "/libqemuInterface_plugin.so";

                    // Simulating command-line arguments
                    std::vector<std::string> args = {"QEMU", "-plugin", plugin_path, m_workload_path};

                    // Create argc and argv
                    int argc = static_cast<int>(args.size());
                    char **argv = new char *[argc];

                    for (int i = 0; i < argc; ++i)
                    {
                        argv[i] = new char[args[i].size() + 1];
                        std::strcpy(argv[i], args[i].c_str());
                    }

                    // Your program logic goes here...
                    boot({argc, argv, nullptr});
                }

                auto boot(const iss::qemu::qemuArgs_t &args) -> void
                {
                    // Your program logic goes here...
                    m_qemu_if->bootQemuThread(args);
                };

                auto _run(sparta::Scheduler::Tick tick) -> void override
                {
                    boot();
                    m_global_scheduler.run(tick, false, false);
                };

                auto _createISS() -> iss::abstractISS::UniquePtr override
                {
                    iss::qemu::m_simulated_cpu_number++;
                    return std::make_unique<iss::qemu::qemuISS>();
                }

            public:
                // Parallelized Tick Count
                sparta::Scheduler::Tick m_tick_quantum = 1000;

            private:
                // Sparta Clock and Scheduler
                sparta::ClockManager m_clock_manager;
                sparta::Scheduler m_global_scheduler;
                sparta::Clock::Handle m_global_clock;
                std::vector<sparta::Clock::Handle> m_thread_global_clocks;
                sparta::EventSet m_global_event_set;
                // Global Tick End Event
                std::unique_ptr<sparta::Event<sparta::SchedulingPhase::Tick>> m_global_tick_end_event;
                // Per-thread clock and scheduler
                std::vector<std::unique_ptr<sparta::Scheduler>> m_thread_local_schedulers;
                std::vector<sparta::Clock::Handle> m_thread_local_clocks;
                std::vector<std::unique_ptr<sparta::PayloadEvent<hartId_t>>> m_thread_tick_events;
                std::vector<std::thread> m_thread_local;
                // QEMU Interface Instance
                iss::qemu::qemuInterface::ptrType m_qemu_if;
            };

        } // namespace qemu
    }     // namespace system

} // namespace archXplore
