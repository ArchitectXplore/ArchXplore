from archXplore import *

import sys

import archXplore


a = A()
b1 = B()
b1.abc = 123
bptrOfa = a.getBptr()
bptrOfa = b1
a.printAbcOfBptr()