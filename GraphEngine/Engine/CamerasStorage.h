#pragma once

#include "Camera.h"


namespace eng {
	class CamerasStorage {
		friend class GraphEngine;

		std::vector<size_t> cameras_index_;
		std::vector<size_t> free_camera_id_;
		std::vector<std::pair<size_t, Camera>> cameras_;

		CamerasStorage() noexcept {
		}

		CamerasStorage& operator=(const CamerasStorage& other)& noexcept = default;

		CamerasStorage& operator=(CamerasStorage&& other)& noexcept = default;

		void swap(CamerasStorage& other) noexcept {
			std::swap(cameras_index_, other.cameras_index_);
			std::swap(free_camera_id_, other.free_camera_id_);
			std::swap(cameras_, other.cameras_);
		}

	public:
		using Iterator = std::vector<std::pair<size_t, Camera>>::iterator;
		using ConstIterator = std::vector<std::pair<size_t, Camera>>::const_iterator;

		Camera& operator[](size_t id) {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid camera id.\n\n");
			}

			return cameras_[cameras_index_[id]].second;
		}

		const Camera& operator[](size_t id) const {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "operator[], invalid camera id.\n\n");
			}

			return cameras_[cameras_index_[id]].second;
		}

		size_t get_memory_id(size_t id) const {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_memory_id, invalid camera id.\n\n");
			}

			return cameras_index_[id];
		}

		size_t get_id(size_t memory_id) const {
			if (cameras_.size() <= memory_id) {
				throw EngOutOfRange(__FILE__, __LINE__, "get_id, invalid memory id.\n\n");
			}

			return cameras_[memory_id].first;
		}

		bool contains(size_t id) const noexcept {
			return id < cameras_index_.size() && cameras_index_[id] < std::numeric_limits<size_t>::max();
		}

		bool contains_memory(size_t memory_id) const noexcept {
			return memory_id < cameras_.size();
		}

		size_t size() const noexcept {
			return cameras_.size();
		}

		bool empty() const noexcept {
			return cameras_.empty();
		}

		Iterator begin() noexcept {
			return cameras_.begin();
		}

		Iterator end() noexcept {
			return cameras_.end();
		}

		ConstIterator begin() const noexcept {
			return cameras_.begin();
		}

		ConstIterator end() const noexcept {
			return cameras_.end();
		}

		CamerasStorage& erase(size_t id) {
			if (!contains(id)) {
				throw EngOutOfRange(__FILE__, __LINE__, "erase, invalid camera id.\n\n");
			}

			free_camera_id_.push_back(id);

			cameras_index_[cameras_.back().first] = cameras_index_[id];
			std::swap(cameras_[cameras_index_[id]], cameras_.back());

			cameras_.pop_back();
			cameras_index_[id] = std::numeric_limits<size_t>::max();
			return *this;
		}

		CamerasStorage& clear() noexcept {
			cameras_index_.clear();
			free_camera_id_.clear();
			cameras_.clear();
			return *this;
		}

		size_t insert(const Camera& camera) noexcept {
			size_t free_camera_id = cameras_index_.size();
			if (free_camera_id_.empty()) {
				cameras_index_.push_back(cameras_.size());
			} else {
				free_camera_id = free_camera_id_.back();
				free_camera_id_.pop_back();
				cameras_index_[free_camera_id] = cameras_.size();
			}

			cameras_.push_back({ free_camera_id, camera });
			return free_camera_id;
		}
	};
}

