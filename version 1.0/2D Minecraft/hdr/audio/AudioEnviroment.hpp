#pragma once

// Dependencies
#include "Audio.hpp"

namespace engine {
	namespace audio {
		extern int timeUntilNextMusicTrack; 

		struct SoundEvent {
			enum class EventType { 
				Generic,			Mining,				
				TileBroken,			TilePlacement,		
				ItemDrop,			ItemPickup,			
				Walking,			MobAmbience,		
				MobHurt,			
			};
			enum Id {
				None,				Stone,				
				Dirt,				Gravel, 
				Sand,				Wood, 
				ZombieAmbient,		ZombieHurt,
				ZombieDeath,		Eating, 
				Burp,				ZombieWalk,
				Hurt,				Pop, 
				SkeletonAmbient,	SkeletonHurt,
				SkeletonDeath,		SkeletonWalk,
				CreeperHurt,		CreeperDeath,
				Explode,			ArrowShoot, 
				ArrowHit,			WeakHit, 
				Deepslate,			Cloth, 
				Bamboo,				PigAmbient, 
				PigDeath,			CowAmbient,
				CowHurt,			CowWalk, 
				End
			};

			SoundEffect startSoundEffect,
				endSoundEffect; 

			SoundEvent(); 
			SoundEvent(SoundEffect startSoundEffect); 
			SoundEvent(SoundEffect startSoundEffect, SoundEffect endSoundEffect); 
			~SoundEvent() = default; 

			void playSoundEvent(EventType eventType, float distance = 0.0f); 

			static const int soundEventCooldownDuration; 
			static SoundEvent soundEvents[End];
			static std::vector<sf::Sound*> soundEventsPlaying; 
		};

		void generateTimeUntilNextMusicTrack(); 

		void initAudioEnviroment(); 
		void updateAudioEnviroment(); 
	}
}