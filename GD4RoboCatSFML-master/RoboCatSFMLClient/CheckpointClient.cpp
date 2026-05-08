#include "RoboCatClientPCH.hpp"

CheckpointClient::CheckpointClient() 
{
	mSpriteComponent.reset(new SpriteComponent(this));
	mSpriteComponent->SetTexture(TextureManager::sInstance->GetTexture("checkpoint"));
}