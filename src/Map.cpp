
#include "Engine.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "Log.h"
#include "Physics.h"

#include <math.h>

Map::Map() : Module(), mapLoaded(false)
{
    name = "map";
}

// Destructor
Map::~Map()
{}

// Called before render is available
bool Map::Awake()
{
    name = "map";
    LOG("Loading Map Parser");

    return true;
}

bool Map::Start() {

    return true;
}

bool Map::Update(float dt)
{
    bool ret = true;

    if (mapLoaded) {

        // L07 TODO 5: Prepare the loop to draw all tiles in a layer + DrawTexture()
        // iterate all tiles in a layer
        for (const auto& mapLayer : mapData.layers) 
        {
            if (mapLayer->properties.GetProperty("Draw") != NULL and mapLayer->properties.GetProperty("Draw")->value)
            {
                for (int i = 0; i < mapData.height; i++) 
                {
                    for (int j = 0; j < mapData.width; j++) 
                    {
                        // L07 TODO 9: Complete the draw function
                    
                        //Get the gid from tile
                        uint32_t gid = mapLayer->Get(i, j);

                        //Check if the gid is different from 0 - some tiles are empty
                        if (gid != 0) 
                        {
                            // Decode flip flags from GID
                            const uint32_t FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
                            const uint32_t FLIPPED_VERTICALLY_FLAG = 0x40000000;
                            const uint32_t FLIPPED_DIAGONALLY_FLAG = 0x20000000;
                            const uint32_t TILE_ID_MASK = 0x1FFFFFFF;

                            //Get Flip Variables and Correct Tile GID
                            bool flipH = (gid & FLIPPED_HORIZONTALLY_FLAG) != 0;
                            bool flipV = (gid & FLIPPED_VERTICALLY_FLAG) != 0;
                            bool flipD = (gid & FLIPPED_DIAGONALLY_FLAG) != 0;
                            uint32_t tileId = gid & TILE_ID_MASK; 

                            // Determine rotation and final horizontal flip
                            float rotation = 0.0f;
                            SDL_FlipMode sdlFlip = SDL_FLIP_NONE;

                            if (!flipD) 
                            {
                                if (flipH && flipV) { rotation = 180.0f;}
                                else if (flipH) { sdlFlip = SDL_FLIP_HORIZONTAL;}
                                else if (flipV) { sdlFlip = SDL_FLIP_VERTICAL; }
                            }
                            else // Diagonal Flip  == True
                            { 
                                if (!flipH && !flipV) { rotation = 90.0f; sdlFlip = SDL_FLIP_HORIZONTAL; }
                                else if (flipH && !flipV) { rotation = 90.0f;}
                                else if (!flipH && flipV) { rotation = 270.0f;}
                                else if (flipH && flipV) { rotation = 270.0f; sdlFlip = SDL_FLIP_HORIZONTAL;}
                            }

                            //L09: TODO 3: Obtain the tile set using GetTilesetFromTileId
                            TileSet* tileSet = GetTilesetFromTileId(tileId);

                            if (tileSet != nullptr) {
                                //Get the Rect from the tileSetTexture;
                                SDL_FRect tileRect;
                                tileRect.x = tileSet->GetRect(tileId).x;
                                tileRect.y = tileSet->GetRect(tileId).y;
                                tileRect.w = tileSet->GetRect(tileId).w;
                                tileRect.h = tileSet->GetRect(tileId).h;

                                //Get the screen coordinates from the tile coordinates
                                Vector2D mapCoord = MapToWorld(i, j);

                                // Center point for rotation
                                SDL_FPoint center = { tileRect.w / 2, tileRect.h / 2 };

                                // Destination rectangle
                                SDL_FRect dstRect = {
                                    (int)mapCoord.getX(),
                                    (int)mapCoord.getY(),
                                    tileRect.w,
                                    tileRect.h
                                };


                                //Draw the texture
                                SDL_RenderTextureRotated(Engine::GetInstance().render->renderer, tileSet->texture, &tileRect, &dstRect, rotation, &center, sdlFlip);
                                //Engine::GetInstance().render->DrawTexture(tileSet->texture, (int)mapCoord.getX(), (int)mapCoord.getY(), &tileRect, 0, rotation);
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

// L09: TODO 2: Implement function to the Tileset based on a tile id
TileSet* Map::GetTilesetFromTileId(int gid) const
{
    for (const auto& tileset : mapData.tilesets)
    {
        if (gid >= tileset->firstGid and gid < (tileset->firstGid + tileset->tileCount))
        {
            return tileset;
        }
    }
}

// Called before quitting
bool Map::CleanUp()
{
    LOG("Unloading map");

    // L06: TODO 2: Make sure you clean up any memory allocated from tilesets/map
    for (const auto& tileset : mapData.tilesets) {
        delete tileset;
    }
    mapData.tilesets.clear();

    // L07 TODO 2: clean up all layer data
    for (const auto& layer : mapData.layers)
    {
        delete layer;
    }
    mapData.layers.clear();

    return true;
}

// Load new map
bool Map::Load(std::string path, std::string fileName)
{
    bool ret = false;

    // Assigns the name of the map file and the path
    mapFileName = fileName;
    mapPath = path;
    std::string mapPathName = mapPath + mapFileName;

    pugi::xml_document mapFileXML;
    pugi::xml_parse_result result = mapFileXML.load_file(mapPathName.c_str());

    if(result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", mapPathName.c_str(), result.description());
		ret = false;
    }
    else {

        // L06: TODO 3: Implement LoadMap to load the map properties
        // retrieve the paremeters of the <map> node and store the into the mapData struct
        mapData.width = mapFileXML.child("map").attribute("width").as_int();
        mapData.height = mapFileXML.child("map").attribute("height").as_int();
        mapData.tileWidth = mapFileXML.child("map").attribute("tilewidth").as_int();
        mapData.tileHeight = mapFileXML.child("map").attribute("tileheight").as_int();

        // L06: TODO 4: Implement the LoadTileSet function to load the tileset properties
       
        //Iterate the Tileset
        for(pugi::xml_node tilesetNode = mapFileXML.child("map").child("tileset"); tilesetNode!=NULL; tilesetNode = tilesetNode.next_sibling("tileset"))
		{
            //Load Tileset attributes
			TileSet* tileSet = new TileSet();
            tileSet->firstGid = tilesetNode.attribute("firstgid").as_int();
            tileSet->name = tilesetNode.attribute("name").as_string();
            tileSet->tileWidth = tilesetNode.attribute("tilewidth").as_int();
            tileSet->tileHeight = tilesetNode.attribute("tileheight").as_int();
            tileSet->spacing = tilesetNode.attribute("spacing").as_int();
            tileSet->margin = tilesetNode.attribute("margin").as_int();
            tileSet->tileCount = tilesetNode.attribute("tilecount").as_int();
            tileSet->columns = tilesetNode.attribute("columns").as_int();

			//Load the tileset image
			std::string imgName = tilesetNode.child("image").attribute("source").as_string();
            tileSet->texture = Engine::GetInstance().textures->Load((mapPath+imgName).c_str());

			mapData.tilesets.push_back(tileSet);
		}

        // L07: TODO 3: Iterate all layers in the TMX and load each of them
        for (pugi::xml_node layerNode = mapFileXML.child("map").child("layer"); layerNode != NULL; layerNode = layerNode.next_sibling("layer")) {

            // L07: TODO 4: Implement the load of a single layer 
            //Load the attributes and saved in a new MapLayer
            MapLayer* mapLayer = new MapLayer();
            mapLayer->id = layerNode.attribute("id").as_int();
            mapLayer->name = layerNode.attribute("name").as_string();
            mapLayer->width = layerNode.attribute("width").as_int();
            mapLayer->height = layerNode.attribute("height").as_int();
            
            //L09: TODO 6 Call Load Layer Properties
            LoadProperties(layerNode, mapLayer->properties);

            //Iterate over all the tiles and assign the values in the data array
            for (pugi::xml_node tileNode = layerNode.child("data").child("tile"); tileNode != NULL; tileNode = tileNode.next_sibling("tile")) {
                mapLayer->tiles.push_back(tileNode.attribute("gid").as_uint());
            }

            //add the layer to the map
            mapData.layers.push_back(mapLayer);
        }

        for (pugi::xml_node objectGroupNode = mapFileXML.child("map").child("objectgroup"); objectGroupNode != NULL; objectGroupNode = objectGroupNode.next_sibling("objectgroup")) {

            // L07: TODO 4: Implement the load of a single layer 
            //Load the attributes and saved in a new MapLayer
            ObjectGroup* objectgroup = new ObjectGroup();

            for (pugi::xml_node objectNode = objectGroupNode.child("object"); objectNode != NULL; objectNode = objectNode.next_sibling("object")) {
                ObjectGroup::Object* o = new ObjectGroup::Object;
                o->id = objectNode.attribute("id").as_int();
                o->x = objectNode.attribute("x").as_float();
                o->y = objectNode.attribute("y").as_float();
                o->width = objectNode.attribute("width").as_float();
                o->height = objectNode.attribute("height").as_float();

                if (objectNode.child("polygon").attribute("points") != NULL)
                {
                   std::string pointString = objectNode.child("polygon").attribute("points").as_string();
                   size_t start = 0;

                   while (start < pointString.length()) 
                   {
                       size_t end = pointString.find(' ', start);
                       if (end == std::string::npos) { end = pointString.length(); }

                       std::string pair = pointString.substr(start, end - start);
                       size_t comma = pair.find(',');

                       if (comma != std::string::npos) 
                       {
                           b2Vec2 pointPos = { stoi(pair.substr(0, comma))+o->x,  stoi(pair.substr(comma + 1)) + o->y};
                           o->points.push_back(pointPos);
                       }

                       start = end + 1;
                   }
                }
                objectgroup->objects.push_back(o);
            }

            LoadProperties(objectGroupNode, objectgroup->properties);

            //Add the layer to the map
            mapData.objectGroups.push_back(objectgroup);
        }

        // L08 TODO 3: Create colliders
        // L08 TODO 7: Assign collider type
        // Later you can create a function here to load and create the colliders from the map
        for (const auto& objectsGroups : mapData.objectGroups)
        {
            if (objectsGroups->properties.GetProperty("Square") != NULL and objectsGroups->properties.GetProperty("Square")->value)
            {
                for (const auto& obj : objectsGroups->objects)
                {
                    PhysBody* collider = Engine::GetInstance().physics.get()->CreateRectangle(obj->x + obj->width/2, obj->y + obj->height/2, obj->width, obj->height, STATIC);
                    collider->ctype = ColliderType::PLATFORM;
                }
            }
            else if (objectsGroups->properties.GetProperty("Circle") != NULL and objectsGroups->properties.GetProperty("Circle")->value)
            {
                for (const auto& obj : objectsGroups->objects)
                {
                    PhysBody* collider = Engine::GetInstance().physics.get()->CreateCircle(obj->x + obj->width / 2, obj->y + obj->height / 2, obj->width/2, STATIC);
                    collider->ctype = ColliderType::PLATFORM;
                }
            }
            else if(objectsGroups->properties.GetProperty("Triangle") != NULL and objectsGroups->properties.GetProperty("Triangle")->value)
            {
                for (const auto& obj : objectsGroups->objects)
                {
                    int* points = new int[obj->points.size() * 2];

                    for (size_t i = 0; i < obj->points.size(); i++)
                    {
                        points[i * 2] = obj->points[i].x;
                        points[i * 2 + 1] = obj->points[i].y;
                    }
                    PhysBody* collider = Engine::GetInstance().physics.get()->CreateChain(PIXEL_TO_METERS(obj->x/2), PIXEL_TO_METERS(obj->y/2), points, obj->points.size()*2, STATIC);
                    collider->ctype = ColliderType::PLATFORM;
                }
            }

        }

        ret = true;

        // L06: TODO 5: LOG all the data loaded iterate all tilesetsand LOG everything
        if (ret == true)
        {
            LOG("Successfully parsed map XML file :%s", fileName.c_str());
            LOG("width : %d height : %d", mapData.width, mapData.height);
            LOG("tile_width : %d tile_height : %d", mapData.tileWidth, mapData.tileHeight);
            LOG("Tilesets----");

            //iterate the tilesets
            for (const auto& tileset : mapData.tilesets) {
                LOG("name : %s firstgid : %d", tileset->name.c_str(), tileset->firstGid);
                LOG("tile width : %d tile height : %d", tileset->tileWidth, tileset->tileHeight);
                LOG("spacing : %d margin : %d", tileset->spacing, tileset->margin);
            }
            			
            LOG("Layers----");

            for (const auto& layer : mapData.layers) {
                LOG("id : %d name : %s", layer->id, layer->name.c_str());
				LOG("Layer width : %d Layer height : %d", layer->width, layer->height);
            }   
        }
        else {
            LOG("Error while parsing map file: %s", mapPathName.c_str());
        }

        if (mapFileXML) mapFileXML.reset();

    }

    mapLoaded = ret;
    return ret;
}

// L07: TODO 8: Create a method that translates x,y coordinates from map positions to world positions
Vector2D Map::MapToWorld(int i, int j) const
{
    Vector2D ret;

    ret.setX((float)(j * mapData.tileWidth));
    ret.setY((float)(i * mapData.tileHeight));

    return ret;
}

// L09: TODO 6: Load a group of properties from a node and fill a list with it
bool Map::LoadProperties(pugi::xml_node& node, Properties& properties)
{
    bool ret = true;

    for (pugi::xml_node propertiesNode = node.child("properties").child("property"); propertiesNode != NULL; propertiesNode = propertiesNode.next_sibling("property"))
    {
        Properties::Property* p = new Properties::Property;
        p->name = propertiesNode.attribute("name").as_string();
        p->value = propertiesNode.attribute("value").as_bool();
        properties.propertiesList.push_back(p);
    }

    return ret;
}

Vector2D Map::GetMapSizeInPixels()
{
    Vector2D sizeInPixels;
    sizeInPixels.setX((float)(mapData.width * mapData.tileWidth));
    sizeInPixels.setY((float)(mapData.height * mapData.tileHeight));
    return sizeInPixels;
}

