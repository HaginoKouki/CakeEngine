#include "Platform.h"

namespace Cake {

void Platform::Initialize(HINSTANCE hInstance, HWND hwnd) {
	input_.Initialize(hInstance, hwnd);
}

void Platform::BeginFrame() {
	time_.Tick();
	input_.Update(time_.GetUnscaledDeltaTime());
}

} // namespace Cake
