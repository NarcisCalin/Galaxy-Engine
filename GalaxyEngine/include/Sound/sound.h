#pragma once

struct GESound {

	static Sound intro;

	static Sound soundButtonHover1;
	static Sound soundButtonHover2;
	static Sound soundButtonHover3;
	static Sound soundButtonEnable;
	static Sound soundButtonDisable;

	static Sound soundSliderSlide;

	static std::vector<Sound> soundButtonHover1Pool;
	static std::vector<Sound> soundButtonHover2Pool;
	static std::vector<Sound> soundButtonHover3Pool;
	static std::vector<Sound> soundButtonEnablePool;
	static std::vector<Sound> soundButtonDisablePool;

	static std::vector<Sound> soundSliderSlidePool;

	int soundPoolSize = 30;
	int soundSliderSlidePoolSize = 150;

	Music currentMusic;
	int currentSongIndex = 0;
	bool musicPlaying = false;
	bool isFirstTimePlaying = true;
	bool hasTrackChanged = false;

	float globalVolume = 0.4f;
	float menuVolume = 0.25f;
	float musicVolume = 0.4f;

	float musicVolMultiplier = 1.0f;

	std::vector<std::string> playlist = {
	"Sounds/Soundtrack/Passage of the Stars.wav",
	"Sounds/Soundtrack/Star Born Microbes.wav",
	"Sounds/Soundtrack/Celestial Bodies.wav",
	"Sounds/Soundtrack/Beyond the Solar Alignment.wav",
	"Sounds/Soundtrack/200 Light-Years From Here.wav",
	};


	void loadSounds();

	void soundtrackLogic();

	void unloadSounds();

};