#pragma once

// Dependencies
#include "../util/Loader.hpp"

namespace engine {
	namespace audio {
		enum class MusicTrack { 
			None = -1,				DoorRemix, 
			Hal1,					Hal2, 
			Hal3,					Hal4, 
			End
		};
		enum class SoundEffect { 
			None = -1,				ButtonClick, 
			SwitcherClick,			Stone1, 
			Stone2,					Stone3, 
			Stone4,					Dirt1,
			Dirt2,					Dirt3, 
			Dirt4,					Gravel1, 
			Gravel2,				Gravel3, 
			Gravel4,				Sand1, 
			Sand2,					Sand3, 
			Sand4,					Wood1, 
			Wood2,					Wood3, 
			Wood4,					SmallFall, 
			LargeFall,				ZombieAmbient1, 
			ZombieAmbient2,			ZombieAmbient3, 
			ZombieHurt1,			ZombieHurt2, 
			ZombieDeath,			Eat1, 
			Eat2,					Eat3, 
			Burp,					ZombieWalk1, 
			ZombieWalk2,			ZombieWalk3,
			ZombieWalk4,			ZombieWalk5,
			Hurt1,					Hurt2,
			Hurt3,					Pop, 
			Break,					SkeletonAmbient1,
			SkeletonAmbient2,		SkeletonAmbient3, 
			SkeletonHurt1,			SkeletonHurt2, 
			SkeletonHurt3,			SkeletonHurt4,
			SkeletonDeath,			SkeletonWalk1,
			SkeletonWalk2,			SkeletonWalk3,
			SkeletonWalk4,			CreeperHurt1, 
			CreeperHurt2,			CreeperHurt3,
			CreeperHurt4,			CreeperDeath, 
			Explode1,				Explode2,
			Explode3,				Explode4,
			ArrowShoot,				ArrowHit1,
			ArrowHit2,				ArrowHit3,
			ArrowHit4,				WeakHit1, 
			WeakHit2,				WeakHit3,
			WeakHit4,				Deepslate1,
			Deepslate2,				Deepslate3,
			Deepslate4,				Cloth1, 
			Cloth2,					Cloth3,
			Cloth4,					Bamboo1,
			Bamboo2,				Bamboo3,
			Bamboo4,				Bamboo5, 
			Bamboo6,				PigAmbient1, 
			PigAmbient2,			PigAmbient3, 
			PigDeath,				CowAmbient1,
			CowAmbient2,			CowAmbient3,
			CowAmbient4,			CowHurt1,
			CowHurt2,				CowHurt3,
			CowWalk1,				CowWalk2, 
			CowWalk3,				CowWalk4, 
			End
		};

		extern sf::Music musicTracks[static_cast<int>(MusicTrack::End)]; 
		extern sf::SoundBuffer soundBuffers[static_cast<int>(SoundEffect::End)];
		extern sf::Sound soundEffects[static_cast<int>(SoundEffect::End)];
		extern int musicTrackSelected; 
		extern float musicVolume; 
		extern float soundVolume; 

		void adjustMusicVolume(float newVolume); 
		void adjustSoundVolume(float newVolume); 

		void playMusicTrack(int trackIndex);
		void playMusicTrack(MusicTrack musicTrack);
		void playSoundEffect(
			int soundIndex, float volumeAmplifier = 1.0f, float pitch = 1.0f
		); 
		void playSoundEffect(
			SoundEffect soundEffect, float volumeAmplifier = 1.0f, 
			float pitch = 1.0f
		);
		void pauseMusicTrack(bool pause = true); 

		void loadMusicTracks(); 
		void loadSoundEffects(); 
	}
}