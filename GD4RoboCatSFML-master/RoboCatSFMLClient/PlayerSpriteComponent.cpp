#include "RoboCatClientPCH.hpp"

PlayerSpriteComponent::PlayerSpriteComponent(GameObject* inGameObject) :
	SpriteComponent(inGameObject)
{}

sf::Sprite& PlayerSpriteComponent::GetSprite()
{
	// Update the sprite based on the game object stuff.
	auto pos = mGameObject->GetLocation();
	auto rot = mGameObject->GetRotation();
	m_sprite.setPosition(pos.mX, pos.mY);
	m_sprite.setRotation(rot);
	PlayerCar* player = dynamic_cast<PlayerCar*>(mGameObject);
	Vector3 playerColor = player->GetColor();
	m_sprite.setColor(sf::Color(playerColor.mX, playerColor.mY, playerColor.mZ, 255));

	return m_sprite;
}
