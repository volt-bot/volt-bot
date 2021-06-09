#include <iostream>
#include <complex>
#include <chrono>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_thread.h>

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* uniTex;

SDL_Color white = { 0xff, 0xff, 0xff, 0xff };
SDL_Color black = { 0x00, 0x00, 0x00, 0xff };
SDL_Color yellow = { 0xff, 0xff, 0xff, 0xff };
SDL_Color custom_a = { 0xff, 0xff, 0xff, 0xff };

int WW = 720;
int WH = 480;

struct vi2d { int x; int y; };
struct vd2d { double x; double y; };

int* pFractal = nullptr;

int init();
void updateMandelBrot2(double& iteration);
int genTextTexture(std::string txta, std::string fontNum, float x, float y, unsigned short int aColor, unsigned short int font_sz, unsigned short int fStyle, bool dontPrint);


double minR = -2.0, maxR = 1.0, minI = -1.0, maxI = 1.0;
bool zoom = false;

double mapToReal(int x, int& ww, double& minR, double& maxR)
{
	double range = maxR - minR;
	return x * (range / ww) + minR;
}

double mapToImag(int y, int& wh, double& minI, double& maxI)
{
	double range = maxI - minI;
	return y * (range / wh) + minI;
}

int findMandelBrot(double& cr, double& ci, double& maxIters)
{
	int i = 0;
	double zr = 0.0, zi = 0.0;
	while (i < maxIters && zr * zr + zi * zi < 4.0)
	{
		double temp = zr * zr - zi * zi + cr;
		zi = zr * zi * 2.0 + ci;
		zr = temp;
		i++;
	}
	return i;
}

void updateMandelBrot1(double& maxN)
{
	SDL_SetRenderDrawColor(renderer, 34, 34, 34, 0xff);
	SDL_RenderClear(renderer);

	auto tp1 = std::chrono::high_resolution_clock::now();

	for (int y = 0; y < WH; y++)
	{
		for (int x = 0; x < WW; x++)
		{
			double cr = mapToReal(x, WW, minR, maxR);
			double ci = mapToImag(y, WH, minI, maxI);

			int n = 0;
			double zr = 0.0, zi = 0.0;
			while (n < maxN && zr * zr + zi * zi < 4.0)
			{
				double temp = zr * zr - zi * zi + cr;
				zi = zr * zi * 2.0 + ci;
				zr = temp;
				n++;
			}

			pFractal[y * WW + x] = n;
		}
	}

	for (int y = 0; y < WH; y++)
	{
		for (int x = 0; x < WW; x++) {
			int n = pFractal[y * WW + x];
			float a = 0.1f;
			int r = (0.5f * sin(a * n) + 0.5f) * 255;
			int g = (0.5f * sin(a * n + 2.094f) + 0.5f) * 255;
			int b = (0.5f * sin(a * n + 4.188f) + 0.5f) * 255;
			SDL_SetRenderDrawColor(renderer, r, g, b, 0xff);
			SDL_RenderDrawPoint(renderer, x, y);
		}
	}

	auto tp2 = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsedTime = tp2 - tp1;
	std::string time = "time: " + std::to_string(elapsedTime.count()) + "s";
	if (genTextTexture(time, "-2", 0, 0, 0, 80, 0, true) != -1)
	{
		SDL_Rect dest = { 0,0, (int)(time.size() * 12), 22 };
		SDL_RenderCopy(renderer, uniTex, NULL, &dest);
	}
	std::string iter = "iterations: " + std::to_string((int)maxN);
	if (genTextTexture(iter, "-2", 0, 0, 0, 80, 0, true) != -1)
	{
		SDL_Rect dest = { 0,25, (int)(iter.size() * 10), 22 };
		SDL_RenderCopy(renderer, uniTex, NULL, &dest);
	}
	std::string func = "function: 1";
	if (genTextTexture(func, "-2", 0, 0, 0, 80, 0, true) != -1)
	{
		SDL_Rect dest = { 0,50, (int)(func.size() * 10), 22 };
		SDL_RenderCopy(renderer, uniTex, NULL, &dest);
	}

	SDL_RenderPresent(renderer);
	return;
}


