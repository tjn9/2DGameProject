#include "Entity.h"
#include "simple_logger.h"

typedef struct Entity_Manager_S
{
	Uint32 max_entity;
	Entity *ent_list;
}EntityManager;


/**
* @brief Initialize entity
* @param ent
* @param sprite
* @param ent_parent
*/
void entity_init(Entity *ent, Sprite *sprite, Entity *ent_parent, float frame)
{
	ent->sprite = sprite;
	ent->currentFrame = 0;
	ent->speed.x = 0;
	ent->speed.y = 0;


}

/**
* @brief Updates entity
* @param ent - the entity pointer
*/
void entity_update(Entity *ent)
{

}

