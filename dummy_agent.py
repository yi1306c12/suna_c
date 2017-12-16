
from unified_neural_model import unified_neural_model
import numpy as np

@profile
def memory_test():
    inputs,outputs = 24,7
    agent = unified_neural_model()
    agent.init(inputs,outputs)

    trials = int(3000)
    retry = 10
    steps = 1000

    for i in range(trials):
        print(i)
        for r in range(retry*steps):
            agent.step(np.random.random(inputs),np.random.random())

        agent.endEpisode(0)

memory_test()