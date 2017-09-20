#ifndef __ENTITY_2D_
#define __ENTITY_2D_

#include "gf2d_sprite.h"

/**
 * @brief initialize entity
 */

typedef struct Entity_S
{
	Vector4D colorShift;
	Vector2D position;
	Vector2D speed;
	float currentFrame;
	float health;
	float jumpHeight;
	Sprite *sprite;
	Bool canMove;
	Bool active;
	void(*update)(struct Entity_S *self);
}Entity;

/**
* @brief Initialize entity
* @param ent
* @param sprite
* @param ent_parent 
*/
void entity_init(Entity *ent, Sprite *sprite, Entity *ent_parent, float frame);

/**
* @brief Updates entity
* @param ent - the entity pointer
*/
void entity_update(Entity *ent);

/**
* @brief Gets rid of entity
* @param sprite - the sprite linked to the entity
*/
void entity_free(Sprite *sprite);



#endif
