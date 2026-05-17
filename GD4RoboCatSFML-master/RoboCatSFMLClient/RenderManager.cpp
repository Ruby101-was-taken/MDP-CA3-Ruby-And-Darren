#include "RoboCatClientPCH.hpp"

std::unique_ptr< RenderManager >	RenderManager::sInstance;


RenderManager::RenderManager()
{
	view.reset(sf::FloatRect(0, 0, 1280, 720));
	WindowManager::sInstance->setView(view);
	background_.setTexture(*TextureManager::sInstance->GetTexture("water"));
	background_.setOrigin({640, 360});
}



void RenderManager::StaticInit()
{

	sInstance.reset(new RenderManager());
}


void RenderManager::AddComponent(SpriteComponent* inComponent)
{
	mComponents.emplace_back(inComponent);
}

void RenderManager::RemoveComponent(SpriteComponent* inComponent)
{
	int index = GetComponentIndex(inComponent);

	if (index != -1)
	{
		int lastIndex = mComponents.size() - 1;
		if (index != lastIndex)
		{
			mComponents[index] = mComponents[lastIndex];
		}
		mComponents.pop_back();
	}
}

int RenderManager::GetComponentIndex(SpriteComponent* inComponent) const
{
	for (int i = 0, c = mComponents.size(); i < c; ++i)
	{
		if (mComponents[i] == inComponent)
		{
			return i;
		}
	}

	return -1;
}


//this part that renders the world is really a camera-
//in a more detailed engine, we'd have a list of cameras, and then render manager would
//render the cameras in order
void RenderManager::RenderComponents()
{
	float world_rotation = WindowManager::sInstance->getView().getRotation();
	//Get the logical viewport so we can pass this to the SpriteComponents when it's draw time
	for (SpriteComponent* c : mComponents)
	{	
		// Ruby White - D00255322
		if (c->IsActive()) {
			sf::Sprite& sprite = c->GetSprite();
			if (c->RotatesWithCamera())
				sprite.setRotation(world_rotation);
			WindowManager::sInstance->draw(sprite);
			if (c->RotatesWithCamera())
				sprite.setRotation(-world_rotation);
		}
	}

	//level collisions debug rendering and such
	//for (sf::FloatRect rect : LevelManager::sInstance->wall_tiles_) {
	//	sf::RectangleShape vis;
	//	vis.setPosition(rect.getPosition());
	//	vis.setSize(rect.getSize());
	//	vis.setFillColor(sf::Color::Red);
	//	WindowManager::sInstance->draw(vis);
	//}
	//for (sf::FloatRect rect : LevelManager::sInstance->grass_tiles_) {
	//	sf::RectangleShape vis;
	//	vis.setPosition(rect.getPosition());
	//	vis.setSize(rect.getSize());
	//	vis.setFillColor(sf::Color::Blue);
	//	WindowManager::sInstance->draw(vis);
	//}
}

void RenderManager::Render()
{
	//
	// Darren Meidl - D00255479
	// Center camera on local player if present
	//
	if (NetworkManagerClient::sInstance && NetworkManagerClient::sInstance->GetPlayerId() != 0)
	{
		uint32_t localPlayerId = NetworkManagerClient::sInstance->GetPlayerId();
		const auto& gameObjects = World::sInstance->GetGameObjects();
		for (const auto& goPtr : gameObjects)
		{
			PlayerCar* cat = goPtr->GetAsCar();
			if (cat && cat->GetPlayerId() == localPlayerId)
			{
				Vector3 loc = cat->GetLocation() + cat->GetForwardVector()*250;
				view.setCenter(loc.mX, loc.mY);
				background_.setPosition(loc.mX, loc.mY);
				// Ruby White - D00255322
				float rot = cat->GetRotation();
				view.setRotation(rot);				
				background_.setRotation(rot);


				World::sInstance->SetAngle(rot);

				WindowManager::sInstance->setView(view);
				break;
			}
		}
	}

	//
	// Clear the back buffer
	//
	WindowManager::sInstance->clear(sf::Color(100, 149, 237, 255));
	WindowManager::sInstance->draw(background_);
	


	RenderManager::sInstance->RenderComponents();

	HUD::sInstance->Render();

	//
	// Present our back buffer to our front buffer
	//
	WindowManager::sInstance->display();

}
