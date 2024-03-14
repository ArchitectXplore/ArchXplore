#include "ClockedObject.hpp"
#include "system/AbstractSystem.hpp"

namespace archXplore
{

    ClockedObject::ClockedObject(TreeNode *parent, const std::string &name)
        : TreeNode(parent, name, name + " clocked object"){};

    ClockedObject::~ClockedObject(){};

    ClockedObject *ClockedObject::setClockDomain(uint32_t rank, const sparta::Clock::Frequency &freq)
    {
        m_clock_domain_set = true;
        m_freq = freq;
        m_rank = rank;
        return this;
    };

    ClockedObject *ClockedObject::setClockFrequency(const sparta::Clock::Frequency &freq)
    {
        setClockDomain(m_rank, freq);
        return this;
    };

    ClockedObject *ClockedObject::setRank(uint32_t rank)
    {
        setClockDomain(rank, m_freq);
        return this;
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

    void ClockedObject::onConfiguring_()
    {
        if (m_clock_domain_set)
        {
            // Propagate the clock domain from parent to children
            m_freq = getClockFrequency();
            m_rank = getRank();
            auto sys = archXplore::system::AbstractSystem::getSystemPtr();
            sys->registerClockDomain(this, m_rank, m_freq);
        }
    };

    void ClockedObject::onBindTreeEarly_()
    {
        // Build topology for this object
        buildTopology();
    };

} // namespace archXplore
