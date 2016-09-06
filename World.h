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
    void init(const char * mapName, std::vector<Player*>* players);
    void update(sf::Time elapsedTime);
    void processEvents();
    std::vector<int16_t> areasForEntity(const Entity &entity);
    std::vector<Player*>* players;

    sf::FloatRect bounds;
    //Every position in the first vector represents an area (area 0, 1, ..., n)
    //Every vector in each position has references to every object in the area
    std::vector<std::vector<Entity*>> static_entities;
    void randomSpawn(Player *p);

private:
    std::vector<Area*> areas;
    float area_size;
    int noAreasX;
    int noAreasY;

    std::vector<std::vector<Entity*>> moving_entities;
    std::vector<Entity*> world_entities;
    std::vector<Pickup*> pickups;
    std::vector<sf::Vector2f> spawn_locations;

    void indexStaticEntities();

    void createAreas();

    void reset();

    void indexMovingEntities();

    void checkWallCollisions(Player &player);

    void checkProjectileCollisions(int16_t currentPlayerEntityId, Projectile &proj);

    void readMap2(const char* map);



};


#endif //TEST_SERVER_WORLD_H
