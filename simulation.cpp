#include "simulation.h"
#include "world_type.h"
using namespace std;

bool initWorld(world_t &world, const string &speciesFile, const string &worldFile){   
    world.numCreatures = 0;
    world.numSpecies = 0;
    world.grid.height = 0;
    world.grid.width = 0;

    for(unsigned int r = 0; r < MAXHEIGHT; r++){
        for (unsigned int c = 0; c < MAXWIDTH; c++){
            world.grid.squares[r][c] = nullptr;
        }
    }
    if(!readSpeciesFile(speciesFile, world)) {
        return false;
    }
    if(!readWorldFile(worldFile, world)) {
        return false;
    }
    return true;
}

bool readSpeciesFile(const string &filename, world_t &world) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error: Cannot open file " << filename << endl;
        return false;
    }

    string line;
    getline(file, line);
    string SpeciesProgPath = line;

    while (getline(file, line) && !line.empty()) {
        if (world.numSpecies >= MAXSPECIES) {
            cout << "Error: Too many species!" << endl;
            cout << "Maximum number of species is " << MAXSPECIES << endl;
            return false;
        }

        species_t &species = world.species[world.numSpecies];
        species.name = line;
        if (!readProgram(world.species[world.numSpecies], SpeciesProgPath)) {
            return false;
        }
        world.numSpecies++;
    }
    file.close();
    return true;
}

bool readProgram(species_t &species, const string Path) {
    string programFile = Path + '/' + species.name;
    ifstream file(programFile);
    if (!file.is_open()) {
        cout << "Error: Cannot open file " << programFile << "!" << endl;
        return false;
    }

    string line;
    while (getline(file, line) && !line.empty()) {
        if (species.programSize >= MAXPROGRAM) {
            cout << "Error: Too many instructions for species " << species.name << "!" << endl;
            cout << "Maximal number of instructions is " << MAXPROGRAM << endl;
            return false;
        }

        istringstream instruction(line);
        string option;
        instruction >> option;

        bool ifInstruRecognized = false;
        for (int i = 0; i < 9; i++) {
            if (option == opName[i]) {
                ifInstruRecognized = true;
                instruction_t instruct;
                instruct.op = static_cast<opcode_t>(i);

                if (i >= 4 && i < 8) {  // IF ** instructions
                    unsigned int address = 0;
                    instruction >> address;
                    instruct.address = address - 1;
                } else {
                    instruct.address = 0;
                }

                species.program[species.programSize++] = instruct; // 更新 programSize 并存储指令
                break;
            }
        }
        if (!ifInstruRecognized) {
            cout << "Error: Instruction " << line << " is not recognized!" << endl;
            return false;
        }
    }
    file.close();
    return true;
}

