#include "Background.h"
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

// 构造函数，初始化滚动位置和默认背景模式
BackgroundManager::BackgroundManager() {
	scrollY = 0.0f;
	// 默认显示开始界面
	currentMode = BGMode::START;
	startMusicPlaying = false;
}
// 初始化背景图片资源，加载普通背景、符卡底层和符卡滚动层的图片，并调整它们的大小以适应游戏窗口。
void BackgroundManager::init() {
	loadimage(&bgNormal, L"resource/background/game.png", WIDTH, HEIGHT);
	loadimage(&bgSpellBase, L"resource/background/cdbg05a.png", WIDTH, HEIGHT);
	loadimage(&bgSpellLayer, L"resource/background/cdbg05b.png", WIDTH, WIDTH);
	loadimage(&bg, L"resource/background/bg.png", screenWidth, screenHeight);

	// 加载开始界面图片（图片a）
    loadimage(&bgStart, L"resource/background/background.png", screenWidth, screenHeight);
	// 加载界面B图片
	loadimage(&bgScreenB, L"resource/background/reuim.png", screenWidth, screenHeight);

	// 如果当前处于开始界面，在 init 时启动开始界面音乐（mp3）
	if (currentMode == BGMode::START && !startMusicPlaying) {
		std::wstring path = L"resource/bgm/旧地獄街道を行く-黄昏フロンティア.mp3";
		std::wstring cmdOpen = L"open \"" + path + L"\" alias start_bgm";
		mciSendString(cmdOpen.c_str(), NULL, 0, NULL);
		mciSendString(L"play start_bgm repeat", NULL, 0, NULL);
		startMusicPlaying = true;
	}

}
// 设置当前背景模式，根据游戏状态切换普通背景和符卡背景。
void BackgroundManager::setMode(BGMode mode) {
    if (currentMode == mode) return;
    // 仅当进入开始界面时，打开或恢复开始界面音乐（音乐a）。
	if (mode == BGMode::START) {
		if (startMusicPlaying) {
			// alias 已经打开过，直接 play 即可（resume）
			mciSendString(L"play start_bgm repeat", NULL, 0, NULL);
		}
		else {
			// 首次进入开始界面，打开并播放
			std::wstring path = L"resource/bgm/旧地獄街道を行く-黄昏フロンティア.mp3";
			std::wstring cmdOpen = L"open \"" + path + L"\" alias start_bgm";
			mciSendString(cmdOpen.c_str(), NULL, 0, NULL);
			mciSendString(L"play start_bgm repeat", NULL, 0, NULL);
			startMusicPlaying = true;
		}
	}
	else {
		// 离开开始界面时，暂停开始界面音乐（但不在 SCREEN_B 恢复），保留 alias 以便后续恢复进入 START
		if (startMusicPlaying) {
			mciSendString(L"pause start_bgm", NULL, 0, NULL);
		}
	}

	currentMode = mode;
}
// 更新背景滚动位置，根据当前背景模式调整滚动速度，并在达到背景图像底部时重置滚动位置，实现循环滚动效果。
void BackgroundManager::update() {
	// 向下滚动
	float scrollSpeed = (currentMode == BGMode::BOSS_SPELL) ? 2.0f : 1.0f; // 符卡阶段可以滚快点
	scrollY += scrollSpeed;

	int currentBGHeight = (currentMode == BGMode::BOSS_SPELL) ? bgSpellLayer.getheight() : HEIGHT;

	if (scrollY >= currentBGHeight) {
		scrollY = 1;
	}
}

// 绘制背景，根据当前背景模式选择绘制普通背景或符卡背景，并使用裁剪区域确保只在游戏窗口内绘制。对于符卡背景，底层使用静态图像，顶层使用循环平铺的滚动图像。
void TileDraw(IMAGE* img, float scrollY, int alpha = 255) {
	int imgW = img->getwidth();
	int imgH = img->getheight();

	for (int x = LeftEdge; x < LeftEdge + WIDTH; x += imgW) {
		for (int y = TopEdge - (imgH - (int)scrollY); y < TopEdge + HEIGHT; y += imgH) {
			putimagePNG(x, y, imgW, imgH, img, 0, 0, imgW, imgH);
		}
	}
}

void BackgroundManager::draw() {
    putimage(0, 0, &bg);

	// 如果是开始界面，绘制开始图并返回
	if (currentMode == BGMode::START) {
		putimage(0, 0, &bgStart);
		return;
	}
	if (currentMode == BGMode::SCREEN_B) {
		putimage(0, 0, &bgScreenB);
		return;
	}

	HRGN rgn = CreateRectRgn(LeftEdge, TopEdge, LeftEdge + WIDTH, TopEdge + HEIGHT);
	setcliprgn(rgn);
	if (currentMode == BGMode::NORMAL) {
		//普通背景滚动逻辑
		int offset = (int)scrollY;
		putimage(LeftEdge, TopEdge + offset, &bgNormal);
		putimage(LeftEdge, TopEdge + offset - HEIGHT, &bgNormal);
	}
	else if (currentMode == BGMode::BOSS_SPELL) {
		// 底层
		putimage(LeftEdge, TopEdge, &bgSpellBase);
		// 顶层
		TileDraw(&bgSpellLayer, scrollY);
	}
	setcliprgn(NULL);
	DeleteObject(rgn);
}