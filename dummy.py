import gym
import psutil
import os
pid = os.getpid()
py = psutil.Process(pid)

@profile
def memory_test():
    trials = int(1000)
    retry = 10
    steps = 1000

    accum_rewards = []
    env = gym.make('BipedalWalker-v2')
    for i in range(trials):
        print(i, "memory usage:{}MB".format(py.memory_info()[0]/2**20))
        for r in range(retry):
            observation,reward = env.reset(),0
            for t in range(steps):
                observation, reward, done, info = env.step(env.action_space.sample())
                if done:
                    break
                    
        

memory_test()