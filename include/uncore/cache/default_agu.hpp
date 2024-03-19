#ifndef __DEFAULT_AGU_HPP__
#define __DEFAULT_AGU_HPP__
#include "agu_if.hpp"
#include "sparta/utils/MathUtils.hpp"
#include "sparta/utils/SpartaAssert.hpp"
#include <cassert>
namespace archXplore{
namespace uncore{
/* DefaultAddrDecoder: decodes a 64-bit address. 
* Assume one Block
* Assuming line_size==stride, the address is decoded as below
*    +--------------------------+------+------+
*    |tag                       |idx   |offset|
*    *--------------------------+------+------+
*/
class DefaultAGU: public AGUIf{
public:
        
    DefaultAGU(
        const uint64_t& size, // in kb
        const uint32_t& line_size, // in byte
        const uint32_t& stride, // in byte
        const uint32_t& num_ways 
    ):
        _size(size),
        _line_size(line_size),
        _stride(stride),
        _num_ways(num_ways)
    {
        assert(sparta::utils::is_power_of_2(line_size));
        assert(sparta::utils::is_power_of_2(stride));
        _num_sets = (size * 1024) / (line_size * num_ways);
        _offset_mask = line_size - 1;
        _addr_mask = ~uint64_t(_offset_mask);
        _index_mask = _num_sets - 1;
        _index_shift = sparta::utils::floor_log2(stride);
        _tag_shift = sparta::utils::floor_log2(_num_sets * stride);
    }

    virtual inline auto calcVAddr(const uint64_t& op1, const uint64_t& op2) const -> uint64_t override{
        return op1 + op2; // offset
    }
    virtual inline auto calcTag(const uint64_t& addr) const -> uint64_t override{
        return (addr >> _tag_shift);
    }
    virtual inline auto calcIndex(const uint64_t& addr) const -> uint64_t{
        return (addr >> _index_shift) & _index_mask;
    }
    virtual inline auto calcLineAddr(const uint64_t& addr) const -> uint64_t override{
        return addr & _addr_mask;
    }
    virtual inline auto calcLineOffset(const uint64_t& addr) const -> uint64_t override{
        return addr & _offset_mask;
    }
protected:
    uint64_t _size; // in kb
    uint32_t _line_size; // in byte
    uint32_t _stride; // in byte
    uint32_t _num_ways; 
    uint32_t _num_sets;
    uint32_t _tag_shift;
    uint32_t _index_shift;
    uint32_t _index_mask;
    uint64_t _offset_mask;
    uint64_t _addr_mask;
}; // class DefaultAGU

} // namespace uncore
} // namespace archXplore
#endif // __DEFAULT_AGU_HPP__