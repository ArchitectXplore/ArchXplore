import archXplore
from archXplore import *

import sys

qemu_boot_args = ['QEMU', 
                  "-plugin", 
                  "/home/lzhang/ArchXplore/build/libqemuInterface_plugin.so", 
                  "/home/lzhang/ArchXplore/tests/qemuInterface/hello"]

system = system.qemuSystem()

system.cpu = simpleCPU(system)

system.cpu.Params.tid = 0

system.cpu.Params.frequency = 1000

system.build()
    
system.boot(qemu_boot_args)

system.attachTap("info", sys.stdout)

system.run(1000)
