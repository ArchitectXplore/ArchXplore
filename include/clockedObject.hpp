#pragma once

#include "sparta/sparta.hpp"
#include "sparta/log/Tap.hpp"
#include "sparta/utils/SpartaAssert.hpp"
#include "system/abstractSystem.hpp"

namespace archXplore
{

    class clockedObject : public sparta::TreeNode
    {
    public:
        /**
         * Constructor for the clockedObject class.
         * @param parent The parent node of this object.
         * @param name The name of this object.
         * @param desc The description of this object.
         */
        clockedObject(TreeNode *parent, const std::string &name, const std::string &desc)
            : TreeNode(parent, name, desc){};

        /**
         * Destructor for the clockedObject class.
         */
        ~clockedObject(){};

        /**
         * Attach a Python Tap to this object.
         * @param category The category of the Tap.
         * @param dest The destination of the Tap.
         * @return A pointer to the created Tap.
         */
        sparta::log::PyTap *attachTap(const std::string *category, pybind11::object &dest)
        {
            return new sparta::log::PyTap(this, category, dest);
        };

        /**
         * Set the clock domain for this object.
         * @param rank The rank of the clock domain.
         * @param freq The frequency of the clock domain.
         * @return A pointer to this object.
         */
        clockedObject *setClockDomain(uint32_t rank, const sparta::Clock::Frequency &freq)
        {
            sparta_assert(rank != 0, "Cannot use rank 0 as it is reserved for the global clock domain.");
            m_freq = freq;
            m_rank = rank;
            return this;
        };

        /**
         * Set the clock frequency for this object.
         * @param freq The frequency of the clock domain.
         * @return A pointer to this object.
         */
        clockedObject *setClockFrequency(const sparta::Clock::Frequency &freq)
        {
            m_freq = freq;
            return this;
        };
        
        /**
         * Set the clock rank for this object.
         * @param rank The rank of the clock domain.         
         * @return A pointer to this object.
         */
        clockedObject *setRank(uint32_t rank)
        {
            sparta_assert(rank != 0, "Cannot use rank 0 as it is reserved for the global clock domain.");
            m_rank = rank;
            return this;
        };
        
        void onConfiguring_() override
        {
            auto sys = archXplore::system::abstractSystem::getSystemPtr();
            sys->registerClockDomain(this, m_rank, m_freq);
        };

    private:
        // Private data members
        uint32_t m_rank = 0;
        sparta::Clock::Frequency m_freq = 1000;
    };

} // namespace archXplore
