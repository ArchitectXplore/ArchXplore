#ifndef __BASIC_CACHE_SET_HPP__
#define __BASIC_CACHE_SET_HPP__
#include <cinttypes>
#include "agu_if.hpp"
#include "replacement_if.hpp"
#include "basic_cache_line.hpp"
#include <vector>
#include <memory>
#include <cassert>
#include <type_traits>
#include "utils/MathUtils.hpp"

namespace archXplore{
namespace cache{
template<class CacheLineT>
class BasicCacheSet{
    static_assert(std::is_base_of<BasicCacheLine ,CacheLineT>::value, "CacheLineT must be a subclass of BasicCacheLine");
protected:
    using VecIt = typename std::vector<CacheLineT>::iterator;
    using CVecIt = typename std::vector<CacheLineT>::const_iterator;
    std::unique_ptr<ReplacementIf> _replacement_policy;
    std::vector<CacheLineT> _ways;
    uint32_t _num_ways;
public:
    BasicCacheSet(
        const uint32_t& num_ways,
        const CacheLineT& default_line,
        const AGUIf* agu,
        const ReplacementIf& rep
    ):
        _num_ways(num_ways)
    {
        _replacement_policy.reset(rep.clone());
        _ways.resize(num_ways, default_line);
    }
    BasicCacheSet(const BasicCacheSet& BasicCacheSet):
        _num_ways(BasicCacheSet._num_ways),
        _ways(BasicCacheSet._ways)
    {
        _replacement_policy.reset(BasicCacheSet._replacement_policy->clone());
    }
    BasicCacheSet() = default;
    auto peekLine(const uint64_t& tag) const -> const CacheLineT*  {
        const CacheLineT* line = nullptr;
        for(int i = 0; i < _num_ways; i ++){
            if(_ways[i].isValid() &&
                (_ways[i].getTag() == tag)
            ){
                line = &_ways[i];
                break;
            }
        }
        return line;
    }

    auto getLine(const uint64_t& tag) -> CacheLineT*{
        CacheLineT* line = nullptr;
        for(int i = 0; i < _num_ways; i ++){
            if(_ways[i].isValid() &&
                (_ways[i].getTag() == tag)
            ){
                line = &_ways[i];
                break;
            }
        }
        return line;
    }
    auto peekLineByWay(const uint32_t& way) const -> const CacheLineT* {
        assert(way < _num_ways);
        return &_ways[way];
    }
    auto getLineByWay(const uint32_t& way) -> CacheLineT*{
        assert(way < _num_ways);
        return &_ways[way];
    }

    auto peekLRULine() const -> const CacheLineT* {
        return &_ways[_replacement_policy->getLRUWay()];
    }
    auto getLRULine() -> CacheLineT*{
        return &_ways[_replacement_policy->getLRUWay()];
    }
    auto peekMRULine() const -> const CacheLineT* {
        return &_ways[_replacement_policy->getMRUWay()];
    }
    auto getMRULine() -> CacheLineT*{
        return &_ways[_replacement_policy->getMRUWay()];
    }

    auto getInvalidLineIdx() const -> uint32_t {
        for(int i = 0; i < _num_ways; i ++)
            if(!_ways[i].isValid()) return i;
        return _num_ways;
    }

    auto getForReplacement() -> CacheLineT* {
        uint32_t victim_way = getInvalidLineIdx();
        if(likely(victim_way == _num_ways))
            victim_way = _replacement_policy->getLRUWay();
        return &_ways[victim_way];
    }

    auto getReplacementIf() -> ReplacementIf* {
        return _replacement_policy.get();
    }

    auto peekReplacementIf() const -> const ReplacementIf*{
        return _replacement_policy.get();
    }

    auto hasOpenWay() const -> bool {
        return (getInvalidLineIdx() != _num_ways);
    }

    auto begin() -> VecIt {return _ways.begin();}
    auto end() -> VecIt {return _ways.end();}
    auto begin() const -> CVecIt {return _ways.begin();}
    auto end() const -> CVecIt {return _ways.end();}
    
}; // class BasicCacheSet


} // namespace cache
} // namespace archXplore
#endif // __BASIC_CACHE_SET_HPP__