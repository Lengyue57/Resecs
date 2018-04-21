#include <Resecs\Resecs.h>
#include <gtest\gtest.h>
#include "Resecs\Resecs.h"

using namespace Resecs;

class Vector3 {
public:
	float x;
	float y;
	float z;
	bool operator==(const Vector3& ano) const {
		return x == ano.x && y == ano.y && z == ano.z;
	}
};

class PositionComponent {
public:
	PositionComponent()
	{
			
	}
	PositionComponent(float x,float y,float z)
	{
		val.x = x;
		val.y = y;
		val.z = z;
	}
	Vector3 val;
};

class VelocityComponent {
public:
	VelocityComponent()
	{

	}
	VelocityComponent(float x, float y, float z)
	{
		val.x = x;
		val.y = y;
		val.z = z;
	}
	Vector3 val;
};

class FlagComponent{};

TEST(WorldTest, EntityCreationTest) {
	World testWorld; 
	
	ASSERT_TRUE(testWorld.EntityCount() == 1);	//should be a singleton there.
	ASSERT_FALSE(testWorld.CheckEntityAlive(EntityID(1, 0)));
	auto entity = testWorld.Create();
	ASSERT_TRUE(testWorld.CheckEntityAlive(entity.entityID));
	ASSERT_TRUE(testWorld.EntityCount() == 2);
	auto entity2 = testWorld.Create();
	ASSERT_TRUE(testWorld.EntityCount() == 3);
	entity.Destroy();
	entity2.Destroy();
	for (size_t i = 0; i < World::MAX_ENTITY_COUNT - 1; i++)
	{
		auto tempEntity = testWorld.Create();
	}
	ASSERT_TRUE(testWorld.EntityCount() == World::MAX_ENTITY_COUNT);

	ASSERT_ANY_THROW(
		auto tempEntity = testWorld.Create();	//should cause a overflow.
	);
}

TEST(WorldTest, EntityDestroyTest) {
	World world;
	auto entity = world.Create();
	entity.Add(PositionComponent(0, 0, 1));
	ASSERT_TRUE(entity.IsAlive());
	entity.Destroy();
	ASSERT_TRUE(world.EntityCount() == 1);
	ASSERT_FALSE(entity.IsAlive());
	ASSERT_ANY_THROW(
		entity.Get<PositionComponent>();
	);
	ASSERT_ANY_THROW(
		entity.Has<PositionComponent>();
	);
}

TEST(WorldTest, ComponentEventTest) {
	World testWorld;
	ComponentEventArgs testArg;
	auto signal = testWorld.OnComponentChanged.Connect(
		[&](ComponentEventArgs arg) {
		testArg = arg;
	});
	
	auto entity = testWorld.Create();

	entity.Add(PositionComponent(0, 0, 1));
	entity.Add(VelocityComponent(0, 0, 0));
	ASSERT_TRUE(
		testArg.type == ComponentEventType::Added
	);
	ASSERT_TRUE(
		testArg.componentTypeIndex == testWorld.ConvertComponentTypeToIndex<VelocityComponent>()
	);
	ASSERT_TRUE(
		testArg.entity == entity.entityID
	);

	auto entity2 = testWorld.Create();
	entity2.Add(VelocityComponent(0, 0, 0));
	ASSERT_TRUE(
		testArg.type == ComponentEventType::Added
	);
	ASSERT_TRUE(
		testArg.componentTypeIndex == testWorld.ConvertComponentTypeToIndex<VelocityComponent>()
	);
	ASSERT_TRUE(
		testArg.entity == entity2.entityID
	);

	entity.Remove<PositionComponent>();
	ASSERT_TRUE(testArg.type == ComponentEventType::Removed);
	ASSERT_TRUE(
		testArg.componentTypeIndex == testWorld.ConvertComponentTypeToIndex<PositionComponent>()
	);
	ASSERT_TRUE(
		testArg.entity == entity.entityID
	);
}

TEST(ComponentTest, AddComponentTest) {
	World testWorld;
	auto entity = testWorld.Create();
	ASSERT_FALSE(entity.Has<PositionComponent>());
	entity.Add(PositionComponent(0, 0, 1));
	ASSERT_TRUE(entity.Has<PositionComponent>());
	ASSERT_TRUE(entity.Get<PositionComponent>()->val == PositionComponent(0, 0, 1).val);

	for (size_t i = 0; i < 5; i++)
	{
		auto ent = testWorld.Create();
		ent.Add(PositionComponent(i, 0, 0));
		ent.Add(VelocityComponent(0, 1, 0));
		if (i != 3) {
			ent.Add(FlagComponent());
		}
	}
}

