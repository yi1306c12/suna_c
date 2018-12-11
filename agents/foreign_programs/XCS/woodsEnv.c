/*
/       (XCS-C 1.2)
/	------------------------------------
/	Learning Classifier System based on accuracy
/
/     by 
/     Martin V. Butz
/     Illinois Genetic Algorithms Laboratory (IlliGAL)
/     University of Illinois at Urbana/Champaign
/     butz@illigal.ge.uiuc.edu
/
/     Last modified: 09-30-2003
/
/     The woods environment coded binary
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "woodsEnv.h"
#include "xcsMacros.h"

int xp=0,yp=0;
int xs=0,ys=0;
int xe1=0,ye1=0;
char *env=NULL;
int *noise=NULL;
int enemyHidden=0;
char lastEnemyState[WOODS_LENGTH_OF_ONE_ATTRIBUTE];

/*####################### construct and destruct the environment ############################*/


/**
 * Initialize the environment reading in the currently selected maze.
 */ 
int initEnv(FILE *fp)
{
  char s[100], *help;
  int i, sublength, *helpInt;
  xs=0, ys = 0;
  
  if(ENEMY_EXISTS){
    lastEnemyState[0]='0';
    if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>1){
      lastEnemyState[1]='0';
      if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>2){
	lastEnemyState[2]='0';
      }
    }
  }

  while(fscanf(fp,"%s",s)==1){
    if(xs==0){
      xs=strlen(s);
      assert((env=calloc(xs*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1,sizeof(char)))!=NULL);
      assert((help=calloc(1,sizeof(char)))!=NULL);/* this is necessary as help always is freed */
      assert((noise=calloc(xs,sizeof(int)))!=NULL);
      assert((helpInt=calloc(1,sizeof(int)))!=NULL);/* this is necessary as help always is freed */
      sublength=0;
    }else{
      if(xs!=strlen(s)){
        printf("Error in File of environment!\n");
        return 0;
      }
      help=env;
      sublength=strlen(help);
      assert((env=calloc(strlen(s)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+sublength+1,sizeof(char)))!=NULL);
      strncpy(env,help,sublength);
      helpInt = noise;
      assert((noise=calloc(strlen(s)+ sublength/WOODS_LENGTH_OF_ONE_ATTRIBUTE, sizeof(int)))!=NULL);
      for(i=0; i<sublength/WOODS_LENGTH_OF_ONE_ATTRIBUTE; i++)
	noise[i]=helpInt[i];
    }
    for(i=0; i<xs; i++){
      switch(s[i]){
      case 'O':
	env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE +sublength]=OBSTACLEO0;
	if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>1){
	  env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1 +sublength]=OBSTACLEO1;
	  if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>2){
	    env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2 +sublength]=OBSTACLEO2;
	  }
	} 
	noise[i+sublength/WOODS_LENGTH_OF_ONE_ATTRIBUTE]=0;
	break;
      case 'Q':
	env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE +sublength]=OBSTACLEQ0;
	if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>1){
	  env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1 +sublength]=OBSTACLEQ1;
	  if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>2){
	    env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2 +sublength]=OBSTACLEQ2;
	  }
	}
	noise[i+sublength/WOODS_LENGTH_OF_ONE_ATTRIBUTE]=0;
	break;
      case 'F':
	env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE +sublength]=FOODF0;
	if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>1){
	  env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1 +sublength]=FOODF1;
	  if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>2){
	    env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2 +sublength]=FOODF2;
	  }
	}
	noise[i+sublength/WOODS_LENGTH_OF_ONE_ATTRIBUTE]=0;
	break;
      case 'G':
	env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE +sublength]=FOODG0;
	if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>1){
	  env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1 +sublength]=FOODG1;
	  if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>2){
	    env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2 +sublength]=FOODG2;
	  }
	}
	noise[i+sublength/WOODS_LENGTH_OF_ONE_ATTRIBUTE]=0;
	break;
      case '*':
	env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE +sublength]=FREE0;
	if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>1){
	  env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1 +sublength]=FREE1;
	  if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>2){
	    env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2 +sublength]=FREE2;
	  }
	}
	noise[i+sublength/WOODS_LENGTH_OF_ONE_ATTRIBUTE]=0;
	break;
      case '1':
	env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE +sublength]=FREE0;
	if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>1){
	  env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1 +sublength]=FREE1;
	  if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>2){
	    env[i*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2 +sublength]=FREE2;
	  }
	}
	noise[i+sublength/WOODS_LENGTH_OF_ONE_ATTRIBUTE]=1;
	break;
      default:
	printf("Mistake in reading maze file.\n");
	return 0;
	break;
      }
    }
    env[sublength+ xs*WOODS_LENGTH_OF_ONE_ATTRIBUTE]='\0';
    free(help);
    free(helpInt);
  }

  ys=strlen(env)/(xs*WOODS_LENGTH_OF_ONE_ATTRIBUTE);
  xe1=0;ye1=0;
  /* set the enemy (demon) if applied */
  if(ENEMY_EXISTS){
    setFreeRandPos(&xe1,&ye1);
    lastEnemyState[0]=env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE];
    env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=ENEMY0;
    if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>1){
      lastEnemyState[1]=env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1];
      env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=ENEMY1;
      if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>2){
        lastEnemyState[1]=env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2];
        env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=ENEMY2;
      }
    }
  }
  enemyHidden=0;
  printf("Read in: %s\n",env);
  return 1;
}

