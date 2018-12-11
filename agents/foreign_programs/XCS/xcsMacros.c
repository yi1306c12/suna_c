/*
/       (XCS-C 1.1)
/	------------------------------------
/	Learning Classifier System based on accuracy
/
/     by Martin Butz
/     University of Wuerzburg / University of Illinois at Urbana/Champaign
/     butz@illigal.ge.uiuc.edu
/     Last modified: 09-04-2003
/
/     The pseudo random number generator.
*/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include "xcsMacros.h"

long _Q = _M/_A;     /* M / A */
long _R = _M%_A;     /* M mod A */
long _seed;          /* a number between 1 and m-1 */
int haveUniNum=0;
double uniNum=0;

/* returns a floating-point random number generated according to uniform distribution from [0,1) */
double urand()
{
  long lo,hi,test;
  
  hi   = _seed / _Q;
  lo   = _seed % _Q;
  test = _A*lo - _R*hi;
  
  if (test>0)
    _seed = test;
  else
    _seed = test+_M;

  return (double)(_seed)/_M;
}

/* returns a floating-point random number generated according to a normal distribution with 
 * mean 0 and standard deviation 1 */
double nrand()
{
  float x1, x2, w;

  if(haveUniNum){
    haveUniNum=0;
    return uniNum;
  }else{
    do {
      x1 = 2.0 * urand() - 1.0;
      x2 = 2.0 * urand() - 1.0;
      w = x1 * x1 + x2 * x2;
    } while ( w >= 1.0 );
    
    w = sqrt( (-2.0 * log( w ) ) / w );
    uniNum = x1 * w;
    haveUniNum=1;
    return x2 * w;
  }
  
}

/* sets the random seed */
int setSeed(int newSeed)
{
  return (int)(_seed = (long)newSeed);
}

int getSeed()
{
  return (int)_seed;
}

/* get int value of string - does not check for any incorrect format*/
int getIntValue(char *string)
{
  int i, l, value=0;
  l = strlen(string);
  for(i=0; i<l; i++){
    if(string[i]=='.' || string[i]==',')
      break;
    value = value*10 + (int)string[i]-(int)('0');
  }
  return value;
}

/* get double value of string - does not check for any incorrect format! */
double getDoubleValue(char *string)
{
  int i, l;
  double value=0., divider=0;

  l = strlen(string);
  for(i=0; i<l; i++){
    if(string[i]=='.' || string[i]==',')
      divider = 10.;
    else if(divider==0)
      value = value*10. + (double)((int)string[i]-(int)('0'));
    else{
      value += (double)((int)string[i]-(int)('0'))/divider;
      divider*=10.;
    }
  }
  return value;
}


/* get the abs value of value */
double absDouble(double value)
{
  if(value>=0.)
    return value;
  else
    return value*(-1.);
}


/* randomize the pseudo random generator */
void randomize(void)
{
  setSeed((long)(time(NULL)%10000+1));
}

double NChooseK(double n, double k)
{
  double result=1;
  double i,j;
 
  if(k<0 || k>n || n<0) {
    printf("Error in N choose K call!!!|n");
    return -1;
  }
  if(k==0 || k==n)
    return 1;
 
   
  if(k > n-k){
    i=k+1;
    j=n-k;
  }else{
    i=n-k+1;
    j=k;
  }
                                                                                
  while(i<=n){
    if( i<=n)
      result *= i;
    if(j>0)
      result /= j;
    i++,j--;
  }
  return result;
}
