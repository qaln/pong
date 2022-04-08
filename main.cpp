#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string>
#include <fstream>

const int SCREEN_WIDTH = 600;
const int SCREEN_HEIGHT = 800;

int musicvolume = 128;
int fxvolume = 128;


SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
Mix_Music* music = NULL;
Mix_Chunk* buttonHover = NULL;
Mix_Chunk* clickSound = NULL;
SDL_Rect infoTab = { 0,0,SCREEN_WIDTH,80 };

void infoTabRender()
{
	SDL_SetRenderDrawColor(renderer, 0x00, 0x90, 0xFF, 0xFF);
	SDL_RenderFillRect(renderer, &infoTab);
}

class wTexture
{
public:
public:
	wTexture()
	{
		texture = NULL;
		width = 0;
		height = 0;
	}

	//Deallocates texture
	void free()
	{
		if (texture != NULL)
		{
			SDL_DestroyTexture(texture);
			texture = NULL;
			width = 0;
			height = 0;
		}
	}
	//Deallocates memory
	~wTexture()
	{
		free();
	}

	//Loads image at specified path
	void loadFromFile(std::string path, bool transparent=false)
	{
		free();
		//Load image at specified path
		SDL_Surface* surface = IMG_Load(path.c_str());
		
		//Set transparent color
		SDL_SetColorKey(surface, transparent, SDL_MapRGB(surface->format, 0xFF, 0x0, 0x0));
		texture = SDL_CreateTextureFromSurface(renderer, surface);
		width = surface->w;
		height = surface->h;
		SDL_FreeSurface(surface);
	}


	void loadFromRenderedText(std::string textureText, SDL_Color textColor, TTF_Font* font)
	{
		free();
		//Render text surface
		SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
		texture = SDL_CreateTextureFromSurface(renderer, textSurface);
		width = textSurface->w;
		height = textSurface->h;
		SDL_FreeSurface(textSurface);
	 }


	//Set color modulation
	//void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	//void setBlendMode(SDL_BlendMode blending);

	//Set alpha modulation
	//void setAlpha(Uint8 alpha);

	//Renders texture at given point
	void render(int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE)
	{
		SDL_Rect renderRect = { x, y, width, height };
		//Set clip rendering dimensions
		if (clip != NULL)
		{
			renderRect.w = clip->w;
			renderRect.h = clip->h;
		}

		SDL_RenderCopyEx(renderer, texture, clip, &renderRect, angle, center, flip);
	}

	//Gets image dimensions
	int getWidth()
	{
		return width;
	}
	int getHeight()
	{
		return height;
	}

private:
	//The actual hardware texture
	SDL_Texture* texture;

	//Image dimensions
	int width;
	int height;
};

