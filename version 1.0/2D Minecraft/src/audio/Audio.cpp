#include "../../hdr/audio/Audio.hpp"
#include "../../hdr/graphics/Render.hpp"

namespace engine {
	namespace audio {
		sf::Music musicTracks[static_cast<int>(MusicTrack::End)];
		sf::SoundBuffer soundBuffers[static_cast<int>(SoundEffect::End)];
		sf::Sound soundEffects[static_cast<int>(SoundEffect::End)];
		int musicTrackSelected = static_cast<int>(MusicTrack::None);
		float musicVolume = 20.0f;
		float soundVolume = 50.0f;

		void adjustMusicVolume(float newVolume) {
			musicVolume = newVolume; 

			for (auto& musicTrack : musicTracks)
				musicTrack.setVolume(musicVolume); 
		}
		void adjustSoundVolume(float newVolume) {
			soundVolume = newVolume; 

			for (auto& soundEffect : soundEffects)
				soundEffect.setVolume(soundVolume); 
		}

		void playMusicTrack(int trackIndex) {
			const int emptyTrackIndex = static_cast<int>(MusicTrack::None); 

			if (musicTrackSelected != emptyTrackIndex)
				musicTracks[musicTrackSelected].stop();
			
			if (trackIndex != emptyTrackIndex) 
				musicTracks[trackIndex].play(); 

			musicTrackSelected = trackIndex; 
		}
		void playMusicTrack(MusicTrack musicTrack) {
			playMusicTrack(static_cast<int>(musicTrack)); 
		}
		void playSoundEffect(int soundIndex, float volumeAmplifier, float pitch) {
			soundEffects[soundIndex].setVolume(soundVolume * volumeAmplifier); 
			soundEffects[soundIndex].setPitch(pitch); 
			soundEffects[soundIndex].play(); 
		}
		void playSoundEffect(SoundEffect soundEffect, float volumeAmplifier, float pitch) {
			playSoundEffect(static_cast<int>(soundEffect), volumeAmplifier, pitch); 
		}
		void pauseMusicTrack(bool pause) {
			if (musicTrackSelected != static_cast<int>(MusicTrack::None)) {
				if (pause)
					musicTracks[musicTrackSelected].pause();
				else if (musicTracks[musicTrackSelected].getStatus() 
						== sf::Music::Paused)
					musicTracks[musicTrackSelected].play(); 
			}
		}

		void loadMusicTracks() {
			for (int musicTrackIndex = 0; musicTrackIndex < 
				static_cast<int>(MusicTrack::End); musicTrackIndex++)
			{
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::MusicTrack, render::assetDirectory
						+ "audio/music/musicTrack" + toString(musicTrackIndex) 
						+ ".ogg", &musicTracks[musicTrackIndex]
				));
			}

			adjustMusicVolume(musicVolume); 
		}
		void loadSoundEffects() {
			for (int soundEffectIndex = 0; soundEffectIndex < 
				static_cast<int>(SoundEffect::End); soundEffectIndex++)
			{
				addResourceLoader(ResourceLoader(
					ResourceLoader::Type::SoundEffect, render::assetDirectory
						+ "audio/sound/soundEffect" + toString(soundEffectIndex)
						+ ".ogg", &soundBuffers[soundEffectIndex], 
					&soundEffects[soundEffectIndex]
				));
			}

			adjustSoundVolume(soundVolume); 
		}
	}
}