/**
 * Free the allocated memory.
 */
void freeEnv()
{
  free(env);
}

/**
 * Returns the number of actions (usually set to eight). 
 */
int getNumberOfActions()
{
  return WOODS_NUMBER_OF_ACTIONS;
}

/**
 * This is the function that is called to move the animat,
 * move also the enemy (demon), and set the perceived reward.
 * Returns if a reset must take place - reset has to be called separately
 */
double doAction(char *state, int action, int *reset)
{
  int moved;
  double reward;

  moved=moveObject(action,&xp,&yp);

  reward=checkReward(xp,yp);

  if(WOODS_ADD_NOISE_TO_ACTION){
    if( (WOODS_ADD_NOISE_TO_ACTION>>action)%2 == 1)
      reward += WOODS_MU + nrand()*WOODS_SIGMA;
  }else if(noise[((((yp)-1+ys)%ys)*xs+(xp))]==1){
    reward += WOODS_MU + nrand()*WOODS_SIGMA;
  }

  if(ENEMY_EXISTS && !enemyHidden)
    moveEnemy();
  /* sets the new entered state */
  setState(state);

  if(reward > 0 && RESET_ON_REWARD )
    *reset = 1;
  else
    *reset = 0;

  return reward;
}

/**
 * Changes x and y if the action leads to a free spot, 
 * returns if a movement took place. 
 */
int moveObject(int action,int *x,int *y)
{
  int moved=0;
  if(action==0){
    if( noise[((((*y)-1+ys)%ys)*xs+(*x))]==0 || (noise[((((*y)-1+ys)%ys)*xs+(*x))]==1 && urand()<1.-SLIPPROB))
      moved=north(x,y);
    else
      if(urand()<0.5)
        moved=northwest(x,y);
      else
        moved=northeast(x,y);
  }else if(action==1){
    if( noise[((((*y)-1+ys)%ys)*xs+(*x))]==0 || (noise[((((*y)-1+ys)%ys)*xs+(*x))]==1 && urand()<1.-SLIPPROB))
      moved=northeast(x,y);
    else
      if(urand()<0.5)
        moved=north(x,y);
      else
        moved=east(x,y);
  }else if(action==2){
    if( noise[((((*y)-1+ys)%ys)*xs+(*x))]==0 || (noise[((((*y)-1+ys)%ys)*xs+(*x))]==1 && urand()<1.-SLIPPROB))
      moved=east(x,y);
    else
      if(urand()<0.5)
        moved=northeast(x,y);
      else
        moved=southeast(x,y);
  }else if(action==3){
    if( noise[((((*y)-1+ys)%ys)*xs+(*x))]==0 || (noise[((((*y)-1+ys)%ys)*xs+(*x))]==1 && urand()<1.-SLIPPROB))
      moved=southeast(x,y);
    else
      if(urand()<0.5)
        moved=south(x,y);
      else
        moved=east(x,y);
  }else if(action==4){
    if( noise[((((*y)-1+ys)%ys)*xs+(*x))]==0 || (noise[((((*y)-1+ys)%ys)*xs+(*x))]==1 && urand()<1.-SLIPPROB))
      moved=south(x,y);
    else
      if(urand()<0.5)
        moved=southwest(x,y);
      else
        moved=southeast(x,y);
  }else if(action==5){
    if( noise[((((*y)-1+ys)%ys)*xs+(*x))]==0 || (noise[((((*y)-1+ys)%ys)*xs+(*x))]==1 && urand()<1.-SLIPPROB))
      moved=southwest(x,y);
    else
      if(urand()<0.5)
        moved=west(x,y);
      else
        moved=south(x,y);
  }else if(action==6){
    if( noise[((((*y)-1+ys)%ys)*xs+(*x))]==0 || (noise[((((*y)-1+ys)%ys)*xs+(*x))]==1 && urand()<1.-SLIPPROB))
      moved=west(x,y);
    else
      if(urand()<0.5)
        moved=northwest(x,y);
      else
        moved=southwest(x,y);
  }else if(action==7){
    if( noise[((((*y)-1+ys)%ys)*xs+(*x))]==0 || (noise[((((*y)-1+ys)%ys)*xs+(*x))]==1 && urand()<1.-SLIPPROB))
      moved=northwest(x,y);
    else
      if(urand()<0.5)
        moved=north(x,y);
      else
        moved=west(x,y);
  }else{
    printf("Mistake in action execution. Action: %d\n",action);
  }
  return moved;
}