void init() 
{
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
	window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
	IMG_Init(IMG_INIT_PNG);
	TTF_Init();
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
	buttonHover = Mix_LoadWAV("sounds/buttonHover.mp3");
	clickSound = Mix_LoadWAV("sounds/click.mp3");
	music = Mix_LoadMUS("sounds/song.mp3");
	if (music == NULL)
	{
		printf("Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError());
	}
}

void close()
{
	//Free loaded images
	//texture.free();



	//Destroy window	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
	Mix_Quit();
}

enum ButtonSprite
{
	BUTTON_SPRITE_MOUSE_OUT = 0,
	BUTTON_SPRITE_MOUSE_OVER_MOTION = 1,
	BUTTON_SPRITE_MOUSE_DOWN = 2,
	BUTTON_SPRITE_TOTAL = 3
};

class Button
{
public:
	Button()
	{
		Position.x = Position.y = 0; BUTTON_WIDTH = BUTTON_HEIGHT = 0;
		CurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
		lastEventWasInside = false;
	}
	void loadText(std::string text, TTF_Font* font)
	{	
		sprites[BUTTON_SPRITE_MOUSE_OUT].loadFromRenderedText(text, { 0x0, 0x0, 0x0 }, font);
		sprites[BUTTON_SPRITE_MOUSE_OVER_MOTION].loadFromRenderedText(text, { 0xFF, 0xFF, 0xFF }, font);
		sprites[BUTTON_SPRITE_MOUSE_DOWN].loadFromRenderedText(text, { 0xFF, 0x0, 0x0 }, font);
		BUTTON_WIDTH = sprites[BUTTON_SPRITE_MOUSE_OUT].getWidth();
		BUTTON_HEIGHT = sprites[BUTTON_SPRITE_MOUSE_OUT].getHeight();
	}
	void setPosition(int x, int y)
	{
		Position.x = x;
		Position.y = y;
	}
	void free()
	{
		for (int i = 0; i < BUTTON_SPRITE_TOTAL; i++)
			sprites[i].~wTexture();
	}
	bool handleEvent(SDL_Event* e)
	{
		bool clicked = false;
		int x, y;
		SDL_GetMouseState(&x, &y);
		bool inside = true;
		if (x < Position.x)
			inside = false;
		else if (x > Position.x + BUTTON_WIDTH)
			inside = false;

		else if (y < Position.y)
			inside = false;
		else if (y > Position.y + BUTTON_HEIGHT)
			inside = false;
		if (!inside) {
			CurrentSprite = BUTTON_SPRITE_MOUSE_OUT;
			lastEventWasInside = false;
		}
		else
		{
			switch (e->type)
			{
			case SDL_MOUSEMOTION:
				CurrentSprite = BUTTON_SPRITE_MOUSE_OVER_MOTION;
				if (!lastEventWasInside)
					Mix_PlayChannel(-1, buttonHover, 0);
				lastEventWasInside = true;

				break;

			case SDL_MOUSEBUTTONDOWN:
				CurrentSprite = BUTTON_SPRITE_MOUSE_DOWN;
				Mix_PlayChannel(-1, clickSound, 0);
				clicked = true;
				break;
			}
			const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
			if (currentKeyStates[SDL_SCANCODE_SPACE])
			{
				CurrentSprite = BUTTON_SPRITE_MOUSE_DOWN;
				Mix_PlayChannel(-1, clickSound, 0);
				clicked = true;
			}

		}
		return clicked;
	}
	void render()
	{
		sprites[CurrentSprite].render(Position.x, Position.y);
	}
	int getWidth(){		
		return BUTTON_WIDTH;
	}
	int getHeight() {
		return BUTTON_HEIGHT;
	}
	

private:
	SDL_Point Position;
	ButtonSprite CurrentSprite;
	wTexture sprites[BUTTON_SPRITE_TOTAL];
	int BUTTON_WIDTH;
	int BUTTON_HEIGHT;
	bool lastEventWasInside;
};

int distanceSquared(int x1, int y1, int x2, int y2)
{
	int deltaX = x2 - x1;
	int deltaY = y2 - y1;
	return deltaX * deltaX + deltaY * deltaY;
}

class Ball
{
public:
	Ball() {
		ballTexture.loadFromFile("sprites/ball.png",true);
		posx = 300; posy = 300;
		speed = 2;	
		radius = ballTexture.getHeight() >> 1;
		vely = speed;
		velx = speed;
	}
	void render()
	{
		ballTexture.render(posx,posy);
	}
	void moveControl(int ticks, SDL_Rect * rect) {
		int prevX = posx, prevY = posy;
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		if (currentKeyStates[SDL_SCANCODE_UP])
		{
			posy -= speed * ticks;
			if (posy < 80)
				posy = 80;
		}
		else if (currentKeyStates[SDL_SCANCODE_DOWN])
		{
			posy += speed * ticks;
			if (posy > SCREEN_HEIGHT - 30 - ballTexture.getHeight())
				posy = SCREEN_HEIGHT - 30 - ballTexture.getHeight();
		}
		else if (currentKeyStates[SDL_SCANCODE_LEFT])
		{
			posx -= speed * ticks;
			if (posx < 0)
				posx = 0;

		}
		else if (currentKeyStates[SDL_SCANCODE_RIGHT])
		{
			posx += speed * ticks;
			if (posx > SCREEN_WIDTH - ballTexture.getWidth())
				posx = SCREEN_WIDTH - ballTexture.getWidth();
		}
		if (isColliding(rect)) {
			posx = prevX;
			posy = prevY;
			velx = -velx;
		}
	}

	int move(int ticks, SDL_Rect * rect) {
		int prevX = posx, prevY = posy;
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		posy += vely * ticks;
		if (posy < 80){ //|| posy > SCREEN_HEIGHT - 30 - ballTexture.getHeight()) {
			//vely = -vely;
			//posy += vely * ticks;
			Mix_PlayChannel(-1, buttonHover, 0);
			return -1;
		}
		if (posy > SCREEN_HEIGHT - 15 - ballTexture.getHeight()) {
			Mix_PlayChannel(-1, clickSound, 0);
			return -1;
		}
		
		posx += velx * ticks;
		if (posx < 0 || posx > SCREEN_WIDTH - ballTexture.getWidth()) {
			velx = -velx;
			posx += velx;
			Mix_PlayChannel(-1, buttonHover, 0);
		}

		int collisionTest = isColliding(rect);
		if (collisionTest>0){			posx = prevX;
			posy = prevY;
			vely = -vely;

			

			if (collisionTest > 1) {
				velx = -velx;
				posx += 2 * velx * ticks;
				posy += 2 * vely * ticks;
			}
			//move(ticks, rect);
			Mix_PlayChannel(-1, buttonHover, 0);
			return 1;
		}
		return 0;
	}

	int isColliding(SDL_Rect* rect)
	{
		//Closest point on collision box
		int centerX=posx+(ballTexture.getWidth()>>1);
		int centerY = posy + (ballTexture.getHeight() >> 1);
		int offsetX;
		int offsetY;
		
		int sideCollision = 0;
		if (centerX < rect->x) {
			offsetX = rect->x;
			sideCollision = 1;
		}
		else if (centerX > rect->x + rect->w)
		{
			offsetX = rect->x + rect->w;
			sideCollision = 1;
		}
		else offsetX = centerX;

		if (centerY < rect->y)
			offsetY = rect->y;
		else if (centerY > rect->y + rect->h)
			offsetY = rect->y + rect->h;
		else offsetY = centerY;

		if (distanceSquared(centerX, centerY, offsetX, offsetY) < radius * radius)
		{
			//This box and the circle have collided

			return 1+sideCollision;
		}
		//system("CLS");
		return 0;
	}

	void setPos(int x, int y)
	{
		posx = x;
		posy = y;
	}
	int getPosx() { return posx; } int getPosy() { return posy; }
	int getVely() { return vely; }
	void resetVely() { vely = speed; }
private:
	wTexture ballTexture;
	int posx, posy;
	int speed;
	int radius;
	int velx, vely;
};

class Player
{
public:
	//SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
	Player(int y)
	{
		Rect = { SCREEN_WIDTH / 4, y, 80, 20 };
	}
	void move(int ticks)
	{
		const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
		if (currentKeyStates[SDL_SCANCODE_W])
		{
			Rect.y -= speed*ticks;
			if (Rect.y < 80)
				Rect.y = 80;
		}
		else if (currentKeyStates[SDL_SCANCODE_S])
		{
			Rect.y += speed*ticks;
			if (Rect.y > SCREEN_HEIGHT-30 - Rect.h)
				Rect.y = SCREEN_HEIGHT-30 - Rect.h;
		}
		else if (currentKeyStates[SDL_SCANCODE_A])
		{
			Rect.x -= speed*ticks;
			if (Rect.x < 0)
				Rect.x = 0;

		}
		else if (currentKeyStates[SDL_SCANCODE_D])
		{
			Rect.x += speed*ticks;
			if (Rect.x > SCREEN_WIDTH - Rect.w)
				Rect.x = SCREEN_WIDTH - Rect.w;
		}
	}

	void moveAI(int ticks, int ballx, int bally, int speedy)
	{
		speed = 3;
		int distanceCoefficient;
		if ((bally > 750 || bally < 80))// || (speedy >0))
			distanceCoefficient = 0;
		else
			distanceCoefficient = (SCREEN_HEIGHT - 50 - bally + 80);
		if (ballx < Rect.x)
		{
			Rect.x -= (speed+distanceCoefficient>>7) * ticks;
			if (Rect.x < 0)
				Rect.x = 0;

		}
		else if (ballx > Rect.x + Rect.w)
		{
			Rect.x += (speed + distanceCoefficient>>7) * ticks;
			if (Rect.x > SCREEN_WIDTH - Rect.w)
				Rect.x = SCREEN_WIDTH - Rect.w;
		}
	}

	void render()
	{
		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
		SDL_RenderFillRect(renderer, &Rect);
	}
	SDL_Rect* getRect() {
		return &Rect;
	}

private:
	SDL_Rect Rect;
	int xVel, yVel;
	int speed = 2;
};

enum Buttons {
	PLAY=0, OPTIONS=1, HIGH_SCORE=2, CREDITS=3, QUIT=4, TOTAL_BUTTONS=5
};

bool updateScore(int score)
{
	std::string line;
	int record[3], i = 0;
	std::ifstream scores;
	scores.open("score/score.txt");
	while (getline(scores, line))
	{
		record[i] = std::stoi(line);
		i++;
	}
	scores.close();
	if (score > record[0]) {
		record[2] = record[1];
		record[1] = record[0];
		record[0] = score;
		std::ofstream scores;
		scores.open("score/score.txt");
		for (i = 0; i < 3; i++)
			scores << record[i] << "\n";
		scores.close();
		return true;
	}
	return false;
}



int main(int argc, char* args[])
{
	init();
	Mix_PlayMusic(music, -1);
	bool quit = false;
	SDL_Event e;
	wTexture mainMenu, scoreText, textHolder;
	
	mainMenu.loadFromFile("sprites/mainMenu.png");
	TTF_Font* font68 = TTF_OpenFont("fonts/pong.ttf", 68), *font40 = TTF_OpenFont("fonts/pong.ttf", 45);
	TTF_Font* infoFont = TTF_OpenFont("fonts/pongv2.ttf", 40);
	TTF_Font* infoFontLarge = TTF_OpenFont("fonts/pongv2.ttf", 80);
	
	SDL_Color textColor = { 0x0, 0x0, 0x0 };
	
	
	//Declaring button array
	Button buttons[TOTAL_BUTTONS], backButton, musicInc, musicDec, fxInc, fxDec, sourceCode;
	//Loading text into buttons
	buttons[PLAY].loadText("Play", font68);
	buttons[OPTIONS].loadText("Options", font40);
	buttons[HIGH_SCORE].loadText("High Score", font40);
	buttons[CREDITS].loadText("Credits", font40);
	buttons[QUIT].loadText("Quit", font40);
	backButton.loadText("BACK",infoFont);
	musicInc.loadText("+", font68);
	musicDec.loadText("-", font68);
	fxInc.loadText("+", font68);
	fxDec.loadText("-", font68);
	sourceCode.loadText("View source code", infoFont);
	sourceCode.setPosition((SCREEN_WIDTH - sourceCode.getWidth()) / 2, 550);

	
	//Setting position for each button
	for (int i=0;i<TOTAL_BUTTONS;++i)
		buttons[i].setPosition((SCREEN_WIDTH - buttons[i].getWidth()) / 2, (i==0)?280:300+60*i);
	backButton.setPosition(20, 20);
	musicInc.setPosition(370, 455);
	musicDec.setPosition(470, 450);
	fxInc.setPosition(370, 375);
	fxDec.setPosition(470, 370);

	Player player(SCREEN_HEIGHT - 50), enemy(110);
	Ball ball;
	int tickDifference, currentFrameTicks, lastFrameTicks;
	bool lost;
	int score;
	bool playerHitBall;
	int ballState;


	while (!quit)
	{
		lost = false;
		ball.setPos(300, 300);
		score = 0;
	
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			
			//User requests quit
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			for (int i = 0; i < TOTAL_BUTTONS; ++i)
				if (buttons[i].handleEvent(&e))
					switch (i)
					{
						//MAIN GAME LOOP
					case PLAY:
						ball.resetVely();
						playerHitBall = false;
						//SDL_Delay(3000);
						lastFrameTicks = SDL_GetTicks();
						while (!lost)
						{

							currentFrameTicks = SDL_GetTicks();
							tickDifference = currentFrameTicks - lastFrameTicks;
							lastFrameTicks = currentFrameTicks;
							if (SDL_PollEvent(&e) != 0)
							{
								if (e.type == SDL_QUIT)
								{
									quit = true;
									break;
								}
								if (backButton.handleEvent(&e))
									lost = true;


								if (e.key.keysym.sym == SDLK_p)
								{
									Mix_PlayChannel(-1, clickSound, 0);
									textHolder.loadFromRenderedText("PAUSED", textColor, infoFontLarge);
									textHolder.render((SCREEN_WIDTH - textHolder.getWidth()) / 2, SCREEN_HEIGHT * 2 / 5);
									SDL_RenderPresent(renderer);
									bool released = false;
									if (e.type == SDL_KEYDOWN) {//|| e.type == SDL_MOUSEBUTTONDOWN) {
										while (e.type != SDL_KEYUP)
										{
											SDL_PollEvent(&e);
										}
										while (1)
										{
											SDL_PollEvent(&e);
											if (e.key.keysym.sym == SDLK_p && e.type == SDL_KEYUP) {
												Mix_PlayChannel(-1, clickSound, 0);
												break;
											}
											if (e.type == SDL_QUIT)
											{
												lost = true;
												quit = true;
												break;
											}
										}

										lastFrameTicks = SDL_GetTicks();
									}

								}

							}
							SDL_SetRenderDrawColor(renderer, 0x0, 0xFF, 0xBF, 0xFF);
							SDL_RenderClear(renderer);

							infoTabRender();
							scoreText.loadFromRenderedText("SCORE:" + std::to_string(score), textColor, infoFont);
							scoreText.render(SCREEN_WIDTH - scoreText.getWidth() - 10, 20);

							backButton.render();


							//Calculating ticks passed to feed into move function

							player.move(tickDifference >> 2);

							player.render();

							enemy.moveAI(tickDifference >> 2, ball.getPosx(), ball.getPosy(), ball.getVely());
							enemy.render();
							if (playerHitBall)
								ballState = ball.move(tickDifference >> 2, enemy.getRect());
							else
								ballState = ball.move(tickDifference >> 2, player.getRect());
							switch (ballState)
							{
							case -1:
								lost = true;
								if (playerHitBall) {
									textHolder.loadFromRenderedText("YOU WIN", textColor, infoFontLarge);
									textHolder.render((SCREEN_WIDTH - textHolder.getWidth()) / 2, SCREEN_HEIGHT * 2 / 5);
									if (updateScore(score))
									{
										textHolder.loadFromRenderedText("NEW HIGH SCORE!", { 0xFF,0,0 }, font40);
										textHolder.render((SCREEN_WIDTH - textHolder.getWidth()) / 2, SCREEN_HEIGHT * 2 / 5 + 125);
									}
									SDL_RenderPresent(renderer);
									while (1) {
										SDL_PollEvent(&e);
										if (e.type == SDL_MOUSEBUTTONDOWN)
										{
											Mix_PlayChannel(-1, clickSound, 0);
											break;
										}
										if (e.type == SDL_QUIT)
										{
											lost = true;
											quit = true;
											break;
										}
									}
								}
								else
								{
									textHolder.loadFromRenderedText("YOU LOSE", textColor, infoFontLarge);
									textHolder.render((SCREEN_WIDTH - textHolder.getWidth()) / 2, SCREEN_HEIGHT * 2 / 5);
									
									
									if (updateScore(score))
									{
										textHolder.loadFromRenderedText("NEW HIGH SCORE!", { 0xFF,0,0 }, font40);
										textHolder.render((SCREEN_WIDTH - textHolder.getWidth()) / 2, SCREEN_HEIGHT * 2 / 5 + 125);
									}
									SDL_RenderPresent(renderer);
									while (1) {
										SDL_PollEvent(&e);
										if (e.type == SDL_MOUSEBUTTONDOWN)
										{
											Mix_PlayChannel(-1, clickSound, 0);
											break;
										}
										if (e.type == SDL_QUIT)
										{
											lost = true;
											quit = true;
											break;
										}
									}
								}

								break;
							case 1:
								if (!playerHitBall)
									score++;
								playerHitBall = !playerHitBall; break;
							}
							ball.render();

							//SDL_RenderFillRect(renderer, &fillRect);
							SDL_RenderPresent(renderer);

						}
						break;

					case OPTIONS:
						
						while (!backButton.handleEvent(&e))
						{
							SDL_PollEvent(&e);
							if (e.type == SDL_QUIT)
							{
								quit = true; break;
							}
							if (fxInc.handleEvent(&e))
							{
								if(fxvolume<128)
									fxvolume += 16;
								Mix_Volume(-1,fxvolume);
							}
							if (fxDec.handleEvent(&e))
							{
								if(fxvolume>0)
									fxvolume -= 16;
								Mix_Volume(-1, fxvolume);
							}
							if (musicInc.handleEvent(&e))
							{
								if (musicvolume < 128)
									musicvolume += 16;
								Mix_VolumeMusic(musicvolume);
							}
							if (musicDec.handleEvent(&e))
							{
								if (musicvolume > 0)
									musicvolume -= 16;
								Mix_VolumeMusic(musicvolume / 2);
							}

							SDL_RenderClear(renderer);
							mainMenu.render(0, 0);
							backButton.render();
							SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
							textHolder.loadFromRenderedText("SFX volume:", textColor, font40);
							textHolder.render((SCREEN_WIDTH - textHolder.getWidth()) / 2 - 100, 300 + 80);
							textHolder.loadFromRenderedText("Music volume:", textColor, font40);
							textHolder.render((SCREEN_WIDTH - textHolder.getWidth()) / 2 - 100, 300 + 160);

							musicInc.render();
							musicDec.render();
							fxInc.render();
							fxDec.render();

							SDL_RenderPresent(renderer);
						}
						break;
					case CREDITS:
					{
						while (!backButton.handleEvent(&e))
						{
							SDL_PollEvent(&e);
							if (e.type == SDL_QUIT)
							{
								quit = true; break;
							}
							if(sourceCode.handleEvent(&e))
								SDL_OpenURL("https://github.com/alexmru/pong");

							SDL_RenderClear(renderer);
							mainMenu.render(0,0);
							backButton.render();
							textHolder.loadFromRenderedText("Programming & Music",textColor,infoFont);
							textHolder.render((SCREEN_WIDTH-textHolder.getWidth())/2, 350);
							textHolder.loadFromRenderedText("Moraru Alexandru",textColor,infoFont);
							textHolder.render((SCREEN_WIDTH - textHolder.getWidth()) / 2, 450);

							sourceCode.render();
							SDL_RenderPresent(renderer);
						}
					}

					case HIGH_SCORE:
					{
						std::string line;
						int record[3], i = 0;
						std::ifstream scores;
						scores.open("score/score.txt");
						while (getline(scores, line))
						{
							record[i] = std::stoi(line);
							i++;
						}
						while (!backButton.handleEvent(&e))
						{
							SDL_PollEvent(&e);
							if (e.type == SDL_QUIT)
							{
								quit = true; break;
							}
							SDL_RenderClear(renderer);
							SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
							mainMenu.render(0, 0);
							backButton.render();
							for (i = 0; i < 3; i++) {
								textHolder.loadFromRenderedText("No. " + std::to_string(i + 1) + ":",textColor , font68);
								textHolder.render((SCREEN_WIDTH - textHolder.getWidth()) / 2-100, 300 + 80 * i);
								textHolder.loadFromRenderedText(std::to_string(record[i]), { 0xFF,0x0,0x0 }, font68);
								textHolder.render((SCREEN_WIDTH - textHolder.getWidth()) / 2+150, 300 + 80 * i);
							}
							SDL_RenderPresent(renderer);
						}
						break;
					}
					case QUIT:
						quit = true;
						SDL_Delay(200);
						break;
			}
			
		}
		//Clear screen


		SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(renderer);

		mainMenu.render(0,0);
		for (int i = 0; i < TOTAL_BUTTONS; ++i)
			buttons[i].render();

		SDL_RenderPresent(renderer);
	}


	return 0;

}