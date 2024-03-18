#ifndef __AGU_IF_HPP__
#define __AGU_IF_HPP__
#include <cinttypes>
namespace archXplore{
namespace cache{

class AGUIf{
public:
    virtual inline auto calcVAddr(const uint64_t& op1, const uint64_t& op2) const -> uint64_t= 0;
    virtual inline auto calcTag(const uint64_t& addr) const -> uint64_t= 0;
    virtual inline auto calcIndex(const uint64_t& addr) const -> uint64_t= 0;
    virtual inline auto calcLineAddr(const uint64_t& addr) const -> uint64_t= 0;
    virtual inline auto calcLineOffset(const uint64_t& addr) const -> uint64_t= 0;
}; // class AGUIf

} // namespace cache
} // namespace archXplore
#endif // __AGU_IF_HPP__