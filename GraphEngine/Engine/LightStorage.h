#pragma once

#include "../Light/Light.h"


namespace eng {
	class LightStorage {
		friend class GraphEngine;

		size_t max_count_lights_;
		std::vector<size_t> lights_index_;
		std::vector<size_t> free_light_id_;
		std::vector<std::pair<size_t, Light*>> lights_;

		LightStorage() noexcept {
			max_count_lights_ = 0;
		}

		LightStorage& operator=(const LightStorage& other)& noexcept = default;

		LightStorage& operator=(LightStorage&& other)& noexcept = default;

		void swap(LightStorage& other) noexcept {
			std::swap(max_count_lights_, other.max_count_lights_);
			std::swap(lights_index_, other.lights_index_);
			std::swap(free_light_id_, other.free_light_id_);
			std::swap(lights_, other.lights_);
		}

	public:
		using Iterator = std::vector<std::pair<size_t, Light*>>::iterator;
		using ConstIterator = std::vector<std::pair<size_t, Light*>>::const_iterator;

		Light*& operator[](size_t id) {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid light id.\n\n");
			}

			return lights_[lights_index_[id]].second;
		}

		const Light* const& operator[](size_t id) const {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid light id.\n\n");
			}

			return lights_[lights_index_[id]].second;
		}

		size_t get_memory_id(size_t id) const {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_memory_id, invalid light id.\n\n");
			}

			return lights_index_[id];
		}

		size_t get_id(size_t memory_id) const {
			if (lights_.size() <= memory_id) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_id, invalid memory id.\n\n");
			}

			return lights_[memory_id].first;
		}

		size_t get_max_count_lights() const noexcept {
			return max_count_lights_;
		}

		bool contains(size_t id) const noexcept {
			return id < lights_index_.size() && lights_index_[id] < std::numeric_limits<size_t>::max();
		}

		bool contains_memory(size_t memory_id) const noexcept {
			return memory_id < lights_.size();
		}

		size_t size() const noexcept {
			return lights_.size();
		}

		bool empty() const noexcept {
			return lights_.empty();
		}

		Iterator begin() noexcept {
			return lights_.begin();
		}

		Iterator end() noexcept {
			return lights_.end();
		}

		ConstIterator begin() const noexcept {
			return lights_.begin();
		}

		ConstIterator end() const noexcept {
			return lights_.end();
		}

		LightStorage& erase(size_t id) {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "erase, invalid object id.\n\n");
			}

			free_light_id_.push_back(id);

			lights_index_[lights_.back().first] = lights_index_[id];
			std::swap(lights_[lights_index_[id]], lights_.back());

			lights_.pop_back();
			lights_index_[id] = std::numeric_limits<size_t>::max();
			return *this;
		}

		LightStorage& clear() noexcept {
			lights_index_.clear();
			free_light_id_.clear();
			lights_.clear();
			return *this;
		}

		size_t insert(Light* light) {
			if (lights_.size() == max_count_lights_) {
				throw EngRuntimeError(__FILE__, __LINE__, "insert, too many lights created.\n\n");
			}

			size_t free_object_id = lights_index_.size();
			if (free_light_id_.empty()) {
				lights_index_.push_back(lights_.size());
			} else {
				free_object_id = free_light_id_.back();
				free_light_id_.pop_back();
				lights_index_[free_object_id] = lights_.size();
			}

			lights_.push_back({ free_object_id, light });
			return free_object_id;
		}
	};
}
