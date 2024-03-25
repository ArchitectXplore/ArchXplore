
import archXplore
from archXplore import *
import sys

rtn = sparta.RootTreeNode()

scheduler = sparta.Scheduler()

clk = sparta.Clock("clock", scheduler)

rtn.setClock(clk)

base_addr = 0x0
size = 0x100
num_cores = 2
num_itr = 100

t0 = RandomTester(rtn, "random_tester_0")
t0.Params.base_addr = base_addr
t0.Params.size = size
t0.Params.cpuid = 0
t0.Params.num_iters = num_itr
t0.Params.seed = 114514
t0.Params.req_stride = 5

t1 = RandomTester(rtn, "random_tester_1")
t1.Params.base_addr = base_addr
t1.Params.size = size
t1.Params.cpuid = 1
t1.Params.num_iters = num_itr
t1.Params.seed = 114514
t1.Params.req_stride = 3

dram = DRAMSim3(rtn, "dramsim3")
dram.Params.config_file = "/root/ArchXplore/ext/DRAMsim3/configs/DDR3_4Gb_x4_1600.ini"
dram.Params.config_file = "/root/ArchXplore/tests/uncoreTest"
dram.Params.num_ports = num_cores 

mem = FakeMemory(rtn, "fake_memory")
mem.Params.base_addr = base_addr
mem.Params.size = size

correct_mem = FakeMemory(rtn, "correct_memory")
correct_mem.Params.base_addr = base_addr
correct_mem.Params.size = size


rtn.enterConfiguring()
rtn.enterFinalized()

# tester 0 <> dram3
t0.Ports.lower_req_out << dram.Ports.upper_req_in_0
dram.Ports.upper_req_credit_out_0 << t0.Ports.lower_req_credit_in
dram.Ports.upper_resp_out_0 << t0.Ports.lower_resp_in
t0.Ports.lower_resp_credit_out << dram.Ports.upper_resp_credit_in_0

# tester 1 <> dram3
t1.Ports.lower_req_out << dram.Ports.upper_req_in_1
dram.Ports.upper_req_credit_out_1 << t1.Ports.lower_req_credit_in
dram.Ports.upper_resp_out_1 << t1.Ports.lower_resp_in
t1.Ports.lower_resp_credit_out << dram.Ports.upper_resp_credit_in_1

# dram3 <> mem
dram.Ports.lower_req_out << mem.Ports.upper_req_in
mem.Ports.upper_resp_out << dram.Ports.lower_resp_in

# tester <> mem
t0.Ports.debug_mem_req_out << correct_mem.Ports.upper_req_in
t1.Ports.debug_mem_req_out << correct_mem.Ports.upper_req_in

print(t0.Ports.lower_resp_in.isBound())

rtnInfoTap = rtn.attachTap("info", sys.stdout)
rtnInfoTap = rtn.attachTap("debug", sys.stdout)

scheduler.finalize()


scheduler.run()

rtn.enterTeardown()
