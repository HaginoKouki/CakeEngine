// UpdateContext.cpp
#include "UpdateContext.h"

#include "Engine/Platform/Input/InputManager.h"

namespace Cake {

KeyState UpdateContext::GetKey(KeyCode key) const {
	return input.GetGameKeyState(key);
}
bool UpdateContext::IsKeyHeld(KeyCode key) const {
	return IsPressed(GetKey(key));
}
bool UpdateContext::IsKeyDown(KeyCode key) const {
	return GetKey(key) == KeyState::Pressed;
}
bool UpdateContext::IsKeyUp(KeyCode key) const {
	return GetKey(key) == KeyState::Release;
}

} // namespace Cake
