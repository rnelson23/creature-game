#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct Creature {
    int type;
    int index;
    int *numCreatures;
    struct Room *room;
    struct Creature *creatures;
};

struct Room {
    int state;
    int index;
    int numCreatures;
    struct Room *neighbors[4];
    struct Creature *creatures[10];
};

struct Dictionary {
    char *states[3];
    char *creatures[3];
    char *reactions[4];
    char *directions[4];
};

enum {Local = 0, Global = 1};
enum {Clean = 0, Dirty = 2};

enum {CleanAction = -1, DirtyAction = 1};
enum {HumanGrumble = -1, AnimalGrowl = 1};

enum {Player = 0, Animal = 1, Human = 2};
enum {North = 0, South = 1, East = 2, West = 3};

void createRooms(struct Room rooms[], int r);
void createCreatures(struct Room rooms[], struct Creature creatures[], int *c);

void look();
void move(struct Creature *creature, int direction);
void clean(struct Creature *actor);
void dirty(struct Creature *actor);

void reaction(struct Creature *reactor, struct  Creature *actor, int action);
void removeCreature(struct Creature *creature, int scope);

int pickDirection(struct Room *room);
int doOutput(struct Room *room);

int respect = 40;
struct Creature *player;
struct Dictionary *dict;

int main(void) {
    printf("Welcome to the game! Please enter starting parameters below:\n");

    srand(time(NULL));

    dict = malloc(sizeof(struct Dictionary));
    *dict = (struct Dictionary) {
        .reactions = {"licks your face", "smiles", "growls", "grumbles"},
        .directions = {"north", "south", "east", "west"},
        .creatures = {"Player", "Animal", "Human"},
        .states = {"clean", "half-dirty", "dirty"}
    };

    int r = 0;
    int c = 0;

    printf("\nNumber of rooms:\n\n > ");
    scanf("%d", &r);

    struct Room *rooms = malloc(sizeof(struct Room) * r);
    createRooms(rooms, r);

    printf("\nNumber of creatures:\n\n > ");
    scanf("%d", &c);

    struct Creature *creatures = malloc(sizeof(struct Creature) * c);
    createCreatures(rooms, creatures, &c);

    printf("\nCommands:");

    do {
        char input[9];
        char command[6];
        struct Creature *creature;

        printf("\n\n > ");
        scanf("%s", input);

        for (int i = 0; i < 9; i++) {
            if (input[i] == ':') {
                char *substr;
                creature = &creatures[strtol(input, &substr, 10)];
                strncpy(command, ++substr, 6);
                break;

            } else {
                creature = player;
                strncpy(command, input, 6);
            }
        }

        if (strcmp(command, "look") == 0) {
            look();

        } else if (strcmp(command, "north") == 0) {
            move(creature, North);

        } else if (strcmp(command, "south") == 0) {
            move(creature, South);

        } else if (strcmp(command, "east") == 0) {
            move(creature, East);

        } else if (strcmp(command, "west") == 0) {
            move(creature, West);

        } else if (strcmp(command, "clean") == 0) {
            clean(creature);

        } else if (strcmp(command, "dirty") == 0) {
            dirty(creature);

        } else if (strcmp(command, "exit") == 0) {
            break;
        }

    } while (respect > 0 && respect <= 80);

    if (respect <= 0) {
        printf("\n\nShame on you! You lose\n");

    } else if (respect > 80) {
        printf("\n\nCongrats! You won\n");

    } else {
        printf("\nGoodbye!\n");
    }

    free(dict);
    free(rooms);
    free(creatures);

    getchar();
    getchar();

    return 0;
}

void createRooms(struct Room rooms[], int r) {
    printf("\nCleanliness value and neighboring room numbers:\n\n");

    for (int i = 0; i < r; i++) {
        int state, north, south, east, west;
        struct Room *room = &rooms[i];

        printf(" > ");
        scanf("%d %d %d %d %d", &state, &north, &south, &east, &west);

        room->index = i;
        room->state = state;
        room->numCreatures = 0;

        room->neighbors[0] = north == -1 ? NULL : &rooms[north];
        room->neighbors[1] = south == -1 ? NULL : &rooms[south];
        room->neighbors[2] = east == -1 ? NULL : &rooms[east];
        room->neighbors[3] = west == -1 ? NULL : &rooms[west];
    }
}

void createCreatures(struct Room rooms[], struct Creature creatures[], int *c) {
    printf("\nCreature type and location:\n\n");

    for (int i = 0; i < *c; i++) {
        int type, roomNumber;

        printf(" > ");
        scanf("%d %d", &type, &roomNumber);

        struct Creature *creature = &creatures[i];
        struct Room *room = &rooms[roomNumber];

        creature->index = i;
        creature->type = type;
        creature->room = room;
        creature->numCreatures = c;
        creature->creatures = creatures;

        room->creatures[room->numCreatures] = creature;
        room->numCreatures++;

        if (type == Player) player = creature;
    }
}

