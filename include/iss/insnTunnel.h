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

template <typename InstructionType>
class insnTunnel
{
// Type definations
public:
    class tunnelRank{
    private:
        const size_t m_rank_depth;
        bool m_producing;
        bool m_consuming;
        std::mutex m_lock;
        std::condition_variable m_cond;
        std::deque<InstructionType> m_rank;
    public:
        tunnelRank(const size_t depth) : 
            m_rank_depth(depth), m_producing(true), m_consuming(false){};
        tunnelRank(const tunnelRank& that) : 
            m_rank_depth(that.m_rank_depth), m_producing(true), m_consuming(false){};
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
        void produce(const InstructionType& insn){
            assert(m_rank.size() < m_rank_depth);
            m_rank.push_back(insn);
        };
        InstructionType consume(){
            assert(m_rank.size() > 0);
            InstructionType insn = m_rank.front();
            m_rank.pop_front();
            return insn;
        }

    };
// Constructors
public:
    insnTunnel(
        size_t rankNumber = DEFAULT_TUNNEL_RANK_NUMBER, 
        size_t rankDepth = DEFAULT_TUNNEL_RANK_DEPTH) : 
        m_tunnel(rankNumber,tunnelRank(rankDepth)), m_rank_depth(rankDepth),
        m_producer_exited(false)
    {
        m_rank_head = m_tunnel.begin();
        m_rank_tail = m_tunnel.begin();
    };
    ~insnTunnel(){};
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
    void pop(bool& exit, InstructionType& insn) {
        tunnelRank& curRank = *m_rank_head;
        // fprintf(stderr,"CONSUMER TRY LOCK RANK %ld -> IS PRODUCINE %d\n", m_rank_head.ptr, curRank.is_producing());
        m_rank_head->consumer_lock();
        insn = m_rank_head->consume();
        if(m_rank_head->is_empty()){
            m_rank_head->consumer_unlock();
            if(++m_rank_head == m_tunnel.end()){
                m_rank_head = m_tunnel.begin();
            }
            exit = consumer_exit();
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
    }
    bool consumer_exit(){
        if(m_producer_exited) {
            return is_tunnel_empty();
        } else {
            return false;
        }
    };
    void producer_do_exit(){
        m_producer_exited = true;
        m_rank_tail->producer_unlock();
    };
    void lock_thread(){
        thread_lock.lock();
    };
    void unlock_thread(){
        thread_lock.unlock();
    };

private:
    bool m_producer_exited;
    std::mutex thread_lock;
    size_t m_rank_depth;
    typename std::vector<tunnelRank>::iterator m_rank_head;
    typename std::vector<tunnelRank>::iterator m_rank_tail;
    std::vector<tunnelRank> m_tunnel;
};



} // namespace iss
} // namespace archXplore

