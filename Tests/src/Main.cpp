#include "Mango/Core/ECS.h"

#define CATCH_CONFIG_MAIN
#include "catch.hpp"

struct Position {
	float x, y, z, w;
};
struct Health {
	float hp;
};
struct Dummy {
	int a, b;
	float c;
	uint16_t d, e;
};

using namespace Mango;

TEST_CASE("basic", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	Health h{ 100 };

	ECS::Entity entity = registry.Create();
	registry.Insert(entity, h);
	registry.Insert(entity, p);
	REQUIRE(registry.Has<Position>(entity));
	REQUIRE(registry.Has<Health>(entity));
	REQUIRE(!registry.Has<Dummy>(entity));

	const Position& outPos = registry.Get<Position>(entity);
	const Health& outHealth = registry.Get<Health>(entity);

	REQUIRE(outPos.x == Approx(p.x));
	REQUIRE(outPos.y == Approx(p.y));
	REQUIRE(outPos.z == Approx(p.z));
	REQUIRE(outHealth.hp == Approx(h.hp));
}

TEST_CASE("arch query", "[core,ecs]") {
	Position p{ 0, 1, 2, 3 };
	Health h{ 100 };
	ECS::Archetype arch;
	arch.InsertArray(ECS::Hash<Position>(), new ECS::ComponentArray<Position>());
	arch.InsertArray(ECS::Hash<Health>(), new ECS::ComponentArray<Health>());

	REQUIRE(arch.HasTypes<Position>());
	REQUIRE(arch.HasTypes<Health>());
	REQUIRE(!arch.HasTypes<Dummy>());
	REQUIRE(arch.HasTypes<Position, Health>());
	REQUIRE(!arch.HasTypes<Position, Health, Dummy>());
	REQUIRE(!arch.HasTypes<Position, Dummy>());
	REQUIRE(!arch.HasTypes<Dummy>());
	REQUIRE(!arch.HasTypes<Health, Dummy>());
	REQUIRE(!arch.HasTypes<Dummy, Health>());
	REQUIRE(arch.HasTypes<Health, Position>());
}

TEST_CASE("Basic query", "[core,ecs]") {
	ECS::Registry registry;

	ECS::Entity eid = {};

	for (int i = 0; i < 10; ++i) {
		Position p{ 0, static_cast<float>(i), 0 };
		Health h{ static_cast<float>(i) };
		auto currentEntity = registry.Create();
		registry.Insert(currentEntity, p);
		registry.Insert(currentEntity, h);
		if (i == 4) {
			eid = currentEntity;
		}
	}

	auto query = registry.Query<Position, Health>();
	REQUIRE(query.size() == 1);
	REQUIRE(std::get<0>(query[0]) == 10);

	const Position* p = std::get<1>(query[0]);
	const Health* h = std::get<2>(query[0]);
	size_t count = std::get<0>(query[0]);
	REQUIRE(count == 10);
	for (size_t i = 0; i < count; ++i) {
		const Position& currP = p[i];
		const Health& currH = h[i];
		REQUIRE(currP.x == Approx(0.0f));
		REQUIRE(currP.y == Approx(i));
		REQUIRE(currP.z == Approx(0.0f));

		REQUIRE(currH.hp == Approx(i));
	}

	// testing look up by entity, we saved an id during creation and we want to
	// look it up
	Position posCmp = registry.Get<Position>(eid);
	REQUIRE(posCmp.x == Approx(0.0f));
	REQUIRE(posCmp.y == Approx(4));
	REQUIRE(posCmp.z == Approx(0.0f));
}

TEST_CASE("Check on not existing component", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	Health h{ 100 };
	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	registry.Insert(eid, h);

	REQUIRE(!registry.Has<Dummy>(eid));
}

TEST_CASE("Entity growth over limit", "[core,ecs]") {
	const int capacity = 10;
	const int toIterate = capacity * 5;
	ECS::Registry registry;
	ECS::Entity eid{};
	for (int i = 0; i < toIterate; ++i) {
		ECS::Entity id = registry.Create();
		registry.Insert<Position>(id, { 0, static_cast<float>(i), static_cast<float>(i) });
		if (i == (capacity + 1)) {
			eid = id;
		}
	}
	Position posCmp = registry.Get<Position>(eid);
	REQUIRE(posCmp.x == Approx(0.0f));
	REQUIRE(posCmp.y == Approx(capacity + 1));
	REQUIRE(posCmp.z == Approx(capacity + 1));
}

