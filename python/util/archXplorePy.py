from archXplore import *

import sys

import archXplore

print(dir(archXplore))

rtn = sparta.RootTreeNode()

scheduler = sparta.Scheduler()

device_node = MyDeviceNode(rtn)

clk = sparta.Clock("clock", scheduler)

rtn.setClock(clk)

ps = sparta.PortSet(rtn, "out_ports")

ps.a_delay_out = sparta.DataOutPort_uint32(ps, "a_delay_out")

ps.a_delay_out2 = sparta.DataOutPort_uint32(ps, "a_delay_out2")

def registerPorts(treenode : sparta.TreeNode) : 
    for port in treenode.getAllChildren() : 
        name = port.getName()
        treenode.__setattr__(name, treenode.getChildAsPort(name))
    
def travel(treenode : sparta.TreeNode, level : int, registerPorts : bool) : 
    for subnode in treenode.getAllChildren() : 
        print(level * "\t", subnode.getName())
        if(registerPorts) :
            treenode.__setattr__(subnode.getName(), treenode.getChildAsPort(subnode.getName()))
        else :
            treenode.__setattr__(subnode.getName(), subnode)   
            
        if(subnode.getName() == "ports") :
            travel(treenode.__getattribute__(subnode.getName()), level + 1, True)
        else :
            travel(treenode.__getattribute__(subnode.getName()), level + 1, False)

# def bindParams(treenode : sparta.TreeNode) : 
#     for param in treenode.getAllChildren() : 
#         name = param.getName()
#         treenode.__setattr__(name, treenode.getChild(name))


rtn.enterConfiguring()
rtn.enterFinalized()

travel(rtn, 0, False)

rtn.ports.a_delay_out << rtn.MyDeviceNode.ports.a_delay_in

info_log = sparta.PyLogTap(device_node, "info", sys.stdout)

scheduler.finalize()

rtn.ports.a_delay_out.send(1234)

scheduler.run()

rtn.enterTeardown()
