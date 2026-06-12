#include "Audio.h"
#include <windows.h>
#include <iostream>
#include <mmsystem.h>
//创建音频管理器时，默认标记「音频未加载」，为后续的初始化加载做准备。
AudioManager::AudioManager() { loaded = false; }
//程序关闭时，自动释放所有音频占用的系统资源，防止音频泄露。
AudioManager::~AudioManager() {
	mciSendString(L"close all", NULL, 0, NULL);
}
// 初始化音频资源，预加载所有必要的音效和 BGM，并为高频音效创建播放池。
void AudioManager::init() {
	if (loaded) return;
	sounds[L"fire"] = L"resource/sound/fire.wav";
	sounds[L"hit"] = L"resource/sound/se_damage00.wav";
	sounds[L"break"] = L"resource/sound/se_enep00.wav"; 
	sounds[L"barrage"] = L"resource/sound/barrage_tan2.wav";
	sounds[L"spell"] = L"resource/sound/spell.wav";
	sounds[L"clear"] = L"resource/sound/se_enep00.wav";
	sounds[L"dead"] = L"resource/sound/se_pldead00.wav";
	sounds[L"pickup"] = L"resource/sound/se_item00.wav"; 
	sounds[L"breakBoss"] = L"resource/sound/break_boss.wav"; 

	pooledSounds = { L"barrage", L"break", L"clear", L"hit", L"pickup" };

	sounds[L"bgm_stage1"] = L"resource/bgm/【東方風神録】～ 神々が恋した幻想郷 ～　.mp3";
	sounds[L"bgm_sanae"] = L"resource/bgm/【東方風神録】～ 信仰は儚き人間の為に ～　.mp3";

	// 预加载 
	for (auto const& [name, path] : sounds) {
		// 检查这个音效是否属于高频音效
		bool isPooled = std::find(pooledSounds.begin(), pooledSounds.end(), name) != pooledSounds.end();

		if (isPooled) {
			// 如果是高频音效，加载 POOL_SIZE 次
			for (int i = 0; i < POOL_SIZE; ++i) {
				std::wstring alias = name + L"_" + std::to_wstring(i);
				std::wstring cmd = L"open \"" + path + L"\" alias " + alias;
				mciSendString(cmd.c_str(), NULL, 0, NULL);
			}
			poolIndex[name] = 0; // 初始化索引
			lastPlayTime[name] = 0;
		}
		else {
			// 普通音效只加载一次
			std::wstring cmd = L"open \"" + path + L"\" alias " + name;
			mciSendString(cmd.c_str(), NULL, 0, NULL);
		}
	}

	loaded = true;
	currentBGM = L"";
}

// 播放指定名称的音效，如果是高频音效则使用循环池进行播放，并且对特定音效进行限流，防止过度触发导致性能问题。
void AudioManager::play(const std::wstring& name) {
	// 安全检查
	if (sounds.find(name) == sounds.end()) return;
	// 音效限流 
	DWORD now = GetTickCount();

	// 针对高频音效进行限流
	if (name == L"barrage" || name == L"break" || name == L"hit") {
		// 设定最小间隔为 40ms，约每秒 25 次
		const int MIN_INTERVAL = 40;
		// 如果距离上次播放同一音效的时间不足最小间隔，直接返回不播放
		if (now - lastPlayTime[name] < MIN_INTERVAL) {
			return; 
		}
		lastPlayTime[name] = now;
	}

	// 正常播放其他音效
	bool isPooled = std::find(pooledSounds.begin(), pooledSounds.end(), name) != pooledSounds.end();
	// 如果是高频音效，使用循环池播放
	if (isPooled) {
		int idx = poolIndex[name];
		std::wstring alias = name + L"_" + std::to_wstring(idx);
		std::wstring cmd = L"play " + alias + L" from 0";
		mciSendString(cmd.c_str(), NULL, 0, NULL);
		poolIndex[name] = (idx + 1) % POOL_SIZE;
	}
	// 普通音效直接播放
	else {
		std::wstring cmd = L"play " + name + L" from 0";
		mciSendString(cmd.c_str(), NULL, 0, NULL);
	}
}

void AudioManager::playBGM(const std::wstring& name) {
	// 如果请求的 BGM 已经在播放，直接返回
	if (currentBGM == name) return;

	// 如果当前有 BGM 在播放，先停止它
	if (!currentBGM.empty()) {
		std::wstring cmdStop = L"stop " + currentBGM;
		mciSendString(cmdStop.c_str(), NULL, 0, NULL);
	}
	// 检查请求的 BGM 是否存在，如果不存在则不播放任何 BGM，并清空当前 BGM 状态。
	if (sounds.find(name) == sounds.end()) {
		currentBGM = L""; 
		return;
	}

	// 播放新的 BGM，设置为循环播放
	std::wstring cmdPlay = L"play " + name + L" repeat";
	mciSendString(cmdPlay.c_str(), NULL, 0, NULL);
	currentBGM = name;
}
// 停止当前正在播放的 BGM，如果没有 BGM 在播放则不执行任何操作。
void AudioManager::stopBGM() {
	if (!currentBGM.empty()) {
		std::wstring cmd = L"stop " + currentBGM;
		mciSendString(cmd.c_str(), NULL, 0, NULL);
		currentBGM = L"";
	}
}