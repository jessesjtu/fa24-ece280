#ifndef SIMULATION_H
#define SIMULATION_H

#include "world_type.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
using namespace std;

bool initWorld(world_t &world, const string &speciesFile, const string &worldFile);
void simulateCreature(creature_t &creature, world_t &world, bool verbose, void (*fn[])(creature_t &, world_t &));
void printGrid(const grid_t &grid);
//point_t adjacentPoint(point_t pt, direction_t dir);
void left(creature_t &creature, world_t &world);
void right(creature_t &creature, world_t &world);
void deleteCreature(creature_t *&creature);
//instruction_t getInstruction(const creature_t &creature);
//creature_t *getCreature(const grid_t &grid, point_t location);
bool readSpeciesFile(const string &filename, world_t &world);
bool readProgram(species_t &species, const string creature_file);
creature_t* getCreatureInDirection(creature_t &creature, world_t &world);
point_t calculateNextPosition(creature_t &creature, direction_t dir);
bool readWorldFile(const string &worldFile, world_t &world);
void hop(creature_t &creature, world_t &world);
void infect(creature_t &creature, world_t &world);
void ifempty(creature_t &creature, world_t &world);
void ifenemy(creature_t &creature, world_t &world);
void ifsame(creature_t &creature, world_t &world);
void ifwall(creature_t &creature, world_t &world);
void go(creature_t &creature, world_t &world);



#endif
