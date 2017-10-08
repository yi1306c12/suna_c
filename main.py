import gym
#env = gym.make('CartPole-v1')
#env = gym.make('Pendulum-v0')
#env = gym.make('LunarLanderContinuous-v2')
env = gym.make('BipedalWalker-v2')
#env = gym.make('Copy-v0')
#env = gym.make('MountainCarContinuous-v0')
print(env.observation_space,env.action_space)
#inputs,outputs = env.observation_space.shape[0], 1#cartpole
inputs,outputs = env.observation_space.shape[0], env.action_space.shape[0]#pendulum,lunarlander,bipedwalker,MountainCarContinuous
#inputs,outputs = 1,3#copy

from unified_neural_model import unified_neural_model
agent = unified_neural_model()
agent.init(inputs,outputs)

trials = int(2e5)
steps = 500

accum_rewards = []

import numpy as np
for i in range(trials):
    if i%100 == 0 and i != 0:
        print i//100, max(accum_rewards)
        accum_rewards = []



    observation,reward = env.reset(),0
    accum_reward = 0
    for t in range(steps):
        observation = list(observation)#pendulum-v0

        agent.step(observation,reward)
        #agent.step([observation],reward)#copy
        #action = [int(agent.action()>1)]#cartpole
        action = np.array(agent.action())#pendulum,lunarlander,bipedwalker
        #action = np.clip(action,-1,1)#lunarlander

        #action = agent.action()
        #action[0] = int(action[0] > 1)
        #action[1] = int(action[1] > 1)
        #action[2] = int(max(0,min(4.99,action[2])))

        observation, reward, done, info = env.step(action)
        #env.render()
        accum_reward += reward
        if done:
        #    print("episode finish at {} times".format(t))
            break
#           observation,reward = env.reset(),0
    agent.endEpisode(reward)
    accum_rewards.append(accum_reward)