TEST_CASE("Add component to entity", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 16, 32 };
	Health h{ 100 };
	Dummy d{ 0, 1, 3.0f, 5, 6 };

	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	auto query1 = registry.Query<Position>();

	// Testing we only get one archetype and one item in it
	REQUIRE(query1.size() == 1);
	const Position* p0 = std::get<1>(query1[0]);
	REQUIRE(std::get<0>(query1[0]) == 1);
	REQUIRE(p0[0].x == Approx(0));
	REQUIRE(p0[0].y == Approx(1));
	REQUIRE(p0[0].z == Approx(16));
	REQUIRE(p0[0].w == Approx(32));

	// let us add a health component
	registry.Insert(eid, h);
	REQUIRE(registry.Has<Health>(eid));

	// similarly now we query and check that only get one archetype that has
	// both components
	auto query2 = registry.Query<Position, Health>();
	REQUIRE(query2.size() == 1);
	const Position* p2 = std::get<1>(query2[0]);
	const Health* h2 = std::get<2>(query2[0]);
	REQUIRE(std::get<0>(query2[0]) == 1);
	REQUIRE(p2[0].x == Approx(0));
	REQUIRE(p2[0].y == Approx(1));
	REQUIRE(p2[0].z == Approx(16));
	REQUIRE(p2[0].w == Approx(32));
	REQUIRE(h2[0].hp == Approx(100));

	// let check that the the look up by entity is still valid
	Position movedPos = registry.Get<Position>(eid);
	Health movedHealth = registry.Get<Health>(eid);
	REQUIRE(movedPos.x == Approx(0));
	REQUIRE(movedPos.y == Approx(1));
	REQUIRE(movedPos.z == Approx(16));
	REQUIRE(movedPos.w == Approx(32));
	REQUIRE(movedHealth.hp == Approx(100));

	// we need to make sure the old query for position only,returns one, since
	// the entity got moved.
	query1 = registry.Query<Position>();
	REQUIRE(query1.size() == 1);

	// adding only two positions entities
	p.x = 30;
	ECS::Entity eid2 = registry.Create();
	registry.Insert(eid2, p);
	p.x = 40;
	ECS::Entity eid3 = registry.Create();
	registry.Insert(eid3, p);
	auto query3 = registry.Query<Position>();

	// now we check that two archetypes actually match the query
	REQUIRE(query3.size() == 2);
	const Position* p3 = std::get<1>(query3[0]);
	REQUIRE(std::get<0>(query3[0]) == 2);
	REQUIRE(p3[0].x == Approx(30));
	REQUIRE(p3[1].x == Approx(40));

	// next we check archetype 1, the first returned which is the one with
	// position and health
	const Position* p3_1 = std::get<1>(query3[1]);
	REQUIRE(p3_1[0].z == Approx(16));
	REQUIRE(registry.Has<Dummy>(eid) == false);
	REQUIRE(registry.Has<Dummy>(eid3) == false);

	// next we add a dummy component to one of the position only entity, the
	// last one created
	registry.Insert(eid3, d);

	auto query4 = registry.Query<Position>();
	// we now expect 3 archetypes (position), (position,health),(position,dummy)
	REQUIRE(query4.size() == 3);
	const Position* p4 = std::get<1>(query4[0]);
	REQUIRE(std::get<0>(query4[0]) == 1);
	// it should only have one entity in it the one with position 30
	REQUIRE(p4[0].x == Approx(30));

	// next we check archetype 1, the first returned which is the one with
	// position and health
	REQUIRE(std::get<1>(query4[1])[0].z == Approx(16));

	// finally  we check the new archetype
	Position* q4_2pos = std::get<1>(query4[2]);
	REQUIRE(q4_2pos[0].x == Approx(40));

	// also checking that the original entity did not have a dummy, buy the one
	// we actually added the component to does have it
	REQUIRE(registry.Has<Dummy>(eid) == false);
	REQUIRE(registry.Has<Dummy>(eid3) == true);

	auto query5 = registry.Query<Position, Dummy>();
	REQUIRE(query5.size() == 1);
	REQUIRE(std::get<0>(query5[0]) == 1);
	REQUIRE(std::get<1>(query5[0])[0].x == Approx(40));
	REQUIRE(std::get<2>(query5[0])[0].a == 0);
	REQUIRE(std::get<2>(query5[0])[0].b == 1);
	REQUIRE(std::get<2>(query5[0])[0].c == Approx(3.0f));
	REQUIRE(std::get<2>(query5[0])[0].d == 5);
	REQUIRE(std::get<2>(query5[0])[0].e == 6);

	// let us perform the look up by entityId and check of parameters for all 3
	// entities
	movedPos = registry.Get<Position>(eid);
	movedHealth = registry.Get<Health>(eid);
	REQUIRE(movedPos.x == Approx(0));
	REQUIRE(movedPos.y == Approx(1));
	REQUIRE(movedPos.z == Approx(16));
	REQUIRE(movedPos.w == Approx(32));
	REQUIRE(movedHealth.hp == Approx(100));

	movedPos = registry.Get<Position>(eid2);
	REQUIRE(movedPos.x == Approx(30));
	REQUIRE(movedPos.y == Approx(1));
	REQUIRE(movedPos.z == Approx(16));
	REQUIRE(movedPos.w == Approx(32));

	movedPos = registry.Get<Position>(eid3);
	Dummy movedDummy = registry.Get<Dummy>(eid3);
	REQUIRE(movedPos.x == Approx(40));
	REQUIRE(movedPos.y == Approx(1));
	REQUIRE(movedPos.z == Approx(16));
	REQUIRE(movedPos.w == Approx(32));

	REQUIRE(movedDummy.a == 0);
	REQUIRE(movedDummy.b == 1);
	REQUIRE(movedDummy.c == Approx(3.0f));
	REQUIRE(movedDummy.d == 5);
	REQUIRE(movedDummy.e == 6);
}

