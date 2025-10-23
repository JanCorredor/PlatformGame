#include "Bullet.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"

Bullet::Bullet() : Entity(EntityType::ITEM)
{
	name = "item";
}

Bullet::~Bullet() {}

bool Bullet::Awake() {
	return true;
}

bool Bullet::Start() {

	//initilize textures
	texture = Engine::GetInstance().textures->Load("Assets/Textures/goldCoin.png");

	// L08 TODO 4: Add a physics to an item - initialize the physics body
	Engine::GetInstance().textures.get()->GetSize(texture, texW, texH);
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX() + texH / 2, (int)position.getY() + texH / 2, texH / 2, bodyType::DYNAMIC);

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::TRAP;

	// Set this class as the listener of the pbody
	pbody->listener = this;   // so Begin/EndContact can call back to Item

	return true;
}

bool Bullet::Update(float dt)
{
	if (!active) return true;
	b2Vec2 velocity = { -speed, 0 };
	//Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);

	//// L08 TODO 4: Add a physics to an item - update the position of the object from the physics.  
	//int x, y;
	//pbody->GetPosition(x, y);
	//position.setX((float)x);
	//position.setY((float)y);

	//Engine::GetInstance().render->DrawTexture(texture, x - texW / 2, y - texH / 2);

	return true;
}

void Bullet::OnCollision(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
	case ColliderType::PLAYER:
		LOG("Bullet Collision");
		Destroy();
		break;

	case ColliderType::UNKNOWN:
		LOG("Bullet UNKNOWN Collision");
		break;
	default:
		break;
	}
}

bool Bullet::CleanUp()
{
	Engine::GetInstance().textures->UnLoad(texture);
	Engine::GetInstance().physics->DeletePhysBody(pbody);
	return true;
}

bool Bullet::Destroy()
{
	LOG("Destroying item");
	active = false;
	Engine::GetInstance().entityManager->DestroyEntity(shared_from_this());
	return true;
}