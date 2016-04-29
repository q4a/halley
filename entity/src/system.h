#pragma once

#include <vector>
#include <initializer_list>

#include "family_binding.h"
#include "family_mask.h"
#include "family_type.h"
#include "entity.h"

namespace Halley {
	class HalleyAPI;

	class System
	{
	public:
		System(std::initializer_list<FamilyBindingBase*> uninitializedFamilies);
		virtual ~System() {}

	protected:
		HalleyAPI& doGetAPI() const { return *api; }
		World& doGetWorld() const { return *world; }

		virtual void updateBase(Time) {}
		virtual void renderBase(Painter&) {}

		template <typename T, typename M, typename U, typename V>
		static void invokeIndividual(T* obj, M method, U& p, V& fam)
		{
			for (auto& e : fam) {
				(obj->*method)(p, e);
			}
		}

	private:
		friend class World;

		int nsTaken = 0;
		std::vector<FamilyBindingBase*> families;
		World* world;
		String name;
		HalleyAPI* api;

		void doUpdate(Time time);
		void doRender(Painter& painter);
		void onAddedToWorld(World& world);
	};

}

#define REGISTER_SYSTEM(sys) Halley::System* halleyCreate##sys##() { return new sys(); }
