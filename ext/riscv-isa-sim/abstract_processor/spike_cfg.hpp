#ifndef __SPIKE_CFH_HPP__
#define __SPIKE_CFH_HPP__
#include "../riscv/cfg.h"
#include "util.hpp"
class SpikeCfg: public cfg_t{
public:
    SpikeCfg(std::pair<reg_t, reg_t> default_initrd_bounds,
        const char *default_bootargs,
        const char *default_isa, const char *default_priv,
        const char *default_varch,
        const bool default_misaligned,
        const endianness_t default_endianness,
        const reg_t default_pmpregions,
        const std::vector<mem_cfg_t> &default_mem_layout,
        const std::vector<size_t> default_hartids,
        bool default_real_time_clint,
        const reg_t default_trigger_count,
        const reg_t& default_page_size 
        ):
    cfg_t(default_initrd_bounds,
        default_bootargs,
        default_isa, 
        default_priv,
        default_varch,
        default_misaligned,
        default_endianness,
        default_pmpregions,
        default_mem_layout,
        default_hartids,
        default_real_time_clint,
        default_trigger_count
    ), page_size(default_page_size)
    {
    }

    reg_t page_size = 1 << 12; // default is 4kb
};

#endif //  __SPIKE_CFH_HPP__
