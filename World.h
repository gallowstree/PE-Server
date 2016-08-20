//
// Created by Alejandro on 19/08/16.
//

#ifndef TEST_SERVER_WORLD_H
#define TEST_SERVER_WORLD_H


#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Area.h"
#include "Player.h"

class World
{
public:
    World();
    void init(const char * mapName, std::vector<Player>* players);
    void update(sf::Time elapsedTime);
    void processEvents();
    std::vector<int16_t> areasForEntity(const Entity &entity);
    std::vector<Player>* players;

private:
    sf::FloatRect bounds;
    std::vector<Area*> areas;
    float area_size;

    //Every position in the first vector represents an area (area 0, 1, ..., n)
    //Every vector in each position has references to every object in the area
    std::vector<std::vector<Entity*>> static_entities;
    std::vector<Entity> world_entities;

    void readMap(const char *name);

    int parseMapParameter(std::string line);

    void populateStaticEntities();

    void createAreas();
};


#endif //TEST_SERVER_WORLD_H
