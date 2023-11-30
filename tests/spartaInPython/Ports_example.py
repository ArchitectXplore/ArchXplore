from archXplore import *

import sys
import pybind11

import archXplore

rtn = sparta.RootTreeNode()

scheduler = sparta.Scheduler()

device_node = MyDeviceNode(rtn)

clk = sparta.Clock("clock", scheduler)

rtn.setClock(clk)

ps = sparta.PortSet(rtn, "out_ports")

ps.a_delay_out = sparta.DataOutPort_uint32(ps, "a_delay_out")

ps.a_delay_out2 = sparta.DataOutPort_uint32(ps, "a_delay_out2")

    
def travel(treenode : sparta.TreeNode, level : int) : 
    for subnode in treenode.getAllChildren() : 
        print(level * "\t", subnode.getName())
        if(subnode.getName() == "ports") :
            treenode.__setattr__(subnode.getName(), subnode.asPortSet())   
            ports : sparta.PortSet = treenode.__getattribute__(subnode.getName())
            for port in ports.getAllChildren() : 
                print((level+1) * "\t", port.getName())
                ports.__setattr__(port.getName(), ports.getPort(port.getName()))
        elif(subnode.getName() == "params") :
            treenode.__setattr__(subnode.getName(), subnode.asParamSet())   
            params : sparta.ParameterSet = treenode.__getattribute__(subnode.getName())
            for param in params.getAllChildren() : 
                print((level+1) * "\t", param.getName(), params.getParameter(param.getName()).getTypeName())
                subnode.__setattr__(param.getName(), params.getParameter(param.getName()))
        else:
            treenode.__setattr__(subnode.getName(), subnode)   
            travel(treenode.__getattribute__(subnode.getName()), level + 1)


rtn.enterConfiguring()
rtn.enterFinalized()

travel(rtn, 0)

rtn.ports.a_delay_out << rtn.MyDeviceNode.ports.a_delay_in

info_log = sparta.PyLogTap(device_node, "info", sys.stdout)

scheduler.finalize()

rtn.ports.a_delay_out.send(1234)

print(rtn.MyDeviceNode.params.my_device_param)
rtn.MyDeviceNode.params.my_device_param = False
print(rtn.MyDeviceNode.params.my_device_param)

scheduler.run()



rtn.enterTeardown()
