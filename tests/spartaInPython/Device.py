import sys
sys.path.append(r"/root/projects/ArchXplore/tests/spartaInPython")
import pysparta
import packedPysparta
from abc import ACB, abstractmethod

class Device:
    def __init__(self, name, desc, search_scope = None):
        self.rtn = packedPysparta.RootTreeNode(name, desc, search_scope)
        scheduler = packedPysparta.Scheduler()
        clk = packedPysparta.Clock(self.rtn, "clock", scheduler)
        self.rtn.traverseTree()
        self.rtn.clock.scheduler = scheduler



    @abstractmethod
    def buildTree(self):
        pass

    @abstractmethod
    def configureTree(self):
        self.rtn.enterConfgiure()

    def finalizeTree(self):
        self.rtn.enterFinalized()