//
// Created by Alejandro on 19/08/16.
//

#include <fstream>
#include "World.h"
#include "Wall.h"
#include "Pickup.h"
#include <string.h>
#include <cmath>
#include <sstream>

World::World()
{

}

void World::init(const char *mapName, std::vector<Player*> *players)
{
    srand(time(0));
    reset();
    this->players = players;
    readMap2(mapName);
    createAreas();
    indexStaticEntities();
}

void World::indexStaticEntities()
{
    for (auto& entity : world_entities)
    {
        if (entity->isStatic)
        {
            for (auto& area : areasForEntity(*entity))
            {
                static_entities[area].push_back(entity);
            }
        }
    }
}

void World::createAreas()
{
    noAreasX = 0;
    noAreasY = 0;
    noAreasX = bounds.width / area_size;
    noAreasY = bounds.height / area_size;

    for (int x = 0; x < noAreasX; x++)
    {
        for (int y = 0; y < noAreasY; y++)
        {
            Area* newArea = new Area(y*area_size, x*area_size, area_size, area_size);
            areas.push_back(newArea);
            static_entities.push_back(std::vector<Entity*>());
            moving_entities.push_back(std::vector<Entity*>());
        }
    }
}

std::vector<int16_t> World::areasForEntity(const Entity &entity)
{
    std::vector<int16_t> found;
    int x = static_cast<int>(std::floor(entity.boundingBox.left / area_size));
    int y = static_cast<int>(std::floor(entity.boundingBox.top / area_size));

    if (x < 0 || y < 0)
        return found;

    //x + noAreasY * y

    int i = 0;
    for (auto &area : areas)
    {
        if (area->rect.intersects(entity.boundingBox))
        {
            found.push_back(i);
        }
        i++;
    }
    return found;
}


void World::update(sf::Time elapsedTime)
{
    indexMovingEntities();

    for (auto &player : *players)
    {
        checkWallCollisions(*player);
        auto currentPlayerEntityId = player->entityId;
        for (auto &proj : player->projectiles)
        {
            checkProjectileCollisions(currentPlayerEntityId, proj);
        }
        player->update(elapsedTime);
    }

    for (auto &pickup : pickups)
    {
        pickup->update(elapsedTime);
    }

}

void World::checkProjectileCollisions(int16_t currentPlayerEntityId, Projectile &proj)
{
    if (!proj.valid) return;
    for (auto& area: areasForEntity(proj))
    {

        for (auto& other_entity : static_entities[area])
        {
            sf::FloatRect intersection;
            if (other_entity->boundingBox.intersects(proj.boundingBox, intersection))
            {
                proj.intersectedWith(other_entity, intersection);
            }
        }

        if (!proj.valid)
            return;

        for (auto& target : moving_entities[area])
        {
            if (target->entityId == currentPlayerEntityId)
                continue;
            sf::FloatRect intersection;
            if (target->boundingBox.intersects(proj.boundingBox, intersection) && proj.valid)
            {
                target->intersectedWith(&proj, intersection);
                proj.intersectedWith(target, intersection);
            }
        }
    }
}

void World::checkWallCollisions(Player &player)
{
    for (auto& area: areasForEntity(player))
    {
        for (auto& other_entity : static_entities[area])
        {
            sf::FloatRect intersection;
            if (other_entity->boundingBox.intersects(player.boundingBox, intersection))
            {
                player.intersectedWith(other_entity, intersection);
            }
        }
    }
}

void World::reset() {
    areas.clear();

    //Delete all allocated players and clear the vector
    for (auto it = world_entities.begin() ; it != world_entities.end(); ++it)
    {
        delete (*it);
    }

    static_entities.clear();
    world_entities.clear();
    moving_entities.clear();
    pickups.clear();
}
void World::indexMovingEntities()
{
    moving_entities.clear();
    for (auto& entity : *players)
    {
        for (auto& area : areasForEntity(*entity))
        {
            moving_entities[area].push_back(entity);
        }
    }
}


void World::readMap2(const char *name)
{
    std::ifstream mapFile(name);
    std::string line = "";

    while (std::getline(mapFile, line))
    {
        std::vector<const char*> params;
        std::stringstream ss(line);
        std::string item;
        while (getline(ss, item, ','))
        {
            auto paramString = (char *) malloc(item.length());
            strcpy(paramString, item.c_str());
            params.push_back(paramString);
        }

        if (strncmp(params[0], "1", strlen(params[0])) == 0) //World info
        {
            bounds.width  = atoi(params[1]);
            bounds.height = atoi(params[2]);
            area_size     = atoi(params[3]);

            printf("%f, %f, %f \n", bounds.width, bounds.height, area_size);
        }
        else if (strncmp(params[0], "0", strlen(params[0])) == 0) //Wall
        {
            world_entities.push_back(new Wall(atoi(params[1]), atoi(params[2]), atoi(params[3]), atoi(params[4])));
        }
        else if (strncmp(params[0], "3", strlen(params[0])) == 0)//Pickup
        {
            auto pickup = new Pickup(atoi(params[1]), atoi(params[2]), atoi(params[3]),
                                     atoi(params[4]), atoi(params[5]), atoi(params[6]), atoi(params[7]));
            world_entities.push_back(pickup);
            pickups.push_back(pickup);
        }
        else if (strncmp(params[0], "4", strlen(params[0])) == 0)
        {
            spawn_locations.push_back(sf::Vector2f(atof(params[1]), atof(params[2])));
        }

        for (auto& param : params)
        {
            free((void *)param);
        }
    }

}

void World::randomSpawn(Player* p)
{
    int random = rand() % spawn_locations.size();
    auto position = spawn_locations[random];
    p->boundingBox.left = position.x;
    p->boundingBox.top = position.y;

    spawn_locations.erase(spawn_locations.begin() + random);
}


