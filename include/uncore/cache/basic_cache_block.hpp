#ifndef __BASIC_CACHE_BLOCK_HPP__
#define __BASIC_CACHE_BLOCK_HPP__
namespace archXplore{
namespace cache{
#include "baisc_cache_set.hpp"
template <class CacheLineT>
class BasicCacheBlock{
    static_assert(std::is_base_of<BasicCacheLine ,CacheLineT>::value, "CacheLineT must be a subclass of BasicCacheLine");
protected:
    using CacheSetT = BasicCacheSet<CacheLineT>;
    using VecIt = typename std::vector<CacheSetT>::iterator;
    using CVecIt = typename std::vector<CacheSetT>::const_iterator;
    uint32_t _num_sets;
    std::vector<CacheSetT> _sets;
    const AGUIf* _aguif;
public:
    BasicCacheBlock(
        const uint32_t& num_sets,
        const uint32_t& num_ways,
        const CacheLineT& default_line,
        const AGUIf* aguif,
        const ReplacementIf& rep
    ):
        _num_sets(num_sets)
    {
        _sets.resize(num_sets);
        _aguif = aguif;
    }
    BasicCacheBlock() = default;

    auto setAGUIf(const AGUIf* aguif) -> void{
        _aguif = aguif;
    } 
    auto getAGUIf() const -> const AGUIf*{
        return _aguif;
    }

    inline auto peekCacheSet(const uint64_t& addr) const -> const CacheSetT * {
        uint32_t set_idx = _aguif->calcIndex(addr);
        assert(set_idx < _num_sets);
        return &_sets[set_idx];
    }
    inline auto getCacheSet(const uint64_t& addr)  -> CacheSetT * {
        uint32_t set_idx = _aguif->calcIndex(addr);
        assert(set_idx < _num_sets);
        return &_sets[set_idx];
    }
    inline auto peekCacheSetByIdx(const uint32_t& set_idx) const -> const CacheSetT * {
        assert(set_idx < _num_sets);
        return &_sets[set_idx];
    }
    inline auto getCacheSetByIdx(const uint32_t& set_idx) -> CacheSetT * {
        assert(set_idx < _num_sets);
        return &_sets[set_idx];
    }
    
    inline auto getLine(const uint64_t& addr) -> CacheLineT *{
        uint64_t tag = _aguif->calcTag(addr);
        return getCacheSet(addr)->getLine(tag);
    }
    inline auto peekLine(const uint64_t& addr) const -> const CacheLineT *{
        uint64_t tag = _aguif->calcTag(addr);
        return peekCacheSet(addr)->peekLine(tag);
    }
    inline auto getLineByIdxWay(const uint32_t& set_idx, const uint32_t& way) -> CacheLineT *{
        return _sets[set_idx]->getLineByWay(way);
    }
    inline auto peekLineByIdxWay(const uint32_t& set_idx, const uint32_t& way) const -> const CacheLineT *{
        return _sets[set_idx]->peekLineByWay(way);
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
        return _num_sets;
    }

    auto begin() -> VecIt {return _sets.begin();}
    auto end() -> VecIt {return _sets.end();}
    auto begin() const -> CVecIt {return _sets.begin();}
    auto end() const -> CVecIt {return _sets.end();}


}; // class BasicCacheBlock
#endif // __BASIC_CACHE_BLOCK_HPP__