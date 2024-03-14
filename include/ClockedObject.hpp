#pragma once

#include "sparta/sparta.hpp"
#include "sparta/log/Tap.hpp"
#include "sparta/utils/SpartaAssert.hpp"

namespace archXplore
{

    enum SchedulePhase_t
    {
        UNKNOWN_PHASE,
        SCHEDULE_PHASE,
        BOUND_PHASE,
        WEAVE_PHASE,
        NUM_PHASES
    };

    class ClockedObject : public sparta::TreeNode
    {
    public:
        /**
         * Constructor for the ClockedObject class.
         * @param parent The parent node of this object.
         * @param name The name of this object.
         * @param desc The description of this object.
         */
        ClockedObject(TreeNode *parent, const std::string &name);

        /**
         * Destructor for the ClockedObject class.
         */
        ~ClockedObject();

        /**
         * Set the clock domain for this object.
         * @param phase The phase of the clock domain.
         * @param rank The rank of the clock domain.
         * @param freq The frequency of the clock domain.
         * @return A pointer to this object.
         */
        ClockedObject *setClockDomain(const SchedulePhase_t &phase,
                                      const uint32_t &rank, const sparta::Clock::Frequency &freq);

        /**
         * Set the clock frequency for this object.
         * @param freq The frequency of the clock domain.
         * @return A pointer to this object.
         */
        ClockedObject *setFrequency(const sparta::Clock::Frequency &freq);

        /**
         * Set the clock rank for this object.
         * @param rank The rank of the clock domain.
         * @return A pointer to this object.
         */
        ClockedObject *setRank(const uint32_t& rank);

        /**
         * Set the clock phase for this object.
         * @param phase The phase of the clock domain.
         * @return A pointer to this object.
         */
        ClockedObject *setPhase(const SchedulePhase_t& phase);

        /**
         * Set the clock domain for this object to the bound phase.
         * @return A pointer to this object.
         */
        ClockedObject *toBoundPhase();

        /**
         * Set the clock domain for this object to the weave phase.
         * @return A pointer to this object.
         */
        ClockedObject *toWeavePhase();

        /**
         * Get the clock rank for this object.
         * @return The rank of the clock domain.
         */
        int32_t getRank();

        /**
         * Get the clock frequency for this object.
         * @return The frequency of the clock domain.
         */
        sparta::Clock::Frequency getClockFrequency();

        /**
         * Get the clock phase for this object.
         * @return The phase of the clock domain.
         */
        SchedulePhase_t getPhase();

        /**
         * @brief OnConfiguring is called by the TreeNode when it is being configured.
         * This is where we set up the clock domain and frequency.
         */
        void onConfiguring_() override;

        /**
         * @brief onBindTreeEarly is called by the TreeNode when it is being bound to the tree.
         * This is where we set up the clock domain and frequency.
         */
        void onBindTreeEarly_() override;

        /**
         * @brief buildTopology is called by the TreeNode when it is being bound to the tree.
         * This is where we build the topology of the object.
         */
        virtual void buildTopology(){};

    private:
        // Flag to check if the clock domain has been set
        bool m_clock_domain_set = false;
        // Private data members
        SchedulePhase_t m_phase = UNKNOWN_PHASE;
        int32_t m_rank = -1;
        sparta::Clock::Frequency m_freq = -1;
    };

    /**
     * This is a wrapper class for the ClockedObject class that allows us to
     * override the buildTopology function in Python.
     */
    class PyClockedObject : public ClockedObject
    {
    public:
        /* Inherit the constructors */
        using ClockedObject::ClockedObject;

        /* Trampoline (need one for each virtual function) */
        void buildTopology() override
        {
            PYBIND11_OVERRIDE(
                void,          /* Return type */
                ClockedObject, /* Parent class */
                buildTopology  /* Name of function in C++ (must match Python name) */
            );
        }
    };
} // namespace archXplore
