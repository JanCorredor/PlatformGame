#include "Player.h"
#include "Engine.h"
#include "Textures.h"
#include "Audio.h"
#include "Input.h"
#include "Render.h"
#include "Scene.h"
#include "Log.h"
#include "Physics.h"
#include "EntityManager.h"
#include "Map.h"

Player::Player() : Entity(EntityType::PLAYER)
{
	name = "Player";
}

Player::~Player() {

}

bool Player::Awake() {

	//L03: TODO 2: Initialize Player parameters
	spawnPoint = Vector2D(32, 192*3);
	position = spawnPoint;
	return true;
}

bool Player::Start() {

	// load
	std::unordered_map<int, std::string> aliases = { {30,"idle"},{60,"move"}, { 85,"moveEffect" }, { 22,"jumpEffect" },{40,"jump"},{0,"jumpEffect"}, {20,"death"}};
	anims.LoadFromTSX("Assets/Textures/playerSpritesheet.tsx", aliases);
	anims.SetCurrent("idle");

	//L03: TODO 2: Initialize Player parameters
	texture = Engine::GetInstance().textures->Load("Assets/Textures/playerSpritesheet.png");

	// L08 TODO 5: Add physics to the player - initialize physics body
	//Engine::GetInstance().textures->GetSize(texture, texW, texH);
	texW = 32;
	texH = 32;
	pbody = Engine::GetInstance().physics->CreateCircle((int)position.getX(), (int)position.getY(), texW / 3, bodyType::DYNAMIC); //teW/2 OG Radius

	// L08 TODO 6: Assign player class (using "this") to the listener of the pbody. This makes the Physics module to call the OnCollision method
	pbody->listener = this;

	// L08 TODO 7: Assign collider type
	pbody->ctype = ColliderType::PLAYER;

	//initialize audio effect
	pickCoinFxId = Engine::GetInstance().audio->LoadFx("Assets/Audio/Fx/coin-collision-sound-342335.wav");

	return true;
}

bool Player::Update(float dt)
{
	if (!isDying)
	{
		if (Engine::GetInstance().input.get()->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) //Toggle Godmode
		{
			godMode = !godMode;
		}

		GetPhysicsValues();
		Move();

		CameraFollows();

		Jump();
		Dash();

		ApplyPhysics();
	}


	Draw(dt);

	return true;
}

void Player::GetPhysicsValues() {
	// Read current velocity
	velocity = Engine::GetInstance().physics->GetLinearVelocity(pbody);

	// Reset horizontal velocity by default, this way the player stops when no key is pressed
	if (godMode)
	{
		velocity = { 0,0 };
	}
	else
	{
		velocity = { velocity.x / 10, velocity.y };  // {0, velocity.y }
	}
}

void Player::Move() {
	
	if (!godMode)
	{
		// Move left/right
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			velocity.x += -speed;
			playerDirection = false;
			anims.SetCurrent("move");
		}
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			velocity.x += speed;
			playerDirection = true;
			anims.SetCurrent("move");
		}
	}
	else
	{
		// Move left/right
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) {
			velocity.x += -speed;
			playerDirection = false;
			anims.SetCurrent("move");
		}
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) {
			velocity.x += speed;
			playerDirection = true;
			anims.SetCurrent("move");
		}
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) {
			velocity.y += -speed;
			anims.SetCurrent("move");
		}
		if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) {
			velocity.y += speed;
			anims.SetCurrent("move");
		}
	}

}

void Player::Jump() {
	// This function can be used for more complex jump logic if needed
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN && isJumping == false) {
		Engine::GetInstance().physics->ApplyLinearImpulseToCenter(pbody, 0.0f, -jumpForce, true);
		anims.SetCurrent("jump");
		isJumping = true;
	}
}

void Player::Dash() 
{
	if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_DOWN && hasDashed == false)  //SDL_SCANCODE_LSHIFT
	{
		float dashX;
		if (playerDirection == false)
		{
			dashX = -dashForce;
		}
		else
		{
			dashX = dashForce;
		}
		velocity.x += dashX;

		//float dashY = 0;
		//if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) 
		//{
		//	dashY = dashForce/10;
		//}
		//if (Engine::GetInstance().input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		//{
		//	dashY = -dashForce/10;
		//}
		//velocity.y += dashY;

		hasDashed = true;
	}
}


void Player::ApplyPhysics() {
	// Preserve vertical speed while jumping
	if (isJumping == true) {
		velocity.y = Engine::GetInstance().physics->GetYVelocity(pbody);
	}

	// Apply velocity via helper
	Engine::GetInstance().physics->SetLinearVelocity(pbody, velocity);
}

