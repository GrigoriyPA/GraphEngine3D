#pragma once

#include "../Utils/Functions.hpp"


// Interface of private associative object storage
namespace gre {
	template <typename ValueType>
	class AssociativeStorage {
		std::vector<size_t> values_index_;
		std::vector<size_t> free_value_id_;
		std::vector<std::pair<size_t, ValueType>> values_;

	protected:
		AssociativeStorage() noexcept {
		}

		AssociativeStorage(const AssociativeStorage<ValueType>& other)
			: values_index_(other.values_index_)
			, free_value_id_(other.free_value_id_)
			, values_(other.values_)
		{}

		AssociativeStorage(AssociativeStorage<ValueType>&& other) noexcept {
			swap(other);
		}

		AssociativeStorage<ValueType>& operator=(const AssociativeStorage<ValueType>& other)& {
			AssociativeStorage<ValueType> object(other);
			swap(object);
			return *this;
		}

		AssociativeStorage<ValueType>& operator=(AssociativeStorage<ValueType>&& other)& noexcept {
			deallocate();
			swap(other);
			return *this;
		}

		virtual void swap(AssociativeStorage& other) noexcept {
			values_index_.swap(other.values_index_);
			free_value_id_.swap(other.free_value_id_);
			values_.swap(other.values_);
		}

		virtual void deallocate() noexcept {
		}

	public:
		using Iterator = std::vector<std::pair<size_t, ValueType>>::iterator;
		using ConstIterator = std::vector<std::pair<size_t, ValueType>>::const_iterator;

		ValueType& operator[](size_t id) {
			GRE_ENSURE(contains(id), GreOutOfRange, "invalid value id");

			return values_[values_index_[id]].second;
		}

		const ValueType& operator[](size_t id) const {
			GRE_ENSURE(contains(id), GreOutOfRange, "invalid value id");

			return values_[values_index_[id]].second;
		}

		size_t get_memory_id(size_t id) const {
			GRE_ENSURE(contains(id), GreOutOfRange, "invalid value id");

			return values_index_[id];
		}

		size_t get_id(size_t memory_id) const {
			GRE_ENSURE(memory_id < values_.size(), GreOutOfRange, "invalid memory id");

			return values_[memory_id].first;
		}

		bool contains(size_t id) const noexcept {
			return id < values_index_.size() && values_index_[id] < std::numeric_limits<size_t>::max();
		}

		bool contains_memory(size_t memory_id) const noexcept {
			return memory_id < values_.size();
		}

		size_t size() const noexcept {
			return values_.size();
		}

		bool empty() const noexcept {
			return values_.empty();
		}

		Iterator begin() noexcept {
			return values_.begin();
		}

		Iterator end() noexcept {
			return values_.end();
		}

		ConstIterator begin() const noexcept {
			return values_.begin();
		}

		ConstIterator end() const noexcept {
			return values_.end();
		}

		void erase(size_t id) {
			GRE_ENSURE(contains(id), GreOutOfRange, "invalid value id");

			free_value_id_.push_back(id);

			values_index_[values_.back().first] = values_index_[id];
			std::swap(values_[values_index_[id]], values_.back());

			values_.pop_back();
			values_index_[id] = std::numeric_limits<size_t>::max();
		}

		void clear() noexcept {
			values_index_.clear();
			free_value_id_.clear();
			values_.clear();
		}

		size_t insert(const ValueType& camera) {
			size_t free_value_id = values_index_.size();
			if (free_value_id_.empty()) {
				values_index_.push_back(values_.size());
			}
			else {
				free_value_id = free_value_id_.back();
				free_value_id_.pop_back();
				values_index_[free_value_id] = values_.size();
			}

			values_.push_back({ free_value_id, camera });
			return free_value_id;
		}

		virtual ~AssociativeStorage() {
			deallocate();
		}
	};
}  // namespace gre
