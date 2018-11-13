from module_wrapper import module

m = module(2, 2)
m.structuralMutation(200)
for i in range(100):
    m.weightMutation()

import numpy
observation = numpy.array([10,1],dtype=numpy.float64)
#print("check input:",m.check_input(observation,2))

for i in range(10):
    print(m.process(observation))

