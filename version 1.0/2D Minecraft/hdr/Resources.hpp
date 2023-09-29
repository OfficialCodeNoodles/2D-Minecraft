#pragma once

// Dependencies
#include <vector>
#include <fstream>
#include <filesystem>
#include <memory>
#include <random>
#include <ctime>
#include <time.h>

#include <Glass/glass.hpp>
#include <SFML/Audio.hpp>

#include <Windows.h>
#include <shellapi.h>

// Macros

//#define mDebug
#define mMajorVersion 1
#define mMinorVersion 0

#define toString std::to_string

// Globals

enum class MenuState { 
	LoadingScreen, TitleScreen, WorldSelection, WorldEdit, WorldCreate, 
	ChangeLog, Credits, Settings, InGame 
};
enum class GameMode { Survival, Creative, Spectator };

extern gs::util::State menuState; 
extern GameMode gameMode; 