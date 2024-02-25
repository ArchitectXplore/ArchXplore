import archXplore
from archXplore import *

import sys
import time

system = system.qemuSystem()

# system = system.qemuParallelSystem()

system.cpus = [simpleCPU(system, "simpleCPU" + str(i)) for i in range(3)]

system.workload = "/root/pthread_test_riscv"

system.build()

for cpu in system.cpus : 
    cpu.attachTap("info", sys.stdout)

start = time.perf_counter()

system.run()

end = time.perf_counter()

print(end-start)

