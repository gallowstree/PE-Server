#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <cctype>
#include <SFML/Graphics.hpp>
#include <queue>
#include "serialization.h"
#include "Player.h"
#include "command.h"
#include "Projectile.h"
#include "Wall.h"
#include "Area.h"
#include <unistd.h>
#include <fstream>

const sf::Time TimePerFrame = sf::seconds(1.f/120.f);
const sf::Time TimePerNetworkUpdate = sf::seconds(1.f/30.f);
const ushort COMMAND_BUFFER_SIZE = 1024;

const int16_t s_players_command = 0;
const int16_t s_projectiles_command = 1;
int16_t const s_player_id_command = 2;
const int16_t c_input_command = 0;
const int16_t c_join_game_command = 2;
int message_number = 0;

uint currentFrame = 0;
std::queue<command_t> commandQueue;
pthread_mutex_t commandQueueMutex;

std::vector<Player> players;
//std::map<int32_t, Projectile[]> projectilesInMessage;

const char* serverIP = "127.0.0.1";
std::vector<Area*> areas;
std::vector<const char *> maps = {"maps/level1.txt","maps/level2.txt"};
//Every position in the first vector represents an area (area 0, 1, ..., n)
//Every vector in each position has references to every object in the area
std::vector<std::vector<Entity*>> static_objects;


std::vector<Entity> world_entities;



void networkUpdate();

void init()
{
    nextProjectileId = 0;
    pthread_mutex_init(&commandQueueMutex, NULL);
}




void *listenToClients(void * args)
{
    int udpSocket;
    ssize_t nBytes;
    char buffer[COMMAND_BUFFER_SIZE];
    struct sockaddr_in serverAddr, clientAddr;
    struct sockaddr_storage serverStorage;
    socklen_t addr_size, client_addr_size;
    /*Create UDP socket*/
    udpSocket = socket(PF_INET, SOCK_DGRAM, 0);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(50420);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /*Bind socket with address struct*/
    bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverStorage;

    while(1)
    {

        int16_t commandType;
        nBytes = recvfrom(udpSocket, buffer, COMMAND_BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &addr_size);
        Serialization::charsToShort(buffer, commandType, 0);
        command_t command;
        command.commandType = commandType;

        if (commandType == c_input_command)
        {
            Serialization::charsToShort(buffer, command.playerId, 2);
            Serialization::charsToInt(buffer, command.msgNum, 4);
            Serialization::charsToInt(buffer, command.controls, 8);
            Serialization::charsToFloat(buffer, command.rotation, 12);
        }
        else if (commandType == c_join_game_command)
        {
            command.client_ip = inet_ntoa(clientAddr.sin_addr);
            printf("cliente hueco: %s ip\n",command.client_ip);
        }

        pthread_mutex_lock(&commandQueueMutex);
        commandQueue.push(command);
        pthread_mutex_unlock(&commandQueueMutex);
    }

    return 0;
}


std::vector<int16_t> areasForEntity(const Entity &entity)
{
    std::vector<int16_t> found;

    int i = 0;
    for (auto &area : areas)
    {

        if (area->rect.intersects(entity.boundingBox))
        {
            found.push_back(i);
            //printf("%i ",i);
        }

        i++;
    }
    //printf("\n");
    return found;
}

