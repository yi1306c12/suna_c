from multiprocessing import Process,Queue
import gym
environment_name = 'BipedalWalker-v2'
gym.logger.set_level(40)#exclude waning message
from unified_neural_model import unified_neural_model

import os
pid = os.getpid()
import psutil
py = psutil.Process(pid)


def f(agent,queue,steps,retry):
    env = gym.make(environment_name)
    observation, reward = env.reset(), 0
    
    sum_reward = 0
    for r in range(retry):
        agent.resetAgent()
        observation, reward = env.reset(),0
        for s in range(steps):
            action = agent.step(observation,reward)
            observation, reward, done, info = env.step(action)
            sum_reward += reward
            if done:
                break
    queue.put(sum_reward)


def main(generations, steps, retry):
    env = gym.make(environment_name)
    inputs, outputs = env.observation_space.shape[0], env.action_space.shape[0]

    agent = unified_neural_model()
    agent.init(inputs,outputs)

    q = Queue()
    for g in range(generations):
        accum_reward = []
        for t in range(100):
            p = Process(target=f,args=(agent,q,steps,retry))
            p.start()
            p.join()
            reward = q.get()
            agent.endEpisode(reward)
            accum_reward.append(reward)
        #print(g,max(accum_reward), "memory_usage:{}MB".format(py.memory_info()[0]/2**20))
        print(g,max(accum_reward),*accum_reward,sep=',')

    agent.saveAgent("dna_best_individual")



if __name__=='__main__':
    import sys
    generations = int(sys.argv[1])
    steps = int(sys.argv[2])
    retry = int(sys.argv[3])

    main(generations,steps,retry)