void loop() {

	double maxIter = 64;

	updateMandelBrot1(maxIter);

	unsigned short mode = 1;

	SDL_Event event;
	bool mainDone = false;

	srand(time(NULL));
	Uint32 totalFrameTicks = 0;
	Uint32 totalFrames = 0;
	Uint32 lastUpdate = 0;
	//Uint32  = 0;

	while (!mainDone)
	{
		// Start frame timing
		totalFrames++;
		Uint32 startTicks = SDL_GetTicks();
		Uint64 startPerf = SDL_GetPerformanceCounter();
		int mx = 0, fsx = 0;

		while (SDL_WaitEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				//delete[] pFractal;
				SDL_Quit();
				mainDone = true;
				break;
			case SDL_MOUSEMOTION:
				mx = event.button.x;
			case SDL_KEYDOWN:
				if (mx < WW - (WW / 3)) {
					fsx = (mx * (-2)) / (WW - (WW / 3));
				}
				else {
					//mx-WW - (WW / 3)
					//fsx= mx/(WW/3)
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_E)
				{
					SDL_Quit();
					mainDone = true;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_Z) {
					minR -= 0.5;
					minI -= 0.5;
					maxR += 0.25;
					maxI += 0.5;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_X) {

					minR += 0.5;
					minI += 0.5;
					maxR -= 0.25;
					maxI -= 0.5;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_UP) { maxIter += 64; }
				else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN)
				{
					maxIter -= 64;
					if (maxIter <= 64) maxIter = 64;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_1) { mode = 1; }
				else if (event.key.keysym.scancode == SDL_SCANCODE_2) { mode = 2; }
				else if (event.key.keysym.scancode == SDL_SCANCODE_3) { mode = 3; }
				break;
			default:
				break;
			}

			switch (mode)
			{
			case 1:
				updateMandelBrot1(maxIter);
				break;
			case 2:
				//updateMandelBrot2(maxIter);
				break;
			case 3:
				//updateMandelBrot3(maxIter);
				break;
			default:
				break;
			}
		}
		//loop();
		SDL_Delay(rand() % 25);
		SDL_RenderPresent(renderer);
	}
	return;
}

void drawtext() {
	SDL_SetRenderDrawColor(renderer, 34, 34, 34, 0xff);
	SDL_RenderClear(renderer);
	std::string s = "Aeat my shorts says Gideon";
	genTextTexture(s, "-2", 0, 0, 0, 80, 0, 0);
	//SDL_Rect dest = { 0,50, (int)(s.size() * 15), 22 };
	//SDL_RenderCopy(renderer, uniTex, NULL, &dest);
	SDL_RenderPresent(renderer);

	return;
}

int main(int argc, char** argv)
{
	init();
	loop();
	return 0;
}


int init()
{
	TTF_Init();
	window = SDL_CreateWindow("mandelbrot set", 100, 100, WW, WH, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_SetRenderDrawColor(renderer, 34, 34, 34, 0xff);
	SDL_RenderClear(renderer);

	pFractal = new int[WW * WH]{ 0 };

	return 0;
}

int genTextTexture(std::string txta, std::string fontNum, float x, float y, unsigned short int aColor,
	unsigned short int font_sz,
	unsigned short int fStyle,
	bool dontPrint) {
	TTF_Font* font;
	SDL_Color* forecol;
	SDL_Color* backcol;
	backcol = &black;

	switch (aColor) {
	case 0:
		forecol = &white;
		break;
	case 1:
		forecol = &black;
		break;
	case 2:
		forecol = &yellow;
		break;
	case 3:
		forecol = &custom_a;
		break;
	default:
		forecol = &white;
		SDL_Log("!warning! null text color, text: '%s' set to default 'white' !", txta.c_str());
		break;
	}

	std::string fontFile = ("C:\\Mandelbrot Set\\mandelbrotset\\fonts\\font_" + fontNum + ".ttf");

	int fCount = 0;

	while (!(font = TTF_OpenFont(fontFile.c_str(), font_sz))) {
		SDL_Log("couldn't open font: %s", fontFile.c_str());
		fontFile = "fonts\\font_5.ttf";

		if (fCount > 5)return -1;
		fCount++;
	} //	fontb = TTF_OpenFont("AmaticBold.ttf", fnt_sz);

	switch (fStyle) {
	case 0:
		TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
		break;
	case 1:
		TTF_SetFontStyle(font, TTF_STYLE_BOLD);
		break;
	case 2:
		TTF_SetFontStyle(font, TTF_STYLE_ITALIC);
		break;
	case 3:
		TTF_SetFontStyle(font, TTF_STYLE_UNDERLINE);
		break;
	default:
		TTF_SetFontStyle(font, TTF_STYLE_NORMAL);
		break;
	}


	TTF_SetFontOutline(font, 0); //	TTF_SetFontOutline(fontb, 0);
	TTF_SetFontKerning(font, 1); //	TTF_SetFontKerning(fontb, 1);
	TTF_SetFontHinting(font, TTF_HINTING_MONO); //TTF_SetFontHinting(font, TTF_HINTING_NORMAL);

	SDL_Surface* text = TTF_RenderText_Blended(font, txta.c_str(), *forecol); //SDL_Surface *text = TTF_RenderText_Shaded(font, txta.c_str(), *forecol, *backcol);
	//SDL_Surface* text = TTF_RenderUTF8_Blended_Wrapped(font, txta.c_str(), *forecol, (txta.size() * 18));
	TTF_CloseFont(font);
	SDL_Rect messageRect = { (int)x,(int)y, text->w, text->h };
	SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, text);

	static short rc = 0;
	if (rc == 0) {
		rc++;
	}
	else {
		SDL_DestroyTexture(uniTex);
	}
	uniTex = NULL;
	uniTex = SDL_CreateTextureFromSurface(renderer, text);

	SDL_FreeSurface(text);
	text = NULL;
	if (!dontPrint) {
		SDL_RenderCopy(renderer, message, NULL, &messageRect);
	}
	SDL_DestroyTexture(message);

	return 0;
}