bool readWorldFile(const string &worldFile, world_t &world) {
    ifstream file;
    file.open(worldFile);
    if (!file) {
        cout << "Error: Cannot open file " << worldFile << "!" << endl;
        return false;
    }

    string line;
    getline(file, line);
    if (!(stoi(line) >= 1 && stoi(line) <= static_cast<int>(MAXWIDTH))) {
        cout << "Error: The grid width is illegal!" << endl;
        return false;
    }
    world.grid.width = stoi(line);  // init width

    getline(file, line);
    if (!(stoi(line) >= 1 && stoi(line) <= static_cast<int>(MAXHEIGHT))) {
        cout << "Error: The grid height is illegal!" << endl;
        return false;
    }
    world.grid.height = stoi(line); // init height

    string creature_info;           // init creature(name & direction)
    while (getline(file, creature_info) && creature_info != "") {
        std::istringstream creature;
        string aline;
        bool species_readed = false;
        creature.str(creature_info);
        creature >> aline;

        for (int i = 0; (unsigned int)i < world.numSpecies; i++) {
            if (aline == world.species[i].name) {
                world.numCreatures++;
                if (world.numCreatures > MAXCREATURES) {
                    cout << "Error: Too many creatures!" << endl;
                    cout << "Maximal number of creatures is " << MAXCREATURES << "." << endl;
                    return false;
                }

                world.creatures[world.numCreatures - 1].species = &world.species[i];
                world.creatures[world.numCreatures - 1].programID = 1;

                creature >> aline;
                bool direction_readed = false;
                for (int j = 0; j < 4; j++) {
                    if (aline == directName[j]) {
                        world.creatures[world.numCreatures - 1].direction = static_cast<direction_t>(j);
                        direction_readed = true;
                        break;
                    }
                }
                if (!direction_readed) {
                    cout << "Error: Direction " << aline << " is not recognized!" << endl;
                    return false;
                }

                creature >> aline;
                int r = stoi(aline);
                creature >> aline;
                int c = stoi(aline);

                if (!(r >= 0 && r < static_cast<int>(world.grid.height) && c >= 0 && c < static_cast<int>(world.grid.width))) {
                    cout << "Error: Creature (" << creature_info << ") is out of bound!" << endl;
                    cout << "The grid size is "<<world.grid.height<<"-by-"<<world.grid.width<<"."<<endl;
                    return false;
                }
                else if (world.grid.squares[r][c] != nullptr) {
                    cout << "Error: Creature (" << creature_info << ") overlaps with creature ("
                         << world.grid.squares[r][c]->species->name << " "
                         << directName[world.grid.squares[r][c]->direction] << " "
                         << world.grid.squares[r][c]->location.r << " "
                         << world.grid.squares[r][c]->location.c << ")!" << endl;
                    return false;
                }

                world.creatures[world.numCreatures - 1].location.r = r;
                world.creatures[world.numCreatures - 1].location.c = c;
                world.grid.squares[r][c] = &world.creatures[world.numCreatures - 1];
                species_readed = true;
                break;
            }
        }
        if (!species_readed) {
            cout << "Error: Species " << aline << " not found!" << endl;
            return false;
        }
    }
    return true;
}

void simulateCreature(creature_t &creature, world_t &world, bool verbose, void(*fn[])(creature_t &, world_t &)) {
    if (creature.species == nullptr) {
        cout << "Error: Invalid species pointer for creature at (" << creature.location.r << ", " << creature.location.c << ")" << endl;
        exit(1); 
    }

    if (verbose) {
        cout << endl;
        while (creature.programID < creature.species->programSize && (int)creature.species->program[creature.programID - 1].op > 3) {
            cout << "Instruction " << creature.programID << ": " << opName[creature.species->program[creature.programID - 1].op] << " " << creature.species->program[creature.programID - 1].address << endl;
            //cout << "Creature species: " << creature.species->name << endl;
            //cout << "Creature location: " << creature.location.r << ", " << creature.location.c << endl;
            //cout << "Creature direction: " << directName[creature.direction] << endl;
            //cout << "Current programID: " << creature.programID << endl;
            //cout << "Program size: " << creature.species->programSize << endl;
            fn[creature.species->program[creature.programID - 1].op](creature, world);
            printGrid(world.grid);
        }
    } else {
        while (creature.programID < creature.species->programSize && (int)creature.species->program[creature.programID - 1].op > 3) {
            fn[creature.species->program[creature.programID - 1].op](creature, world);
        }
        cout << opName[creature.species->program[creature.programID - 1].op] << endl;
        //cout << "Creature species: " << creature.species->name << endl;
        //cout << "Creature location: " << creature.location.r << ", " << creature.location.c << endl;
        //cout << "Creature direction: " << directName[creature.direction] << endl;
        //cout << "Current programID: " << creature.programID << endl;
        //cout << "Program size: " << creature.species->programSize << endl;
        fn[creature.species->program[creature.programID - 1].op](creature, world);
    }
}


void printGrid(const grid_t &grid){
    for(int i = 0; (unsigned int)i < grid.height; i++){
        for(int j = 0; (unsigned int)j < grid.width; j++){
            if(grid.squares[i][j] == nullptr){
                cout << "____ ";
            }
            else{
                cout << grid.squares[i][j]->species->name[0] << grid.squares[i][j]->species->name[1] << "_" << directShortName[grid.squares[i][j]->direction] << " ";
            }
        }
        cout << endl;
    }
}