void Player::Draw(float dt) {

	anims.Update(dt);
	const SDL_Rect& animFrame = anims.GetCurrentFrame();

	//Last Death Frame
	if (animFrame.x == 224 && animFrame.y == 64 && !godMode)
	{
		Death();
		isDying = false;
		anims.SetCurrent("idle");
	}

	if (velocity.x == 0 && isJumping == false && isDying == false)
	{
		anims.SetCurrent("idle");
	}

	// Update render position using your PhysBody helper
	int x, y;
	pbody->GetPosition(x, y);
	position.setX((float)x);
	position.setY((float)y);

	if (hasDashed)
	{
		Uint8* r = new Uint8; Uint8* g = new Uint8; Uint8* b = new Uint8; SDL_GetTextureColorMod(texture, r, g, b); //Get original texture RGB
		SDL_SetTextureColorMod(texture, 255, 0, 0); //Set to X color 
		int dashX = x - texW / 2;
		int dashY = y - texH / 2;
		if (playerDirection)
		{
			dashX -= speed * 2; //Cambiar
		}
		else
		{
			dashX += speed * 2;
		}
		if (Engine::GetInstance().physics->GetYVelocity(pbody) != 0)
		{
			dashY -= Engine::GetInstance().physics->GetYVelocity(pbody);
		}

		Engine::GetInstance().render->DrawTexture(texture, dashX, dashY, &animFrame);
		SDL_SetTextureColorMod(texture, *r, *g, *b);
	}

	const SDL_FRect& fanimFrame = { (float)animFrame.x, (float)animFrame.y, (float)animFrame.w, (float)animFrame.h };
	//SDLFlip
	SDL_FlipMode sdlFlip = SDL_FLIP_NONE;
	if (!playerDirection)
	{
		sdlFlip = SDL_FLIP_HORIZONTAL;
	}

	b2Vec2 camerapos = { Engine::GetInstance().render->camera.x, Engine::GetInstance().render->camera.y };
	SDL_FPoint center = { x - texW / 2, y - texH / 2 };
	SDL_FRect dstRect = {position.getX() - texW / 2 + camerapos.x,position.getY() - texH / 2 + camerapos.y, texW,	texH};

	SDL_RenderTextureRotated(Engine::GetInstance().render->renderer, texture, &fanimFrame, &dstRect, 0, &center, sdlFlip);

}

void Player::CameraFollows()
{
	float limitLeft = Engine::GetInstance().render->camera.w / 4;
	float limitRight = Engine::GetInstance().map->GetMapSizeInPixels().getX() - Engine::GetInstance().render->camera.w * 3 / 4;
	//L10: TODO 7: Center the camera on the player
	if (position.getX() - limitLeft > 0 and position.getX() < limitRight)
	{
		Engine::GetInstance().render->camera.x = -position.getX() + Engine::GetInstance().render->camera.w / 4;
	}
	else if (position.getX() - limitLeft < 0)
	{
		Engine::GetInstance().render->camera.x = 0;
	}
	else if(Engine::GetInstance().map->GetMapSizeInPixels().getX() - position.getX() < Engine::GetInstance().render->camera.w)
	{
		Engine::GetInstance().render->camera.x = - Engine::GetInstance().map->GetMapSizeInPixels().getX() + Engine::GetInstance().render->camera.w;
	}

}

void Player::Death()
{
	if (godMode == false)
	{	
		isJumping = false;
		hasDashed = false;
		b2Vec2 spawn = { PIXEL_TO_METERS(spawnPoint.getX()), PIXEL_TO_METERS(spawnPoint.getY()) };
		b2Rot rota = {cos(pbody->GetRotation()),sin(pbody->GetRotation())};
		b2Body_SetTransform(pbody->body, spawn, rota);
		anims.SetCurrent("idle");
	}
}

bool Player::CleanUp()
{
	LOG("Cleanup player");
	Engine::GetInstance().textures->UnLoad(texture);
	return true;
}

// L08 TODO 6: Define OnCollision function for the player. 
void Player::OnCollision(PhysBody* physA, PhysBody* physB) {
	switch (physB->ctype)
	{
	case ColliderType::CHECKPOINT:
		spawnPoint = position;
		LOG("Collision Checkpoint");
		physB->~PhysBody();
		break;
	case ColliderType::PLATFORM:
		LOG("Collision PLATFORM");
		//reset the jump flag when touching the ground

		isJumping = false;
		hasDashed = false;
		break;
	case ColliderType::ITEM:
		LOG("Collision ITEM");
		Engine::GetInstance().audio->PlayFx(pickCoinFxId);
		physB->listener->Destroy();
		break;
	case ColliderType::TRAP:
	case ColliderType::ENEMY:
	case ColliderType::BULLET:
		if (!isDying && !godMode)
		{
			anims.SetCurrent("death");
			isDying = true;
			velocity = { 0,0 };
		}

		LOG("Player Death");
		break;
	case ColliderType::UNKNOWN:
		LOG("Collision UNKNOWN");
		break;
	default:
		break;
	}
}

void Player::OnCollisionEnd(PhysBody* physA, PhysBody* physB)
{
	switch (physB->ctype)
	{
	case ColliderType::PLATFORM:
		LOG("End Collision PLATFORM");
		break;
	case ColliderType::ITEM:
		LOG("End Collision ITEM");
		break;
	case ColliderType::UNKNOWN:
		LOG("End Collision UNKNOWN");
		break;
	default:
		break;
	}
}

