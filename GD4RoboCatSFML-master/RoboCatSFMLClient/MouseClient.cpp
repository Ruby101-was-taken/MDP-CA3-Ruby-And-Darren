#include "RoboCatClientPCH.hpp"

void MouseClient::Update() {
	Mouse::Update();

	
}

MouseClient::MouseClient()
{
	sprite_component_.reset(new SpriteComponent(this));
	sprite_component_->SetTexture(TextureManager::sInstance->GetTexture("star"));
	sprite_component_->SetRotateWithCamera(true);
}
