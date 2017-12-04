
from environments import double_cart_pole
from unified_neural_model import unified_neural_model
import numpy as np

@profile
def main():
    env = double_cart_pole()

    agent = unified_neural_model()
    agent.init(env.observations,env.actions)

    trials = int(3)
    steps = 500

    accum_rewards = []

    for i in range(trials):
        print(i)
#        if i%100 == 0 and i != 0:
#            print(i//100, max(accum_rewards))
#            accum_rewards = []

        observation,reward = env.reset(),0
        accum_reward = 0
        for t in range(env.MAX_STEPS):

            agent.step(env.last_observation(),reward)
            reward = env.step(agent.action())

            accum_reward += reward
        agent.endEpisode(reward)
        accum_rewards.append(accum_reward)


    #agent.saveAgent("dna_best_individual")

if __name__=='__main__':
    main()