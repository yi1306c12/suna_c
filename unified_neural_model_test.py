from unified_neural_model import unified_neural_model
agent = unified_neural_model()
agent.init(2,3)
agent.step([1,2],0)
agent.step([1,2],0)
agent.step([1,2],0)
agent.step([1,2],0)
print(agent.action())