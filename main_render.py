import os
experiment_name = 'BipedalWalker1008'
dirpath = os.path.expanduser('~/experiments/'+experiment_name)
if not os.path.exists(dirpath):
    os.makedirs(dirpath)


import gym
#env = gym.make('MountainCarContinuous-v0')
#env = gym.make('CartPole-v1')
env = gym.make('BipedalWalker-v2')
inputs,outputs = env.observation_space.shape[0], env.action_space.shape[0]#pendulum,lunarlander,bipedwalker,MountainCarContinuous
#inputs,outputs = env.observation_space.shape[0], 1

from unified_neural_model import unified_neural_model   
agent = unified_neural_model(inputs,outputs)

trials = int(2e5)
steps = 500
accum_rewards = []

from PIL import Image
import numpy as np
for i in range(trials):
    if i%100 == 0 and i != 0:
        print max(accum_rewards)
        #print str(accum_rewards)[1:-1]
        #accum_rewards = []


    observation,reward = env.reset(),0
    accum_reward = 0
    frames = list()
    for t in range(steps):
        observation = list(observation)#pendulum-v0

        agent.step(observation,reward)
        action = np.array(agent.action())#pendulum,lunarlander,bipedwalker
#        action = int(agent.action()[0] > 0)#cartpole

        observation, reward, done, info = env.step(action)
        accum_reward += reward

        frames.append(Image.fromarray(env.render(mode = 'rgb_array')))
        if done:
            break

    agent.endEpisode(reward)
    accum_rewards.append(accum_reward)
    print i, accum_reward
    
    if int(accum_reward) == 0:
        continue
    
    path = os.path.expanduser('~/experiments/'+experiment_name+'/'+experiment_name+'_{0:06d}reward{1}.gif'.format(i,int(accum_reward)))
    with open(path,'wb') as f:
        im = Image.new('RGB',frames[0].size)
        im.save(f,save_all=True,append_images=frames)