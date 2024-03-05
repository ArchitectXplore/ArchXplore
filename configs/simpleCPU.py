import archXplore
from archXplore import *

import sys
import time

system = system.qemuSystem()

system.cpus = [simpleCPU(system, "simpleCPU" + str(i)) for i in range(1)]

system.workload = "/opt/riscvBenchSuite/spec2006/benchspec/CPU2006/462.libquantum/exe/libquantum_base.riscv"

system.arguments = ["33", "5"]

system.build()
    
start = time.perf_counter()
print("m_cycle[totalCycle]: ", system.cpus[0].Statistics.totalCycle)

system.run()

end = time.perf_counter()

print(end-start)

