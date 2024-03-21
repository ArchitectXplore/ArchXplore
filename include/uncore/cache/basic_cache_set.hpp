#ifndef __BASIC_CACHE_SET_HPP__
#define __BASIC_CACHE_SET_HPP__
#include <cinttypes>
#include <vector>
#include <memory>
#include <cassert>
#include <type_traits>
#include "agu_if.hpp"
#include "replacement_if.hpp"
#include "basic_cache_line.hpp"
#include "sparta/utils/MathUtils.hpp"

namespace archXplore{
namespace uncore{
template<class CacheLineT>
class BasicCacheSet{
    static_assert(std::is_base_of<BasicCacheLine ,CacheLineT>::value, "CacheLineT must be a subclass of BasicCacheLine");
protected:
    using VecIt = typename std::vector<CacheLineT>::iterator;
    using CVecIt = typename std::vector<CacheLineT>::const_iterator;
    std::unique_ptr<ReplacementIf> m_replacement_policy;
    std::vector<CacheLineT> m_ways;
    uint32_t m_num_ways;
public:
    BasicCacheSet(
        const uint32_t& num_ways,
        const CacheLineT& default_line,
        const AGUIf* agu,
        const ReplacementIf* rep
    ):
        m_num_ways(num_ways)
    {
        m_replacement_policy.reset(rep->clone());
        m_ways.resize(num_ways, default_line);
    }
    BasicCacheSet(const BasicCacheSet& BasicCacheSet):
        m_num_ways(BasicCacheSet.m_num_ways),
        m_ways(BasicCacheSet.m_ways)
    {
        m_replacement_policy.reset(BasicCacheSet.m_replacement_policy->clone());
    }
    BasicCacheSet& operator=(const BasicCacheSet& BasicCacheSet){
        m_num_ways = BasicCacheSet.m_num_ways;
        m_ways = BasicCacheSet.m_ways;
        m_replacement_policy.reset(BasicCacheSet.m_replacement_policy->clone());
        return *this;
    }
    BasicCacheSet() = default;
    auto peekLine(const uint64_t& tag) const -> const CacheLineT*  {
        const CacheLineT* line = nullptr;
        for(int i = 0; i < m_num_ways; i ++){
            if(m_ways[i].isValid() &&
                (m_ways[i].getTag() == tag)
            ){
                line = &m_ways[i];
                break;
            }
        }
        return line;
    }

    auto getLine(const uint64_t& tag) -> CacheLineT*{
        CacheLineT* line = nullptr;
        for(int i = 0; i < m_num_ways; i ++){
            if(m_ways[i].isValid() &&
                (m_ways[i].getTag() == tag)
            ){
                line = &m_ways[i];
                break;
            }
        }
        return line;
    }
    auto peekLineByWay(const uint32_t& way) const -> const CacheLineT* {
        assert(way < m_num_ways);
        return &m_ways[way];
    }
    auto getLineByWay(const uint32_t& way) -> CacheLineT*{
        assert(way < m_num_ways);
        return &m_ways[way];
    }

    auto peekLRULine() const -> const CacheLineT* {
        return &m_ways[m_replacement_policy->getLRUWay()];
    }
    auto getLRULine() -> CacheLineT*{
        return &m_ways[m_replacement_policy->getLRUWay()];
    }
    auto peekMRULine() const -> const CacheLineT* {
        return &m_ways[m_replacement_policy->getMRUWay()];
    }
    auto getMRULine() -> CacheLineT*{
        return &m_ways[m_replacement_policy->getMRUWay()];
    }

    auto getInvalidLineIdx() const -> uint32_t {
        for(int i = 0; i < m_num_ways; i ++)
            if(!m_ways[i].isValid()) return i;
        return m_num_ways;
    }

    auto getForReplacement() -> CacheLineT* {
        uint32_t victim_way = getInvalidLineIdx();
        if(victim_way == m_num_ways)
            victim_way = m_replacement_policy->getLRUWay();
        return &m_ways[victim_way];
    }

    auto getReplacementIf() -> ReplacementIf* {
        return m_replacement_policy.get();
    }

    auto peekReplacementIf() const -> const ReplacementIf*{
        return m_replacement_policy.get();
    }

    auto hasOpenWay() const -> bool {
        return (getInvalidLineIdx() != m_num_ways);
    }

    auto begin() -> VecIt {return m_ways.begin();}
    auto end() -> VecIt {return m_ways.end();}
    auto begin() const -> CVecIt {return m_ways.begin();}
    auto end() const -> CVecIt {return m_ways.end();}
    
}; // class BasicCacheSet


} // namespace uncore
} // namespace archXplore
#endif // __BASIC_CACHE_SET_HPP__