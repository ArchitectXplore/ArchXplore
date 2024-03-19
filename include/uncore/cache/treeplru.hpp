#ifndef __TREEPLRU_HPP__
#define __TREEPLRU_HPP__
#include "replacement_if.hpp"
#include <bitset>
#include "sparta/utils/MathUtils.hpp"
#include "sparta/utils/SpartaAssert.hpp"
namespace archXplore{
namespace uncore{
#define MAX_NUM_WAYS 256
class TreePLRU: public ReplacementIf{
protected:
    const uint32_t _num_tree_levels;
    std::bitset<MAX_NUM_WAYS> _plru_bits = 0;
public:
    TreePLRU(const uint32_t& num_ways):
        ReplacementIf(num_ways),
        _num_tree_levels(sparta::utils::floor_log2<uint32_t>(num_ways))
    {
        reset();
    }
    ~TreePLRU(){}

    virtual auto reset() -> void override {
        _plru_bits = 0;
    }
    virtual auto clone() const -> ReplacementIf*  override {
        return new TreePLRU(_num_ways);
    }
    virtual auto touchLRUWay(const uint32_t& way) -> void override{
        uint32_t idx     = way + _num_ways;
        for (uint32_t i=0; i<_num_tree_levels; ++i)
        {
            bool lru_is_to_the_right = idx & 0x1;
            idx = idx >> 1;
            _plru_bits[idx] = lru_is_to_the_right;
        }
    }
    virtual auto touchMRUWay(const uint32_t& way) -> void override{
        uint32_t idx     = way + _num_ways;
        for (uint32_t i=0; i<_num_tree_levels; ++i)
        {
            bool mru_is_to_the_right = idx & 0x1;
            idx = idx >> 1;

            // We need to invert mru_is_to_the_right because the tree is lru
            _plru_bits[idx] = !mru_is_to_the_right;
        }
    }    
    virtual auto lock() -> void override{

    }
    virtual auto getLRUWay() -> uint32_t override{
        uint32_t idx = 1;
        for (uint32_t i=0; i<_num_tree_levels; ++i)
        {
            idx = 2*idx + _plru_bits[idx];
        }

        uint32_t way = idx - _num_ways;
        return way;
    }
    virtual auto getMRUWay() -> uint32_t override{
        uint32_t idx = 1;
        for (uint32_t i=0; i<_num_tree_levels; ++i)
        {
            // Since this is an LRU tree, and we are looking for the
            // MRU way, we need flip the lru bit
            idx = 2*idx + !_plru_bits[idx];
        }

        uint32_t way = idx - _num_ways;

        return way;
    }
}; // class TreePLRU


} // namespace uncore
} // namespace archXplore
#endif // __TREEPLRU_HPP__