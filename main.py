
from environments import double_cart_pole
from unified_neural_model import unified_neural_model
import numpy as np

#@profile
def main():
    print('env')
    env = double_cart_pole()

    print('agent', env.observations, env.actions)
    agent = unified_neural_model(env.observations,env.actions)
    print('end init')

    trials = int(3e5)

    accum_rewards = []

    for i in range(trials):
            
        observation,reward = env.reset(),0.
        accum_reward = 0.

        env_trial = env.trial
        for t in range(env.MAX_STEPS):

            action = agent.step(env.last_observation(),reward)
            reward = env.step(action)
            #print(action, env.last_observation(), reward)

            accum_reward += reward

            if env.trial != env_trial:
                break

        agent.endEpisode(reward)
        accum_rewards.append(accum_reward)

        if i%100 == 0 and i != 0:
            print(i, max(accum_rewards))
            if max(accum_rewards) >= 1e5:
                break
            accum_rewards = []



    agent.saveAgent("dna_best_individual")

if __name__=='__main__':
    print('start')
    main()
