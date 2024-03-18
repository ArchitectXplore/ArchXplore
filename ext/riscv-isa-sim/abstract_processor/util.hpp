#ifndef __SPIKE_UTIL_HPP__
#define __SPIKE_UTIL_HPP__
#include <algorithm>
#include <cstdint>
#include <string.h>
#include <strings.h>
#include <cinttypes>
#include <type_traits>
// TODO:  can not include this folder since it is a spike float
#include "../softfloat/softfloat_types.h"

namespace archXplore{

using reg_t = uint64_t;
using freg_t = float128_t;

const reg_t RV64_BYTE_MASK_R = 0xffffffffffffff00;
const reg_t RV64_BYTE_MASK = ~(RV64_BYTE_MASK_R);
const reg_t RV64_BYTE_HIGHEST = 0x80;

const reg_t RV64_HALF_MASK_R = 0xffffffffffff0000;
const reg_t RV64_HALF_MASK = ~(RV64_HALF_MASK_R);
const reg_t RV64_HALF_HIGHEST = 0x8000;

const reg_t RV64_WORD_MASK_R = 0xffffffff00000000;
const reg_t RV64_WORD_MASK = ~(RV64_WORD_MASK_R);
const reg_t RV64_WORD_HIGHEST = 0x80000000;

}
#endif // __SPIKE_UTIL_HPP__