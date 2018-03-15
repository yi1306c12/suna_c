import gym

#if __name__ == '__main__':
#    env = gym.make(environment_name)
env = gym.make("BipedalWalker-v2")
observation = env.reset()

number_of_observation_vars = env.observation_space.shape[0]
number_of_action_vars = env.action_space.shape[0]

def step(action):
    global observation
    observation, reward, done, info = env.step(action)
    return reward

def restart():
    return env.reset()


print(locals())