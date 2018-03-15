import gym

#if __name__ == '__main__':
#env = gym.make(environment_name)
env = gym.make("BipedalWalker-v2")
observation = env.reset()
done = False

number_of_observation_vars = env.observation_space.shape[0]
number_of_action_vars = env.action_space.shape[0]

def step(action):
    global observation, done
    observation, reward, done, info = env.step(action)
#    print(observation,action,reward,done)
    return reward

def restart():
    return env.reset()


if __name__=='__main__':
    for _ in range(100):
        reward = step(env.action_space.sample())
        print(observation,done,reward)