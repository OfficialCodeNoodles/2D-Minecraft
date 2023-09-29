#include "../../hdr/audio/AudioEnviroment.hpp"
#include "../../hdr/util/Random.hpp"

namespace engine {
	namespace audio {
		int timeUntilNextMusicTrack = 0;

		SoundEvent::SoundEvent() :
			startSoundEffect(SoundEffect::None), 
			endSoundEffect(SoundEffect::None)
		{
		}
		SoundEvent::SoundEvent(SoundEffect startSoundEffect) :
			startSoundEffect(startSoundEffect), 
			endSoundEffect(SoundEffect::None)
		{	
		}
		SoundEvent::SoundEvent(SoundEffect startSoundEffect, SoundEffect endSoundEffect) :
			startSoundEffect(startSoundEffect),
			endSoundEffect(endSoundEffect)
		{
		}

		void SoundEvent::playSoundEvent(EventType eventType, float distance) {
			static Random randomNumberGenerator; 

			const float maxSoundDistance = 24.0f;

			if (distance > maxSoundDistance)
				return; 

			SoundEffect soundEffectSelected = startSoundEffect; 

			if (endSoundEffect != SoundEffect::None) {
				const int soundEffectDelta = static_cast<int>(endSoundEffect)
					- static_cast<int>(startSoundEffect);

				soundEffectSelected = static_cast<SoundEffect>(
					static_cast<int>(startSoundEffect) 
						+ (randomNumberGenerator.generate() % soundEffectDelta)
				);
			}

			float baseVolume = 1.0f; 
			float basePitch = 1.0f; 
			float pitchShift = 0.1f; 

			switch (eventType) {
			case EventType::Mining:
				baseVolume = 0.5f; 
				basePitch = 0.65f; 
				pitchShift = 0.2f; 
				break; 
			case EventType::TileBroken:
				basePitch = 0.9f; 
				pitchShift = 0.2f; 
				break; 
			case EventType::TilePlacement:
				break; 
			case EventType::ItemDrop:
				baseVolume = 0.2f; 
				break; 
			case EventType::ItemPickup:
				baseVolume = 0.2f; 
				basePitch = 2.0f; 
				break; 
			case EventType::Walking:
				baseVolume = 0.2f;  
				break; 
			case EventType::MobAmbience:
				break; 
			case EventType::MobHurt:
				break; 
			}

			const float volumeAmplifier = baseVolume 
				* std::pow(1.15f, -distance);
			const float generatedPitch = basePitch
				+ (generateNormalizedFloat(false) * pitchShift); 

			if (soundEffectSelected != SoundEffect::None) {
				sf::Sound* soundClone = new sf::Sound(
					soundBuffers[static_cast<int>(soundEffectSelected)]
				); 

				soundClone->setVolume(soundVolume * volumeAmplifier);
				soundClone->setPitch(generatedPitch);
				soundClone->play();

				soundEventsPlaying.push_back(soundClone); 
			}
		}
		
		const int SoundEvent::soundEventCooldownDuration = 12; 
		SoundEvent SoundEvent::soundEvents[SoundEvent::End] = {
			{}, 
			{ SoundEffect::Stone1, SoundEffect::Stone4 },
			{ SoundEffect::Dirt1, SoundEffect::Dirt4 },
			{ SoundEffect::Gravel1, SoundEffect::Gravel4 },
			{ SoundEffect::Sand1, SoundEffect::Sand4 },
			{ SoundEffect::Wood1, SoundEffect::Wood4 },
			{ SoundEffect::ZombieAmbient1, SoundEffect::ZombieAmbient3 },
			{ SoundEffect::ZombieHurt1, SoundEffect::ZombieHurt2 },
			{ SoundEffect::ZombieDeath },
			{ SoundEffect::Eat1, SoundEffect::Eat3 },
			{ SoundEffect::Burp },
			{ SoundEffect::ZombieWalk1, SoundEffect::ZombieWalk5 },
			{ SoundEffect::Hurt1, SoundEffect::Hurt3 },
			{ SoundEffect::Pop },
			{ SoundEffect::SkeletonAmbient1, SoundEffect::SkeletonAmbient3 },
			{ SoundEffect::SkeletonHurt1, SoundEffect::SkeletonHurt4 },
			{ SoundEffect::SkeletonDeath },
			{ SoundEffect::SkeletonWalk1, SoundEffect::SkeletonWalk4 },
			{ SoundEffect::CreeperHurt1, SoundEffect::CreeperHurt4 },
			{ SoundEffect::CreeperDeath },
			{ SoundEffect::Explode1, SoundEffect::Explode4 },
			{ SoundEffect::ArrowShoot },
			{ SoundEffect::ArrowHit1, SoundEffect::ArrowHit4 },
			{ SoundEffect::WeakHit1, SoundEffect::WeakHit4 },
			{ SoundEffect::Deepslate1, SoundEffect::Deepslate4 },
			{ SoundEffect::Cloth1, SoundEffect::Cloth4 },
			{ SoundEffect::Bamboo1, SoundEffect::Bamboo6 },
			{ SoundEffect::PigAmbient1, SoundEffect::PigAmbient3 },
			{ SoundEffect::PigDeath },
			{ SoundEffect::CowAmbient1, SoundEffect::CowAmbient4 },
			{ SoundEffect::CowHurt1, SoundEffect::CowHurt3 },
			{ SoundEffect::CowWalk1, SoundEffect::CowWalk4 }
		};
		std::vector<sf::Sound*> SoundEvent::soundEventsPlaying;

		void generateTimeUntilNextMusicTrack() {
			timeUntilNextMusicTrack = 1 + randomGenerator.generate() 
				% (60 * 5); 
		}

		void initAudioEnviroment() {
			generateTimeUntilNextMusicTrack(); 
		}
		void updateAudioEnviroment() {
			for (int soundEventIndex = 0; soundEventIndex <
				SoundEvent::soundEventsPlaying.size(); soundEventIndex++)
			{
				const sf::Sound* soundPlaying = 
					SoundEvent::soundEventsPlaying[soundEventIndex]; 

				if (soundPlaying->getStatus() == sf::Sound::Stopped) {
					SoundEvent::soundEventsPlaying.erase(
						SoundEvent::soundEventsPlaying.begin() 
							+ soundEventIndex
					); 
					delete soundPlaying;
					soundEventIndex--; 
				}
			}

			if (timeUntilNextMusicTrack == 0) {
				playMusicTrack(1 + (randomGenerator.generate() 
					% static_cast<int>(MusicTrack::Hal4))); 
				generateTimeUntilNextMusicTrack(); 
			}

			if (musicTrackSelected == -1 || musicTracks[musicTrackSelected].
					getStatus() == sf::Music::Stopped)
				timeUntilNextMusicTrack = std::max(timeUntilNextMusicTrack - 1, 0); 
		}
	}
}