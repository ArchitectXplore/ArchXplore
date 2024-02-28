import archXplore
from archXplore import *

import sys
import time

system = system.qemuSystem()

system.cpus = [simpleCPU(system, "simpleCPU" + str(i)).setRank(i+1) for i in range(3)]

system.workload = "/root/matrix_mul_riscv"
system.arguments = ["100"]

system.build()

for i in range(len(system.cpus)) :
    system.cpus[i].attachTap("info", "cpu{}_log.txt".format(i))
    
    
start = time.perf_counter()

system.run()

end = time.perf_counter()

print(end-start)

