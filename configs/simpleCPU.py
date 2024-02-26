import archXplore
from archXplore import *

import sys
import time

system = system.qemuSystem()

system.cpus = [simpleCPU(system, "simpleCPU" + str(i)) for i in range(3)]

system.workload = "/root/pthread_test_riscv"
# system.workload = "/root/hello_riscv"

system.build()

for i in range(len(system.cpus)) :
    system.cpus[i].attachTap("info", "cpu{}_log.txt".format(i))
    
start = time.perf_counter()

system.run()

end = time.perf_counter()

print(end-start)

