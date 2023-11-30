from archXplore import *

import sys

import archXplore

rtn = sparta.RootTreeNode()

scheduler = sparta.Scheduler()

device_node = MyDeviceComponent(rtn)

clk = sparta.Clock("clock", scheduler)

rtn.setClock(clk)

ps = sparta.PortSet(rtn, "out_ports")

ps.a_delay_out = sparta.DataOutPort_uint32(ps, "a_delay_out")

rtn.enterConfiguring()
rtn.enterFinalized()

ps.a_delay_out << device_node.Ports.a_delay_in

rtnInfoTap = rtn.attachTap("info", sys.stdout)

scheduler.finalize()

ps.a_delay_out.send(1234)

print(device_node.Params.my_device_param)
# device_node.Params.my_device_param = False
# print(device_node.Params.my_device_param)

scheduler.run()



rtn.enterTeardown()
