#pragma once
#include "IElement2D.h"

class Sprite : public IElement2D
{
public:
	Sprite();
	Sprite(const Vector2& position);
	~Sprite();

	// GETTERS
	const Vector2& GetPosition() const override;
	const Vector2& GetOrigin() const override;
	const Vector2& GetScale() const override;
	const Color& GetTint() const override;
	const float& GetAlpha() const override;
	const float& GetRotation() const override;

	// SETTERS
	void SetPosition(const Vector2& position) override;
	void SetOrigin(const Vector2& origin) override;
	void SetScale(const Vector2& scale) override;
	void SetTint(const Color& color) override;
	void SetAlpha(const float alpha) override;
	void SetRotation(const float rotation) override;

	// RENDER
	void Draw(DirectX::SpriteBatch* spriteBatch) override;

	// LOAD
	void Load(ID3D11Device* device, const wchar_t* file) override;

protected:
	ID3D11Resource*				m_pResource;
	ID3D11ShaderResourceView*	m_pTexture;
	UINT						m_Width;
	UINT						m_Height;
	RECT						m_SourceRect;

	Vector2						m_Position;
	Vector2						m_Origin;
	Vector2						m_Scale;
	Color						m_Tint;
	float						m_Alpha;
	float						m_Rotation;
};

