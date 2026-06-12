#pragma once
#include <graphics.h>
#include "Role.h"

enum class BGMode {
    START,
    SCREEN_B,
	NORMAL,
	BOSS_SPELL
};

class BackgroundManager {
private:
	IMAGE bg;
	IMAGE bgNormal;
	IMAGE bgSpellBase;  
	IMAGE bgSpellLayer; // (滚动层)
    IMAGE bgStart; // 开始界面图片
    IMAGE bgScreenB; // 界面 B 图片
	bool startMusicPlaying;
	BGMode currentMode;

	float scrollY; 

public:
	BackgroundManager();
	void init();
	void update();
	void draw();
	BGMode getMode() const { return currentMode; }
	void setMode(BGMode mode);
};