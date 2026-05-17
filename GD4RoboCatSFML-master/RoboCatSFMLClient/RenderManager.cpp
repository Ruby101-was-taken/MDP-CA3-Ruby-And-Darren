#include "RoboCatClientPCH.hpp"

std::unique_ptr< RenderManager >	RenderManager::sInstance;


RenderManager::RenderManager()
{
	view.reset(sf::FloatRect(0, 0, 1280, 720));
	WindowManager::sInstance->setView(view);
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
				// Ruby White - D00255322
				float rot = cat->GetRotation();
				view.setRotation(rot);

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

	//
	// Darren Meidl - D00255479
	// Background / spatial reference drawing
	//
	{
		sf::Vector2f viewCenter = view.getCenter();
		sf::Vector2f viewSize = view.getSize();
		sf::FloatRect visibleRect(viewCenter.x - viewSize.x * 0.5f, viewCenter.y - viewSize.y * 0.5f, viewSize.x, viewSize.y);

		sf::RectangleShape ground(sf::Vector2f(2560, 2560)); // draw ground as a rectangle covering the visible area
									// Ruby White - D00255322
		ground.setPosition(visibleRect.left-(visibleRect.width/2), visibleRect.top - (visibleRect.height / 2));
		ground.setFillColor(sf::Color(80, 80, 90, 255));
		
		const float GRID_SIZE = 128.f; // Grid overlay
		sf::VertexArray gridLines(sf::Lines);

		float firstVertical = std::floor(visibleRect.left / GRID_SIZE) * GRID_SIZE; // verticals
		for (float x = firstVertical; x <= visibleRect.left + visibleRect.width; x += GRID_SIZE)
		{
			sf::Vertex v1(sf::Vector2f(x, visibleRect.top), sf::Color(200, 200, 200, 50));
			sf::Vertex v2(sf::Vector2f(x, visibleRect.top + visibleRect.height), sf::Color(200, 200, 200, 50));
			gridLines.append(v1);
			gridLines.append(v2);
		}
	
		float firstHorizontal = std::floor(visibleRect.top / GRID_SIZE) * GRID_SIZE; // horizontals
		for (float y = firstHorizontal; y <= visibleRect.top + visibleRect.height; y += GRID_SIZE)
		{
			sf::Vertex v1(sf::Vector2f(visibleRect.left, y), sf::Color(200, 200, 200, 50));
			sf::Vertex v2(sf::Vector2f(visibleRect.left + visibleRect.width, y), sf::Color(200, 200, 200, 50));
			gridLines.append(v1);
			gridLines.append(v2);
		}
		
		sf::RectangleShape worldBorder(sf::Vector2f(3840.f, 2160.f)); // world width & height
		worldBorder.setPosition(0.f, 0.f);
		worldBorder.setFillColor(sf::Color::Transparent);
		worldBorder.setOutlineColor(sf::Color(255, 60, 60, 220)); // Red
		worldBorder.setOutlineThickness(12.f); // thicker border

		WindowManager::sInstance->draw(ground);
		WindowManager::sInstance->draw(gridLines);
		WindowManager::sInstance->draw(worldBorder);
	}

	RenderManager::sInstance->RenderComponents();

	HUD::sInstance->Render();

	//
	// Present our back buffer to our front buffer
	//
	WindowManager::sInstance->display();

}
