#include "../abstract_processor/memif.hpp"
#include "../riscv/cfg.h"
#include <assert.h>
#include <vector>
#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
class FakeUncore:public archXplore::MemIf::UncoreIf{
public:
    struct MemChunk{
        std::string name = "default";
        uint64_t base;
        uint64_t size;
        char* mem;
        char* getMem(const uint64_t& addr, const uint64_t& s){
            if(addr < base || base + size <= addr+s ){
                std::stringstream ss;
                ss << "Bad addr access:";
                ss << "addr " << std::hex << addr << " ";
                ss << "size " << std::hex << s << " ";
                ss << "@ memchunk: " << name << " ";
                ss << "base " << std::hex << base << " ";
                ss << "size " << std::hex << size << " ";
                throw ss.str();
            }
            return mem + (addr - base);
        }
        bool hit(const uint64_t& addr, const uint64_t& s){
            return (base <= addr) && (addr + s < base + size);
        }
        MemChunk(const mem_cfg_t& cfg){
            base = cfg.get_base();
            size = cfg.get_size();
            mem = new char[size];
            memset(mem, 0, size);
        }
        ~MemChunk(){
            delete mem;
        }

    };
    std::vector<std::shared_ptr<MemChunk>> _mem;
    uint64_t tohost_paddr = 0;
    FakeUncore(const std::vector<mem_cfg_t>& cfg_vec, const uint64_t& tohost): tohost_paddr(tohost){
        for(auto cfg: cfg_vec){
            _mem.push_back(std::make_shared<MemChunk>(cfg));
        }
    }
    ~FakeUncore(){

    }

    char* getMem(const uint64_t& addr, const uint64_t& s){
        for(auto mem : _mem){
            if(!mem->hit(addr, s)) continue;
            return mem->getMem(addr, s);
        }
        std::stringstream ss;
        ss << "Bad addr access:";
        ss << "addr " << std::hex << addr << " ";
        ss << "size " << std::hex << s << " ";
        throw ss.str();
    }
    uint64_t peekToHost(){
        char* mem = getMem(tohost_paddr, 8);
        return *((uint64_t*)mem);
    }
    void setbyte(const uint64_t& addr, const uint8_t& val){
        char* mem = getMem(addr, 1);
        *mem = val;
    }
    uint8_t getbyte(const uint64_t& addr){
        char* mem = getMem(addr, 1);
        return (uint8_t)*mem;
    }

    void loadBin(std::string path, uint64_t addr){
        std::ifstream inFile(path, std::ios::in | std::ios::binary);
        if(!inFile){
            std::stringstream ss("");
            ss << "fail to load file: " << path;
            throw ss.str(); 
        }
        inFile.seekg(0, std::ios::end);
        int flen = inFile.tellg();
        inFile.seekg(0, std::ios::beg);

        char* mem = getMem(addr, flen);
        inFile.read(mem, flen);
    }
    void debugShowMem(uint64_t addr){
        char* mem = getMem(addr, 0x1000);
        for(int i = 0; i < 0x100; i ++){
            for(int j = 0; j < 16; j ++){
                std::cout << std::hex << uint32_t(mem[i * 16 + j]) << " ";
            }
            std::cout << std::endl;
        }
    }

    virtual void sendMemResp(archXplore::MemResp::PtrType resp) override final{
        throw "unimpl!";
    }
    virtual void recieveMemReq(archXplore::MemReq::PtrType req) override final{
        throw "unimpl!";
    }
    virtual void accessMemNoEffect(archXplore::MemReq::PtrType req, archXplore::MemResp::PtrType resp){
        uint64_t addr = req->pa;
        uint64_t size = req->size;
        char* mem = getMem(addr, size);
        resp->data = reinterpret_cast<uint64_t>(mem);
    }
};