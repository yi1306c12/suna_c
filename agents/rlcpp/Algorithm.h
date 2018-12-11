#ifndef ALGORITHM_H
#define ALGORITHM_H

# include <iostream>
# include <string>
# include <fstream>
# include <math.h>
# include <vector>
# include <stdlib.h>

//# include "State.h"
//# include "Action.h"

using namespace std ;

class Algorithm {
    public:
        Algorithm() ;
        virtual ~Algorithm() ;
        double max( double * array, int n ) ;
        int argmax( double * array, int n ) ;
        std::vector< int > argmaxAll( double * array, int n ) ;
        void egreedy( double * state, double * action, double epsilon ) ;
        virtual void getMaxAction( double * state, double * action ) =0 ;
        virtual void getRandomAction( double * state, double * action ) =0 ;
//        virtual void explore( double * state, double * action, double explorationRate, std::string explorationType, bool endOfEpisode ) =0 ;
//        virtual void update( double * st, double * action, double rt, double * st_, bool endOfEpisode, double * learningRate, double gamma  ) =0 ;
//        virtual unsigned int getNumberOfLearningRates() =0;
//        virtual bool getContinuousStates() =0;
//        virtual bool getDiscreteStates() =0;
//        virtual bool getContinuousdoubles() =0;
//        virtual bool getDiscretedoubles() =0;
//        virtual const char * getName() =0;

    protected:
        bool continuousStates, discreteStates, continuousActions, discreteActions ;
        int stateDimension, numberOfStates, actionDimension, numberOfActions ;
        double X, maxX ;
        int maxI ;
};

#endif //ALGORITHM_H
