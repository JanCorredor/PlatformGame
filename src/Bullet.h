#pragma once

#include "Entity.h"
#include <SDL3/SDL.h>

struct SDL_Texture;

class Bullet : public Entity
{
public:

	Bullet();
	virtual ~Bullet();

	bool Awake();

	bool Start();

	bool Update(float dt);

	void OnCollision(PhysBody* physA, PhysBody* physB);

	bool CleanUp();

	bool Destroy();

public:
	int speed = 4;

private:

	SDL_Texture* texture;
	const char* texturePath;
	int texW, texH;

	//L08 TODO 4: Add a physics to an Bullet
	PhysBody* pbody;
};
#pragma once
