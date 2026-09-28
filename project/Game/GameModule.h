#pragma once

namespace Cake {
class ShaderLibrary;
}

namespace Game {

void RegisterGameComponents();
void RegisterGameShaders(Cake::ShaderLibrary& shaderLibrary);

}
