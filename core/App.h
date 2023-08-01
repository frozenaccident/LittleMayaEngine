#pragma once

#include "Window.h"
#include "Utils.h"
#include "../render/Device.h"
#include "../render/Renderer.h"
#include "../ecs/GameObject.h"
#include "../render/Model.h"
#include "../render/Descriptors.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include <memory>
#include <vector>

namespace lm {

	struct vec3_hash {
		std::size_t operator()(const glm::vec3& vec) const {
			// Hash each component of a vector
			std::size_t h1 = std::hash<float>()(vec.x);
			std::size_t h2 = std::hash<float>()(vec.y);
			std::size_t h3 = std::hash<float>()(vec.z);
			hashCombine(h1, h2);
			hashCombine(h1, h3);
			return h1;
		}
	};

	struct VertexHash {
		size_t operator()(const lmModel::Vertex& vertex) const {
			/// Hash position, color, and normal vector of the vertex
			return ((vec3_hash()(vertex.position) ^
				(vec3_hash()(vertex.color) << 1)) >> 1) ^
				(vec3_hash()(vertex.normal) << 1);
		}
	};

	struct VertexEqual {
		bool operator()(const lmModel::Vertex& lhs, const lmModel::Vertex& rhs) const {
			/// Compare position, color, and normal vectors of two vertices
			return lhs.position == rhs.position && lhs.color == rhs.color && lhs.normal == rhs.normal;
		}
	};

    class App {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        App();
        ~App();

        App(const App&) = delete;
        App& operator=(const App&) = delete;

        void run();

    private:
        void loadGameObjects();
        lmModel::Data processAiMesh(aiMesh* mesh, const aiScene* scene, const std::string& modelDirectory);
        void processAiNode(aiNode* node, const aiScene* scene, const std::string& modelDirectory, const glm::vec3& scale, const glm::vec3& position);

        lmWindow lmWindow{ WIDTH, HEIGHT, "Little Maya Engine" };
        lmDevice lmDevice{ lmWindow };
        lmRenderer lmRenderer{ lmWindow, lmDevice };

        // NOTE: order of declarations matter
        std::unique_ptr<lmDescriptorPool> globalPool{};

        lmGameObject::Map gameObjects;
        std::unique_ptr<Assimp::Importer> assimpImporter;
    };

} // namespace lm
