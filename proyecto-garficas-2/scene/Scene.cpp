#include <scene/Scene.h>

Object* Scene::addObject(std::unique_ptr<Object> obj) {
	Object* ptr = obj.get();
	ptr->id = nextObjectId++;
	objects.push_back(std::move(obj));
	return ptr;
}

void Scene::removeObject(uint32_t id) {
	objects.erase(std::remove_if(objects.begin(), objects.end(),
		[id](const std::unique_ptr<Object>& obj) { return obj->id == id; }),
		objects.end());
}

void Scene::clear() {
	objects.clear();
}

Object* Scene::findById(uint32_t id) {
	for (const auto& obj : objects) {
		if (obj->id == id) {
			return obj.get();
		}
	}
	return nullptr;
}

void Scene::setup(GLuint shaderProgram) {
	this->shaderProgram = shaderProgram;
	MatrixID = glGetUniformLocation(shaderProgram, "MVP");
	ModelMatrixID = glGetUniformLocation(shaderProgram, "modelMatrix");
	LightDirID = glGetUniformLocation(shaderProgram, "lightDir");
	ObjectColorID = glGetUniformLocation(shaderProgram, "objectColor");
	AlphaID = glGetUniformLocation(shaderProgram, "alpha");
}

void Scene::draw(float ratio) {
	if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
	else glDisable(GL_DEPTH_TEST);

	if (backFaceCullingEnabled) glEnable(GL_CULL_FACE);

	else glDisable(GL_CULL_FACE);

	glm::mat4 Projection = camera->getProjectionMatrix(ratio);
	glm::mat4 View = camera->getViewMatrix();

	glUseProgram(shaderProgram);
	for (const auto& obj : objects) {
		if (obj->mesh) {
			glm::mat4 Model = obj->transform.getModelMatrix();
			glm::mat4 MVP = Projection * View * Model;


			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &Model[0][0]);
			glUniform3f(LightDirID, -0.5f, -1.0f, -0.3f);
			glUniform3f(ObjectColorID, 1.0f, 0.4f, 0.4f);
			glUniform1f(AlphaID, 1.0f);

			obj->mesh->draw();
		}
	}
}