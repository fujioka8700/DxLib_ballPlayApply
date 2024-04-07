//************************************************
// DXライブラリ マウスで玉遊び(応用)
// 作成日：2024/04/04
// 作成者：fujioka8700
// Copyright (c) fujioka8700 All rights reserved.
//************************************************

#include <math.h>
#include <Dxlib.h>

#define WIDTH  640
#define HEIGHT 480
#define BALLPOSX 320
#define BALLPOSY 240

typedef struct {
	float X, Y;
} CRD; // coordinate

typedef struct {
	CRD	  POS;
	CRD   SPEED;
	float RADIUS;
} BALL;

//================================================
// グローバル変数宣言
//================================================
BALL     Ball;
LONGLONG fpsTimer, deltaTimer;
bool     MouseLeftPress, MouseLeftRelease;
CRD      MousePressPos, MouseReleasePos;

CRD      LineStart = { 640, 120 }, LineEnd = { 320, 480 };

//================================================
// FPSの計測と描画
//================================================
void FpsDraw(LONGLONG* p)
{
#ifdef _DEBUG
	static int Fps = 0, FpsCnt = 0;
	LONGLONG now = GetNowHiPerformanceCount();

	FpsCnt++;
	if (now - *p > 1000000)
	{
		Fps = FpsCnt;
		FpsCnt = 0;
		*p = now;
	}
	DrawFormatString(0, 0, GetColor(255, 255, 255), "FPS: %d", Fps);
#endif // !_DEBUG
}

//================================================
// デルタタイムの計測
//================================================
float getDeltaTime(LONGLONG* p)
{
	LONGLONG now = GetNowHiPerformanceCount();
	float    deltaTime = (float)(now - *p) / 1000000.0f;

	*p = now;
	return deltaTime;
}

//================================================
// 初期処理
//================================================
void Init(void)
{
	fpsTimer = deltaTimer = GetNowHiPerformanceCount();

	Ball.POS.X = BALLPOSX, Ball.POS.Y = BALLPOSY;
	Ball.SPEED.X = 0, Ball.SPEED.Y = 0;
	Ball.RADIUS = 20;

	SetDrawScreen(DX_SCREEN_BACK);
}

//================================================
// 終了処理
//================================================
void End(void)
{

}

//================================================
// 入力処理
//================================================
void Input(void)
{
	int button, x, y, type;

	MouseLeftPress = MouseLeftRelease = false;

	if (GetMouseInputLog2(&button, &x, &y, &type, TRUE) == 0)
	{
		if ((button & MOUSE_INPUT_LEFT) != 0)
		{
			if (type == MOUSE_INPUT_LOG_DOWN)
			{
				MouseLeftPress = true;
				MousePressPos.X = (float)x, MousePressPos.Y = (float)y;
			}
			else if (type == MOUSE_INPUT_LOG_UP)
			{
				MouseLeftRelease = true;
				MouseReleasePos.X = (float)x, MouseReleasePos.Y = (float)y;
			}
		}
	}
}

//================================================
// ダブルクリックで、ボールを初期位置に戻す
//================================================
void CheckDoubleClick()
{
	const int DOUBLE_CLICK_INTERVAL = 300;
	static int lastClickTime = 0;
	int now = GetNowCount();

	if (MouseLeftPress && lastClickTime == 0)
	{
		lastClickTime = now;
	}
	else if (MouseLeftPress && now - lastClickTime <= DOUBLE_CLICK_INTERVAL) {
		Ball.POS.X = BALLPOSX, Ball.POS.Y = BALLPOSY;
		Ball.SPEED.X = 0, Ball.SPEED.Y = 0;
		lastClickTime = 0;
	}
	else if (now - lastClickTime > DOUBLE_CLICK_INTERVAL) {
		lastClickTime = 0;
	}
}

//================================================
// 更新処理
//================================================
void Update(void)
{
	float deltaTime = getDeltaTime(&deltaTimer);

	Ball.POS.X += Ball.SPEED.X * deltaTime;
	Ball.POS.Y += Ball.SPEED.Y * deltaTime;
	
	if (Ball.POS.X + Ball.RADIUS >= (float)WIDTH)
	{
		Ball.POS.X = (float)WIDTH - Ball.RADIUS;
		Ball.SPEED.X *= -1;
	}
	else if (Ball.POS.X - Ball.RADIUS < 0.0f) {
		Ball.POS.X = Ball.RADIUS;
		Ball.SPEED.X *= -1;
	}

	if (Ball.POS.Y + Ball.RADIUS >= (float)HEIGHT)
	{
		Ball.POS.Y = (float)HEIGHT - Ball.RADIUS;
		Ball.SPEED.Y *= -1;
	}
	else if (Ball.POS.Y - Ball.RADIUS < 0.0f) {
		Ball.POS.Y = Ball.RADIUS;
		Ball.SPEED.Y *= -1;
	}

	double d = pow(Ball.POS.X - MousePressPos.X, 2) + pow(Ball.POS.Y - MousePressPos.Y, 2);
	static int pressTime = 0;

	if (MouseLeftPress && d <= pow(Ball.RADIUS, 2))
	{
		Ball.SPEED.X = Ball.SPEED.Y = 0;
		pressTime = GetNowCount();
	}
	else if (MouseLeftRelease && pressTime > 0)
	{
		int now = GetNowCount();
		Ball.SPEED.X = (MouseReleasePos.X - MousePressPos.X) * 1000.0f / (now - pressTime);
		Ball.SPEED.Y = (MouseReleasePos.Y - MousePressPos.Y) * 1000.0f / (now - pressTime);

		pressTime = 0;
	}

	CheckDoubleClick();

	float a, b, c, D;
	a = LineEnd.Y - LineStart.Y;
	b = LineStart.X - LineEnd.X;
	c = LineEnd.X * LineStart.Y - LineStart.X * LineEnd.Y;
	D = fabsf(a * Ball.POS.X + b * Ball.POS.Y + c) / sqrtf(powf(a, 2)+ powf(b, 2));
	if (D <= Ball.RADIUS)
	{
		Ball.SPEED.X = Ball.SPEED.Y = 0;
	}
}

//================================================
// 描画処理
//================================================
void Draw(void)
{
	ClearDrawScreen();
	DrawCircleAA(Ball.POS.X, Ball.POS.Y, Ball.RADIUS, 32,GetColor(255, 255, 0), TRUE);

	DrawLineAA(LineStart.X, LineStart.Y, LineEnd.X, LineEnd.Y, GetColor(255, 255, 255));

	FpsDraw(&fpsTimer);
}

//================================================
// 主処理
//================================================
int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd)
{
#ifndef _DEBUG
	SetOutApplicationLogValidFlag(FALSE);
#endif // !_DEBUG

	SetMainWindowText("マウスで玉遊び(基本)");
	SetGraphMode(WIDTH, HEIGHT, 32);
	ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1) return -1;

	Init();

	while (ProcessMessage() == 0)
	{
		Input();
		Update();
		Draw();

		ScreenFlip();
	}

	End();

	DxLib_End();
	return 0;
}