//
// Created by Alejandro on 19/08/16.
//

#include <fstream>
#include "World.h"
#include "Wall.h"

World::World()
{

}

void World::init(const char *mapName, std::vector<Player> *players)
{
    this->players = players;
    readMap(mapName);
    createAreas();
    populateStaticEntities();
}

void World::readMap(const char *name)
{
    std::ifstream mapFile(name);
    std::string line;

    std::getline(mapFile, line);
    bounds.width = parseMapParameter(line);
    bounds.height = parseMapParameter(line);
    area_size = 400;

    while (std::getline(mapFile, line))
    {
        int objectType = parseMapParameter(line);
        int left =  parseMapParameter(line);
        int top =  parseMapParameter(line);
        int width =   parseMapParameter(line);
        int height =  atoi(line.c_str());

        printf("%f,%f,%f,%f,\n",left,top,width,height);
        if (objectType == 0) //wall
            world_entities.push_back(Wall(left, top, width, height));
    }
}

int World::parseMapParameter(std::string line)
{
    auto commaPos = line.find(',');
    char * parameter = (char *)malloc((commaPos+1)*sizeof(char));
    strcpy(parameter,line.substr(0,commaPos).c_str());
    line.erase(0, line.find(',') + 1);
    int value = atoi(parameter);
    free(parameter);
    return value;
}

void World::populateStaticEntities()
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
    int noAreasX = 0;
    int noAreasY = 0;

    noAreasX = bounds.width / area_size;
    noAreasY = bounds.height / area_size;

    for (int x = 0; x < noAreasX; x++)
    {
        for (int y = 0; y < noAreasY; y++)
        {
            Area* newArea = new Area(x*area_size, y*area_size, area_size, area_size);
            areas.push_back(newArea);
            static_entities.push_back(std::vector<Entity*>());
        }
    }
}

std::vector<int16_t> World::areasForEntity(const Entity &entity)
{
    std::vector<int16_t> found;
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
    for (auto &player : *players)
    {
        player.update(elapsedTime);
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
}





