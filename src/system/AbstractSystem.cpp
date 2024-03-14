#include "system/AbstractSystem.hpp"

namespace archXplore
{
    namespace system
    {
        AbstractSystem::AbstractSystem() : AbstractSystem(sparta::Clock::Frequency(1000)){};

        AbstractSystem::AbstractSystem(const sparta::Clock::Frequency &freq)
            : ClockedObject(nullptr, "SystemClockedObject"),
              m_root_node("SystemRoot"), m_global_event_set(this),
              m_info_logger(this, INFO_LOG, getName() + " Info Messages"),
              m_debug_logger(this, DEBUG_LOG, getName() + " Debug Messages"),
              m_warn_logger(this, WARN_LOG, getName() + " Warning Messages"),
              m_system_freq(freq), m_bound_weave_interval(1000 * freq),
              m_bound_weave_event(&m_global_event_set, "BoundWeaveEvent",
                                  CREATE_SPARTA_HANDLER(AbstractSystem, handleBoundWeaveEvent))
        {
            // Add AbstractSystem as a child of the root node
            m_root_node.addChild(this);
            // Set the global system pointer
            m_system_ptr = this;
            // Create clock domains 0 as global clock domain
            this->setClockDomain(SCHEDULE_PHASE, 0, freq);
        };

        AbstractSystem::~AbstractSystem(){};

        auto AbstractSystem::getCPUCount() -> uint32_t
        {
            return m_cpus.size();
        };

        auto AbstractSystem::executablePath() const -> std::string
        {
            char buffer[4096];
            std::string binary_path;
            std::string binary_dir;
            ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
            if (length != -1)
            {
                buffer[length] = '\0';
                binary_path = buffer;
                size_t lastSlash = binary_path.find_last_of("/\\");
                binary_dir = binary_path.substr(0, lastSlash);
            }
            else
            {
                std::cerr << "Can't get binary file location." << std::endl;
                std::abort();
            }
            return std::string(binary_dir);
        };

        auto AbstractSystem::getAppName() const -> std::string
        {
            auto pid = getpid();
            return "ArchXplore_" + std::to_string(pid);
        };

        auto AbstractSystem::newProcess(Process *process) -> Process *
        {
            process->pid = m_processes.size();
            m_processes.emplace_back(process);
            return process;
        };

        auto AbstractSystem::build() -> void
        {
            // Enter configuring state
            m_root_node.enterConfiguring();
            // Build clock domains and rank schedulers
            buildClockDomains();
            // Finalize tree and create resources
            m_root_node.enterFinalized();
            // Register instruction set simulator
            registerISS();
        };

        auto AbstractSystem::finalize() -> void
        {
            // Bind tree early
            m_root_node.bindTreeEarly();
            // Finalize scheduler
            m_schedule_phase.scheduler->finalize();
            for (auto &it : m_bound_phase)
            {
                auto &rank_domain = it.second;
                rank_domain.scheduler->finalize();
            }
            for (auto &it : m_weave_phase)
            {
                auto &rank_domain = it.second;
                rank_domain.scheduler->finalize();
            }
            // Construct thread pool when multi-threading is enabled
            if (m_max_threads == 0)
            {
                m_max_threads = std::max(m_bound_phase.size(), m_weave_phase.size());
            }
            if (m_max_threads > 0)
            {
                m_thread_pool = std::make_unique<utils::ThreadPool>(m_max_threads);
            }
            // Bind tree late
            m_root_node.bindTreeLate();
            // Enter teardown state
            m_root_node.enterTeardown();
            // Boot the system and enter teardown state
            bootSystem();
        };

        auto AbstractSystem::runPhaseEvent(PhaseDomain_t &phase) -> bool
        {
            bool finished = true;
            std::vector<std::future<bool>> futures;
            // Run bound phase schedulers in parallel
            if (SPARTA_EXPECT_TRUE(!phase.empty()))
            {
                for (auto &it : phase)
                {
                    futures.push_back(m_thread_pool->enqueue(
                        [&]
                        {
                            auto &scheduler = it.second.scheduler;
                            scheduler->run(m_bound_weave_interval, true, false);
                            return !scheduler->isFinished();
                        }));
                }
                // Wait for all tasks to complete
                for (auto &f : futures)
                {
                    if (f.get())
                    {
                        finished = false;
                    }
                }
                futures.clear();
            }
            return finished;
        };

        auto AbstractSystem::handleBoundWeaveEvent() -> void
        {

            bool bound_phase_finished = runPhaseEvent(m_bound_phase);
            bool weave_phase_finished = runPhaseEvent(m_weave_phase);

            if (bound_phase_finished && weave_phase_finished)
            {
                m_main_scheduler->stopRunning();
                m_main_scheduler->restartAt(m_main_scheduler->getCurrentTick() + m_bound_weave_interval - 1);
            }
            else
            {
                m_bound_weave_event.scheduleRelativeTick(m_bound_weave_interval, m_main_scheduler);
            }
        };

