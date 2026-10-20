#include "SpriteTesterComponent.h"

#include <algorithm>
#include <utility>
#include <cmath>

#include "Engine/Scene/Object/GameObject.h"
#include "Engine/Scene/Component/SpriteRendererComponent.h"
#include "Engine/Scene/System/UpdateContext.h"
#include "Engine/Scene/Scene.h"

void SpriteTesterComponent::Update(const Cake::UpdateContext& ctx, Cake::GameObjectId self) {
	Cake::GameObject* object = ctx.scene.Find(self);
	Cake::SpriteRendererComponent* sprite = ctx.scene.GetComponent<Cake::SpriteRendererComponent>(self);
	if (object == nullptr || sprite == nullptr) {
		return;
	}
	
	if (ctx.IsKeyHeld(Cake::KeyCode::Right)) {
		Cake::Transform& local = object->GetTransform().GetLocalMutable();
		local.translate.x += speed * ctx.deltaTime;
	}
	if (ctx.IsKeyHeld(Cake::KeyCode::Left)) {
		Cake::Transform& local = object->GetTransform().GetLocalMutable();
		local.translate.x -= speed * ctx.deltaTime;
	}
	if (ctx.IsKeyHeld(Cake::KeyCode::Up)) {
		Cake::Transform& local = object->GetTransform().GetLocalMutable();
		local.translate.y -= speed * ctx.deltaTime;
	}
	if (ctx.IsKeyHeld(Cake::KeyCode::Down)) {
		Cake::Transform& local = object->GetTransform().GetLocalMutable();
		local.translate.y += speed * ctx.deltaTime;
	}

	if (ctx.IsKeyHeld(Cake::KeyCode::Q)) {
		Cake::Transform& local = object->GetTransform().GetLocalMutable();
		local.rotate.z -= rotate * ctx.deltaTime;
	}
	if (ctx.IsKeyHeld(Cake::KeyCode::E)) {
		Cake::Transform& local = object->GetTransform().GetLocalMutable();
		local.rotate.z += rotate * ctx.deltaTime;
	}

	if (ctx.IsKeyDown(Cake::KeyCode::Space)) {
		if (sprite->color == Cake::Vector4{1.0f, 1.0f, 1.0f, sprite->color.w}) {
			sprite->color = {1.0f, 0.0f, 0.0f, sprite->color.w};
		} else {
			sprite->color = {1.0f, 1.0f, 1.0f, sprite->color.w};
		}
	}
	if (ctx.IsKeyDown(Cake::KeyCode::T)) {
		std::swap(sprite->texture, alternateTexture);
	}

	if (ctx.IsKeyHeld(Cake::KeyCode::Z)) {
		sprite->size.x = std::max<float>(0.0f, sprite->size.x - size * ctx.deltaTime);
		sprite->size.y = std::max<float>(0.0f, sprite->size.y - size * ctx.deltaTime);
	}
	if (ctx.IsKeyHeld(Cake::KeyCode::X)) {
		sprite->size.x = std::max<float>(0.0f, sprite->size.x + size * ctx.deltaTime);
		sprite->size.y = std::max<float>(0.0f, sprite->size.y + size * ctx.deltaTime);
	}

	sprite->uvOffset.x += uvScrollSpeed.x * ctx.deltaTime;
	sprite->uvOffset.y += uvScrollSpeed.y * ctx.deltaTime;
	// 0〜1 に収める（WRAP なので見た目は変わらない）。fmod は負の値で負を返すので floor を引く.
	sprite->uvOffset.x -= std::floor(sprite->uvOffset.x);
	sprite->uvOffset.y -= std::floor(sprite->uvOffset.y);
}
