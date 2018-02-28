from multiprocessing import Process,Queue
import gym
environment_name = 'BipedalWalker-v2'
from unified_neural_model import unified_neural_model

steps = 1000

import os
pid = os.getpid()
import psutil
py = psutil.Process(pid)

def f(agent,queue):
    env = gym.make(environment_name)
    observation, reward = env.reset(), 0
    
    sum_reward = 0
    for s in range(steps):
        action = agent.step(observation,reward)
        observation, reward, done, info = env.step(action)
        sum_reward += reward
        if done:
            observation, reward = env.reset(),0
    queue.put(sum_reward)

def main(trials):
    env = gym.make(environment_name)
    inputs, outputs = env.observation_space.shape[0], env.action_space.shape[0]

    agent = unified_neural_model()
    agent.init(inputs,outputs)

    q = Queue()
    for g in range(generations):
        accum_reward = []
        for t in range(100):
            p = Process(target=f,args=(agent,q))
            p.start()
            p.join()
            reward = q.get()
            agent.endEpisode(reward)
            accum_reward.append(reward)
        print(g,max(accum_reward), "memory_usave:{}MB".format(py.memory_info()[0]/2**20))

    agent.saveAgent("dna_best_individual")
        
            

if __name__=='__main__':
    import sys
    generations = int(sys.argv[1])

    main(generations)
