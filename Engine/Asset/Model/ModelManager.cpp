#include "ModelManager.h"

#include <cassert>
#include <cstring>
#include <numbers>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "Engine/Foundation/Debug/DebugLog.h"
#include "Engine/Graphics/Buffer/GPUResourceUtility.h"
#include "Engine/Asset/Material/Material.h"
#include "Engine/Asset/Material/MaterialManager.h"

namespace Cake {
namespace {
constexpr const char* kLogCategory = "ModelManager";
}

namespace {

// パスをディレクトリとファイル名に分解する.
std::pair<std::string, std::string> SplitPath(const std::string& filePath) {
	std::filesystem::path path(filePath);
	return {path.parent_path().string(), path.filename().string()};
}

// サブメッシュ用のGPU頂点バッファを1回だけ確保して書き込む.
void UploadSubMesh(ID3D12Device* device, SubMesh& sub) {
	sub.vertexCount = static_cast<uint32_t>(sub.vertices.size());
	const size_t sizeInBytes = sizeof(VertexData) * sub.vertices.size();

	sub.vertexResource = CreateBufferResource(device, sizeInBytes);
	sub.vbv.BufferLocation = sub.vertexResource->GetGPUVirtualAddress();
	sub.vbv.SizeInBytes = static_cast<UINT>(sizeInBytes);
	sub.vbv.StrideInBytes = sizeof(VertexData);

	VertexData* dst = nullptr;
	sub.vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&dst));
	std::memcpy(dst, sub.vertices.data(), sizeInBytes);
}

} // namespace

