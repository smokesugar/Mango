#pragma once

#include <unordered_set>

#include "Base.h"

namespace Mango { namespace ECS {
	
	using ID = uint32_t;

	struct Entity {
		ECS::ID ID;
		size_t Index;
	};

	static size_t _Hash() {
		static size_t next = 0;
		return next++;
	}

	template<typename T>
	static size_t Hash() {
		static const size_t id = _Hash();
		return id;
	}

	class _ComponentArray {
	public:
		virtual ~_ComponentArray() {}
		virtual _ComponentArray* NewOfSameType() const = 0;
		virtual void Copy(ID id, _ComponentArray* other) = 0;
		virtual void Erase(ID id) = 0;
		virtual size_t GetHash() const = 0;
		virtual bool Contains(ID id) const = 0;
	};

	template<typename T>
	class ComponentArray : public _ComponentArray {
	public:
		ComponentArray()
			: mHash(Hash<T>())
		{}

		virtual _ComponentArray* NewOfSameType() const override {
			return new ComponentArray<T>();
		}

		T& Insert(ID id, const T& data) {
			MG_CORE_ASSERT(!Contains(id), "Array already contains entity.");
			mIndices[id] = Size();
			mEntities.push_back(id);
			mComponents.push_back(data);
			return At(id);
		}

		virtual void Erase(ID id) override {
			MG_CORE_ASSERT(Contains(id), "Array does not contain entity.");
			size_t index = mIndices[id];

			mEntities[index] = mEntities.back();
			mComponents[index] = mComponents.back();
			mIndices[mEntities.back()] = index;

			mEntities.pop_back();
			mComponents.pop_back();
			mIndices.erase(id);
		}

		inline size_t Size() const {
			return mIndices.size();
		}

		inline virtual bool Contains(ID id) const override { return mIndices.find(id) != mIndices.end(); }

		T& At(ID id) {
			MG_CORE_ASSERT(Contains(id), "Array does not contain entity.");
			return mComponents[mIndices[id]];
		}

		virtual void Copy(ID id, _ComponentArray* other) override {
			MG_CORE_ASSERT(mHash == other->GetHash(), "Cannot copy across different ComponentArray types.");
			Insert(id, ((ComponentArray<T>*)other)->At(id));
		}

		virtual size_t GetHash() const override { return mHash; }

		inline auto begin() { return mComponents.begin(); }
		inline auto end() { return mComponents.end(); }
	private:
		size_t mHash;
		std::unordered_map<ID, size_t> mIndices;
		std::vector<ID> mEntities;
		std::vector<T> mComponents;
	};

	class Archetype {
	public:
		Archetype() = default;

		template<typename T>
		T& Get(ID id) {
			MG_CORE_ASSERT(ContainsArray(Hash<T>()), "This archetype doesn't contain that component.");
			return (std::static_pointer_cast<ComponentArray<T>>(mMap[Hash<T>()]))->At(id);
		}

		template<typename T>
		T& Insert(ID id, const T& data) {
			MG_CORE_ASSERT(ContainsArray(Hash<T>()), "This archetype doesn't contain that component.");
			return (std::static_pointer_cast<ComponentArray<T>>(mMap[Hash<T>()]))->Insert(id, data);
		}

		void Destroy(ID id) {
			for (auto& pair : mMap)
				pair.second->Erase(id);
		}

		template<typename T>
		bool Has(ID id) {
			if (!ContainsArray(Hash<T>())) return false;
			return mMap[Hash<T>()]->Contains(id);
		}

		std::unordered_set<size_t> GetHashes() {
			std::unordered_set<size_t> hashes;
			for (auto& pair : mMap)
				hashes.insert(pair.first);
			return hashes;
		}

		void InsertArray(size_t hash, _ComponentArray* array) {
			MG_CORE_ASSERT(!ContainsArray(hash), "This archetype already contains this component type.");
			mMap[hash].reset(array);
		}

		inline std::unordered_map<size_t, Ref<_ComponentArray>>& GetMap() {
			return mMap;
		}

		void Move(ID id, Archetype* other) {
			for (auto& pair : mMap) {
				if(other->ContainsArray(pair.first))
					pair.second->Copy(id, other->GetMap()[pair.first].get());
			}

			for (auto& pair : other->GetMap()) {
				pair.second->Erase(id);
			}
		}
	private:
		inline bool ContainsArray(size_t hash) const { return mMap.find(hash) != mMap.end(); }
	private:
		std::unordered_map<size_t, Ref<_ComponentArray>> mMap;
	};

