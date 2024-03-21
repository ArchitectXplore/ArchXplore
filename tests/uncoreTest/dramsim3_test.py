
import archXplore
from archXplore import *
import sys

rtn = sparta.RootTreeNode()

scheduler = sparta.Scheduler()



clk = sparta.Clock("clock", scheduler)

rtn.setClock(clk)

base_addr = 0x0
size = 0x1000
num_cores = 2

t0 = RandomTester(rtn, "random_tester 0")
t0.Params.base_addr = base_addr
t0.Params.size = size
t0.Params.cpuid = 0
t0.Params.num_iters = 10

t1 = RandomTester(rtn, "random_tester 1")
t0.Params.base_addr = base_addr
t0.Params.size = size
t0.Params.cpuid = 1
t0.Params.num_iters = 10

dram = DRAMSim3(rtn, "dramsim3")
dram.Params.config_file = "/root/ArchXplore/ext/DRAMsim3/configs/DDR3_4Gb_x4_1600.ini"
dram.Params.config_file = "/root/ArchXplore/tests/uncoreTest"
dram.Params.num_ports = num_cores 

mem = FakeMemory(rtn, "fake_memory")
mem.Params.base_addr = base_addr
mem.Params.size = size


rtn.enterConfiguring()
rtn.enterFinalized()




rtnInfoTap = rtn.attachTap("info", sys.stdout)
# rtnInfoTap = rtn.attachTap("debug", sys.stdout)

scheduler.finalize()


scheduler.run()

rtn.enterTeardown()
