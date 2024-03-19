#include <iomanip>
#include <iostream>
namespace archXplore
{
namespace utils
{
/** Macro to simplify printing of hex values
 *  @param val The value to print
 *  @param width The number of digits to print
 */
#ifndef HEX
#define HEX(val, width) \
        std::setw(width) << std::setfill('0') << std::hex \
        << std::right << val << std::setfill(' ') << std::dec
#endif

/** Macro to simplify printing of 16 digit hex value
 *  @param val The value to print
 */
#ifndef HEX16
#define HEX16(val) HEX(val, 16)
#endif

/** Macro to simplify printing of 8 digit hex values
 *  @param val The value to print
 */
#ifndef HEX8
#define HEX8(val) HEX(val, 8)
#endif

#ifndef HEX2
#define HEX2(val) HEX(val, 2)
#endif
} // namespace utils
} // namespace archXplore