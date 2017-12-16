import gym

@profile
def memory_test():
    env = gym.make('BipedalWalker-v2')
    trials = int(3000)
    retry = 10
    steps = 1000

    accum_rewards = []
    for i in range(trials):
        print(i)
        for r in range(retry):
            observation,reward = env.reset(),0
            for t in range(steps):
                observation, reward, done, info = env.step(env.action_space.sample())
                if done:
                    break

memory_test()