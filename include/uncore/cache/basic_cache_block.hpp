#ifndef __BASIC_CACHE_BLOCK_HPP__
#define __BASIC_CACHE_BLOCK_HPP__
#include "basic_cache_set.hpp"
namespace archXplore{
namespace uncore{
template <class CacheLineT>
class BasicCacheBlock{
    static_assert(std::is_base_of<BasicCacheLine ,CacheLineT>::value, "CacheLineT must be a subclass of BasicCacheLine");
protected:
    using CacheSetT = BasicCacheSet<CacheLineT>;
    using VecIt = typename std::vector<CacheSetT>::iterator;
    using CVecIt = typename std::vector<CacheSetT>::const_iterator;
    uint32_t m_num_sets;
    std::vector<CacheSetT> m_sets;
    const AGUIf* m_aguif;
public:
    BasicCacheBlock(
        const uint32_t& num_sets,
        const uint32_t& num_ways,
        const CacheLineT& default_line,
        const AGUIf* aguif,
        const ReplacementIf* rep
    ):
        m_num_sets(num_sets)
    {
        m_sets.resize(num_sets, CacheSetT(num_ways, default_line, aguif, rep));
        m_aguif = aguif;
    }
    BasicCacheBlock() = default;

    auto setAGUIf(const AGUIf* aguif) -> void{
        m_aguif = aguif;
    } 
    auto getAGUIf() const -> const AGUIf*{
        return m_aguif;
    }

    inline auto peekCacheSet(const uint64_t& addr) const -> const CacheSetT * {
        uint32_t set_idx = m_aguif->calcIndex(addr);
        assert(set_idx < m_num_sets);
        return &m_sets[set_idx];
    }
    inline auto getCacheSet(const uint64_t& addr)  -> CacheSetT * {
        uint32_t set_idx = m_aguif->calcIndex(addr);
        assert(set_idx < m_num_sets);
        return &m_sets[set_idx];
    }
    inline auto peekCacheSetByIdx(const uint32_t& set_idx) const -> const CacheSetT * {
        assert(set_idx < m_num_sets);
        return &m_sets[set_idx];
    }
    inline auto getCacheSetByIdx(const uint32_t& set_idx) -> CacheSetT * {
        assert(set_idx < m_num_sets);
        return &m_sets[set_idx];
    }
    
    inline auto getLine(const uint64_t& addr) -> CacheLineT *{
        uint64_t tag = m_aguif->calcTag(addr);
        return getCacheSet(addr)->getLine(tag);
    }
    inline auto peekLine(const uint64_t& addr) const -> const CacheLineT *{
        uint64_t tag = m_aguif->calcTag(addr);
        return peekCacheSet(addr)->peekLine(tag);
    }
    inline auto getLineByIdxWay(const uint32_t& set_idx, const uint32_t& way) -> CacheLineT *{
        return m_sets[set_idx]->getLineByWay(way);
    }
    inline auto peekLineByIdxWay(const uint32_t& set_idx, const uint32_t& way) const -> const CacheLineT *{
        return m_sets[set_idx]->peekLineByWay(way);
    }

    inline auto getLRULine(const uint64_t& addr) -> CacheLineT *{
        return getCacheSet(addr)->getLRULine();
    }
    inline auto peekLRULine(const uint64_t& addr) const -> const CacheLineT *{
        return peekCacheSet(addr)->peekLRULine();
    }

    inline auto getReplacementIf(const uint64_t& addr) -> ReplacementIf*{
        return getCacheSet(addr)->getReplacementIf();
    }
    inline auto peekReplacementIf(const uint64_t& addr) const -> const ReplacementIf*{
        return peekCacheSet(addr)->peekReplacementIf();
    }

    inline auto getNumSets() const -> uint32_t {
        return m_num_sets;
    }

    inline auto read(const uint64_t& addr, const uint32_t& size, uint8_t* data) const -> bool{
        const CacheLineT* line = peekLine(addr);
        if(line == nullptr){
            return false;
        }
        return line->read(addr, size, data);
    }
    inline auto write(const uint64_t& addr, const uint32_t& size, const uint8_t* data) -> bool{
        CacheLineT* line = getLine(addr);
        if(line == nullptr){
            return false;
        }
        return line->write(addr, size, data);
    }

    auto begin() -> VecIt {return m_sets.begin();}
    auto end() -> VecIt {return m_sets.end();}
    auto begin() const -> CVecIt {return m_sets.begin();}
    auto end() const -> CVecIt {return m_sets.end();}


}; // class BasicCacheBlock
} // namespace uncore
} // namespace archXplore
#endif // __BASIC_CACHE_BLOCK_HPP__