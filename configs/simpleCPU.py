import archXplore
from archXplore import *

import sys
import time

system = system.qemuSystem()

system.cpus = [simpleCPU(system, "simpleCPU" + str(i)).setRank(i+1) for i in range(3)]

system.workload = "/opt/riscvBenchSuite/spec2006/benchspec/CPU2006/462.libquantum/exe/libquantum_base.riscv"
system.arguments = ["33", "5"]

system.build()

print("hello")
print(system.cpus[0].Statistics.cycle.get())


for i in range(len(system.cpus)) :
    system.cpus[i].attachTap("info", "cpu{}_log.txt".format(i))
    
    
start = time.perf_counter()

system.run()

end = time.perf_counter()

print(111)
print(end-start)

