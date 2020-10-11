#pragma once

#include <unordered_set>

#include "Base.h"

namespace Mango { namespace ECS {
	
	using Entity = uint32_t;

	static size_t _Hash() {
		static size_t next = 0;
		return next++;
	}

	template<typename T>
	static size_t Hash() {
		static const size_t id = _Hash();
		return id;
	}

	// Component Arrays -----------------------------------------------------------------------------------------------

	class _ComponentArray {
	public:
		virtual ~_ComponentArray() {}
		virtual _ComponentArray* NewOfSameType() const = 0;
		virtual void Copy(Entity entity, _ComponentArray* other) = 0;
		virtual void Erase(Entity entity) = 0;
		virtual size_t GetHash() const = 0;
		virtual bool Contains(Entity entity) const = 0;
		virtual size_t Size() const = 0;
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

		T& Insert(Entity entity, const T& data) {
			MG_CORE_ASSERT(!Contains(entity), "Array already contains entity.");
			mIndices[entity] = Size();
			mEntities.push_back(entity);
			mComponents.push_back(data);
			return At(entity);
		}

		virtual void Erase(Entity entity) override {
			MG_CORE_ASSERT(Contains(entity), "Array does not contain entity.");
			size_t index = mIndices[entity];

			mEntities[index] = mEntities.back();
			mComponents[index] = mComponents.back();
			mIndices[mEntities.back()] = index;

			mEntities.pop_back();
			mComponents.pop_back();
			mIndices.erase(entity);
		}

		inline virtual size_t Size() const override {
			return mIndices.size();
		}

		inline virtual bool Contains(Entity entity) const override { return mIndices.find(entity) != mIndices.end(); }

		T& At(Entity entity) {
			MG_CORE_ASSERT(Contains(entity), "Array does not contain entity.");
			return mComponents[mIndices[entity]];
		}

		virtual void Copy(Entity entity, _ComponentArray* other) override {
			MG_CORE_ASSERT(mHash == other->GetHash(), "Cannot copy across different ComponentArray types.");
			Insert(entity, ((ComponentArray<T>*)other)->At(entity));
		}

		virtual size_t GetHash() const override { return mHash; }

		T* Data() { return mComponents.data(); }

		inline auto begin() { return mComponents.begin(); }
		inline auto end() { return mComponents.end(); }
	private:
		size_t mHash;
		std::unordered_map<Entity, size_t> mIndices;
		std::vector<Entity> mEntities;
		std::vector<T> mComponents;
	};

	// Archetype --------------------------------------------------------------------------------------------------------

	class Archetype {
	public:
		Archetype() = default;

		template<typename T>
		T& Get(Entity entity) {
			MG_CORE_ASSERT(ContainsArray(Hash<T>()), "This archetype doesn't contain that component.");
			return (std::static_pointer_cast<ComponentArray<T>>(mMap[Hash<T>()]))->At(entity);
		}

		template<typename T>
		T& Insert(Entity entity, const T& data) {
			MG_CORE_ASSERT(ContainsArray(Hash<T>()), "This archetype doesn't contain that component.");
			return (std::static_pointer_cast<ComponentArray<T>>(mMap[Hash<T>()]))->Insert(entity, data);
		}

		void Destroy(Entity entity) {
			for (auto& pair : mMap)
				pair.second->Erase(entity);
		}

		template<typename T>
		bool Has(Entity entity) {
			if (!ContainsArray(Hash<T>())) return false;
			return mMap[Hash<T>()]->Contains(entity);
		}

		std::unordered_set<size_t> GetHashes() {
			std::unordered_set<size_t> hashes;
			for (auto& pair : mMap)
				hashes.insert(pair.first);
			return hashes;
		}

		size_t Size() {
			return mMap.begin()->second->Size();
		}

		void InsertArray(size_t hash, _ComponentArray* array) {
			MG_CORE_ASSERT(!ContainsArray(hash), "This archetype already contains this component type.");
			mMap[hash].reset(array);
		}

		inline std::unordered_map<size_t, Ref<_ComponentArray>>& GetMap() {
			return mMap;
		}

		template<typename T>
		inline T* Data() {
			MG_CORE_ASSERT(HasTypes<T>(), "Archetype does not contain this type.");
			return std::static_pointer_cast<ComponentArray<T>>(mMap[Hash<T>()])->Data();
		}

		void Move(Entity entity, Archetype* other) {
			for (auto& pair : mMap) {
				if(other->ContainsArray(pair.first))
					pair.second->Copy(entity, other->GetMap()[pair.first].get());
			}

			for (auto& pair : other->GetMap()) {
				pair.second->Erase(entity);
			}
		}
		
		template<typename... T>
		bool HasTypes() {
			bool has = true;
			(ContainsArrayBool<T>(has), ...);
			return has;
		}
	private:
		template<typename T>
		inline void ContainsArrayBool(bool& b) {
			if (!ContainsArray(Hash<T>()))
				b = false;
		}

		inline bool ContainsArray(size_t hash) const { return mMap.find(hash) != mMap.end(); }
	private:
		std::unordered_map<size_t, Ref<_ComponentArray>> mMap;
	};

