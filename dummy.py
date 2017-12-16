import gym

@profile
def memory_test():
    trials = int(300)
    retry = 10
    steps = 1000

    accum_rewards = []
    for i in range(trials):
        print(i)
        env = gym.make('BipedalWalker-v2')
        for r in range(retry):
            observation,reward = env.reset(),0
            for t in range(steps):
                observation, reward, done, info = env.step(env.action_space.sample())
                if done:
                    break
        env.close()

memory_test()