TEST_CASE("Delete component to entity", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 16, 32 };
	Health h{ 100 };

	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	registry.Insert(eid, h);
	auto query1 = registry.Query<Position, Health>();

	// testing that we only get one archetype and one item in it
	REQUIRE(query1.size() == 1);
	const Position* p0 = std::get<1>(query1[0]);
	REQUIRE(std::get<0>(query1[0]) == 1);
	REQUIRE(p0[0].x == Approx(0));
	REQUIRE(p0[0].y == Approx(1));
	REQUIRE(p0[0].z == Approx(16));
	REQUIRE(p0[0].w == Approx(32));

	// now we delete a component
	registry.Remove<Health>(eid);
	auto query2 = registry.Query<Position, Health>();
	REQUIRE(query2.size() == 0);

	auto query3 = registry.Query<Position>();
	REQUIRE(query3.size() == 1);
}

TEST_CASE("Add component to existing archetype", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 16, 32 };
	Health h{ 100 };

	// first we create an entity with both position and health
	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	registry.Insert(eid, h);
	auto query1 = registry.Query<Position, Health>();

	REQUIRE(query1.size() == 1);
	REQUIRE(std::get<0>(query1[0]) == 1);

	// next we create an entity with only position
	p.x = 100;
	ECS::Entity eid2 = registry.Create();
	registry.Insert(eid2, p);
	auto query2 = registry.Query<Position>();
	// at this point we should have two archetype returned in the query
	// and each archetype has one entity in it
	REQUIRE(query2.size() == 2);
	REQUIRE(std::get<0>(query2[0]) == 1);

	// now we wish to add a health component to our entity with just position
	h.hp = 900;
	registry.Insert(eid2, h);
	// now we should still have one archetype, a new one should not be created
	auto query3 = registry.Query<Position, Health>();
	REQUIRE(query3.size() == 1);
	// and the current archetype should have two entities with position and
	// health

	REQUIRE(std::get<0>(query3[0]) == 2);

	// we also want to query we get the right entity components
	const Position* p3 = std::get<1>(query3[0]);
	const Health* h3 = std::get<2>(query3[0]);
	REQUIRE(p3[0].x == Approx(0));
	REQUIRE(p3[0].y == Approx(1));
	REQUIRE(p3[0].z == Approx(16));
	REQUIRE(p3[0].w == Approx(32));
	REQUIRE(p3[1].x == Approx(100));
	REQUIRE(p3[1].y == Approx(1));
	REQUIRE(p3[1].z == Approx(16));
	REQUIRE(p3[1].w == Approx(32));

	REQUIRE(h3[0].hp == Approx(100));
	REQUIRE(h3[1].hp == Approx(900));

	// the query for position only should return just one archetype
	// because the other archetype is empty
	auto query4 = registry.Query<Position>();
	REQUIRE(query4.size() == 1);
}

