import archXplore
from archXplore import *

import sys
import time

qemu_boot_args = ['QEMU', 
                  "-plugin", 
                  "/home/lzhang/ArchXplore/build/libqemuInterface_plugin.so", 
                #   "/home/lzhang/ArchXplore/tests/qemuInterface/hello"
                  "/home/lzhang/pthread_test_riscv"
                  ]

system = system.qemuSystem()

system.cpus = [simpleCPU(system, "simpleCPU" + str(i)) for i in range(3)]

system.build()
    
system.boot(qemu_boot_args)

for cpu in system.cpus : 
    cpu.attachTap("info", sys.stdout)

start = time.perf_counter()

system.run()

end = time.perf_counter()

print(end-start)

