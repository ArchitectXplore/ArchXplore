import archXplore
from archXplore import *

import sys
import time

qemu_boot_args = ['QEMU', 
                  "-plugin", 
                  "/home/lzhang/ArchXplore/build_debug/libqemuInterface_plugin.so", 
                  "/home/lzhang/ArchXplore/tests/qemuInterface/hello"
                  ]

system = system.qemuSystem()

system.cpu = simpleCPU(system)

system.cpu.Params.tid = 0

system.cpu.Params.frequency = 1000

system.build()
    
system.boot(qemu_boot_args)

system.cpu.attachTap("info", sys.stdout)

start = time.perf_counter()

system.run()

end = time.perf_counter()

print(end-start)

system.cleanUp()

