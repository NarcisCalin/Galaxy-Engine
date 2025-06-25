#include "Sound/sound.h"

Sound GESound::intro;
Sound GESound::soundButtonHover1;
Sound GESound::soundButtonHover2;
Sound GESound::soundButtonHover3;
Sound GESound::soundButtonEnable;
Sound GESound::soundButtonDisable;

Sound GESound::soundSliderSlide;

// I make a pool of sounds so they don't cut eachother off if the same sound plays twice in a row too fast
std::vector<Sound> GESound::soundButtonHover1Pool;
std::vector<Sound> GESound::soundButtonHover2Pool;
std::vector<Sound> GESound::soundButtonHover3Pool;
std::vector<Sound> GESound::soundButtonEnablePool;
std::vector<Sound> GESound::soundButtonDisablePool;

std::vector<Sound> GESound::soundSliderSlidePool;

void GESound::loadSounds() {
	InitAudioDevice();

	SetMasterVolume(globalVolume);

	intro = LoadSound("Sounds/MenuSounds/Intro.mp3");

    SetSoundVolume(intro, 0.7f);

	soundButtonHover1 = LoadSound("Sounds/MenuSounds/buttonHover1.mp3");
	soundButtonHover2 = LoadSound("Sounds/MenuSounds/buttonHover2.mp3");
	soundButtonHover3 = LoadSound("Sounds/MenuSounds/buttonHover3.mp3");
	soundButtonEnable = LoadSound("Sounds/MenuSounds/buttonEnable.mp3");
	soundButtonDisable = LoadSound("Sounds/MenuSounds/buttonDisable.mp3");

    soundSliderSlide = LoadSound("Sounds/MenuSounds/sliderSlide.mp3");

	for (size_t i = 0; i < soundPoolSize; i++) {
		soundButtonHover1Pool.push_back(LoadSoundAlias(soundButtonHover1));
	}
	for (size_t i = 0; i < soundPoolSize; i++) {
		soundButtonHover2Pool.push_back(LoadSoundAlias(soundButtonHover2));
	}
	for (size_t i = 0; i < soundPoolSize; i++) {
		soundButtonHover3Pool.push_back(LoadSoundAlias(soundButtonHover3));
	}
	for (size_t i = 0; i < soundPoolSize; i++) {
		soundButtonEnablePool.push_back(LoadSoundAlias(soundButtonEnable));
	}
	for (size_t i = 0; i < soundPoolSize; i++) {
		soundButtonDisablePool.push_back(LoadSoundAlias(soundButtonDisable));
	}

    for (size_t i = 0; i < soundSliderSlidePoolSize; i++) {
        soundSliderSlidePool.push_back(LoadSoundAlias(soundSliderSlide));
    }

	PlaySound(intro);
}

void GESound::soundtrackLogic() {
    SetMasterVolume(globalVolume);
    SetMusicVolume(currentMusic, musicVolume * musicVolMultiplier);

    for (Sound& s : soundButtonHover1Pool) SetSoundVolume(s, menuVolume);
    for (Sound& s : soundButtonHover2Pool) SetSoundVolume(s, menuVolume);
    for (Sound& s : soundButtonHover3Pool) SetSoundVolume(s, menuVolume);
    for (Sound& s : soundButtonEnablePool) SetSoundVolume(s, menuVolume);
    for (Sound& s : soundButtonDisablePool) SetSoundVolume(s, menuVolume);

    for (Sound& s : soundSliderSlidePool) SetSoundVolume(s, menuVolume + 0.15f);

    if (isFirstTimePlaying) {
        currentMusic = LoadMusicStream(playlist[currentSongIndex].c_str());
        if (currentMusic.ctxData != nullptr) {
            PlayMusicStream(currentMusic);
            musicPlaying = true;
            isFirstTimePlaying = false;
        }
    }

    if (hasTrackChanged) {
        UnloadMusicStream(currentMusic);

        if (currentSongIndex < 0) {
            currentSongIndex = playlist.size() - 1;
        }
        else {
            currentSongIndex %= playlist.size();
        }

        currentMusic = LoadMusicStream(playlist[currentSongIndex].c_str());
        if (currentMusic.ctxData != nullptr) {
            PlayMusicStream(currentMusic);
        }
        else {
            musicPlaying = false;
        }
        hasTrackChanged = false;
    }

    else if (musicPlaying) {
        UpdateMusicStream(currentMusic);

        float timePlayed = GetMusicTimePlayed(currentMusic);
        float timeLength = GetMusicTimeLength(currentMusic);

        if (timePlayed >= timeLength - 0.1f) {
            UnloadMusicStream(currentMusic);
            currentSongIndex = (currentSongIndex + 1) % playlist.size();
            currentMusic = LoadMusicStream(playlist[currentSongIndex].c_str());

            if (currentMusic.ctxData != nullptr) {
                PlayMusicStream(currentMusic);
            }
            else {
                musicPlaying = false;
            }
        }
    }
}

void GESound::unloadSounds() {
	UnloadSound(intro);

	UnloadSound(soundButtonHover1);
	UnloadSound(soundButtonHover2);
	UnloadSound(soundButtonHover3);

	UnloadSound(soundButtonEnable);
	UnloadSound(soundButtonDisable);

	UnloadMusicStream(currentMusic);

	CloseAudioDevice();
}