void look() {
    struct Room *room = player->room;
    printf("\nRoom %d: %s\nNeighbors: ", room->index, dict->states[room->state]);

    for (int i = 0; i < 4; i++) {
        if (room->neighbors[i] != NULL) {
            printf("Room %d to the %s ", room->neighbors[i]->index, dict->directions[i]);
        }
    }

    printf("\nCreatures: ");

    for (int i = 0; i < room->numCreatures; i++) {
        printf("%s %d ", dict->creatures[room->creatures[i]->type], room->creatures[i]->index);
    }
}

void move(struct Creature *creature, int direction) {
    struct Room *room = creature->room;
    struct Room *neighbor = room->neighbors[direction];

    if (neighbor != NULL && neighbor->numCreatures < 10) {
        neighbor->creatures[neighbor->numCreatures] = creature;
        neighbor->numCreatures++;

        if (doOutput(room)) {
            char *type = dict->creatures[creature->type];
            printf("\n%s %d leaves towards the %s", type, creature->index, dict->directions[direction]);
        }

        removeCreature(creature, Local);
        creature->room = neighbor;

        if (creature->type == Animal && neighbor->state == Dirty) clean(creature);
        else if (creature->type == Human && neighbor->state == Clean) dirty(creature);

    } else if (neighbor != NULL && doOutput(room)) {
        printf("\nRoom %d is full", room->index);

        if (creature->type != Player) {
            reaction(creature, player, creature->type == Animal ? AnimalGrowl : HumanGrumble);
        }

    } else if (doOutput(room)) {
        printf("\nYou cannot go that way");
    }
}

void clean(struct Creature *actor) {
    struct Room *room = actor->room;
    room->state--;

    if (doOutput(room)) {
        printf("\n%s %d cleans the room", dict->creatures[actor->type], actor->index);
        struct Room roomCopy = *actor->room;

        for (int i = 0; i < roomCopy.numCreatures; i++) {
            reaction(roomCopy.creatures[i], actor, CleanAction);
        }
    }
}

void dirty(struct Creature *actor) {
    struct Room *room = actor->room;
    room->state++;

    if (doOutput(room)) {
        printf("\n%s %d dirties the room", dict->creatures[actor->type], actor->index);
        struct Room roomCopy = *actor->room;

        for (int i = 0; i < roomCopy.numCreatures; i++) {
            reaction(roomCopy.creatures[i], actor, DirtyAction);
        }
    }
}

void reaction(struct Creature *reactor, struct Creature *actor, int action) {
    if (reactor->type == Player) return;
    struct Room *room = reactor->room;

    char *type = dict->creatures[reactor->type];
    char *react = dict->reactions[reactor->type == Animal ? 1 + action : 2 - action];

    respect = reactor->type == Animal ? respect - action : respect + action;

    if (reactor->index == actor->index) {
        respect = reactor->type == Animal ? respect - (action * 2) : respect + (action * 2);
        printf("\n%s %d %s a lot, respect is now %d", type, reactor->index, react, respect);

    } else {
        printf("\n%s %d %s, respect is now %d", type, reactor->index, react, respect);
    }

    if (reactor->type == Animal ? room->state == Dirty : room->state == Clean) {
        int direction = pickDirection(room);

        if (direction != -1) {
            move(reactor, direction);

        } else {
            printf("\n%s %d leaves the house", type, reactor->index);
            removeCreature(reactor, Global);

            for (int i = 0; i < room->numCreatures; i++) {
                struct Creature *newReactor = room->creatures[i];
                reaction(newReactor, actor, newReactor->type == Animal ? AnimalGrowl : HumanGrumble);
            }
        }
    }
}

void removeCreature(struct Creature *creature, int scope) {
    struct Room *room = creature->room;
    struct Creature *creatures = creature->creatures;

    for (int i = 0; i < room->numCreatures; i++) {
        if (room->creatures[i]->index == creature->index) {
            room->numCreatures--;

            for (int j = i; j < room->numCreatures; j++) {
                room->creatures[j] = room->creatures[j + 1];
            }

            break;
        }
    }

    if (scope == Global) {
        *creature->numCreatures -= 1;
        for (int i = creature->index; i < *creature->numCreatures; i++) {
            creatures[i] = creatures[i + 1];
        }
    }
}

int pickDirection(struct Room *room) {
    int directions[4];
    int numRooms = 0;

    for (int i = 0; i < 4; i++) {
        struct Room *neighbor = room->neighbors[i];

        if (neighbor != NULL && neighbor->numCreatures < 10) {
            directions[numRooms] = i;
            numRooms++;
        }
    }

    if (numRooms > 0) return directions[rand() % numRooms];
    return -1;
}

int doOutput(struct Room *room) {
    if (room->index == player->room->index) return 1;
    return 0;
}