/**
 * The eight possible movements: 
 * get the point that is desired. 
 * If the point is free, set x and y to the new point 
 */
int north(int *x,int *y)
{
  char point[WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  int i;
  
  for(i=0; i<WOODS_LENGTH_OF_ONE_ATTRIBUTE; i++)
    point[i]=env[((((*y)-1+ys)%ys)*xs+(*x))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+i];
  
  if(((point[0]==FREE0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FREE1)) && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FREE2))
     ||((point[0]==FOODF0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODF1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODF2))
     ||((point[0]==FOODG0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODG1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODG2)))
    {
      (*y)=(*y)+ys-1;(*y)%=ys;
      return 1;
    }
  return 0;
}
int northeast(int *x,int *y)
{
  char point[WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  int i;
  
  for(i=0; i<WOODS_LENGTH_OF_ONE_ATTRIBUTE; i++)
    point[i]=env[((((*y)+ys-1)%ys)*xs+(((*x)+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+i];
  
  if(((point[0]==FREE0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FREE1)) && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FREE2 ))
     ||((point[0]==FOODF0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODF1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODF2))
     ||((point[0]==FOODG0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODG1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODG2)))
    {
      (*y)=(*y)+ys-1;(*y)%=ys;
      (*x)++;(*x)%=xs;
      return 1;
    }
  return 0;
}
int east(int *x,int *y)
{
  char point[WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  int i;
  
  for(i=0; i<WOODS_LENGTH_OF_ONE_ATTRIBUTE; i++)
    point[i]=env[((*y)*xs+(((*x)+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+i];
  
  if(((point[0]==FREE0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FREE1)) && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FREE2 ))
     ||((point[0]==FOODF0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODF1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODF2))
     ||((point[0]==FOODG0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODG1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODG2)))
    {
      (*x)++;(*x)%=xs;
      return 1;
    }
  return 0;
}
int southeast(int *x,int *y)
{
  char point[WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  int i;
  
  for(i=0; i<WOODS_LENGTH_OF_ONE_ATTRIBUTE; i++)
    point[i]=env[((((*y)+1)%ys)*xs+(((*x)+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+i];
  
  if(((point[0]==FREE0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FREE1)) && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FREE2 ))
     ||((point[0]==FOODF0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODF1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODF2))
     ||((point[0]==FOODG0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODG1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODG2)))
    {
      (*y)++;(*y)%=ys;
      (*x)++;(*x)%=xs;
      return 1;
    }
  return 0;
}
int south(int *x,int *y)
{
  char point[WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  int i;
  
  for(i=0; i<WOODS_LENGTH_OF_ONE_ATTRIBUTE; i++)
    point[i]=env[((((*y)+1)%ys)*xs+(*x))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+i];
  
  if(((point[0]==FREE0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FREE1)) && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FREE2 ))
     ||((point[0]==FOODF0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODF1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODF2))
     ||((point[0]==FOODG0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODG1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODG2)))
    {
      (*y)++;(*y)%=ys;
      return 1;
    }
  return 0;
}
int southwest(int *x,int *y)
{
  char point[WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  int i;
  
  for(i=0; i<WOODS_LENGTH_OF_ONE_ATTRIBUTE; i++)
    point[i]=env[((((*y)+1)%ys)*xs+(((*x)+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+i];
  
  if(((point[0]==FREE0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FREE1)) && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FREE2 ))
     ||((point[0]==FOODF0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODF1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODF2))
     ||((point[0]==FOODG0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODG1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODG2)))
    {
      (*y)++;(*y)%=ys;
      (*x)=(*x)+xs-1;(*x)%=xs;
      return 1;
    }
  return 0;
}
int west(int *x,int *y)
{
  char point[WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  int i;
  
  for(i=0; i<WOODS_LENGTH_OF_ONE_ATTRIBUTE; i++)
    point[i]=env[((*y)*xs+(((*x)+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+i];
  
  if(((point[0]==FREE0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FREE1)) && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FREE2 ))
     ||((point[0]==FOODF0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODF1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODF2))
     ||((point[0]==FOODG0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODG1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODG2)))
    {
      (*x)=(*x)+xs-1;(*x)%=xs;
      return 1;
    }
  return 0;
}
int northwest(int *x,int *y)
{
  char point[WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  int i;
  
  for(i=0; i<WOODS_LENGTH_OF_ONE_ATTRIBUTE; i++)
    point[i]=env[((((*y)+ys-1)%ys)*xs+(((*x)+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+i];
  
  if(((point[0]==FREE0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FREE1)) && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FREE2 ))
     ||((point[0]==FOODF0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODF1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODF2))
     ||((point[0]==FOODG0 && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || point[1]==FOODG1)) && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || point[2]==FOODG2)))
    {
      (*y)=(*y)+ys-1;(*y)%=ys;
      (*x)=(*x)+xs-1;(*x)%=xs;
      return 1;
    }
  return 0;
}

/**
 * returns the amount of reward given at position x,y 
 */
double checkReward(int x,int y)
{
  if(((env[(y*xs+x)*WOODS_LENGTH_OF_ONE_ATTRIBUTE]==FOODF0 
      && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || env[(y*xs+x)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]==FOODF1))
      && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || env[(y*xs+x)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]==FOODF2)
     )||(( 
      env[(y*xs+x)*WOODS_LENGTH_OF_ONE_ATTRIBUTE]==FOODG0 
      && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || env[(y*xs+x)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]==FOODG1))
      && ( WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || env[(y*xs+x)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]==FOODG2)
     ))
    return WOODS_PAYMENT_RANGE;
  else
    return 0;
}

/** 
 * Set the currently perceived state of the animat.
 */
void setState(char *state)
{
  state[0*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=env[(((yp+ys-1)%ys)*xs+xp)*WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  state[1*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=env[(((yp+ys-1)%ys)*xs+((xp+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  state[2*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=env[(yp*xs+((xp+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  state[3*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=env[(((yp+1)%ys)*xs+((xp+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  state[4*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=env[(((yp+1)%ys)*xs+xp)*WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  state[5*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=env[(((yp+1)%ys)*xs+((xp+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  state[6*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=env[(yp*xs+((xp+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  state[7*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=env[(((yp+ys-1)%ys)*xs+((xp+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE];
  if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>1){
    state[0*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=env[(((yp+ys-1)%ys)*xs+xp)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1];
    state[1*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=env[(((yp+ys-1)%ys)*xs+((xp+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1];
    state[2*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=env[(yp*xs+((xp+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1];
    state[3*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=env[(((yp+1)%ys)*xs+((xp+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1];
    state[4*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=env[(((yp+1)%ys)*xs+xp)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1];
    state[5*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=env[(((yp+1)%ys)*xs+((xp+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1];
    state[6*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=env[(yp*xs+((xp+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1];
    state[7*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=env[(((yp+ys-1)%ys)*xs+((xp+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1];
    if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>2){
      state[0*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=env[(((yp+ys-1)%ys)*xs+xp)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2];
      state[1*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=env[(((yp+ys-1)%ys)*xs+((xp+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2];
      state[2*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=env[(yp*xs+((xp+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2];
      state[3*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=env[(((yp+1)%ys)*xs+((xp+1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2];
      state[4*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=env[(((yp+1)%ys)*xs+xp)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2];
      state[5*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=env[(((yp+1)%ys)*xs+((xp+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2];
      state[6*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=env[(yp*xs+((xp+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2];
      state[7*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=env[(((yp+ys-1)%ys)*xs+((xp+xs-1)%xs))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2];
    }
  }
  /* incase of an additional random bit, switch it with 50% probability */
  if(RANDOMBIT_EXISTS){
    state[WOODS_CONDITION_LENGTH]=getRandomChar();
    state[WOODS_CONDITION_LENGTH+1]='\0';
  }else{
    state[WOODS_CONDITION_LENGTH]='\0';
  }
}

/**
 * Moves the enemy (demon) randomly.
 */
int moveEnemy()
{
  int xeo,yeo;
  int act;

  act = (int)(urand()*8);
  xeo=xe1;yeo=ye1;
  if(moveObject(act,&xe1,&ye1)){
    /* if the enemy moved, set the enemy detection in the environment */
    env[(yeo*xs+xeo)*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=lastEnemyState[0];
    lastEnemyState[0]=env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE];
    env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE]=ENEMY0;
    
    if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>1){
    
      env[(yeo*xs+xeo)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=lastEnemyState[1];
      lastEnemyState[1]=env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1];
      env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]=ENEMY1;
      
      if(WOODS_LENGTH_OF_ONE_ATTRIBUTE>2){
        env[(yeo*xs+xeo)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=lastEnemyState[2];
        lastEnemyState[2]=env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2];
        env[(ye1*xs+xe1)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]=ENEMY2;
      }
    }
  }
  return 1;
}


/*###################### generating a new random position #################################*/

/**
 * After a reset a new position is determined. 
 */
void resetState(char *state){
  setStartPos(state);
}

/**
 * Set the animat to a new randomly chosen position and get the perceptions.
 */
void setStartPos(char *state)
{
  setFreeRandPos(&xp,&yp);
  setState(state);
}

/**
 * Set x,y to a new randomly chosen free position.
 */
void setFreeRandPos(int *x,int *y)
{
  double help;
  do{
    help=urand()*xs;
    (*x)=help;
    help=urand()*ys;
    (*y)=help;
  }while(!( env[((*y)*xs+(*x))*WOODS_LENGTH_OF_ONE_ATTRIBUTE]==FREE0
            && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || env[((*y)*xs+(*x))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]==FREE1)
            && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || env[((*y)*xs+(*x))*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]==FREE2)));
}


/*###################### get environmental properties for testing purposes ####################*/


/**
 * Returns the width of the environment.
 */
int getEnvXSize()
{
  return xs;
}

/**
 * Returns the length of the environment.
 */
int getEnvYSize()
{
  return ys;
}

/**
 * Returns the current x position.
 */
int getXPos()
{
  return xp;
}

/**
 * Returns the current y position.
 */
int getYPos()
{
  return yp;
}

/**
 * If x and y point to a free place, 
 * set the state to this place (for testing purposes) 
 */
int setPos(int x,int y,char *state)
{
  x=abs(x)%xs;
  y=abs(y)%ys;
  if(!(env[(y*xs+x)*WOODS_LENGTH_OF_ONE_ATTRIBUTE]==FREE0
       && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<2 || env[(y*xs+x)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+1]==FREE1)
       && (WOODS_LENGTH_OF_ONE_ATTRIBUTE<3 || env[(y*xs+x)*WOODS_LENGTH_OF_ONE_ATTRIBUTE+2]==FREE2)))
    return 0;
  xp=x;
  yp=y;
  setState(state);
  return 1;
}

/**
 * Get '0' or '1', is used for additional random bits.
 */
char getRandomChar()
{
  double type;
  char c;

  type=urand()*1;

  switch((int)type){
  case 0:c='0';break;
  case 1:c='1';break;
  default:c='X';break;
  }
  return c;
}

/**
 * Returns the lenght of one perception string.
 */
int getConditionLength() {return WOODS_CONDITION_LENGTH;}

/**
 * Returns the payment range of the problem.
 */
int getPaymentRange() {return WOODS_PAYMENT_RANGE;} 

/**
 * All parameters are set using Macros.
 */
int setEnvParam(char *type, double value) {return 0;}

/**
 * Writes all parameter (macro) settings to file. 
 */
void fprintEnv(FILE *outfile)
{
  fprintf(outfile, "# MazeEnv: length of one attribute: %d condition length %d number of actions %d\n", WOODS_LENGTH_OF_ONE_ATTRIBUTE, WOODS_CONDITION_LENGTH, WOODS_NUMBER_OF_ACTIONS);
  fprintf(outfile, "# payment range: %d add noise to action: %d mean noise: %4.6f sigma noise: %4.6f\n",WOODS_PAYMENT_RANGE, WOODS_ADD_NOISE_TO_ACTION, WOODS_MU, WOODS_SIGMA);
  fprintf(outfile, "# enemy exists: %d random bit exists: %d slipprob: %4.2f reset on reward: %d\n",ENEMY_EXISTS, RANDOMBIT_EXISTS, SLIPPROB, RESET_ON_REWARD);
}