TEST_CASE("Preserve entity tracking on entity deletion", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	Health h{ 100 };
	Position p2{ 4, 5, 6, 7 };
	Health h2{ 900 };
	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	registry.Insert(eid, h);
	ECS::Entity eid2 = registry.Create();
	registry.Insert(eid2, p2);
	registry.Insert(eid2, h2);
	const ECS::Entity& e2 = eid2;

	// we are about to remove a component, this will move the
	// the entity from one archetype to the other, so we are going
	// to extract the entity index from the entity before and after
	// the component remove, it should change
	const size_t e2idxPre = registry.GetIndex(eid2);
	registry.Remove<Position>(eid2);
	const size_t e2idxPost = registry.GetIndex(eid2);
	REQUIRE(e2idxPost != e2idxPre);

	Position pPost = registry.Get<Position>(eid);
	REQUIRE(pPost.x == Approx(p.x));
	REQUIRE(pPost.y == Approx(p.y));
	REQUIRE(pPost.z == Approx(p.z));
	REQUIRE(pPost.w == Approx(p.w));
}

TEST_CASE("Entity with no components", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	registry.Remove<Position>(eid);
	bool result = registry.Has<Position>(eid);
	REQUIRE(result == false);
}

TEST_CASE("Delete entity", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	registry.Destroy(eid);
	REQUIRE(!registry.Valid(eid));
}

TEST_CASE("Add - Delete entities", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	ECS::Entity eid1 = registry.Create();
	registry.Insert(eid1, p);
	registry.Destroy(eid1);
	auto query1 = registry.Query<Position>();
	REQUIRE(query1.empty());
	p.x = 10;
	ECS::Entity eid2 = registry.Create();
	registry.Insert(eid2, p);
	REQUIRE(!registry.Valid(eid1));
	REQUIRE(registry.Valid(eid2));
	auto query2 = registry.Query<Position>();
	REQUIRE(query2.size() == 1);
}

TEST_CASE("Multiple deletes", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	ECS::Entity eid1 = registry.Create();
	registry.Insert(eid1, p);
	p.x = 10;
	ECS::Entity eid2 = registry.Create();
	registry.Insert(eid2, p);
	p.x = 20;
	ECS::Entity eid3 = registry.Create();
	registry.Insert(eid3, p);

	// we start by removing the first entity, should get
	// eid3 to move in the archetype
	registry.Destroy(eid2);
	auto query = registry.Query<Position>();
	REQUIRE(query.size() == 1);
	REQUIRE(std::get<0>(query[0]) == 2);
	// e3 got moved in the archetype
	REQUIRE(registry.GetIndex(eid3) == 1);
	Position p3 = registry.Get<Position>(eid3);
	REQUIRE(p3.x == Approx(20));

	// delete now entity1 , so entity3 should be moved again
	registry.Destroy(eid1);
	query = registry.Query<Position>();
	REQUIRE(query.size() == 1);
	REQUIRE(std::get<0>(query[0]) == 1);
	// e3 got moved in the archetype
	REQUIRE(registry.GetIndex(eid3) == 1);
	p3 = registry.Get<Position>(eid3);
	REQUIRE(p3.x == Approx(20));
}

