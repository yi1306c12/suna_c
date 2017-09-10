#ifndef GROUP_SETTING_H
#define GROUP_SETTING_H

#include"structural_dna.h"

typedef struct _group_setting
{
  int min_adaptation_speed;//ok
  int type_capacities[NUMBER_OF_NEURON_TYPES];//underconstruction
  bool neuromodulation;//ok
  bool from_to_restriction;//underconstruction
  int neuron_capacity;//ok
  //connection_capacity -> group_adjacent
}group_setting;

#endif
