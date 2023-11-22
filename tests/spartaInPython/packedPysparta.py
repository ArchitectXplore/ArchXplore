import sys
sys.path.append(r"/root/projects/ArchXplore/tests/spartaInPython")
import pysparta



TreeNode = pysparta.TreeNode

def traverseNode(self, getChildAsDemand):
    children = self.getAllChildren()
    for child in children:
        name = child.getName()
        if not hasattr(self, name):
            setattr(self, name , getChildAsDemand(self, name))

def tarversePortSet(self):
    children = self.getAllChildren()
    for child in children:
        name = child.getName()
        # if not hasattr(self, name):
        #     if child.getDirection() == pysparta.Port.IN:
        #         setattr(self, name, getChildAsInPort(name))
        #     if child.getDirection() == pysparta.Port.OUT:
        #         setattr(self, name, getChildAsOutPort(name))

def traverseTree(self):
    children = self.getAllChildren()
    name = self.getName()
    if name == "ports":
        self.traverseNode(TreeNode.getChildAsPort)
    for child in children:
        name = child.getName()
        if not hasattr(self, name):
            setattr(self, name , child)
            if(name == "ports"):
                setattr(self, name, self.getChildAsPortSet("ports"))
        traverseTree(child)


TreeNode.traverseNode = traverseNode
TreeNode.traverseTree = traverseTree