	class Registry {
	public:
		Registry()
			: mNextId(0)
		{
			// Index 0 archetype
			mArchetypes.push_back(Archetype());
		}

		Entity Create() {
			ID id = mNextId++;
			mAliveEntities.insert(id);
			return { id, 0 };
		}

		template<typename T, typename...Args>
		T& Emplace(Entity& entity, Args...args) {
			return Insert<T>(entity, std::forward<Args>(args)...);
		}

		template<typename T>
		T& Insert(Entity& entity, const T& data)
		{
			size_t hashT = Hash<T>();
			std::unordered_set<size_t> hashes = mArchetypes[entity.Index].GetHashes();
			MG_CORE_ASSERT(hashes.find(hashT) == hashes.end(), "Entity already owns this component type.");

			hashes.insert(hashT);
			size_t newIndex = FindArchetype(hashes, &mArchetypes[entity.Index]);

			if (newIndex == mArchetypes.size()) {
				// No matching archetype was found and one must be created
				MG_CORE_INFO("Registry::Insert: Creating a new archetype.");
				mArchetypes.push_back(Archetype()); // "newIndex" now points to our new archetype
				for (auto& pair : mArchetypes[entity.Index].GetMap())
					mArchetypes[newIndex].InsertArray(pair.first, pair.second->NewOfSameType()); // Copy all the component types
				mArchetypes[newIndex].InsertArray(hashT, new ComponentArray<T>()); // Add the additional array
			}
			else {
				MG_CORE_INFO("Registry::Insert: Found an existing archetype.");
			}

			mArchetypes[newIndex].Move(entity.ID, &mArchetypes[entity.Index]);
			entity.Index = newIndex;
			return mArchetypes[entity.Index].Insert<T>(entity.ID, data); // Finally add the component
		}

		template<typename T>
		void Remove(Entity& entity) {
			size_t hashT = Hash<T>();
			std::unordered_set<size_t> hashes = mArchetypes[entity.Index].GetHashes();
			MG_CORE_ASSERT(hashes.find(hashT) != hashes.end(), "Entity does not own this component type.");

			hashes.erase(hashT);
			size_t newIndex = FindArchetype(hashes, &mArchetypes[entity.Index]);

			if (newIndex == mArchetypes.size()) {
				// No matching archetype was found and one must be created
				MG_CORE_INFO("Registry::Remove: Creating a new archetype.");
				mArchetypes.push_back(Archetype()); // "newIndex" now points to our new archetype
				for (auto& pair : mArchetypes[entity.Index].GetMap()) {
					if(pair.first != hashT) // Exclude the one to be erased
						mArchetypes[newIndex].InsertArray(pair.first, pair.second->NewOfSameType()); // Copy all the component types
				}
			}
			else {
				MG_CORE_INFO("Registry::Remove: Found an existing archetype.");
			}

			mArchetypes[newIndex].Move(entity.ID, &mArchetypes[entity.Index]);
			entity.Index = newIndex;
		}

		void Destroy(Entity& entity) {
			MG_CORE_ASSERT(mAliveEntities.find(entity.ID) != mAliveEntities.end(), "This entity is not valid.");
			mArchetypes[entity.Index].Destroy(entity.ID);
			mAliveEntities.erase(entity.ID);
		}

		template<typename T>
		T& Get(const Entity& entity) {
			return mArchetypes[entity.Index].Get<T>(entity.ID);
		}

		template<typename T>
		bool Has(const Entity& entity) {
			return mArchetypes[entity.Index].Has<T>(entity.ID);
		}
	private:
		size_t FindArchetype(const std::unordered_set<size_t> signature, Archetype* excluding)
		{
			size_t index = mArchetypes.size();
			for (size_t i = 0; i < mArchetypes.size(); i++)
			{
				auto& arch = mArchetypes[i];
				if (&arch == excluding || arch.GetMap().size() != signature.size())
					continue;

				bool matching = true;
				for (size_t hash : signature)
				{
					if (arch.GetMap().find(hash) == arch.GetMap().end()) {
						matching = false;
						break;
					}
				}

				if (matching) {
					index = i;
					break;
				}
			}
			return index;
		}
	private:
		ID mNextId;
		std::unordered_set<ID> mAliveEntities;
		std::vector<Archetype> mArchetypes;
	};

}}