namespace {

// 平面プリミティブ（Triangle / Quad）のオモテ面を向ける先。
// 既定のゲームカメラは Transform 無回転で前方が +Z なので、原点を写すには -Z 側に立つ。
// そこから見えるようにオモテを -Z へ向ける（+1.0f にすれば逆側を向く）.
constexpr float kPlaneFacingZ = -1.0f;

// 1メッシュ・1サブメッシュ・1マテリアルの ModelData を組み立てる.
// GPU頂点バッファもここで確保する（UploadSubMesh を再利用）.
ModelData BuildSingleMeshModel(
	ID3D12Device* device,
	MaterialManager* materialManager,
	const std::string& name,
	std::vector<VertexData> vertices
) {
	ModelData data;
	data.name = name;
	data.filePath = name; // キャッシュキーをそのまま識別子として持たせる.

	Mesh mesh;
	mesh.name = name;

	MaterialHandle material = materialManager->GetErrorMaterial();
	mesh.materialSlots.push_back(material);
	if (const Material* m = materialManager->Resolve(material)) {
		data.defaultMaterialKeys.push_back(m->GetName());
	}

	SubMesh sub;
	sub.materialSlot = 0;
	sub.vertices = std::move(vertices);
	UploadSubMesh(device, sub);
	mesh.subMeshes.push_back(std::move(sub));

	data.meshes.push_back(std::move(mesh));
	return data;
}

// 平面用のUV軸を作る。オモテ面から見て outU が右、outV がテクスチャの下を向く。
// 右手系の外積で outU × outV = normal になる組（立方体・球と同じ規約）.
void MakePlaneUVAxis(const Cake::Vector3& normal, Cake::Vector3& outU, Cake::Vector3& outV) {
	// テクスチャの下向きの基準。真上・真下を向く面だけはYが使えないので+Zへ逃がす.
	const Cake::Vector3 down = (std::fabs(normal.y) > 0.999f)
	                               ? Cake::Vector3(0.0f, 0.0f, 1.0f)
	                               : Cake::Vector3(0.0f, -1.0f, 0.0f);

	// 法線成分を抜いて面へ寝かせる.
	outV = Vector3::Normalize(down - normal * Vector3::DotProduct(down, normal));
	// (outU, outV, normal) が右手系になる軸。単位ベクトルの直交外積なので正規化は不要.
	outU = Vector3::CrossProduct(outV, normal);
}

// 同一平面上の点列から、オモテ面が kPlaneFacingZ 側を向く三角形リストを作る。
//
// points は多角形の周回順。時計回り・反時計回りのどちらで渡されても、
// 巻き順と法線はここで揃えるので呼び出し側は気にしなくてよい。
// UVは面を正面から見たときの外接矩形を 0..1 へ引き伸ばして貼るので、
// 「どの頂点が左上か」という暗黙の約束も要らない.
std::vector<VertexData> BuildPlanePolygon(const Cake::Vector3* points, size_t count) {
	std::vector<VertexData> vertices;
	if (points == nullptr || count < 3) {
		return vertices;
	}

	// 巻き順 0→1→2 に対するオモテ側の法線（右手系の外積＝この規約でのオモテ）.
	Cake::Vector3 normal = Vector3::Normalize(
		Vector3::CrossProduct(points[1] - points[0], points[2] - points[0])
	);

	// 逆側を向いていたら、巻き順ごと裏返す.
	const bool flip = (normal.z * kPlaneFacingZ < 0.0f);
	if (flip) {
		normal = -normal;
	}

	Cake::Vector3 uAxis, vAxis;
	MakePlaneUVAxis(normal, uAxis, vAxis);

	// 面上での外接矩形を求める.
	float uMin = Vector3::DotProduct(points[0], uAxis);
	float uMax = uMin;
	float vMin = Vector3::DotProduct(points[0], vAxis);
	float vMax = vMin;
	for (size_t i = 1; i < count; ++i) {
		const float u = Vector3::DotProduct(points[i], uAxis);
		const float v = Vector3::DotProduct(points[i], vAxis);
		uMin = (std::min)(uMin, u);
		uMax = (std::max)(uMax, u);
		vMin = (std::min)(vMin, v);
		vMax = (std::max)(vMax, v);
	}
	// 潰れた面でも0除算しない.
	const float uRange = (uMax - uMin > 1e-6f) ? (uMax - uMin) : 1.0f;
	const float vRange = (vMax - vMin > 1e-6f) ? (vMax - vMin) : 1.0f;

	// 点ごとの頂点を先に作る（三角形化で使い回す）.
	std::vector<VertexData> ring;
	ring.reserve(count);
	for (size_t i = 0; i < count; ++i) {
		VertexData vertex;
		vertex.position = {points[i].x, points[i].y, points[i].z, 1.0f};
		vertex.texcoord = {
			(Vector3::DotProduct(points[i], uAxis) - uMin) / uRange,
			(Vector3::DotProduct(points[i], vAxis) - vMin) / vRange
		};
		vertex.normal = normal;
		ring.push_back(vertex);
	}

	// 点0を要にした扇で三角形へ分解する（凸多角形前提）.
	vertices.reserve((count - 2) * 3);
	for (size_t i = 1; i + 1 < count; ++i) {
		vertices.push_back(ring[0]);
		vertices.push_back(flip ? ring[i + 1] : ring[i]);
		vertices.push_back(flip ? ring[i] : ring[i + 1]);
	}
	return vertices;
}

// 三角形.
std::vector<VertexData> BuildTriangleVertices(const Cake::Triangle& triangle) {
	return BuildPlanePolygon(triangle.vertices, 3);
}

// 四角形（2枚の三角形）.
std::vector<VertexData> BuildQuadVertices(const Cake::Quad& quad) {
	return BuildPlanePolygon(quad.vertices, 4);
}

// 立方体（原点中心、1辺 sideLength、6面×2三角形＝36頂点）.
std::vector<VertexData> BuildCubeVertices(float sideLength) {
	const float h = sideLength * 0.5f;

	// 各面の (法線, u軸, v軸)。u×v = 法線（右手系）になるよう選んである.
	struct Face {
		Cake::Vector3 normal;
		Cake::Vector3 u;
		Cake::Vector3 v;
	};
	const Face faces[6] = {
		{{1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},  // +X.
		{{-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},  // -X.
		{{0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}},  // +Y.
		{{0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},  // -Y.
		{{0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},   // +Z.
		{{0.0f, 0.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // -Z.
	};

	std::vector<VertexData> vertices;
	vertices.reserve(36);

	for (const Face& f : faces) {
		Cake::Vector3 center = f.normal * h;
		Cake::Vector3 p0 = center - f.u * h - f.v * h; // (-u,-v).
		Cake::Vector3 p1 = center + f.u * h - f.v * h; // (+u,-v).
		Cake::Vector3 p2 = center + f.u * h + f.v * h; // (+u,+v).
		Cake::Vector3 p3 = center - f.u * h + f.v * h; // (-u,+v).

		auto makeVertex = [&](const Cake::Vector3& p, float u, float v) {
			VertexData vertex;
			vertex.position = {p.x, p.y, p.z, 1.0f};
			vertex.texcoord = {u, v};
			vertex.normal = f.normal;
			return vertex;
		};

		VertexData v0 = makeVertex(p0, 1.0f, 1.0f);
		VertexData v1 = makeVertex(p1, 0.0f, 1.0f);
		VertexData v2 = makeVertex(p2, 0.0f, 0.0f);
		VertexData v3 = makeVertex(p3, 1.0f, 0.0f);

		// (p0,p1,p2) / (p0,p2,p3)。RH外積が +normal＝外向き.
		vertices.push_back(v0);
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v0);
		vertices.push_back(v2);
		vertices.push_back(v3);
	}
	return vertices;
}

// 球（緯度経度分割）。inversed=true で内向き（天球用：法線反転＋巻き順反転）.
std::vector<VertexData> BuildSphereVertices(float radius, int32_t subdivision, bool inversed) {
	if (subdivision < 3) {
		subdivision = 3; // 最低限の分割数を保証する.
	}
	const uint32_t kSubdivision = static_cast<uint32_t>(subdivision);
	const float kPi = std::numbers::pi_v<float>;
	const float kLonEvery = 2.0f * kPi / static_cast<float>(kSubdivision); // 経度φの1分割.
	const float kLatEvery = kPi / static_cast<float>(kSubdivision);        // 緯度θの1分割.

	// 緯度経度から1頂点を作る.
	auto makeVertex = [&](float lat, float lon, float u, float v) {
		Cake::Vector3 dir = {
			std::cos(lat) * std::cos(lon),
			std::sin(lat),
			std::cos(lat) * std::sin(lon)
		};
		VertexData vertex;
		vertex.position = {dir.x * radius, dir.y * radius, dir.z * radius, 1.0f};
		vertex.normal = inversed ? -dir : dir;          // 内向きは法線反転.
		vertex.texcoord = {inversed ? 1.0f - u : u, v}; // 内向きはU反転（内側から文字が正しく読める）.
		return vertex;
	};

	std::vector<VertexData> vertices;
	vertices.reserve(static_cast<size_t>(kSubdivision) * kSubdivision * 6);

	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -kPi / 2.0f + kLatEvery * static_cast<float>(latIndex); // θ.
		float latNext = lat + kLatEvery;
		float v = 1.0f - static_cast<float>(latIndex) / static_cast<float>(kSubdivision);
		float vNext = 1.0f - static_cast<float>(latIndex + 1) / static_cast<float>(kSubdivision);

		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = static_cast<float>(lonIndex) * kLonEvery; // φ.
			float lonNext = lon + kLonEvery;
			float u = static_cast<float>(lonIndex) / static_cast<float>(kSubdivision);
			float uNext = static_cast<float>(lonIndex + 1) / static_cast<float>(kSubdivision);

			VertexData a = makeVertex(lat, lon, u, v);                 // (lat,   lon).
			VertexData b = makeVertex(latNext, lon, u, vNext);         // (lat+1, lon).
			VertexData c = makeVertex(lat, lonNext, uNext, v);         // (lat,   lon+1).
			VertexData d = makeVertex(latNext, lonNext, uNext, vNext); // (lat+1, lon+1).

			if (!inversed) {
				// 外向き: a,b,c / c,b,d.
				vertices.push_back(a);
				vertices.push_back(b);
				vertices.push_back(c);
				vertices.push_back(c);
				vertices.push_back(b);
				vertices.push_back(d);
			} else {
				// 内向き: 上の巻き順を反転する.
				vertices.push_back(a);
				vertices.push_back(c);
				vertices.push_back(b);
				vertices.push_back(c);
				vertices.push_back(d);
				vertices.push_back(b);
			}
		}
	}
	return vertices;
}

// 正四面体（原点中心、外接球の半径 circumRadius。4面×1三角形＝12頂点）.
std::vector<VertexData> BuildTetrahedronVertices(float circumRadius) {
	const float r = circumRadius;

	// |(baseRadius, baseY)| = r になるよう選んである（底面3点も外接球の上に乗る）.
	const float baseRadius = 2.0f * std::numbers::sqrt2_v<float> / 3.0f * r;
	const float baseY = -r / 3.0f;

	const Cake::Vector3 apex = {0.0f, r, 0.0f};

	Cake::Vector3 base[3];
	for (int32_t i = 0; i < 3; ++i) {
		const float angle = 2.0f * std::numbers::pi_v<float> * static_cast<float>(i) / 3.0f;
		base[i] = {baseRadius * std::cos(angle), baseY, baseRadius * std::sin(angle)};
	}

	// 立方体・球と同じ規約に揃える：右手系の外積 (p1-p0)×(p2-p0) が外向きになる巻き順.
	// 側面は底面の周回と逆向きに、底面はそのまま並べるとこの向きになる.
	struct Face {
		Cake::Vector3 p0;
		Cake::Vector3 p1;
		Cake::Vector3 p2;
	};
	const Face faces[4] = {
		{apex, base[1], base[0]},
		{apex, base[2], base[1]},
		{apex, base[0], base[2]},
		{base[0], base[1], base[2]}, // 底面（法線は -Y）.
	};

	std::vector<VertexData> vertices;
	vertices.reserve(12);

	for (const Face& f : faces) {
		const Cake::Vector3 normal = Vector3::Normalize(Vector3::CrossProduct(f.p1 - f.p0, f.p2 - f.p0));

		auto makeVertex = [&](const Cake::Vector3& p, float u, float v) {
			VertexData vertex;
			vertex.position = {p.x, p.y, p.z, 1.0f};
			vertex.texcoord = {u, v};
			vertex.normal = normal;
			return vertex;
		};

		vertices.push_back(makeVertex(f.p0, 0.5f, 0.0f));
		vertices.push_back(makeVertex(f.p1, 1.0f, 1.0f));
		vertices.push_back(makeVertex(f.p2, 0.0f, 1.0f));
	}
	return vertices;
}

} // namespace

void ModelManager::Initialize(ID3D12Device* device, TextureManager* textureManager, MaterialManager* materialManager) {
	DebugLog::GetInstance().LogInitStart(kLogCategory);

	device_ = device;
	textureManager_ = textureManager;
	materialManager_ = materialManager;

	// キーは PrimitiveShape.h と一致させる（AssetDatabase がここを索引に載せる）.
	primitives_[static_cast<size_t>(PrimitiveShape::Triangle)] = CreateTriangleModel(GetPrimitiveKey(PrimitiveShape::Triangle), Cake::Triangle({{-0.5f, -0.29f, 0}, {0, 0.58f, 0}, {0.5f, -0.29f, 0}}));
	primitives_[static_cast<size_t>(PrimitiveShape::Quad)] = CreateQuadModel(GetPrimitiveKey(PrimitiveShape::Quad), Cake::Quad({{0.5f, 0.5f, 0}, {-0.5f, 0.5f, 0}, {-0.5f, -0.5f, 0}, {0.5f, -0.5f, 0}}));
	primitives_[static_cast<size_t>(PrimitiveShape::Cube)] = CreateCubeModel(GetPrimitiveKey(PrimitiveShape::Cube), 1.0f);
	primitives_[static_cast<size_t>(PrimitiveShape::Sphere)] = CreateSphereModel(GetPrimitiveKey(PrimitiveShape::Sphere), 0.5f, 12);
	primitives_[static_cast<size_t>(PrimitiveShape::Tetrahedron)] = CreateTetrahedronModel(GetPrimitiveKey(PrimitiveShape::Tetrahedron), 1.0f);

	DebugLog::GetInstance().LogInitComplete(kLogCategory);
}

ModelHandle ModelManager::Emplace(ModelData&& data) {
	const uint32_t index = static_cast<uint32_t>(pool_.size());
	pool_.push_back(std::move(data));
	generations_.push_back(1); // 1 始まり（0 は無効ハンドルの初期値と区別）.

	ModelHandle handle;
	handle.index = index;
	handle.generation = generations_[index];
	return handle;
}

bool ModelManager::IsAlive(ModelHandle handle) const {
	if (!handle.IsValid() || handle.index >= pool_.size()) {
		return false;
	}
	return generations_[handle.index] == handle.generation;
}

ModelHandle ModelManager::Load(const std::string& filePath, const ModelLoadDesc* desc) {
	// キャッシュ検索（複製せず、既読なら同じハンドルを返す）.
	auto it = byPath_.find(filePath);
	if (it != byPath_.end()) {
		return it->second;
	}

	auto [directoryPath, filename] = SplitPath(filePath);

	// ModelData をローカルで組み立ててから pool_ へ move する.
	ModelData data;
	data.name = filename;
	data.filePath = filePath;

	ModelLoadDesc useDesc = (desc != nullptr) ? *desc : ModelLoadDesc{};
	LoadObjFile(data, directoryPath, filename, useDesc);

	ModelHandle handle = Emplace(std::move(data));
	byPath_[filePath] = handle;
	return handle;
}

#pragma region 基本図形モデルの作成
ModelHandle ModelManager::CreateTriangleModel(const std::string& cacheKey, const Cake::Triangle& triangle) {
	if (auto it = byPath_.find(cacheKey); it != byPath_.end()) {
		return it->second;
	}

	ModelData data = BuildSingleMeshModel(device_, materialManager_, cacheKey, BuildTriangleVertices(triangle));
	ModelHandle handle = Emplace(std::move(data));
	return byPath_.emplace(cacheKey, handle).first->second;
}

ModelHandle ModelManager::CreateQuadModel(const std::string& cacheKey, const Cake::Quad& quad) {
	if (auto it = byPath_.find(cacheKey); it != byPath_.end()) {
		return it->second;
	}

	ModelData data = BuildSingleMeshModel(device_, materialManager_, cacheKey, BuildQuadVertices(quad));
	ModelHandle handle = Emplace(std::move(data));
	return byPath_.emplace(cacheKey, handle).first->second;
}

ModelHandle ModelManager::CreateCubeModel(const std::string& cacheKey, float sideLength) {
	if (auto it = byPath_.find(cacheKey); it != byPath_.end()) {
		return it->second;
	}

	ModelData data = BuildSingleMeshModel(device_, materialManager_, cacheKey, BuildCubeVertices(sideLength));
	ModelHandle handle = Emplace(std::move(data));
	return byPath_.emplace(cacheKey, handle).first->second;
}

ModelHandle ModelManager::CreateSphereModel(const std::string& cacheKey, float radius, int32_t subdivision) {
	if (auto it = byPath_.find(cacheKey); it != byPath_.end()) {
		return it->second;
	}

	ModelData data = BuildSingleMeshModel(device_, materialManager_, cacheKey, BuildSphereVertices(radius, subdivision, false));
	ModelHandle handle = Emplace(std::move(data));
	return byPath_.emplace(cacheKey, handle).first->second;
}

ModelHandle ModelManager::CreateTetrahedronModel(const std::string& cacheKey, float circumRadius) {
	if (auto it = byPath_.find(cacheKey); it != byPath_.end()) {
		return it->second;
	}

	ModelData data = BuildSingleMeshModel(device_, materialManager_, cacheKey, BuildTetrahedronVertices(circumRadius));
	ModelHandle handle = Emplace(std::move(data));
	return byPath_.emplace(cacheKey, handle).first->second;
}
#pragma endregion

std::span<const Mesh> ModelManager::ResolveMeshes(ModelHandle handle) const {
	if (!IsAlive(handle)) {
		return {};
	}
	return pool_[handle.index].meshes;
}

std::span<Mesh> ModelManager::ResolveMeshesMutable(ModelHandle handle) {
	if (!IsAlive(handle)) {
		return {};
	}
	return pool_[handle.index].meshes;
}

const Mesh* ModelManager::ResolveMesh(MeshHandle handle) const {
	if (!IsAlive(handle.model)) {
		return nullptr;
	}
	const ModelData& model = pool_[handle.model.index];
	if (handle.meshIndex >= model.meshes.size()) {
		return nullptr;
	}
	return &model.meshes[handle.meshIndex];
}

Mesh* ModelManager::ResolveMeshMutable(MeshHandle handle) {
	if (!IsAlive(handle.model)) {
		return nullptr;
	}
	ModelData& model = pool_[handle.model.index];
	if (handle.meshIndex >= model.meshes.size()) {
		return nullptr;
	}
	return &model.meshes[handle.meshIndex];
}

const ModelData* ModelManager::Resolve(ModelHandle handle) const {
	if (!IsAlive(handle)) {
		return nullptr;
	}
	return &pool_[handle.index];
}

ModelHandle ModelManager::FindHandle(const std::string& filePath) const {
	auto it = byPath_.find(filePath);
	if (it == byPath_.end()) {
		return {};
	}
	return it->second;
}

ModelHandle ModelManager::HandleFromIndex(uint32_t index) const {
	if (index >= pool_.size()) {
		return {};
	}
	ModelHandle handle;
	handle.index = index;
	handle.generation = generations_[index];
	return handle;
}

std::unordered_map<std::string, MaterialHandle> ModelManager::LoadMtlFile(ModelData& out, const std::string& directoryPath, const std::string& mtlFilename, const ModelLoadDesc& desc) {
	std::unordered_map<std::string, MaterialHandle> nameToMaterial; // OBJ内の名前 → マテリアルハンドル.

	std::ifstream file(directoryPath + "/" + mtlFilename);
	if (!file.is_open()) {
		DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, ".mtl が開けません: " + mtlFilename);
		return nameToMaterial;
	}

	MaterialHandle current{}; // 今設定中のマテリアル（ハンドル）.
	std::string currentName;  // OBJ内での名前.
	std::string line;

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "newmtl") {
#pragma region マテリアルの開始（MaterialManager に生成させる）
			s >> currentName;
			// CreateMaterial はハンドルを返す。実キー名は Resolve して取得する.
			current = materialManager_->CreateMaterial(currentName, desc.defaultShaderName, MaterialOrigin::Embedded);
			if (const Material* mat = materialManager_->Resolve(current)) {
				out.defaultMaterialKeys.push_back(mat->GetName());
			}
			nameToMaterial[currentName] = current;
#pragma endregion
		} else if (identifier == "map_Kd") {
#pragma region ディフューズテクスチャ
			// ハンドルから実体を取り出してテクスチャを設定する.
			if (Material* mat = materialManager_->Resolve(current)) {
				std::string textureFilename;
				s >> textureFilename;

				const ShaderDefinition* shader = mat->GetShader();
				if (shader != nullptr && !shader->textures.empty()) {
					// map_Kd = ディフューズ → t0 のスロットへ.
					auto it = std::find_if(
						shader->textures.begin(), shader->textures.end(),
						[](const TextureSlotDesc& s) { return s.registerIndex == 0; }
					);
					if (it != shader->textures.end()) {
						mat->SetTexture(it->name, textureManager_->Load(directoryPath + "/" + textureFilename));
					}
				}
			}
#pragma endregion
		}
		// ※ Kd / Ks / Ns などの既存パース処理があればここに移植する.
	}

