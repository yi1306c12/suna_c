import gym
env = gym.make('MountainCarContinuous-v0')
inputs,outputs = env.observation_space.shape[0], env.action_space.shape[0]#pendulum,lunarlander,bipedwalker,MountainCarContinuous

from unified_neural_model import unified_neural_model   
agent = unified_neural_model()
agent.init(inputs,outputs)

trials = int(2e5)
steps = 100
accum_rewards = []

from PIL import Image
import numpy as np
for i in range(trials):
    if i%100 == 0 and i != 0:
        print str(accum_rewards)[1:-1]
        accum_rewards = []


    observation,reward = env.reset(),0
    accum_reward = 0
    frames = list()
    for t in range(steps):
        observation = list(observation)#pendulum-v0

        agent.step(observation,reward)
        action = np.array(agent.action())#pendulum,lunarlander,bipedwalker

        observation, reward, done, info = env.step(action)
        accum_reward += reward

        frames.append(Image.fromarray(env.render(mode = 'rgb_array')))
        if done:
            break

    agent.endEpisode(reward)
    accum_rewards.append(accum_reward)
    
    if int(accum_reward) == 0:
        continue
    
    with open('/home/yihome/experiments/MCContv0_1004/MCContv0_1004_{0:06d}reward{1}.gif'.format(i,int(accum_reward)),'wb') as f:
        im = Image.new('RGB',frames[0].size)
        im.save(f,save_all=True,append_images=frames)