creature_t* getCreatureInDirection(creature_t &creature, world_t &world) {
    point_t newLocation = calculateNextPosition(creature, creature.direction);
    return world.grid.squares[newLocation.r][newLocation.c];
}

point_t calculateNextPosition(creature_t &creature, direction_t dir) {
    point_t newLocation = { creature.location.r, creature.location.c };
    switch (dir) {
        case NORTH: newLocation.r--; break;
        case SOUTH: newLocation.r++; break;
        case EAST: newLocation.c++; break;
        case WEST: newLocation.c--; break;
    }
    return newLocation;
}

void hop(creature_t &creature, world_t &world){
    if(creature.direction == NORTH && creature.location.r>0 && world.grid.squares[creature.location.r - 1][creature.location.c]==nullptr){
        world.grid.squares[creature.location.r][creature.location.c]=nullptr;
        creature.location.r--;
        world.grid.squares[creature.location.r][creature.location.c] = &creature;
        creature.programID++;
        if (creature.programID > creature.species->programSize){ creature.programID = (creature.programID+1) % (creature.species->programSize);}
    }
    else if(creature.direction == WEST && creature.location.c>0 && world.grid.squares[creature.location.r][creature.location.c - 1] ==nullptr){
        world.grid.squares[creature.location.r][creature.location.c]=nullptr;
        creature.location.c--;
        world.grid.squares[creature.location.r][creature.location.c] = &creature;
        creature.programID++;
        if (creature.programID > creature.species->programSize){creature.programID = (creature.programID+1) % (creature.species->programSize);}
    }
    else if(creature.direction == SOUTH && (unsigned int)creature.location.r < world.grid.height - 1 && world.grid.squares[creature.location.r + 1][creature.location.c]==nullptr){
        world.grid.squares[creature.location.r][creature.location.c]=nullptr;
        creature.location.r++;
        world.grid.squares[creature.location.r][creature.location.c] = &creature;
        creature.programID++;
        if (creature.programID > creature.species->programSize){ creature.programID = (creature.programID+1) % (creature.species->programSize);}
    }
    else if(creature.direction == EAST && (unsigned int)creature.location.c < world.grid.width - 1 && world.grid.squares[creature.location.r][creature.location.c + 1]==nullptr){
        world.grid.squares[creature.location.r][creature.location.c]=nullptr;
        creature.location.c++;
        world.grid.squares[creature.location.r][creature.location.c] = &creature;
        creature.programID++;
        if (creature.programID > creature.species->programSize){creature.programID = (creature.programID+1) % (creature.species->programSize);}
    }

    else {
        creature.programID++;
        if(creature.programID>creature.species->programSize) {creature.programID = (creature.programID+1) % (creature.species->programSize);}
    }
}

void ifsame(creature_t &creature, world_t &world){
    creature_t* frontCreature = getCreatureInDirection(creature, world);
    if (frontCreature && frontCreature->species == creature.species) {
        creature.programID = creature.species->program[creature.programID - 1].address;
    } else {
        creature.programID = (creature.programID + 1) % creature.species->programSize;
    }
}

void ifwall(creature_t &creature, world_t &world){
    if (creature.direction == NORTH && creature.location.r == 0){
        creature.programID = creature.species->program[creature.programID-1].address;}
    else if (creature.direction == WEST && creature.location.c == 0){
        creature.programID = creature.species->program[creature.programID-1].address;}
    else if (creature.direction == SOUTH && (unsigned int)creature.location.r == world.grid.height-1){
        creature.programID = creature.species->program[creature.programID-1].address;}
    else if (creature.direction == EAST && (unsigned int)creature.location.c == world.grid.width-1){
        creature.programID = creature.species->program[creature.programID-1].address;}
    
    else{
        creature.programID++;
        if(creature.programID > creature.species->programSize){ creature.programID = (creature.programID+1) % (creature.species->programSize);}
    }
}

