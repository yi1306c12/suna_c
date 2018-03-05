from multiprocessing import Process,Queue
import gym
environment_name = 'BipedalWalker-v2'
input_select = [
#   0,	#hull_angle
#   1,	#hull_angularVelocity
#   2,	#vel_x
#   3,	#vel_y
    4,	#hip_joint_1_angle
#   5,	#hip_joint_1_speed
    6,	#knee_joint_1_angle
#   7,	#knee_joint_1_speed
    8,	#leg_1_ground_contact_flag
    9,	#hip_joint_2_angle
#   10,	#hip_joint_2_speed
    11,	#knee_joint_2_angle
#   12,	#knee_joint_2_speed
    13	#leg_2_ground_contact_flag
#   14-23   #10 lidar readings
]
gym.logger.set_level(40)#exclude waning message

class limited_observation:
    def __init__(self,env,limitation):
        self.env = env
        self.limitation = limitation

    def step(self,action):
        observation, reward, done, info = self.env.step(action)
        return observation[input_select], reward, done, info

    def reset(self):
        return self.env.reset()[input_select]


from unified_neural_model import unified_neural_model

import os
pid = os.getpid()
import psutil
py = psutil.Process(pid)


def f(agent,queue,steps,retry):
#    env = gym.make(environment_name)
    env = limited_observation(gym.make(environment_name), input_select)
    observation, reward = env.reset(), 0
    
    sum_reward = 0
    for r in range(retry):
        agent.resetAgent()
        observation, reward = env.reset(),0
        for s in range(steps):
            action = agent.step(observation,reward)
            observation, reward, done, info = env.step(action)
            sum_reward += reward
            if done:
                break
    queue.put(sum_reward)


def main(generations, steps, retry):
    env = gym.make(environment_name)
#    inputs, outputs = env.observation_space.shape[0], env.action_space.shape[0]
    inputs, outputs = len(input_select), env.action_space.shape[0]

    agent = unified_neural_model()
    agent.init(inputs,outputs)

    q = Queue()
    for g in range(generations):
        accum_reward = []
        for t in range(100):
            p = Process(target=f,args=(agent,q,steps,retry))
            p.start()
            p.join()
            reward = q.get()
            agent.endEpisode(reward)
            accum_reward.append(reward)
        #print(g,max(accum_reward), "memory_usage:{}MB".format(py.memory_info()[0]/2**20))
        print(g,max(accum_reward),*accum_reward,sep=',')

    agent.saveAgent("dna_best_individual")



if __name__=='__main__':
    import sys
    generations = int(sys.argv[1])
    steps = int(sys.argv[2])
    retry = int(sys.argv[3])

    main(generations,steps,retry)