        auto AbstractSystem::buildRank(RankDomain_t &rank_domain) -> void
        {
            rank_domain.scheduler = std::make_unique<sparta::Scheduler>("Scheduler" + rank_domain.name);
            rank_domain.clock_manager = std::make_unique<sparta::ClockManager>(rank_domain.scheduler.get());
            rank_domain.clock_manager->makeRoot(&m_root_node, rank_domain.name + "RootClock");
            rank_domain.clock = rank_domain.clock_manager->makeClock(rank_domain.name + "_clock",
                                                                     rank_domain.clock_manager->getRoot(), m_system_freq);
            for (auto &domain : rank_domain.domains)
            {
                auto &freq = domain.first;
                auto &clock_domain = domain.second;
                clock_domain.clock = rank_domain.clock_manager->makeClock("clock_domain_" + std::to_string(uint64_t(freq)) + "Mhz",
                                                                          rank_domain.clock, freq);
                for (auto &node : clock_domain.nodes)
                {
                    node->setClock(clock_domain.clock.get());
                }
            }
            rank_domain.clock_manager->normalize();
        };

        auto AbstractSystem::buildClockDomains() -> void
        {
            buildRank(m_schedule_phase);

            for (auto &it : m_bound_phase)
            {
                buildRank(it.second);
            }

            for (auto &it : m_weave_phase)
            {
                buildRank(it.second);
            }

            m_main_scheduler = m_schedule_phase.scheduler.get();
            m_global_clock = m_schedule_phase.clock;

            // Enable multi-threaded simulation
            m_bound_weave_enabled = m_bound_phase.size() > 0 || m_weave_phase.size() > 0;
            if (m_bound_weave_enabled)
            {
                sparta::StartupEvent(&m_global_event_set, 
                    CREATE_SPARTA_HANDLER(AbstractSystem, handleBoundWeaveEvent)
                );
            }
        };

        auto AbstractSystem::run(sparta::Scheduler::Tick tick) -> void
        {
            if (SPARTA_EXPECT_FALSE(!m_finalized))
            {
                finalize();
                m_finalized = true;
            }
            if (m_bound_weave_enabled)
            {
                sparta_assert(((tick % m_bound_weave_interval == 0) || tick == sparta::Scheduler::INDEFINITE),
                              "Relative tick must be a multiple of bound weave interval\n");
            }
            m_main_scheduler->run(tick, true, false);
        }

        auto AbstractSystem::getElapsedTime() const -> double
        {
            return m_main_scheduler->getSimulatedPicoSeconds() * 1e-12;
        };

        auto AbstractSystem::registerISS() -> void
        {
            for (auto &cpu : m_cpus)
            {
                cpu->setISS(createISS());
            }
        };

        auto AbstractSystem::getCPUPtr(const HartID_t &tid) -> cpu::AbstractCPU *
        {
            sparta_assert(tid < m_cpus.size(), "Invalid hart id\n");
            return m_cpus[tid];
        };

        auto AbstractSystem::registerCPU(cpu::AbstractCPU *cpu) -> void
        {
            sparta_assert(cpu != nullptr, "CPU pointer is null\n");
            const HartID_t hart_id = m_cpus.size();
            cpu->m_hart_id = hart_id;
            cpu->m_freq = cpu->getClock()->getFrequencyMhz();
            m_cpus.push_back(cpu);
        };

        auto AbstractSystem::registerClockDomain(TreeNode *node, const SchedulePhase_t &phase,
                                                 const uint32_t &rank, const sparta::Clock::Frequency &freq) -> void
        {
            sparta_assert(!(phase == SchedulePhase_t::SCHEDULE_PHASE && rank != 0), "Invalid rank for global phase\n");
            switch (phase)
            {
            case SCHEDULE_PHASE:
                m_schedule_phase.name = "SchedulePhase_Rank";
                m_schedule_phase.domains[freq].nodes.push_back(node);
                break;
            case BOUND_PHASE:
                m_bound_phase[rank].name = "BoundPhase_Rank" + std::to_string(rank);
                m_bound_phase[rank].domains[freq].nodes.push_back(node);
                break;
            case WEAVE_PHASE:
                m_weave_phase[rank].name = "WeavePhase_Rank" + std::to_string(rank);
                m_weave_phase[rank].domains[freq].nodes.push_back(node);
                break;
            default:
                sparta_throw("Invalid schedule phase\n");
                break;
            }
        };

        // PseudoSystem implementation
        PseudoSystem::PseudoSystem() = default;
        PseudoSystem::~PseudoSystem() = default;
        auto PseudoSystem::bootSystem() -> void
        {
            sparta_throw("Can't boot pseudo system!");
        };
        auto PseudoSystem::createISS() -> std::unique_ptr<iss::AbstractISS>
        {
            sparta_throw("Can't create iss within pseudo system!");
            return nullptr;
        };
        auto PseudoSystem::registerCPU(cpu::AbstractCPU *cpu) -> void{};

        AbstractSystem *AbstractSystem::m_system_ptr = new PseudoSystem();

    } // namespace system

} // namespace archXplore
