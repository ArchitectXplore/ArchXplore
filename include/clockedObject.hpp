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
        clockedObject(TreeNode *parent, const std::string &name)
            : TreeNode(parent, name, name + " clocked object"){};

        /**
         * Destructor for the clockedObject class.
         */
        ~clockedObject(){};

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

        /**
         * onConfiguring_ is called by the TreeNode when it is being configured.
         * This is where we register the clock domain with the system.
         */
        void onConfiguring_() override
        {
            auto sys = archXplore::system::abstractSystem::getSystemPtr();
            sys->registerClockDomain(this, m_rank, m_freq);
        };

        /**
         * onBindTreeEarly_ is called by the TreeNode when it is being bound to the tree.
         * This is where we call the topology builder if one has been registered.
         */
        void onBindTreeEarly_() override
        {
            buildTopology();
        };

        virtual void buildTopology()
        {
            std::cerr << SPARTA_CURRENT_COLOR_RED
                      << "\n[Error] Please implement the buildTopology function in your clockedObject derived class["
                      << getLocation() << "] "
                      << "in python!\n"
                      << SPARTA_CURRENT_COLOR_NORMAL
                      << std::endl;
            std::exit(-1);
        };

    private:
        // Private data members
        uint32_t m_rank = 0;
        sparta::Clock::Frequency m_freq = 1000;
    };

    /**
     * This is a wrapper class for the clockedObject class that allows us to
     * override the buildTopology function in Python.
     */
    class pyClockedObject : public clockedObject
    {
    public:
        /* Inherit the constructors */
        using clockedObject::clockedObject;

        /* Trampoline (need one for each virtual function) */
        void buildTopology() override
        {
            PYBIND11_OVERRIDE(
                void,          /* Return type */
                clockedObject, /* Parent class */
                buildTopology  /* Name of function in C++ (must match Python name) */
            );
        }
    };
} // namespace archXplore
