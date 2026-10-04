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