	return nameToMaterial;
}

void ModelManager::LoadObjFile(ModelData& out, const std::string& directoryPath, const std::string& filename, const ModelLoadDesc& desc) {
	// --- パース中の一時構造（o → メッシュ、usemtl → サブメッシュ）---
	struct SubMeshBuild {
		std::string materialName;
		std::vector<VertexData> vertices;
	};
	struct MeshBuild {
		std::string name;
		std::vector<SubMeshBuild> subs;
	};
	std::vector<MeshBuild> meshBuilds;
	int curMesh = -1;
	int curSub = -1;

	auto ensureMesh = [&]() -> MeshBuild& {
		if (curMesh < 0) {
			meshBuilds.push_back({});
			curMesh = static_cast<int>(meshBuilds.size()) - 1;
			curSub = -1;
		}
		return meshBuilds[static_cast<size_t>(curMesh)];
	};
	auto ensureSub = [&]() -> SubMeshBuild& {
		MeshBuild& m = ensureMesh();
		if (curSub < 0) {
			m.subs.push_back({});
			curSub = static_cast<int>(m.subs.size()) - 1;
		}
		return m.subs[static_cast<size_t>(curSub)];
	};

	std::vector<Cake::Vector4> positions; // 位置（ファイル全体で共有）.
	std::vector<Cake::Vector3> normals;   // 法線.
	std::vector<Cake::Vector2> texcoords; // UV.

	std::unordered_map<std::string, MaterialHandle> nameToMaterial; // OBJ内の名前 → ハンドル.
	std::string line;

	std::ifstream file(directoryPath + "/" + filename);
	if (!file.is_open()) {
		DebugLog::GetInstance().Log(LogLevel::Error, kLogCategory, "OBJ が開けません: " + filename);
		return;
	}

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "o") {
#pragma region オブジェクトの開始（新しいメッシュ）
			meshBuilds.push_back({});
			curMesh = static_cast<int>(meshBuilds.size()) - 1;
			curSub = -1;
			s >> meshBuilds[static_cast<size_t>(curMesh)].name;
#pragma endregion
		} else if (identifier == "v") {
#pragma region 位置
			Cake::Vector4 position;
			s >> position.x >> position.y >> position.z;
			if (desc.flipHandedness) {
				position.x *= -1.0f;
			}
			position.w = 1.0f;
			positions.push_back(position);
#pragma endregion
		} else if (identifier == "vt") {
#pragma region UV
			Cake::Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
#pragma endregion
		} else if (identifier == "vn") {
#pragma region 法線
			Cake::Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			if (desc.flipHandedness) {
				normal.x *= -1.0f;
			}
			normals.push_back(normal);
#pragma endregion
		} else if (identifier == "f") {
#pragma region 面（n角形をファン分割して三角形化）
			SubMeshBuild& sub = ensureSub();

			std::vector<VertexData> faceVertices;
			std::string def;
			while (s >> def) {
				std::istringstream v(def);
				uint32_t idx[3] = {0, 0, 0};
				for (int32_t e = 0; e < 3; ++e) {
					std::string t;
					std::getline(v, t, '/');
					idx[e] = t.empty() ? 0 : std::stoi(t);
				}

				Cake::Vector4 position = positions[idx[0] - 1];
				Cake::Vector2 texcoord = (idx[1] != 0) ? texcoords[idx[1] - 1] : Cake::Vector2::Zero;
				Cake::Vector3 normal = (idx[2] != 0) ? normals[idx[2] - 1] : Cake::Vector3::Zero;
				faceVertices.push_back({position, texcoord, normal});
			}

			if (faceVertices.size() < 3) {
				DebugLog::GetInstance().Log(LogLevel::Warn, kLogCategory, "頂点数が3未満の面をスキップしました。");
			} else {
				// ファン分割：(0,1,2), (0,2,3), (0,3,4)... で三角形化する（凸多角形前提）.
				for (size_t i = 1; i + 1 < faceVertices.size(); ++i) {
					VertexData tri[3] = {faceVertices[0], faceVertices[i], faceVertices[i + 1]};
					if (desc.flipHandedness) {
						sub.vertices.push_back(tri[2]);
						sub.vertices.push_back(tri[1]);
						sub.vertices.push_back(tri[0]);
					} else {
						sub.vertices.push_back(tri[0]);
						sub.vertices.push_back(tri[1]);
						sub.vertices.push_back(tri[2]);
					}
				}
			}
#pragma endregion
		} else if (identifier == "mtllib") {
#pragma region マテリアルテンプレート読み込み（MaterialManager に実体を作らせる）
			std::string materialFilename;
			s >> materialFilename;
			nameToMaterial = LoadMtlFile(out, directoryPath, materialFilename, desc);
#pragma endregion
		} else if (identifier == "usemtl") {
#pragma region 使用マテリアル指定（＝新しいサブメッシュ）
			MeshBuild& m = ensureMesh();
			m.subs.push_back({});
			curSub = static_cast<int>(m.subs.size()) - 1;
			s >> m.subs[static_cast<size_t>(curSub)].materialName;
#pragma endregion
		}
	}

	// マテリアルが1つも無ければデフォルトを1つ用意する.
	if (nameToMaterial.empty()) {
		MaterialHandle fallback = materialManager_->CreateMaterial(out.name, desc.defaultShaderName, MaterialOrigin::Embedded);
		if (const Material* mat = materialManager_->Resolve(fallback)) {
			out.defaultMaterialKeys.push_back(mat->GetName());
		}
		nameToMaterial[""] = fallback;
	}

	// OBJ内の名前 → MaterialHandle（見つからなければ先頭にフォールバック）.
	auto resolveMaterial = [&](const std::string& name) -> MaterialHandle {
		auto found = nameToMaterial.find(name);
		if (found != nameToMaterial.end()) {
			return found->second;
		}
		DebugLog::GetInstance().Log(
			LogLevel::Warn, kLogCategory,
			"usemtl '" + name + "' が mtllib に見つかりません。先頭のマテリアルを使用します。"
		);
		return nameToMaterial.begin()->second;
	};

	/*
	 * MeshBuilds から Mesh 実体を構築し、meshes に積む.
	 * ———————————————*/
	out.meshes.reserve(meshBuilds.size());
	for (MeshBuild& mb : meshBuilds) {
		Mesh mesh;
		mesh.name = mb.name;

		std::unordered_map<std::string, uint32_t> localOf; // マテリアル名 → この Mesh 内のスロット番号.

		for (SubMeshBuild& sb : mb.subs) {
			if (sb.vertices.empty()) {
				continue; // 面を持たないサブメッシュは捨てる.
			}

			// スロットを確保（未登録なら materialSlots に Material* を追加）.
			uint32_t slot;
			auto lit = localOf.find(sb.materialName);
			if (lit != localOf.end()) {
				slot = lit->second;
			} else {
				slot = mesh.GetSlotCount();
				localOf[sb.materialName] = slot;
				mesh.materialSlots.push_back(resolveMaterial(sb.materialName));
			}

			SubMesh sub;
			sub.vertices = std::move(sb.vertices);
			sub.materialSlot = slot;
			UploadSubMesh(device_, sub);
			mesh.subMeshes.push_back(std::move(sub));
		}

		if (mesh.subMeshes.empty()) {
			continue; // 空メッシュは捨てる.
		}
		out.meshes.push_back(std::move(mesh));
	}
}

} // namespace Cake