void update(sf::Time elapsedTime)
{
    for (auto &player : players)
    {
        player.update(elapsedTime);

        for (auto& area: areasForEntity(player))
        {
            for (auto& other_entity : static_objects[area])
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

void networkUpdate()
{
    char outbuffer[512];
    char projectiles[1500];

    size_t pos = 0, projectile_pos = 0;
    Serialization::intToChars(message_number++, outbuffer, pos); //idmsg 0 - 3
    pos += 4;
    Serialization::shortToChars(s_players_command, outbuffer, pos); //Command type 4 - 5
    pos += 2;

    Serialization::intToChars(message_number++, projectiles, projectile_pos);
    projectile_pos += 4;
    Serialization::intToChars(s_projectiles_command, projectiles, projectile_pos);
    projectile_pos += 2;

    for (auto &player : players)
    {
        pos += player.serialize(outbuffer, pos);

        for (auto &projectile : player.projectiles)
        {
            projectile_pos += projectile.serialize(projectiles, projectile_pos);
        }
    }

    Serialization::shortToChars(-1, outbuffer, pos);
    pos += 2;

    Serialization::shortToChars(-1, projectiles, projectile_pos);
    projectile_pos += 2;

    //Maybe move this to another thread?
    for (auto &player : players)
    {

        player.send(outbuffer, pos);

        if (projectile_pos > 10)
        {
            player.send(projectiles, projectile_pos);
        }

        if (player.hasNotAckedId)
        {
            printf("has not acked \n");
            char playerIdBuffer[100];
            size_t pId_pos = 0;

            Serialization::intToChars(message_number++, playerIdBuffer, pId_pos);
            pId_pos += 4;
            Serialization::shortToChars(s_player_id_command, playerIdBuffer, pId_pos);
            pId_pos += 2;
            Serialization::shortToChars(player.playerId, playerIdBuffer, pId_pos);
            pId_pos += 2;

            player.send(playerIdBuffer, pId_pos);
        }

    }
}

void processEvents()
{
    pthread_mutex_lock(&commandQueueMutex);
    while (!commandQueue.empty())
    {
        command_t command = commandQueue.front();
        commandQueue.pop();

        if (command.commandType == c_input_command && command.playerId != -1 && players.size() >= command.playerId)
        {
            players[command.playerId].controls = command.controls;
            players[command.playerId].rotation = command.rotation;
            players[command.playerId].hasNotAckedId = false;
        }
        else if (command.commandType == c_join_game_command)
        {


            int16_t playerIndex = -1;
            for(auto &player : players)
            {
                if(strcmp(player.ip,command.client_ip) == 0)
                    playerIndex = player.playerId;
            }
            printf("Processing join request %s %d\n",command.client_ip,playerIndex);
            if(playerIndex == -1)
            {
                int16_t new_player_id = (int16_t)players.size();
                char * c_ip = (char *)malloc(strlen(command.client_ip)+1);
                strcpy(c_ip,command.client_ip);
                Player newPlayer(new_player_id, c_ip, 50421, sf::Vector2f(20.0f,20.0f));
                newPlayer.movementBounds = sf::FloatRect(0.0f, 0.0f, 2400.0f, 2400.0f);
                players.push_back(newPlayer);
                printf("Inserted player %d\n", new_player_id);
            }
            else
            {
                players[playerIndex].hasNotAckedId = true;
            }
        }
    }
    pthread_mutex_unlock(&commandQueueMutex);

}


int parseMapParameter(std::string & line)
{
    auto commaPos = line.find(',');
    char * parameter = (char *)malloc((commaPos+1)*sizeof(char));
    strcpy(parameter,line.substr(0,commaPos).c_str());
    line.erase(0, line.find(',') + 1);
    int value = atoi(parameter);
    free(parameter);
    return value;
}

void readMap(int map)
{
    std::ifstream mapFile(maps[map]);
    std::string line;

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
void createStaticObjects()
{


    readMap(0);

    for (auto& entity : world_entities)
    {
        if (entity.isStatic)
        {
            for (auto& area : areasForEntity(entity))
            {
                static_objects[area].push_back(&entity);
            }
        }
    }

}

void initWorld()
{
    int noAreasX = 0;
    int noAreasY = 0;

    float area_size = 400;
    sf::FloatRect bounds(0,0, 2400, 2400);

    noAreasX = bounds.width / area_size;
    noAreasY = bounds.height / area_size;

    for (int x = 0; x < noAreasX; x++)
    {
        for (int y = 0; y < noAreasY; y++)
        {
            Area* newArea = new Area(x*area_size, y*area_size, area_size, area_size);
            areas.push_back(newArea);
            static_objects.push_back(std::vector<Entity*>());
        }
    }

    createStaticObjects();

}

int main()
{
    init();
    initWorld();
    pthread_t listeningThread;
    pthread_create(&listeningThread, NULL, listenToClients, NULL);

    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    sf::Time timeSinceLastNetworkUpdate = sf::Time::Zero;
    while (true)
    {
        sf::Time elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;
        timeSinceLastNetworkUpdate += elapsedTime;
        while (timeSinceLastUpdate > TimePerFrame)
        {
            currentFrame++;
            timeSinceLastUpdate -= TimePerFrame;
            processEvents();
            update(TimePerFrame);
        }

        while (timeSinceLastNetworkUpdate > TimePerNetworkUpdate)
        {
            timeSinceLastNetworkUpdate -= TimePerNetworkUpdate;
            networkUpdate();
        }
    }

    pthread_join(listeningThread, NULL);
}