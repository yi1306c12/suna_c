
PATH_TO_ZWEIFEL_LIBRARY=$(HOME)/Programs/zweifel

CFLAGS= -O4 -Wall --std=c++11

MAIN_REINFORCEMENT_LEARNING=main.cpp
MAIN_LIVE=live.cpp
ENVIRONMENTS=environments/Double_Cart_Pole.cpp environments/Mountain_Car.cpp environments/Single_Cart_Pole.cpp environments/Function_Approximation.cpp environments/Multiplexer.cpp
AGENTS=agents/Mysterious_Agent.cpp agents/Dummy.cpp agents/Unified_Neural_Model.cpp
#agents/XCS_Butz.cpp
MODELS=agents/modules/Module.cpp
MAPS=agents/self_organized_systems/Som_Map.cpp agents/self_organized_systems/Novelty_Map.cpp agents/self_organized_systems/SOM_Cell.cpp agents/self_organized_systems/ASSOM_Cell.cpp

LDFLAGS=  -L${PATH_TO_ZWEIFEL_LIBRARY}/lib -I${PATH_TO_ZWEIFEL_LIBRARY}/src/ -lzrandom -lzgraph

PYTHON_DIR= $(shell python -c "from distutils.sysconfig import *;print(get_python_inc())")

PYTHON_FLAGS= $(shell python3-config --cflags --ldflags) -lboost_python3 -fPIC -DPIC -shared #-I$(PYTHON_DIR)
NUMPY_FLAGS= $(PYTHON_FLAGS) -lboost_numpy3
#NUMPY_FLAGS= -L/usr/local/lib64 -lboost_numpy $(PYTHON_FLAGS)

all:	main live

live:
	g++ $(CFLAGS) $(MAIN_LIVE) $(AGENTS) $(MODELS) $(MAPS) $(ENVIRONMENTS) $(LDFLAGS) -o rl_live


main:
	g++ $(CFLAGS) $(MAIN_REINFORCEMENT_LEARNING) $(AGENTS) $(MODELS) $(MAPS) $(ENVIRONMENTS) $(LDFLAGS) -o rl


python_lib:
	g++ $(CFLAGS) UnifiedNeuralModel_wrapper.cpp $(AGENTS) $(MODELS) $(MAPS) $(LDFLAGS) -o unified_neural_model.so $(NUMPY_FLAGS)

python_env:
	g++ $(CFLAGS) environments_wrapper.cpp $(ENVIRONMENTS) $(LDFLAGS) -o environments.so $(NUMPY_FLAGS)

python_module:
	g++ $(CFLAGS) module_wrapper.cpp $(MODELS) -o module_wrapper.so $(LDFLAGS) $(NUMPY_FLAGS)

