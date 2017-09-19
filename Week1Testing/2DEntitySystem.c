
#include <SDL_image.h>
#include <stdlib.h>

#include "simple_logger.h"
#include "2DEntitySystem.h"
#include "gf2d_text.h"
#include "gf2d_graphics.h"

typedef struct
{
	Uint32 max_entities;
	Entity * entity_list;
}EntityManager;

static EntityManager entity_manager;

void gf2d_entity_close()
{
	gf2d_entity_clear_all();
	if (entity_manager.entity_list != NULL)
	{
		free(entity_manager.entity_list);
	}
	entity_manager.entity_list = NULL;
	entity_manager.max_entities = 0;
	slog("entity system closed");
}

void gf2d_entity_init(Uint32 max)
{
	if (!max)
	{
		slog("cannot intialize a entity manager for Zero entities!");
		return;
	}
	entity_manager.max_entities = max;
	entity_manager.entity_list = (Entity *)malloc(sizeof(Entity)*max);
	memset(entity_manager.entity_list, 0, sizeof(Entity)*max);
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
	{
		slog("failed to init image: %s", SDL_GetError());
	}
	slog("entity system initialized");
	atexit(IMG_Quit);
	atexit(gf2d_entity_close);
}

void gf2d_entity_delete(Entity *entity)
{
	if (!entity)return;
	if (entity->texture != NULL)
	{
		SDL_DestroyTexture(entity->texture);
	}
	memset(entity, 0, sizeof(Entity));//clean up all other data
}

void gf2d_entity_free(Entity *entity)
{
	if (!entity) return;
	entity->ref_count--;
}

void gf2d_entity_clear_all()
{
	int i;
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		gf2d_entity_delete(&entity_manager.entity_list[i]);// clean up the data
	}
}

Entity *gf2d_entity_new()
{
	int i;
	/*search for an unused entity address*/
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if ((entity_manager.entity_list[i].ref_count == 0) && (entity_manager.entity_list[i].texture == NULL))
		{
			entity_manager.entity_list[i].ref_count = 1;//set ref count
			return &entity_manager.entity_list[i];//return address of this array element        }
		}
	}
	/*find an unused entity address and clean up the old data*/
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (entity_manager.entity_list[i].ref_count == 0)
		{
			gf2d_entity_delete(&entity_manager.entity_list[i]);// clean up the old data
			entity_manager.entity_list[i].ref_count = 1;//set ref count
			return &entity_manager.entity_list[i];//return address of this array element
		}
	}
	slog("error: out of entity addresses");
	return NULL;
}

Entity *gf2d_entity_get_by_filename(char * filename)
{
	int i;
	for (i = 0; i < entity_manager.max_entities; i++)
	{
		if (gf2d_line_cmp(entity_manager.entity_list[i].filepath, filename) == 0)
		{
			return &entity_manager.entity_list[i];
		}
	}
	return NULL;// not found
}

Entity *gf2d_entity_load_all(
	char *filename,
	Sint32 frameWidth,
	Sint32 frameHeight,
	Sint32 framesPerLine
)
{
	SDL_Surface *surface = NULL;
	Entity *entity = NULL;

	entity = gf2d_entity_get_by_filename(filename);
	if (entity != NULL)
	{
		// found a copy already in memory
		entity->ref_count++;
		return entity;
	}

	entity = gf2d_entity_new();
	if (!entity)
	{
		return NULL;
	}

	surface = IMG_Load(filename);
	if (!surface)
	{
		slog("failed to load entity image %s", filename);
		gf2d_entity_free(entity);
		return NULL;
	}

	surface = gf2d_graphics_screen_convert(&surface);
	if (!surface)
	{
		slog("failed to load entity image %s", filename);
		gf2d_entity_free(entity);
		return NULL;
	}

	entity->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_renderer(), surface);
	if (!entity->texture)
	{
		slog("failed to load sprite image %s", filename);
		gf2d_entity_free(entity);
		SDL_FreeSurface(surface);
		return NULL;
	}
	SDL_SetTextureBlendMode(entity->texture, SDL_BLENDMODE_BLEND);
	SDL_UpdateTexture(entity->texture,
		NULL,
		surface->pixels,
		surface->pitch);
	if (frameHeight == -1)
	{
		entity->frame_h = surface->h;
	}
	else entity->frame_h = frameHeight;
	if (frameWidth == -1)
	{
		entity->frame_w = surface->w;
	}
	else entity->frame_w = frameWidth;
	entity->frames_per_line = framesPerLine;
	gf2d_line_cpy(entity->filepath, filename);

	SDL_FreeSurface(surface);
	return entity;
}

// Placing load_image function here due to 'levels of indirection from int' error
Entity *gf2d_entity_load_image(char *filename)
{
	return gf2d_entity_load_all(filename, -1, -1, 1);
}

void gf2d_entity_draw_image(Entity *image, Vector2D position)
{
	gf2d_entity_draw(
		image,
		position,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		0);
}

void gf2d_entity_draw(
	Entity * entity,
	Vector2D position,
	Vector2D * scale,
	Vector2D * scaleCenter,
	Vector3D * rotation,
	Vector2D * flip,
	Vector4D * colorShift,
	Uint32 frame)
{
	SDL_Rect cell, target;
	SDL_RendererFlip flipFlags = SDL_FLIP_NONE;
	SDL_Point r;
	int fpl;
	Vector2D scaleFactor = { 1,1 };
	Vector2D scaleOffset = { 0,0 };
	if (!entity)
	{
		return;
	}

	if (scale)
	{
		vector2d_copy(scaleFactor, (*scale));
	}
	if (scaleCenter)
	{
		vector2d_copy(scaleOffset, (*scaleCenter));
	}
	if (rotation)
	{
		vector2d_copy(r, (*rotation));
		r.x *= scaleFactor.x;
		r.y *= scaleFactor.y;
	}
	if (flip)
	{
		if (flip->x)flipFlags |= SDL_FLIP_HORIZONTAL;
		if (flip->y)flipFlags |= SDL_FLIP_VERTICAL;
	}
	if (colorShift)
	{
		SDL_SetTextureColorMod(
			entity->texture,
			colorShift->x,
			colorShift->y,
			colorShift->z);
		SDL_SetTextureAlphaMod(
			entity->texture,
			colorShift->w);
	}

	fpl = (entity->frames_per_line) ? entity->frames_per_line : 1;
	gf2d_rect_set(
		cell,
		frame%fpl * entity->frame_w,
		frame / fpl * entity->frame_h,
		entity->frame_w,
		entity->frame_h);
	gf2d_rect_set(
		target,
		position.x - (scaleFactor.x * scaleOffset.x),
		position.y - (scaleFactor.y * scaleOffset.y),
		entity->frame_w * scaleFactor.x,
		entity->frame_h * scaleFactor.y);
	SDL_RenderCopyEx(gf2d_graphics_get_renderer(),
		entity->texture,
		&cell,
		&target,
		rotation ? rotation->z : 0,
		rotation ? &r : NULL,
		flipFlags);
	if (colorShift)
	{
		SDL_SetTextureColorMod(
			entity->texture,
			255,
			255,
			255);
		SDL_SetTextureAlphaMod(
			entity->texture,
			255);
	}
}