TEST(ComponentTest, RemoveComponentTest) {
	World testWorld;
	auto entity = testWorld.Create();
	entity.Add(PositionComponent(0, 0, 1));

	entity.Remove<PositionComponent>();
	ASSERT_FALSE(entity.Has<PositionComponent>());
	entity.Add(PositionComponent(0, 0, 1));
	entity.Destroy();
}

TEST(ComponentTest, EditComponentTest) {
	World testWorld;
	auto entity = testWorld.Create();
	ComponentEventArgs temp;
	auto connection = testWorld.OnComponentChanged.Connect(
		[&](ComponentEventArgs arg) {
		temp = arg;
	}
	);
	entity.Add(PositionComponent(0, 0, 1));
	ASSERT_TRUE(
		temp.type == ComponentEventType::Added
	);
	ASSERT_TRUE(temp.entity == entity.entityID);
	ASSERT_TRUE(temp.componentTypeIndex == 0);

	entity.Remove<PositionComponent>();
	ASSERT_TRUE(
		temp.type == ComponentEventType::Removed
	);
	ASSERT_TRUE(temp.entity == entity.entityID);
	ASSERT_TRUE(temp.componentTypeIndex == 0);

	entity.Add(VelocityComponent(0, 0, 1));
	ASSERT_TRUE(
		temp.type == ComponentEventType::Added
	);
	ASSERT_TRUE(temp.entity == entity.entityID);
	ASSERT_TRUE(temp.componentTypeIndex == 1);

	entity.Remove<VelocityComponent>();
	ASSERT_TRUE(
		temp.type == ComponentEventType::Removed
	);
	ASSERT_TRUE(temp.entity == entity.entityID);
	ASSERT_TRUE(temp.componentTypeIndex == 1);
}

TEST(ComponentTest, GetComponentTest) {
	World testWorld;
	auto entity = testWorld.Create();
	ASSERT_TRUE(entity.Get<PositionComponent>() == nullptr);
	auto ptrAdd = entity.Add(PositionComponent(0, 0, 5));
	ASSERT_TRUE(entity.Get<PositionComponent>() == ptrAdd);
	ptrAdd->val.x = 1;
	ASSERT_TRUE(entity.Get<PositionComponent>()->val == PositionComponent(1, 0, 5).val);
}

class SgComponent : public Component, public ISingletonComponent
{
public:
	SgComponent()
	{

	}
	SgComponent(int val) : val(val)
	{

	}
	int val;
};
TEST(SingletonTest, 1) {
	World testWorld;
	auto entity = testWorld.Create();
	ASSERT_TRUE(entity.entityID.index == 1);	//index 0 should be occupied by singleton.


	auto temp = testWorld.Add(SgComponent(1));
	ASSERT_ANY_THROW(
		testWorld.Add(SgComponent(1));
	);
	
	ASSERT_TRUE(testWorld.Get<SgComponent>() == temp);

	testWorld.Remove<SgComponent>();
	ASSERT_TRUE(!testWorld.Has<SgComponent>());

	//Prevent adding singletons to normal entity.
	ASSERT_ANY_THROW(
		entity.Add<SgComponent>();
	);
	ASSERT_ANY_THROW(
		entity.Remove<SgComponent>();
	); 
	ASSERT_ANY_THROW(
		entity.Replace<SgComponent>(SgComponent(1));
	);
}

TEST(GroupTest, EntityCapture) {
	World testWorld;
	auto og = Group::CreateGroup<PositionComponent,VelocityComponent>(&testWorld);
	auto g = og;
	ASSERT_TRUE(g.Count() == 0);

	std::vector<Entity> testEntities;
	for (size_t i = 0; i < 10; i++)
	{
		auto t = testWorld.Create();
		t.Add<PositionComponent>();
		t.Add<VelocityComponent>();
		testEntities.push_back(t);
	}
	ASSERT_TRUE(g.Count() == 10);
	for (auto& t : g) {
		bool flag = false;
		for (size_t i = 0; i < 10; i++)		//It's not guaranteed that group holds all entities in "order"
		{
			if (t.entityID == testEntities[i].entityID)
				flag = true;
		}
		ASSERT_TRUE(flag);
	}

	//Test capture entities created before group.
	auto group2 = Group::CreateGroup<PositionComponent>(&testWorld);
	ASSERT_TRUE(group2.Count() == 10);

	
	testEntities[0].Remove<VelocityComponent>();
	ASSERT_TRUE(g.Count() == 9);
	ASSERT_TRUE(group2.Count() == 10);
}