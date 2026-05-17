#include "RoboCatClientPCH.hpp"

void StarClient::Update() {
	Star::Update();

	
}

StarClient::StarClient()
{
	sprite_component_.reset(new SpriteComponent(this));
	sprite_component_->SetTexture(TextureManager::sInstance->GetTexture("star"));
	sprite_component_->SetRotateWithCamera(true);
}

bool StarClient::HandleCollisionWithCar(PlayerCar* inCar) {
	SoundManager::sInstance->Play("StarGet");
	return false;
}