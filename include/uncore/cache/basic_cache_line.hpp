#ifndef __BASIC_CACHE_LINE_HPP__
#define __BASIC_CACHE_LINE_HPP__
#include <cinttypes>
#include "agu_if.hpp"
namespace archXplore{
namespace cache{
enum BasicCacheLineState{
    InValid,
    Shared,
    Owned,
    Exclusive,
    Modified
}; // enum BasicCacheLineState
// ! implement MI in BasicCacheLine
class BasicCacheLine{
protected:
    uint64_t _tag = 0;
    uint64_t _addr = 0;
    BasicCacheLineState _state = BasicCacheLineState::InValid; 
public: 
    BasicCacheLine() = default;
    BasicCacheLine(const BasicCacheLine&) = default;
    BasicCacheLine& operator=(const BasicCacheLine&) = default;
    virtual ~BasicCacheLine() = default;

    auto setTag(const uint64_t& t) -> void {_tag = t;}
    auto getTag() const -> uint64_t {return _tag;}
    auto setAddr(const uint64_t& a) -> void {_addr = a;}
    auto getAddr() const -> uint64_t {return _addr;}

    virtual auto set(const uint64_t& addr, const uint64_t& tag) -> void{
        _tag = tag;
        _addr = addr;
        _state = BasicCacheLineState::Modified;
    }
    virtual auto unset() -> void {
        setInValid();
    }

    virtual auto read(const uint64_t& offset, const uint32_t& size, uint8_t* buff) const -> bool{
        return true; 
    }
    virtual auto write(const uint64_t& offset, const uint32_t& size, uint8_t* buff) -> bool {
        return true;
    }
    virtual inline auto setModified() -> void {_state = BasicCacheLineState::Modified;}
    virtual inline auto setInValid() -> void {_state = BasicCacheLineState::InValid;}
    virtual inline auto setState(const BasicCacheLineState& s) -> void {_state = s;}
    virtual inline auto isValid() const -> bool {return !(_state == BasicCacheLineState::InValid);}

}; // class BasicCacheLine
#endif // __BASIC_CACHE_LINE_HPP__