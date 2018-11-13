from module_wrapper import module

m = module(2, 2)
m.structuralMutation(200)
for i in range(100):
    m.weightMutation()


import numpy
observation = numpy.array([1,1])
for i in range(10):
    print(m.process(observation))

