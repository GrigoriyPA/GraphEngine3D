#pragma once

#include "GraphObject.h"


namespace gre {
	class GraphObjectStorage {
		friend class GraphEngine;

		std::vector<size_t> objects_index_;
		std::vector<size_t> free_object_id_;
		std::vector<std::pair<size_t, GraphObject>> objects_;

		GraphObjectStorage() noexcept {
		}

		GraphObjectStorage& operator=(const GraphObjectStorage& other)& noexcept = default;

		GraphObjectStorage& operator=(GraphObjectStorage&& other)& noexcept = default;

		void swap(GraphObjectStorage& other) noexcept {
			std::swap(objects_index_, other.objects_index_);
			std::swap(free_object_id_, other.free_object_id_);
			std::swap(objects_, other.objects_);
		}

	public:
		using Iterator = std::vector<std::pair<size_t, GraphObject>>::iterator;
		using ConstIterator = std::vector<std::pair<size_t, GraphObject>>::const_iterator;

		GraphObject& operator[](size_t id) {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid object id.\n\n");
			}
#endif // _DEBUG

			return objects_[objects_index_[id]].second;
		}

		const GraphObject& operator[](size_t id) const {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "operator[], invalid object id.\n\n");
			}
#endif // _DEBUG

			return objects_[objects_index_[id]].second;
		}

		size_t get_memory_id(size_t id) const {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_memory_id, invalid object id.\n\n");
			}
#endif // _DEBUG

			return objects_index_[id];
		}

		size_t get_id(size_t memory_id) const {
#ifdef _DEBUG
			if (objects_.size() <= memory_id) {
				throw GreOutOfRange(__FILE__, __LINE__, "get_id, invalid memory id.\n\n");
			}
#endif // _DEBUG

			return objects_[memory_id].first;
		}

		bool contains(size_t id) const noexcept {
			return id < objects_index_.size() && objects_index_[id] < std::numeric_limits<size_t>::max();
		}

		bool contains_memory(size_t memory_id) const noexcept {
			return memory_id < objects_.size();
		}

		size_t size() const noexcept {
			return objects_.size();
		}

		bool empty() const noexcept {
			return objects_.empty();
		}

		Iterator begin() noexcept {
			return objects_.begin();
		}

		Iterator end() noexcept {
			return objects_.end();
		}

		ConstIterator begin() const noexcept {
			return objects_.begin();
		}

		ConstIterator end() const noexcept {
			return objects_.end();
		}

		bool erase(size_t id, size_t model_id) {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "erase, invalid object id.\n\n");
			}
			if (!objects_[objects_index_[id]].second.models.contains(model_id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "erase, invalid model id.\n\n");
			}
#endif // _DEBUG

			objects_[objects_index_[id]].second.models.erase(model_id);
			if (objects_[objects_index_[id]].second.models.empty()) {
				erase(id);
				return true;
			}
			return false;
		}

		void erase(size_t id) {
#ifdef _DEBUG
			if (!contains(id)) {
				throw GreOutOfRange(__FILE__, __LINE__, "erase, invalid object id.\n\n");
			}
#endif // _DEBUG

			free_object_id_.push_back(id);

			objects_index_[objects_.back().first] = objects_index_[id];
			std::swap(objects_[objects_index_[id]], objects_.back());

			objects_.pop_back();
			objects_index_[id] = std::numeric_limits<size_t>::max();
		}

		void clear() noexcept {
			objects_index_.clear();
			free_object_id_.clear();
			objects_.clear();
		}

		size_t insert(const GraphObject& object) {
			size_t free_object_id = objects_index_.size();
			if (free_object_id_.empty()) {
				objects_index_.push_back(objects_.size());
			} else {
				free_object_id = free_object_id_.back();
				free_object_id_.pop_back();
				objects_index_[free_object_id] = objects_.size();
			}

			objects_.push_back({ free_object_id, object });
			return free_object_id;
		}
	};
}
