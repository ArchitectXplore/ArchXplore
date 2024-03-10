from archXplore import *

import time
import sys

class helloWorld(Process):
    def __init__(self):
        super().__init__()
        self.name = "helloWorld"
        self.max_harts = 1
        self.executable = "/root/hello_riscv"

class blackscholes(Process):
    def __init__(self, threads = 1):
        super().__init__()
        self.name = "blackscholes"
        self.max_harts = threads + 1
        self.executable = "/opt/riscvBenchSuite/parsec-benchmark/pkgs/apps/blackscholes/inst/amd64-linux.gcc/bin/blackscholes"
        self.arguments = [
            str(threads) , 
            "/opt/riscvBenchSuite/parsec-benchmark/pkgs/apps/blackscholes/run/in_16.txt",
            "/opt/riscvBenchSuite/parsec-benchmark/pkgs/apps/blackscholes/run/benchmark.out"
        ]
        
class libquantum(Process):
    def __init__(self):
        super().__init__()
        self.name = "libquantum"
        self.max_harts = 1
        self.executable = "/opt/riscvBenchSuite/spec2006/benchspec/CPU2006/462.libquantum/exe/libquantum_base.riscv"
        self.arguments = ["33", "5"]


class myCPU(SimpleCPU) :
    def __init__(self, system, name) :
        super().__init__(system, name)
        self.Params.fetch_width = 64

threads = 64

system = system.QemuSystem()

# system.attachTap("debug", sys.stdout)

# system.attachTap("trace", sys.stdout)

system.cpus = [myCPU(system, "SimpleCPU" + str(i)).setRank(i) for i in range(threads)]

system.interval = system.MAX_INTERVAL

system.build()

# for i in range(threads):
system.newProcess(blackscholes(16))
    
start = time.perf_counter()

system.run()

end = time.perf_counter()

total_instructions = 0
for cpu in system.cpus:
    total_instructions += cpu.Statistics.totalInstRetired

print("Time elapsed: ", end-start)
print("Total instructions executed: ", total_instructions)
print("Million instructions per second: ", total_instructions/1000000/(end-start))
    
