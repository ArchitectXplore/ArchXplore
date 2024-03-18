#ifndef __REPLACEMENT_IF_HPP__
#define __REPLACEMENT_IF_HPP__
#include <cinttypes>
namespace archXplore{
namespace cache{

class ReplacementIf{
protected:
    uint32_t _num_ways;
    uint32_t _way_mask;
public:
    ReplacementIf(const uint64_t& num_ways) : _num_ways(num_ways), _way_mask(num_ways - 1){}
    virtual ~ReplacementIf(){}
    auto getNumWays() const -> uint64_t {return _num_ways;}
    virtual auto reset() -> void = 0;
    virtual auto clone() const -> ReplacementIf*  = 0;
    virtual auto touchLRUWay(const uint32_t& way) -> void = 0;
    virtual auto touchMRUWay(const uint32_t& way) -> void = 0;
    virtual auto lock() -> void = 0;
    virtual auto getLRUWay() -> uint32_t = 0;
    virtual auto getMRUWay() -> uint32_t = 0;
}; // class ReplacementIf

} // namespace cache
} // namespace archXplore
#endif // __REPLACEMENT_IF_HPP__