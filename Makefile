
PATH_TO_ZWEIFEL_LIBRARY=$(HOME)/Programs/zweifel

CFLAGS= -O4 -Wall
#-Wall -pedantic -ansi
#CFLAGS=-O4

MAIN_REINFORCEMENT_LEARNING=main.cpp
MAIN_LIVE=live.cpp
ENVIRONMENTS=environments/Double_Cart_Pole.cpp environments/Mountain_Car.cpp environments/Single_Cart_Pole.cpp environments/Function_Approximation.cpp environments/Multiplexer.cpp environments/Pendulum_Swing_Up.cpp
AGENTS=agents/Mysterious_Agent.cpp agents/Dummy.cpp agents/Unified_Neural_Model.cpp
#agents/XCS_Butz.cpp
MODELS=agents/modules/Module.cpp
MAPS=agents/self_organized_systems/Som_Map.cpp agents/self_organized_systems/Novelty_Map.cpp agents/self_organized_systems/SOM_Cell.cpp agents/self_organized_systems/ASSOM_Cell.cpp

LDFLAGS=  -L${PATH_TO_ZWEIFEL_LIBRARY}/lib -I${PATH_TO_ZWEIFEL_LIBRARY}/src/ -lzrandom -lzgraph


all:	main live

live:
	g++ $(CFLAGS) $(MAIN_LIVE) $(AGENTS) $(MODELS) $(MAPS) $(ENVIRONMENTS) $(LDFLAGS) -o rl_live


main:
	g++ $(CFLAGS) $(MAIN_REINFORCEMENT_LEARNING) $(AGENTS) $(MODELS) $(MAPS) $(ENVIRONMENTS) $(LDFLAGS) -o rl

i2o:
	g++ $(CFLAGS) main_i2o.cpp $(ENVIRONMENTS) $(MODELS) $(LDFLAGS) -o i2o