TEST_CASE("Three components creation", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	Health h{ 100 };
	Dummy d{ 10, 20 };
	Position p2{ 4, 5, 6, 7 };
	Health h2{ 900 };
	Dummy d2{ 90, 91 };
	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	registry.Insert(eid, h);
	registry.Insert(eid, d);
	ECS::Entity eid2 = registry.Create();
	registry.Insert(eid2, p2);
	registry.Insert(eid2, h2);
	registry.Insert(eid2, d2);

	auto query = registry.Query<Position, Dummy>();
	REQUIRE(query.size() == 1);
	REQUIRE(std::get<0>(query[0]) == 2);
	// just checking the components
	Position* posPtr = std::get<1>(query[0]);
	REQUIRE(posPtr[0].x == Approx(0));
	REQUIRE(posPtr[1].x == Approx(4));

	Dummy* dummyPtr = std::get<2>(query[0]);
	REQUIRE(dummyPtr[0].a == Approx(10));
	REQUIRE(dummyPtr[1].a == Approx(90));

	auto query2 = registry.Query<Position, Health, Dummy>();
	REQUIRE(query2.size() == 1);
	REQUIRE(std::get<0>(query2[0]) == 2);

	posPtr = std::get<1>(query2[0]);
	REQUIRE(posPtr[0].x == Approx(0));
	REQUIRE(posPtr[1].x == Approx(4));

	Health* hPtr = std::get<2>(query2[0]);
	REQUIRE(hPtr[0].hp == Approx(100));
	REQUIRE(hPtr[1].hp == Approx(900));

	dummyPtr = std::get<3>(query2[0]);
	REQUIRE(dummyPtr[0].a == Approx(10));
	REQUIRE(dummyPtr[1].a == Approx(90));
}

TEST_CASE("add remove component multiple times", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	Health h{ 100 };

	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	for (int i = 0; i < 10; ++i) {
		h.hp = static_cast<float>(i);
		registry.Insert(eid, h);
		registry.Remove<Health>(eid);
	}

	registry.Insert(eid, h);
	Health cmp = registry.Get<Health>(eid);
	REQUIRE(cmp.hp == Approx(9));
}

TEST_CASE("add remove component multiple times single component", "[core,ecs]") {
	ECS::Registry registry;
	Health h{ 100 };

	ECS::Entity eid = registry.Create();
	registry.Insert(eid, h);
	for (int i = 0; i < 10; ++i) {
		registry.Remove<Health>(eid);
		h.hp = static_cast<float>(i);
		registry.Insert(eid, h);
	}

	Health cmp = registry.Get<Health>(eid);
	REQUIRE(cmp.hp == Approx(9));
}

TEST_CASE("add remove component multiple times single component - 2", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	Health h{ 100 };

	ECS::Entity eid = registry.Create();
	registry.Insert(eid, h);
	registry.Insert(eid, p);
	for (int i = 0; i < 10; ++i) {
		registry.Remove<Health>(eid);
		registry.Remove<Position>(eid);
		h.hp = static_cast<float>(i);
		p.x = static_cast<float>(i);
		registry.Insert(eid, h);
		registry.Insert(eid, p);
	}

	Health cmpH = registry.Get<Health>(eid);
	Position cmpP = registry.Get<Position>(eid);
	REQUIRE(cmpH.hp == Approx(9));
	REQUIRE(cmpP.x == Approx(9));
}

TEST_CASE("add remove component multiple times single component - 3", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };
	Health h{ 100 };
	Dummy d{};

	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	registry.Insert(eid, h);
	registry.Insert(eid, d);
	for (int i = 0; i < 10; ++i) {
		registry.Remove<Health>(eid);
		registry.Remove<Position>(eid);
		registry.Remove<Dummy>(eid);
		h.hp = static_cast<float>(i);
		p.x = static_cast<float>(i);
		d.a = i;
		registry.Insert(eid, h);
		registry.Insert(eid, p);
		registry.Insert(eid, d);
	}

	Health cmpH = registry.Get<Health>(eid);
	Position cmpP = registry.Get<Position>(eid);
	Dummy cmpD = registry.Get<Dummy>(eid);
	REQUIRE(cmpH.hp == Approx(9));
	REQUIRE(cmpP.x == Approx(9));
	REQUIRE(cmpD.a == Approx(9));
}

TEST_CASE("delete empty entity", "[core,ecs]") {
	ECS::Registry registry;
	Position p{ 0, 1, 2, 3 };

	ECS::Entity eid = registry.Create();
	registry.Insert(eid, p);
	registry.Remove<Position>(eid);
	registry.Destroy(eid);
}