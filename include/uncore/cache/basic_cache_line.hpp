#ifndef __BASIC_CACHE_LINE_HPP__
#define __BASIC_CACHE_LINE_HPP__
#include <cinttypes>
namespace archXplore{
namespace uncore{
enum BasicCacheLineState{
    InValid,
    Shared,
    Owned,
    Exclusive,
    Modified
}; // enum BasicCacheLineState

class BasicCacheLine{
public: 
    BasicCacheLine() = default;
    BasicCacheLine(const BasicCacheLine&) = default;
    BasicCacheLine& operator=(const BasicCacheLine&) = default;
    virtual ~BasicCacheLine() = default;

    auto setTag(const uint64_t& t) -> void {m_tag = t;}
    auto getTag() const -> uint64_t {return m_tag;}
    auto setAddr(const uint64_t& a) -> void {m_addr = a;}
    auto getAddr() const -> uint64_t {return m_addr;}

    virtual auto set(const uint64_t& addr, const uint64_t& tag) -> void{
        m_tag = tag;
        m_addr = addr;
        m_state = BasicCacheLineState::Modified;
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
    inline auto setModified() -> void {m_state = BasicCacheLineState::Modified;}
    inline auto setInValid() -> void {m_state = BasicCacheLineState::InValid;}
    inline auto setState(const BasicCacheLineState& s) -> void {m_state = s;}
    inline auto isValid() const -> bool {return !(m_state == BasicCacheLineState::InValid);}
    inline auto isDirty() const -> bool {return (m_state == BasicCacheLineState::Modified);}
protected:
    uint64_t m_tag = 0;
    uint64_t m_addr = 0;
    BasicCacheLineState m_state = BasicCacheLineState::InValid; 
}; // class BasicCacheLine

} // namespace uncore
} // namespace archXplore
#endif // __BASIC_CACHE_LINE_HPP__