from multiprocessing import Process
import gym
environment_name = 'BipedalWalker-v2'
from unified_neural_model import unified_neural_model

steps = 1000

import os
pid = os.getpid()
import psutil
py = psutil.Process(pid)

def f(agent):
    env = gym.make(environment_name)
    observation, reward = env.reset()
    
    sum_reward = 0
    for s in range(steps):
        action = agent.step(observation,reward)
        observation, reward, done, info = env.step()
        if done:
            break
            #observation, reward = env.reset()
        sum_reward += reward
    return sum_reward


def main(trials):
    env = gym.make(environment_name)
    inputs, outputs = env.observation_space.shape[0], env.action_space.shape[0]

    agent = unified_neural_model()
    agent.init(inputs,outputs)

    for g in range(generations):
        accum_reward = []
        for t in range(100):
            p = Process(targert=f,args=(agent,))
            p.start()
            reward = p.join()
            agent.endEpisode(reward)
            accum_reward.append(reward)
        print(g,max(accum_reward), "memory_usave:{}MB".format(py.memory_info()[0]/2**20))

    agent.saveAgent("dna_best_individual")
        
            

if __name__=='__main__':
    import sys
    generations = int(sys.argv[1])

    main(generations)
