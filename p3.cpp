#include "simulation.h"
#include "world_type.h"
using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "Error: Missing arguments!" << endl;
        cout << "Usage: ./p3 <species-summary> <world-file> <rounds> [v|verbose]" << endl;
        return 0;
    }
    if (atoi(argv[3]) < 0) {
        cout << "Number of simulation rounds is negative!" << endl;
        return 0;
    }
    world_t world;
    string speciesFile = argv[1];
    string worldFile = argv[2];
    if (!initWorld(world, speciesFile, worldFile)) {
        return 0; // Ensure the return value is correctly handled
    }
    bool verbose = false;
    if (argc > 4 && (string(argv[4]) == "v" || string(argv[4]) == "verbose")) {
        verbose = true;
    }
    void (*fn[9])(creature_t&, world_t&) = {hop, left, right, infect, ifempty, ifenemy, ifsame, ifwall, go};
    cout << "Initial state" << endl;
    printGrid(world.grid);
    int round = stoi(argv[3]);
    for (int i = 0; i < round; i++) {
    cout << "Round " << i + 1 << endl;

    for (int j = 0; (unsigned int)j < world.numCreatures; j++) {
        creature_t &creature = world.creatures[j];
        cout << "Creature (" << creature.species->name << " " 
             << directName[creature.direction] << " " 
             << creature.location.r << " " 
             << creature.location.c << ") takes action: ";
        
        simulateCreature(creature, world, verbose, fn);
    }

    if (!verbose) {
        printGrid(world.grid);  // 输出网格状态
    }
}

    return 0;
}