	// Registry - User Interface ------------------------------------------------------------------------------------------

	class Registry {
	public:
		Registry()
			: mNextId(0)
		{
			// Index 0 archetype
			mArchetypes.push_back(Archetype());
		}


		// Entity Creation and Destruction -------------------------------------------------------------------------------

		Entity Create() {
			Entity id = mNextId++;
			mAliveEntities.insert(id);
			mEntityIndexMap[id] = 0;
			return id;
		}

		bool Valid(Entity entity) {
			return mAliveEntities.find(entity) != mAliveEntities.end();
		}

		void Destroy(Entity entity) {
			MG_CORE_ASSERT(mAliveEntities.find(entity) != mAliveEntities.end(), "This entity is not valid.");
			mArchetypes[GetIndex(entity)].Destroy(entity);
			mAliveEntities.erase(entity);
		}

		// Component Interfacing ----------------------------------------------------------------------------------------

		template<typename T, typename...Args>
		T& Emplace(Entity entity, Args...args) {
			return Insert<T>(entity, std::forward<Args>(args)...);
		}

		template<typename T>
		T& Insert(Entity entity, const T& data)
		{
			size_t hashT = Hash<T>();
			std::unordered_set<size_t> hashes = mArchetypes[GetIndex(entity)].GetHashes();
			MG_CORE_ASSERT(hashes.find(hashT) == hashes.end(), "Entity already owns this component type.");

			hashes.insert(hashT);
			size_t newIndex = FindArchetype(hashes, &mArchetypes[GetIndex(entity)]);

			if (newIndex == mArchetypes.size()) {
				// No matching archetype was found and one must be created
				MG_CORE_INFO("Registry::Insert: Creating a new archetype.");
				mArchetypes.push_back(Archetype()); // "newIndex" now points to our new archetype
				for (auto& pair : mArchetypes[GetIndex(entity)].GetMap())
					mArchetypes[newIndex].InsertArray(pair.first, pair.second->NewOfSameType()); // Copy all the component types
				mArchetypes[newIndex].InsertArray(hashT, new ComponentArray<T>()); // Add the additional array
			}
			else {
				MG_CORE_INFO("Registry::Insert: Found an existing archetype.");
			}

			mArchetypes[newIndex].Move(entity, &mArchetypes[GetIndex(entity)]);
			mEntityIndexMap[entity] = newIndex;
			return mArchetypes[GetIndex(entity)].Insert<T>(entity, data); // Finally add the component
		}

		template<typename T>
		void Remove(Entity entity) {
			size_t hashT = Hash<T>();
			std::unordered_set<size_t> hashes = mArchetypes[GetIndex(entity)].GetHashes();
			MG_CORE_ASSERT(hashes.find(hashT) != hashes.end(), "Entity does not own this component type.");

			hashes.erase(hashT);
			size_t newIndex = FindArchetype(hashes, &mArchetypes[GetIndex(entity)]);

			if (newIndex == mArchetypes.size()) {
				// No matching archetype was found and one must be created
				MG_CORE_INFO("Registry::Remove: Creating a new archetype.");
				mArchetypes.push_back(Archetype()); // "newIndex" now points to our new archetype
				for (auto& pair : mArchetypes[GetIndex(entity)].GetMap()) {
					if(pair.first != hashT) // Exclude the one to be erased
						mArchetypes[newIndex].InsertArray(pair.first, pair.second->NewOfSameType()); // Copy all the component types
				}
			}
			else {
				MG_CORE_INFO("Registry::Remove: Found an existing archetype.");
			}

			mArchetypes[newIndex].Move(entity, &mArchetypes[GetIndex(entity)]);
			mEntityIndexMap[entity] = newIndex;
		}

		template<typename T>
		T& Get(Entity entity) {
			return mArchetypes[GetIndex(entity)].Get<T>(entity);
		}

		template<typename T>
		bool Has(Entity entity) {
			return mArchetypes[GetIndex(entity)].Has<T>(entity);
		}

		// Systems ---------------------------------------------------------------------------------------------------

		template<typename... Types>
		void Query(std::vector<std::tuple<size_t, Types*...>>& out) {
			out.clear();
			for (auto& arch : mArchetypes) {
				bool matching = true;
				(DoesArchetypeContain<Types>(matching, &arch), ...);
				if (!matching || arch.Size() == 0) continue;
				out.push_back({ arch.Size(), arch.Data<Types>()... });
			}
		}
	
		size_t GetIndex(Entity entity) {
			return mEntityIndexMap[entity];
		}

		// -----------------------------------------------------------------------------------------------------------
		
	private:
		template<typename T>
		void DoesArchetypeContain(bool& b, Archetype* arch) {
			if (!arch->HasTypes<T>())
				b = false;
		}

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
		Entity mNextId;
		std::unordered_map<Entity, size_t> mEntityIndexMap;
		std::unordered_set<Entity> mAliveEntities;
		std::vector<Archetype> mArchetypes;
	};

}}