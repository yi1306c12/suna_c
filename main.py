from environments import double_cart_pole
env = double_cart_pole()

from unified_neural_model import unified_neural_model
agent = unified_neural_model()
agent.init(env.number_of_observation_vars,env.number_of_action_vars)

trials = int(3e5)
steps = 500

accum_rewards = []

import numpy as np
for i in range(trials):
    if i%100 == 0 and i != 0:
        print(i//100, max(accum_rewards))
        accum_rewards = []


    observation,reward = env.reset(),0
    accum_reward = 0
    for t in range(env.MAX_STEPS):

        agent.step(observation,reward)
        action = np.array(agent.action())#pendulum,lunarlander,bipedwalker

        accum_reward += reward
        if done:
            break
    agent.endEpisode(reward)
    accum_rewards.append(accum_reward)

agent.saveAgent("dna_best_individual")