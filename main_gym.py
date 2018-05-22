import os
pid = os.getpid()
import psutil
py = psutil.Process(pid)

import numpy as np
import gym
from unified_neural_model import unified_neural_model

env = gym.make('BipedalWalker-v2')
#env = gym.make('LunarLanderContinuous-v2')

inputs,outputs = env.observation_space.shape[0], env.action_space.shape[0]#pendulum,lunarlander,bipedwalker,MountainCarContinuous

agent = unified_neural_model()
agent.init(inputs,outputs)

trials = int(3e5)
steps = 1000
retry = 10

def trial():
    accum_reward = 0
    for r in range(retry):
        observation,reward = env.reset(),0
        for t in range(steps):
            action = agent.step(observation,reward)
#            action = np.clip(action,-1.,1.)#lunarlander

            observation, reward, done, info = env.step(action)
            #env.render()
            accum_reward += reward
            if done:
                break
#                observation = env.reset()
    agent.endEpisode(reward)
    return accum_reward/retry

accum_rewards = []
for i in range(trials):
    if i%100 == 0 and i != 0:
        print(i, max(accum_rewards), "memory_usage:{}MB".format(py.memory_info()[0]/2**20))
        accum_rewards = []

    accum_rewards.append(trial())

agent.saveAgent("dna_best_individual")