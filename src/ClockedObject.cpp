#include "ClockedObject.hpp"
#include "system/AbstractSystem.hpp"

namespace archXplore
{

    ClockedObject::ClockedObject(TreeNode *parent, const std::string &name)
        : TreeNode(parent, name, name + " clocked object"){};

    ClockedObject::~ClockedObject(){};

    ClockedObject *ClockedObject::setClockDomain(const SchedulePhase_t &phase, const uint32_t &rank, const sparta::Clock::Frequency &freq)
    {
        m_clock_domain_set = true, m_phase = phase, m_freq = freq, m_rank = rank;
        return this;
    };

    ClockedObject *ClockedObject::setFrequency(const sparta::Clock::Frequency &freq)
    {
        return setClockDomain(m_phase, m_rank, freq);
    };

    ClockedObject *ClockedObject::setRank(const uint32_t &rank)
    {
        return setClockDomain(m_phase, rank, m_freq);
    };

    ClockedObject *ClockedObject::setPhase(const SchedulePhase_t &phase)
    {
        return setClockDomain(phase, m_rank, m_freq);
    };

    ClockedObject *ClockedObject::toBoundPhase()
    {
        return setPhase(BOUND_PHASE);
    };

    ClockedObject *ClockedObject::toWeavePhase()
    {
        return setPhase(WEAVE_PHASE);
    };

    int32_t ClockedObject::getRank()
    {
        if (isConfigured())
        {
            return m_rank;
        }
        else
        {
            if (m_rank == -1)
            {
                sparta::TreeNode *parent = getParent();
                while (parent)
                {
                    ClockedObject *parent_obj = dynamic_cast<ClockedObject *>(parent);
                    if (parent_obj)
                    {
                        return parent_obj->getRank();
                    }
                    else
                    {
                        parent = parent->getParent();
                    }
                }
                return -1;
            }
            else
            {
                return m_rank;
            }
        }
        return m_rank;
    };

    sparta::Clock::Frequency ClockedObject::getClockFrequency()
    {
        if (isConfigured())
        {
            return m_freq;
        }
        else
        {
            if (m_freq == -1)
            {
                sparta::TreeNode *parent = getParent();
                while (parent)
                {
                    ClockedObject *parent_obj = dynamic_cast<ClockedObject *>(parent);
                    if (parent_obj)
                    {
                        return parent_obj->getClockFrequency();
                    }
                    else
                    {
                        parent = parent->getParent();
                    }
                }
                return -1;
            }
            else
            {
                return m_freq;
            }
        }
        return m_freq;
    };

    SchedulePhase_t ClockedObject::getPhase()
    {
        if (isConfigured())
        {
            return m_phase;
        }
        else
        {
            if (m_phase == SchedulePhase_t::UNKNOWN_PHASE)
            {
                sparta::TreeNode *parent = getParent();
                while (parent)
                {
                    ClockedObject *parent_obj = dynamic_cast<ClockedObject *>(parent);
                    if (parent_obj)
                    {
                        return parent_obj->getPhase();
                    }
                    else
                    {
                        parent = parent->getParent();
                    }
                }
                return SchedulePhase_t::UNKNOWN_PHASE;
            }
            else
            {
                return m_phase;
            }
        }
        return m_phase;
    };

    void ClockedObject::onConfiguring_()
    {
        if (m_clock_domain_set)
        {
            // Propagate the clock domain from parent to children
            m_phase = getPhase();
            m_freq = getClockFrequency();
            m_rank = getRank();
            auto sys = archXplore::system::AbstractSystem::getSystemPtr();
            sys->registerClockDomain(this, m_phase, m_rank, m_freq);
        }
    };

    void ClockedObject::onBindTreeEarly_()
    {
        // Build topology for this object
        buildTopology();
    };

} // namespace archXplore
