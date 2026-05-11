#include "RoboCatClientPCH.hpp"

void MouseClient::Update() {
	Mouse::Update();
	if (IsActive())
		SetRotation(GetRotation()+1);

	sprite_component_->SetActive(false);
	if (sprite_component_->IsActive() != IsActive()) {
		sprite_component_->SetActive(IsActive());
	}
		
}

MouseClient::MouseClient()
{
	sprite_component_.reset(new SpriteComponent(this));
	sprite_component_->SetTexture(TextureManager::sInstance->GetTexture("mouse"));
}
