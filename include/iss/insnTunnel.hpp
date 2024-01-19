#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <cassert>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace archXplore
{
namespace iss
{

const size_t DEFAULT_TUNNEL_RANK_NUMBER = 1;
const size_t DEFAULT_TUNNEL_RANK_DEPTH = 10000;


template <typename dataType>
class tunnelRank{
private:
    const size_t m_rank_depth;
    bool m_producing;
    bool m_consuming;
    std::mutex m_lock;
    std::condition_variable m_cond;
    std::deque<dataType> m_rank;
public:
    tunnelRank(const size_t depth) : 
        m_rank_depth(depth), m_producing(true), m_consuming(false){};
    tunnelRank(const tunnelRank<dataType>& that) : 
        m_rank_depth(that.m_rank_depth), m_producing(that.m_producing), m_consuming(that.m_consuming){};
    ~tunnelRank(){};
    bool is_full(){
        return m_rank.size() == m_rank_depth;
    };
    bool is_empty(){
        return m_rank.size() == 0;
    };
    bool is_consuming(){
        // assert(m_consuming ^ m_producing);
        return m_consuming;
    };
    bool is_producing(){
        // assert(m_consuming ^ m_producing);
        return m_producing;
    };
    void consumer_lock(){
        std::unique_lock<std::mutex> lock(m_lock);
        while(is_producing()){
            m_cond.wait(lock);
        }
    }
    void producer_lock(){
        std::unique_lock<std::mutex> lock(m_lock);
        while(is_consuming()){
            m_cond.wait(lock);
        }
    }
    void consumer_unlock(){
        std::unique_lock<std::mutex> lock(m_lock);
        m_consuming = false;
        m_producing = true;
        m_cond.notify_one();
    };
    void producer_unlock(){
        std::unique_lock<std::mutex> lock(m_lock);
        m_consuming = true;
        m_producing = false;
        m_cond.notify_one();
    };
    void produce(const dataType& insn){
        assert(m_rank.size() < m_rank_depth);
        m_rank.emplace_back(insn);
    };
    
    void consume(dataType& insn){
        assert(m_rank.size() > 0);
        insn = m_rank.front();
        m_rank.pop_front();
    };

};

template <typename InstructionType>
class insnTunnel
{
// Constructors
public:
    insnTunnel(
        size_t rankNumber = DEFAULT_TUNNEL_RANK_NUMBER, 
        size_t rankDepth = DEFAULT_TUNNEL_RANK_DEPTH
    ) : 
        m_tunnel(rankNumber,tunnelRank<InstructionType>(rankDepth))
    {
        m_rank_head = m_tunnel.begin();
        m_rank_tail = m_tunnel.begin();
    };
    insnTunnel(const insnTunnel<InstructionType>& that) 
    : m_tunnel(that.m_tunnel){
        m_rank_head = m_tunnel.begin();
        m_rank_tail = m_tunnel.begin();
    };
    ~insnTunnel(){};
    insnTunnel<InstructionType>& operator=(const insnTunnel<InstructionType>& that){
        return insnTunnel(that);
    };
public:
    void push(const InstructionType& insn) {
        // fprintf(stderr,"PRODUCER TRY LOCK RANK %ld -> IS CONSUMING %d\n", m_rank_tail.ptr, curRank.is_consuming());
        m_rank_tail->producer_lock();
        // fprintf(stderr,"PRODUCER LOCK RANK %ld\n", m_rank_tail.ptr);
        m_rank_tail->produce(insn);
        // fprintf(stdout,"PUSH RANK %ld -> %ld\n", m_rank_tail.ptr, insn);
        if(m_rank_tail->is_full()){
            m_rank_tail->producer_unlock();
            // fprintf(stderr,"PRODUCER UNLOCK RANK %ld\n", m_rank_tail.ptr);
            if(++m_rank_tail == m_tunnel.end()){
                m_rank_tail = m_tunnel.begin();
            }
        }
    };
    void pop(InstructionType& insn) {
        tunnelRank<InstructionType>& curRank = *m_rank_head;
        // fprintf(stderr,"CONSUMER TRY LOCK RANK %ld -> IS PRODUCINE %d\n", m_rank_head.ptr, curRank.is_producing());
        m_rank_head->consumer_lock();
        m_rank_head->consume(insn);
        if(m_rank_head->is_empty()){
            m_rank_head->consumer_unlock();
            if(++m_rank_head == m_tunnel.end()){
                m_rank_head = m_tunnel.begin();
            }
        }
        // fprintf(stderr,"CONSUMER LOCK RANK %ld\n", m_rank_head.ptr);
    };
    bool is_tunnel_empty(){
        for(auto it = m_tunnel.begin(); it != m_tunnel.end(); it++){
            if(!it->is_empty()){
                return false;
            }
        }
        return true;
    };
    bool is_tunnel_full(){
        for(auto it = m_tunnel.begin(); it != m_tunnel.end(); it++){
            if(!it->is_full()){
                return false;
            }
        }
    };
    void producer_exit() {
        m_rank_tail->producer_unlock();
    };

private:
    typename std::deque<tunnelRank<InstructionType>>::iterator m_rank_head;
    typename std::deque<tunnelRank<InstructionType>>::iterator m_rank_tail;
    std::deque<tunnelRank<InstructionType>> m_tunnel;
};



} // namespace iss
} // namespace archXplore

