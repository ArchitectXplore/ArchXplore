#include "../riscv/processor.h"
#include "../riscv/devices.h"
#include "../abstract_processor/SpikeAbstractProcessor.hpp"
#include "./fake_uncore.hpp"
#include "./fake_regfile.hpp"
#include "./util.hpp"
#include <iostream>
#include <memory>
#include <sstream>
using namespace std;


int main(int argc, char* argv[]){
    // TODO: rv64ud-p-move: tohost is 0x80002000
    const uint64_t program_start =   0x80000000;
    const uint64_t valid_space_len = 0x10000000;
    const uint64_t tohost_addr = 0x80001000;
    const uint64_t fromhost_addr = 0x80001008;
    const uint64_t test_addr = 0x80002000;
    const uint64_t test_init_addr = program_start;
    int return_flag = 0;

    std::string proj_root = "/work/stu/pguo/code/ArchXplore/ext/riscv-isa-sim";
    // std::string proj_root = getenv("PROJ_ROOT");

    std::vector<mem_cfg_t> mem_cfg { mem_cfg_t(program_start, valid_space_len) };
    std::vector<size_t> hartids = {0};
    SpikeCfg cfg(/*default_initrd_bounds=*/std::make_pair((reg_t)0, (reg_t)0),
            /*default_bootargs=*/nullptr,
            /*default_isa=*/DEFAULT_ISA,
            /*default_priv=*/DEFAULT_PRIV,
            /*default_varch=*/DEFAULT_VARCH,
            /*default_misaligned=*/false,
            /*default_endianness*/endianness_little,
            /*default_pmpregions=*/16,
            /*default_mem_layout=*/mem_cfg,
            /*default_hartids=*/std::vector<size_t>(),
            /*default_real_time_clint=*/false,
            /*default_trigger_count=*/4,
            /*default page size*/1<<12
            );
    isa_parser_t ip(cfg.isa(), cfg.priv());
    FILE* outputFile = fopen("./log.txt", "w");
    std::shared_ptr<FakeUncore> uncore = std::make_shared<FakeUncore>(mem_cfg, tohost_addr);
    const int NXPR = 32;
    const int NFPR = 32;
    bool trace_enable = false;
    if (argc > 2){
        std::string argv_2 = argv[2];
        if(argv_2 == "--trace"){
            trace_enable = true;
        }
    }
    std::shared_ptr<FakeRegFile<reg_t>> xprf = std::make_shared<FakeRegFile<reg_t>>(NXPR, true);
    std::shared_ptr<FakeRegFile<freg_t>> fprf = std::make_shared<FakeRegFile<freg_t>>(NFPR, false);
    archXplore::SpikeAbstractProcessor processor(
        &ip,
        &cfg,
        0,
        false,
        outputFile,
        cout,
        uncore,
        xprf,
        fprf,
        trace_enable
    );
    int steps = 100000;
    try{
        std::string testname(argv[1]);
        std::cout << testname << ": ";
        std::stringstream ss;
        ss << proj_root << "/isatest/";
        ss << testname << ".bin";
        
        
        uncore->loadBin(ss.str(), program_start);
        // uncore->debugShowMem(0x80004000);
        processor.reset();
        processor.setPc(program_start);
        while(steps > 0){
            steps -= archXplore::INTERLEAVE;
            // steps -= 1;
            processor.run(archXplore::INTERLEAVE);
            // processor.step(trace_enable);
            if(uncore->peekToHost() & 0x1 == 0x1)
                break;
        }

        // printf("%d\n", uncore->getbyte(tohost_addr));
        // uncore->setbyte(tohost_addr, 1);
        // printf("%d\n", uncore->getbyte(tohost_addr));
        // printf("%d\n", uncore->peekToHost());
    }
    catch(const char* s){
        std::cout << "test failed. Get Exception:\n";
        std::cout << s << std::endl;
    }

    uint32_t flag = uncore->peekToHost() >> 1;
    if(steps == 0){
        std::cout << "test timeout" << std::endl;
        return_flag = 2;
    }
    else if(flag == 0x0){
        std::cout << "test pass" << std::endl;
        return_flag = 1;
    }
    else{
        std::cout << "test failed @ " << flag << std::endl;
        return_flag = 3;
    }
    fclose(outputFile);
    return return_flag;
}