//
// Created by Alejandro on 19/08/16.
//

#include <fstream>
#include "World.h"
#include "Wall.h"
#include <string.h>
#include <cmath>

World::World()
{

}

void World::init(const char *mapName, std::vector<Player> *players)
{
    reset();
    this->players = players;
    readMap(mapName);
    createAreas();
    indexStaticEntities();
}

int World::parseMapParameter(std::string & line)
{
    auto commaPos = line.find(',');
    char * parameter = (char *)malloc((commaPos+1)*sizeof(char));
    strcpy(parameter,line.substr(0,commaPos).c_str());
    line.erase(0, line.find(',') + 1);
    int value = atoi(parameter);
    free(parameter);
    return value;
}

void World::readMap(const char *name)
{
    std::ifstream mapFile(name);
    std::string line = "";

    std::getline(mapFile, line);
    bounds.width = this->parseMapParameter(line);
    bounds.height = this->parseMapParameter(line);
    area_size = atoi(line.c_str());

    while (std::getline(mapFile, line))
    {
        int objectType = parseMapParameter(line);
        int left =  parseMapParameter(line);
        int top =  parseMapParameter(line);
        int width =   parseMapParameter(line);
        int height =  atoi(line.c_str());

        if (objectType == 0) //wall
            world_entities.push_back(Wall(left, top, width, height));
    }
}


void World::indexStaticEntities()
{
    for (auto& entity : world_entities)
    {
        if (entity.isStatic)
        {
            for (auto& area : areasForEntity(entity))
            {
                static_entities[area].push_back(&entity);
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
        checkWallCollisions(player);
        auto currentPlayerEntityId = player.entityId;
        for (auto &proj : player.projectiles)
        {
            //printf("currplayerentity %i\n", currentPlayerEntityId);
            checkProjectileCollisions(currentPlayerEntityId, proj);
        }
        player.update(elapsedTime);
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
                //printf("Intersected static!!\n");
                proj.intersectedWith(other_entity, intersection);
            }
        }
        if (!proj.valid) return;
        //printf("area %i\n", area);
        for (auto& target : moving_entities[area])
        {
            //printf("Moving entity %i curr player: %i\n", target->entityId, currentPlayerEntityId);
            if (target->entityId == currentPlayerEntityId)
                return;
            sf::FloatRect intersection;
            if (target->boundingBox.intersects(proj.boundingBox, intersection))
            {
                //printf("intersected moving!\n");
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
    static_entities.clear();
    world_entities.clear();
    moving_entities.clear();
}
void World::indexMovingEntities()
{
    moving_entities.clear();
    for (auto& entity : *players)
    {
        for (auto& area : areasForEntity(entity))
        {
            //printf("area: %i, player:%i\n", area, entity.playerId);
            moving_entities[area].push_back(&entity);
        }
    }
}









