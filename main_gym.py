import numpy as np

@profile
def main():
    import gym
    env = gym.make('BipedalWalker-v2')
    inputs,outputs = env.observation_space.shape[0], env.action_space.shape[0]#pendulum,lunarlander,bipedwalker,MountainCarContinuous

    from unified_neural_model import unified_neural_model
    agent = unified_neural_model()
    agent.init(inputs,outputs)

    trials = int(3000)
    steps = 500

    accum_rewards = []
    for i in range(trials):
        if i%100 == 0 and i != 0:
            print(i, max(accum_rewards))
            accum_rewards = []

        observation,reward = env.reset(),0
        accum_reward = 0
        for t in range(steps):
            action = agent.step(observation,reward)
            observation, reward, done, info = env.step(action)
            #env.render()
            accum_reward += reward
            if done:
                break
        agent.endEpisode(reward)
        accum_rewards.append(accum_reward)

    agent.saveAgent("dna_best_individual")

if __name__=='__main__':
    main()