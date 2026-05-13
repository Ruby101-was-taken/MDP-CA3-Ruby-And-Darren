typedef shared_ptr< sf::Texture > TexturePtr;
typedef shared_ptr<sf::Font> FontPtr;

class SpriteComponent
{
public:

	SpriteComponent(GameObject* inGameObject);
	~SpriteComponent();


	void SetTexture(TexturePtr inTexture);
	virtual sf::Sprite& GetSprite();

	// Ruby White - D00255322
	void SetActive(bool is_active);
	bool IsActive();
	void SetRotateWithCamera(bool set);
	bool RotatesWithCamera();

protected:

	sf::Sprite m_sprite;

	// Ruby White - D00255322
	bool is_active_;
	bool rotate_with_camera_;

	//don't want circular reference...
	GameObject* mGameObject;
};

typedef shared_ptr< SpriteComponent >	SpriteComponentPtr;