void ifempty(creature_t &creature, world_t &world){
    creature_t* frontCreature = getCreatureInDirection(creature, world);
    if (!frontCreature) {
        creature.programID = creature.species->program[creature.programID - 1].address;
    } else {
        creature.programID = (creature.programID + 1) % creature.species->programSize;
    }
}


void infect(creature_t &creature, world_t &world){
    creature_t* frontCreature = getCreatureInDirection(creature, world);
    if (frontCreature && frontCreature->species != creature.species) {
        frontCreature->species = creature.species;
        frontCreature->programID = 1;
    }

    creature.programID = (creature.programID + 1) % creature.species->programSize;
}

void ifenemy(creature_t &creature, world_t &world){
    if (creature.direction == NORTH && creature.location.r > 0 && world.grid.squares[creature.location.r - 1][creature.location.c]!=nullptr){
        if (world.grid.squares[creature.location.r - 1][creature.location.c]->species != creature.species){creature.programID = creature.species->program[creature.programID - 1].address;}
        else{
            creature.programID++;
            if(creature.programID > creature.species->programSize)
            {creature.programID = (creature.programID+1) % (creature.species->programSize);}
        }
    }

    else if (creature.direction == SOUTH && (unsigned int)creature.location.r < world.grid.height - 1 && world.grid.squares[creature.location.r + 1][creature.location.c]!=nullptr){
        if (world.grid.squares[creature.location.r + 1][creature.location.c]->species != creature.species){creature.programID = creature.species->program[creature.programID - 1].address;}
        else{
            creature.programID++;
            if (creature.programID > creature.species->programSize){creature.programID = (creature.programID+1) % (creature.species->programSize);}
        }
    }

    else if (creature.direction == EAST && (unsigned int)creature.location.c < world.grid.width - 1 && world.grid.squares[creature.location.r][creature.location.c + 1]!=nullptr){
        if (world.grid.squares[creature.location.r][creature.location.c + 1]->species != creature.species){creature.programID = creature.species->program[creature.programID - 1].address;}
        else{
            creature.programID++;
            if (creature.programID > creature.species->programSize){creature.programID = (creature.programID+1) % (creature.species->programSize);}
        }
    }

    else if (creature.direction == WEST && creature.location.c > 0 && world.grid.squares[creature.location.r][creature.location.c - 1] !=nullptr){
        if (world.grid.squares[creature.location.r][creature.location.c - 1]->species != creature.species){creature.programID = creature.species->program[creature.programID - 1].address;}
        else{
            creature.programID++;
            if (creature.programID > creature.species->programSize){creature.programID = (creature.programID+1) % (creature.species->programSize);}
        }
    }


    else{
        creature.programID++;
        if (creature.programID > creature.species->programSize){creature.programID = (creature.programID+1) % (creature.species->programSize);}
    }
}  


void go(creature_t &creature, world_t &world){
    creature.programID = creature.species->program[creature.programID-1].address;
}

void left(creature_t &creature, world_t &world){
    //cout << "Executing left" << endl;
    //cout << "Creature species: " << creature.species->name << endl;
    //cout << "Creature location: " << creature.location.r << ", " << creature.location.c << endl;
    //cout << "Creature direction: " << directName[creature.direction] << endl;


    creature.direction = static_cast<direction_t>((creature.direction + 3) % 4);

    creature.programID = (creature.programID + 1) % creature.species->programSize;
    if (creature.programID < 0 || creature.programID >= creature.species->programSize) {
        cout << "Error: Invalid programID " << creature.programID << " for species " << creature.species->name << endl;
        exit(1); 
    }
    //cout << "Updated direction: " << directName[creature.direction] << endl;
}

void right(creature_t &creature, world_t &world){
    creature.direction = static_cast<direction_t>((creature.direction + 1) % 4); 
    creature.programID = (creature.programID + 1) % creature.species->programSize;
}


