#ifndef ALGORITHM
#define ALGORITHM

# include "Algorithm.h"

using namespace std ;

Algorithm::Algorithm() {
    discreteStates = false ;
    continuousStates = false ;
    discreteActions = false ;
    continuousActions = false ;
}

Algorithm::~Algorithm() {}

double Algorithm::max( double * array, int n ) {

    maxX    = array[ 0 ] ;

    for ( int i = 1 ; i < n; i++ ) {

        if ( array[i] > maxX ) {
            maxX = array[i] ;
        }

    }

    return maxX ;

}


int Algorithm::argmax( double * array, int n ) {

    maxX    = array[ 0 ] ;
    maxI    = 0 ;

    for ( int i = 1 ; i < n; i++ ) {

        X = array[ i ] ;

        if ( X > maxX ) {
            maxX = X ;
            maxI = i ;
        }

    }

    return maxI ;

}

std::vector<int> Algorithm::argmaxAll( double * array, int n ) {

    maxX    = array[ 0 ] ;
    std::vector<int> maxI ;
    maxI.push_back( 0 ) ;

    for ( int i = 1 ; i < n; i++ ) {

        X = array[ i ] ;

        if ( X > maxX ) {

            maxX = X ;
            maxI.clear() ;
            maxI.push_back( i ) ;

        } else if ( X == maxX ) {

            maxI.push_back( i ) ;

        }

    }

    return maxI ;

}

void Algorithm::egreedy( double* state, double* action, double epsilon ) {

    if ( drand48() < epsilon ) {

        getMaxAction( state, action ) ;

    } else {

        getRandomAction( state, action ) ;

    }

}



#endif //ALGORITHM
