#pragma once

// Dependencies
#include "Gravity.hpp"
#include "../inventory/LootTable.hpp"
#include "../audio/AudioEnviroment.hpp"

namespace engine {
	class Mob : public Entity {
	public:
		enum Type { Zombie, Skeleton, Creeper, Pig, Cow, End } type;
		enum class Behavior { Passive, Hostile } behavior;
		int health;

		Mob();
		Mob(Mob::Type mobType); 
		~Mob(); 

		void init(); 
		void update() override; 
		void hurt(int damage); 

		std::string getSaveString() const override; 

		static const Model::Id mobModelIds[End]; 
		static const gs::Vec2f mobSizes[End]; 
		static const Behavior mobBehaviors[End]; 
		static const bool mobUndead[End]; 
		static const LootTable::Id mobLootIds[End]; 
		static const float mobMeleeDamage[End]; 
		static const audio::SoundEvent::Id mobWalkingSoundEvents[End]; 
		static const audio::SoundEvent::Id mobAmbientSoundEvents[End]; 
		static const audio::SoundEvent::Id mobHurtSoundEvents[End]; 
		static const audio::SoundEvent::Id mobDeathSoundEvents[End]; 
		static const int hostileMobSpawnCap, passiveMobSpawnCap; 
		static const int hostileMobUnloadDistance; 
		static int numOfHostileMobs, numOfPassiveMobs; 
		static bool burnUndeadHostileMobs; 
	private:
		gs::Vec2f prvsPosition; 
		float prvsDistanceWalked; 
		float mobPlayerDistance; 
		float targetXpos; 
		int jumpAttempts; 
		bool targetingPlayer; 
		int burningTicks;
		int timeSinceLastAmbientSoundEvent; 
		int skeletonShootingCooldown; 
		int creeperDetonationTicks; 
	};
}