#include "RoboCatClientPCH.hpp"

ClientTrack::ClientTrack() {
	sprite_component_.reset(new SpriteComponent(this));
	sprite_component_->SetTexture(TextureManager::sInstance->GetTexture("track"));
}
