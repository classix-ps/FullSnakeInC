/**
  * CSFML SETUP:
  * 1. Go to project properties and set active configuration to 'All Configurations'
  * 2. Under C/C++ >> General, set Additional Include Directories to CSFML/include
  * 3. Under Linker >> General, set Additional Include Libraries to CSFML/lib/msvc
  * 4. Under Linker >> Input, set Additional dependencies to csfml-graphics.lib;csfml-window.lib;csfml-system.lib
 */

#include <SFML/Graphics.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

int N = 30, M = 20;
float size = 48.0;
int w = 1440;
int h = 960;

int dir, num;

int b = 0;
int skin = 0;
int delay = 0;

int offset = 0;

struct Snake
{
	int x, y;
}  s[600];

struct Fruit
{
	int x, y;
} f;

struct Save
{
	int highScores[4][10]; // classic, classicHardcore, lava, lavaHardcore
	int completedMaps[2]; // 1 for normal, 2 for hardcore, 3 for both (if all maps on 3, offer easter egg snake skin)
} save;
void loadSaveData() {
	FILE* high = fopen("../save/highScores.txt", "r");
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 10; j++) {
			fscanf(high, "%d", &save.highScores[i][j]);
		}
	}

	FILE* maps = fopen("../save/completedMaps.txt", "r");
	for (int i = 0; i < 2; i++) {
		fscanf(maps, "%d", &save.completedMaps[i]);
	}
}
void updateSaveData(int highScore, int map, int difficulty) {
	for (int i = 0; i < 10; i++) {
		if (highScore > save.highScores[2 * map + difficulty - 1][i]) {
			// change value at position in file
		}
	}
	if (highScore > 20) {
		if (save.completedMaps[map] != difficulty && save.completedMaps[map] != 3) {
			save.completedMaps[map] += difficulty;
		}
	}
}

struct Danger
{
	int x, y;
} d[156];
void setDanger() {
	int i = 0;

	if (!b) {
		for (i; i < 156; i++) {
			d[i].x = -1; d[i].y = -1;
		}
	}
	else if (b == 1) {
		for (int x = 4; x < 26; x++) {
			for (int y = 1; y < 19; y++) {
				if ((x < 8 || x > 21) && ((y > 3 && y < 8) || (y > 11 && y < 16))) {
					if ((x == 7 || x == 22) && (y == 7 || y == 12)) continue;
					d[i].x = x; d[i].y = y; i++;
				}
				else if (((x >= 8 && x < 13) || (x > 16 && x <= 21)) && (y < 7 || y > 12)) {
					if ((x > 10 && x < 19) && (y > 3 && y < 16)) continue;
					d[i].x = x; d[i].y = y; i++;
				}
			}
		}
	}
	else if (b == 2) {
		for (int y = 8; y < 12; y++) {
			for (int x = 0; x < N; x++) {
				if ((x > 4 && x < 7) || (x > 22 && x < 25)) continue;
				d[i].x = x; d[i].y = y; i++;
			}
		}
		while (i < 156) {
			d[i].x = -1; d[i].y = -1; i++;
		}
	}
	else if (b == 3) {
		for (i; i < 156; i++) {
			d[i].x = -1; d[i].y = -1;
		}
	}
}
bool onDanger(int x, int y) {
	for (int i = 0; i < 156; i++) {
		if (d[i].x == x && d[i].y == y) return true;
	}
	return false;
}

/* Resets game values */
void resetStats() {
	s[0].x = 0;
	s[0].y = 0;
	f.x = 15;
	f.y = 7;
	dir = 0;
	num = 1;
}

/* Enter to confirm, any key to cancel */
bool confirm(sfRenderWindow* window) {
	sfEvent e;

	while (1) {
		while (sfRenderWindow_pollEvent(window, &e)) {
			if (e.type == sfEvtClosed) sfRenderWindow_close(window);

			else if (e.type == sfEvtKeyPressed) {
				if (sfKeyboard_isKeyPressed(sfKeyEnter)) return true;
				else return false;
			}
		}
	}
}

/* Draws Window */
void loadBoard(sfRenderWindow* window, sfSprite* board, sfRectangleShape* square, sfSprite* snake[16], sfText* length) {
	sfRenderWindow_clear(window, sfBlack);
	sfVector2f pos;

	////// board //////
	if (board != NULL) {
		sfRenderWindow_drawSprite(window, board, NULL);
	}

	////// snake //////
	pos.x = s[0].x * size; pos.y = s[0].y * size; // head
	sfSprite_setPosition(snake[dir], pos);
	sfRenderWindow_drawSprite(window, snake[dir], NULL);

	offset = (offset + 1) % 2;
	int snakeMapped[600];
	int currentDir, lastDir = dir;
	for (int i = 1; i < num; i++) {
		if (s[i].x > s[i - 1].x && s[i].y == s[i - 1].y) currentDir = 1; // right
		else if (s[i].x < s[i - 1].x && s[i].y == s[i - 1].y) currentDir = 2; // left
		else if (s[i].x == s[i - 1].x && s[i].y > s[i - 1].y) currentDir = 3; // up
		else if (s[i].x == s[i - 1].x && s[i].y < s[i - 1].y) currentDir = 0; // down

		if (i < num - 1) snakeMapped[i] = 4 + (currentDir == 1 || currentDir == 2) * 2 + (i % 2 + offset) % 2; // 4-5 for vertical, 6-7 for horizontal
		else snakeMapped[i] = 12 + currentDir;

		if (currentDir != lastDir) {
			if ((lastDir == 3 && currentDir == 1) || (lastDir == 2 && currentDir == 0)) snakeMapped[i - 1] = 11;
			else if ((lastDir == 3 && currentDir == 2) || (lastDir == 1 && currentDir == 0)) snakeMapped[i - 1] = 10;
			else if ((lastDir == 0 && currentDir == 2) || (lastDir == 1 && currentDir == 3)) snakeMapped[i - 1] = 9;
			else if ((lastDir == 0 && currentDir == 1) || (lastDir == 2 && currentDir == 3)) snakeMapped[i - 1] = 8;
		}

		lastDir = currentDir;
	}
	for (int i = 1; i < num; i++) { // body
		pos.x = s[i].x * size; pos.y = s[i].y * size;
		sfSprite_setPosition(snake[snakeMapped[i]], pos);
		sfRenderWindow_drawSprite(window, snake[snakeMapped[i]], NULL);
	}

	////// food //////
	sfRectangleShape_setFillColor(square, sfGreen);
	pos.x = f.x * size; pos.y = f.y * size;
	sfRectangleShape_setPosition(square, pos); // food
	sfRenderWindow_drawRectangleShape(window, square, NULL);

	////// length count //////
	pos.x = 1420.0f - 20.0f * floorf(log10f((float)num)); pos.y = 0.0;
	char numStr[4]; snprintf(numStr, 4, "%d", num); sfText_setString(length, numStr); sfText_setPosition(length, pos); // length
	sfRenderWindow_drawText(window, length, NULL);

	sfRenderWindow_display(window);
}

void drawSettingsText(sfRenderWindow* window, sfText* settings, sfText* types[3], sfText* bTexts[2], sfText* sTexts[2], sfText* dTexts[2], int selects[3], int select) {
	sfRenderWindow_clear(window, sfBlack);

	sfRenderWindow_drawText(window, settings, NULL);

	for (int i = 0; i < 3; i++) {
		if (i == select) sfText_setStyle(types[i], sfTextUnderlined);
		else sfText_setStyle(types[i], sfTextRegular);

		sfRenderWindow_drawText(window, types[i], NULL);
	}
	sfRenderWindow_drawText(window, bTexts[selects[0]], NULL);
	sfRenderWindow_drawText(window, sTexts[selects[1]], NULL);
	sfRenderWindow_drawText(window, dTexts[selects[2]], NULL);
	
	sfRenderWindow_display(window);
}
void loadSettings(sfRenderWindow* window, sfSprite* boards[4], sfSprite* snakes[4]) {
	sfVector2f pos;
	sfFont* game = sfFont_createFromFile("../fonts/arcade.ttf");
	unsigned int textSize = 70;

	sfText* settings = sfText_create();
	sfText_setFont(settings, game);
	sfText_setCharacterSize(settings, 200);
	pos.x = 375.0; pos.y = 40.0; sfText_setPosition(settings, pos);
	sfText_setString(settings, "Settings");

	sfText* types[3];
	for (int i = 0; i < 3; i++) {
		types[i] = sfText_create();
		sfText_setFont(types[i], game);
		sfText_setCharacterSize(types[i], textSize);
		pos.x = 100.0; pos.y = 300.0f + 240.0f * i; sfText_setPosition(types[i], pos);
	}
	sfText_setString(types[0], "Map");
	sfText_setString(types[1], "Snake");
	sfText_setString(types[2], "Mode");

	sfText* boardTexts[4]; pos.x = 400.0; pos.y = 300.0;
	for (int i = 0; i < 4; i++) {
		boardTexts[i] = sfText_create();
		sfText_setFont(boardTexts[i], game);
		sfText_setCharacterSize(boardTexts[i], textSize);
		sfText_setPosition(boardTexts[i], pos);
	}
	sfText_setString(boardTexts[0], "< Classic >");
	sfText_setString(boardTexts[1], "<  Lava   >");
	sfText_setString(boardTexts[2], "<  Water  >");
	sfText_setString(boardTexts[3], "<  Terra  >");

	sfText* snakeTexts[4]; pos.x = 400.0; pos.y = 540.0;
	for (int i = 0; i < 4; i++) {
		snakeTexts[i] = sfText_create();
		sfText_setFont(snakeTexts[i], game);
		sfText_setCharacterSize(snakeTexts[i], textSize);
        sfText_setPosition(snakeTexts[i], pos);
	}
	sfText_setString(snakeTexts[0], "< Classic >");
	sfText_setString(snakeTexts[1], "<  Lava   >"); if (save.completedMaps[1] < 1) sfText_setFillColor(snakeTexts[1], sfRed);
	sfText_setString(snakeTexts[2], "<  Water  >"); if (save.completedMaps[1] < 1) sfText_setFillColor(snakeTexts[2], sfRed);
	sfText_setString(snakeTexts[3], "<  Terra  >"); if (save.completedMaps[1] < 1) sfText_setFillColor(snakeTexts[3], sfRed);

	sfText* difficultyTexts[2]; pos.x = 400.0; pos.y = 780.0;
	for (int i = 0; i < 2; i++) {
		difficultyTexts[i] = sfText_create();
		sfText_setFont(difficultyTexts[i], game);
		sfText_setCharacterSize(difficultyTexts[i], textSize);
		sfText_setPosition(difficultyTexts[i], pos);
	}
	sfText_setString(difficultyTexts[0], "<  Normal  >");
	sfText_setString(difficultyTexts[1], "< Hardcore >");

	int selects[3] = { b, skin, delay };
	int select = 0;

	sfEvent e;
	while (1) {
		drawSettingsText(window, settings, types, boardTexts, snakeTexts, difficultyTexts, selects, select);

		while (sfRenderWindow_pollEvent(window, &e)) {
			if (e.type == sfEvtClosed) sfRenderWindow_close(window);

			else if (e.type == sfEvtKeyPressed) {
				if (sfKeyboard_isKeyPressed(sfKeyS)) { // down
					select = (select + 1) % 3;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyW)) { // up
					select = ((select - 1) % 3 + 3) % 3;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyD)) { // right
					selects[select] = (selects[select] + 1) % 4;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyA)) { // left
					selects[select] = ((selects[select] - 1) % 4 + 4) % 4;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyTab) || sfKeyboard_isKeyPressed(sfKeyEnter)) { // back
					sfText_destroy(settings); for (int i = 0; i < 3; i++) sfText_destroy(types[i]); for (int i = 0; i < 4; i++) { sfText_destroy(boardTexts[i]); sfText_destroy(snakeTexts[i]); } for (int i = 0; i < 2; i++) sfText_destroy(difficultyTexts[i]);

					b = selects[0]; skin = selects[1]; delay = selects[2];
					setDanger();
					return;
				}
			}
		}
	}
}

void drawScaledSprites(sfRenderWindow* window, int select, sfSprite* sprites[3], sfSprite* borders[3], sfVector2f scaleSize, sfVector2f noScaleSize, sfVector2f scalePositions[3], sfVector2f noScalePositions[3]) {
	for (int i = 0; i < 3; i++) {
		if (i == select) {
			sfSprite_setScale(sprites[i], scaleSize);
			sfSprite_setPosition(sprites[i], scalePositions[i]);
			sfSprite_setPosition(borders[i], scalePositions[i]);
		}
		else {
			sfSprite_setScale(sprites[i], noScaleSize);
			sfSprite_setPosition(sprites[i], noScalePositions[i]);
		}
		sfRenderWindow_drawSprite(window, sprites[i], NULL);
	}
	sfRenderWindow_drawSprite(window, borders[select], NULL);

	sfRenderWindow_display(window);
}

/* START SCREEN */
void drawStartSprites(sfRenderWindow* window, sfSprite* background, sfSprite* sprites[3], int select) {
	sfRenderWindow_clear(window, sfBlack);
	sfRenderWindow_drawSprite(window, background, NULL);

	sfVector2f noScaleSize; noScaleSize.x = 7.0; noScaleSize.y = 7.0;
	sfVector2f scaleSize; scaleSize.x = 8.0; scaleSize.y = 8.0;

	sfTexture* border3Tx = sfTexture_createFromFile("../images/underline3.png", NULL);
	sfSprite* border3 = sfSprite_create(); sfSprite_setTexture(border3, border3Tx, sfFalse);
	sfSprite_setScale(border3, scaleSize);

	sfTexture* border2Tx = sfTexture_createFromFile("../images/underline2.png", NULL);
	sfSprite* border2 = sfSprite_create(); sfSprite_setTexture(border2, border2Tx, sfFalse);
	sfSprite_setScale(border2, scaleSize);

	sfTexture* border1Tx = sfTexture_createFromFile("../images/underline1.png", NULL);
	sfSprite* border1 = sfSprite_create(); sfSprite_setTexture(border1, border1Tx, sfFalse);
	sfSprite_setScale(border1, scaleSize);

	sfSprite* borders[3] = { border1, border2, border3 };

	sfVector2f pos;
	sfVector2f noScalePositions[3];
	sfVector2f scalePositions[3];
	for (int i = 0; i < 3; i++) {
		pos.x = (i == 1) * 350.0f + (i == 2) * 820.0f; pos.y = 720.0;
		noScalePositions[i] = pos;
		pos.x -= 35.0f; pos.y /= 1.01f;
		scalePositions[i] = pos;
	}
	drawScaledSprites(window, select, sprites, borders, scaleSize, noScaleSize, scalePositions, noScalePositions);

	sfTexture_destroy(border3Tx); sfTexture_destroy(border2Tx); sfTexture_destroy(border1Tx); sfSprite_destroy(border3); sfSprite_destroy(border2); sfSprite_destroy(border1);
}
void loadStartScreen(sfRenderWindow* window, sfText* startText, sfSprite* boards[4], sfSprite* snakes[4]) {
	sfTexture* snakeTx = sfTexture_createFromFile("../images/snake.jpeg", NULL);
	sfSprite* snake = sfSprite_create(); sfSprite_setTexture(snake, snakeTx, sfFalse);

	sfTexture* startTx = sfTexture_createFromFile("../images/start.png", NULL);
	sfSprite* start = sfSprite_create(); sfSprite_setTexture(start, startTx, sfFalse);

	sfTexture* settingsTx = sfTexture_createFromFile("../images/settings.png", NULL);
	sfSprite* settings = sfSprite_create(); sfSprite_setTexture(settings, settingsTx, sfFalse);

	sfTexture* highScoresTx = sfTexture_createFromFile("../images/highscores.png", NULL);
	sfSprite* highScores = sfSprite_create(); sfSprite_setTexture(highScores, highScoresTx, sfFalse);

	sfSprite* startScreenSprites[3] = { start, settings, highScores };

	sfEvent e;
	sfClock* clock = sfClock_create();
	while (startText != NULL) {
		sfRenderWindow_clear(window, sfBlack);
		sfRenderWindow_drawSprite(window, snake, NULL);
		if (start != NULL) {
			sfTime t = sfClock_getElapsedTime(clock);
			if (!((int)sfTime_asSeconds(t) % 2)) sfRenderWindow_drawText(window, startText, NULL);
		}
		sfRenderWindow_display(window);

		while (sfRenderWindow_pollEvent(window, &e)) {
			if (e.type == sfEvtClosed) sfRenderWindow_close(window);

			else if (e.type == sfEvtKeyPressed) {
				if (sfKeyboard_isKeyPressed(sfKeyEnter)) {
					sfClock_destroy(clock);
					startText = NULL;
				}
			}
		}
	}

	int select = 0;
	while (1) {
		drawStartSprites(window, snake, startScreenSprites, select);

		while (sfRenderWindow_pollEvent(window, &e)) {
			if (e.type == sfEvtClosed) sfRenderWindow_close(window);

			else if (e.type == sfEvtKeyPressed) {
				if (sfKeyboard_isKeyPressed(sfKeyD)) { // right
					select = (select + 1) % 3;
					drawStartSprites(window, snake, startScreenSprites, select);
				}
				else if (sfKeyboard_isKeyPressed(sfKeyA)) { // left
					select = ((select - 1) % 3 + 3) % 3;
					drawStartSprites(window, snake, startScreenSprites, select);
				}
				else if (sfKeyboard_isKeyPressed(sfKeyEnter)) { // confirm
					switch (select) {
					case 0: sfTexture_destroy(snakeTx); sfTexture_destroy(startTx); sfTexture_destroy(settingsTx); sfTexture_destroy(highScoresTx); sfSprite_destroy(snake); sfSprite_destroy(start); sfSprite_destroy(settings); sfSprite_destroy(highScores); return; // start
					case 1: loadSettings(window, boards, snakes); break; // settings
					case 2: break; // highscores
					}
				}
			}
		}
	}
}

/* END MENU */
void drawEndSprites(sfRenderWindow* window, sfSprite* background, sfSprite* sprites[3], int select) {
	sfRenderWindow_clear(window, sfBlack);
	sfRenderWindow_drawSprite(window, background, NULL);

	sfVector2f noScaleSize; noScaleSize.x = 10.0; noScaleSize.y = 10.0;
	sfVector2f scaleSize; scaleSize.x = 15.0; scaleSize.y = 15.0;

	sfTexture* borderTx = sfTexture_createFromFile("../images/border.png", NULL);
	sfSprite* border = sfSprite_create(); sfSprite_setTexture(border, borderTx, sfFalse);
	sfSprite_setScale(border, scaleSize);

	sfSprite* borders[3] = { border, border, border };

	sfVector2f pos;
	sfVector2f noScalePositions[3];
	sfVector2f scalePositions[3];
	for (int i = 0; i < 3; i++) {
		pos.x = 385.0; pos.y = 500.0f + i * 100.0f;
		noScalePositions[i] = pos;
		pos.x = (-720.0f + 3.0f * pos.x) / 2.0f; pos.y /= 1.04f;
		scalePositions[i] = pos;
	}
	drawScaledSprites(window, select, sprites, borders, scaleSize, noScaleSize, scalePositions, noScalePositions);

	sfTexture_destroy(borderTx); sfSprite_destroy(border);
}
void loadEndScreen(sfRenderWindow* window, sfSprite* boards[4], sfSprite* snakes[4]) {
	sfTexture* gameOverTx = sfTexture_createFromFile("../images/gameover.png", NULL);
	sfSprite* gameOver = sfSprite_create(); sfSprite_setTexture(gameOver, gameOverTx, sfFalse);
	sfVector2f pos; pos.x = 285.0; pos.y = 50.0; sfSprite_setPosition(gameOver, pos);
	sfVector2f scale; scale.x = 15.0; scale.y = 15.0; sfSprite_setScale(gameOver, scale);

	sfTexture* retryTx = sfTexture_createFromFile("../images/retry.png", NULL);
	sfSprite* retry = sfSprite_create(); sfSprite_setTexture(retry, retryTx, sfFalse);

	sfTexture* menuTx = sfTexture_createFromFile("../images/menu.png", NULL);
	sfSprite* menu = sfSprite_create(); sfSprite_setTexture(menu, menuTx, sfFalse);

	sfTexture* highScoresTx = sfTexture_createFromFile("../images/highscores.png", NULL);
	sfSprite* highScores = sfSprite_create(); sfSprite_setTexture(highScores, highScoresTx, sfFalse);

	sfSprite* endScreenSprites[3] = { retry, menu, highScores };

	sfEvent e;
	int select = 0;
	while (1) {
		drawEndSprites(window, gameOver, endScreenSprites, select);

		while (sfRenderWindow_pollEvent(window, &e)) {
			if (e.type == sfEvtClosed) sfRenderWindow_close(window);

			else if (e.type == sfEvtKeyPressed) {
				if (sfKeyboard_isKeyPressed(sfKeyS)) { // down
					select = (select + 1) % 3;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyW)) { // up
					select = ((select -1) % 3 + 3) % 3;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyEnter)) { // confirm
					switch (select) {
					case 0: sfTexture_destroy(gameOverTx); sfTexture_destroy(retryTx); sfTexture_destroy(menuTx); sfTexture_destroy(highScoresTx); sfSprite_destroy(gameOver); sfSprite_destroy(retry); sfSprite_destroy(menu); sfSprite_destroy(highScores); return; // retry
					case 1: sfTexture_destroy(gameOverTx); sfTexture_destroy(retryTx); sfTexture_destroy(menuTx); sfTexture_destroy(highScoresTx); sfSprite_destroy(gameOver); sfSprite_destroy(retry); sfSprite_destroy(menu); sfSprite_destroy(highScores); loadStartScreen(window, NULL, boards, snakes); return; // menu
					case 2: return; // highscores
					}
				}
			}
		}
	}
}

/* Checks if coordinate is on given part of the snake */
bool onSnake(int x, int y, int start) {
	for (int i = start; i < num; i++) {
		if (x == s[i].x && y == s[i].y) {
			return true;
		}
	}
	return false;
}

bool Tick() {
	////// movement //////
	for (int i = num; i > 0; --i) {
		s[i].x = s[i - 1].x;
		s[i].y = s[i - 1].y;
	}

	////// direction //////
	if (dir == 0) s[0].y += 1;
	if (dir == 1) s[0].x -= 1;
	if (dir == 2) s[0].x += 1;
	if (dir == 3) s[0].y -= 1;

	////// rules //////
	if (s[0].x >= N || s[0].x < 0 || s[0].y >= M || s[0].y < 0) { // out of bounds
		return false;
	}
	if (onSnake(s[0].x, s[0].y, 1)) {
		return false;
	}
	if (onDanger(s[0].x, s[0].y)) {
		return false;
	}

	////// food //////
	if ((s[0].x == f.x) && (s[0].y == f.y)) { // if head on food
		num++;
		while (onSnake(f.x, f.y, 0) || onDanger(f.x, f.y)) {
			f.x = rand() % N;
			f.y = rand() % M;
		}
	}
	return true;
}

int main() {
	srand((unsigned int)time(NULL));

	////// window //////
	sfUint32 style = 0;
	sfVideoMode video; video.width = w; video.height = h; video.bitsPerPixel = 4;
	sfRenderWindow* window = sfRenderWindow_create(video, "Snake", style, NULL);
	sfRenderWindow_setTitle(window, "Snake");

	////// rectangles //////
	sfVector2f sfSize; sfSize.x = size; sfSize.y = size;
	sfRectangleShape* square = sfRectangleShape_create(); sfRectangleShape_setSize(square, sfSize);

	////// sprites //////
	sfTexture* boardLavaTx = sfTexture_createFromFile("../images/board1.jpeg", NULL);
	sfSprite* boardLava = sfSprite_create(); sfSprite_setTexture(boardLava, boardLavaTx, sfFalse);
	sfTexture* boardWaterTx = sfTexture_createFromFile("../images/board2.jpeg", NULL);
	sfSprite* boardWater = sfSprite_create(); sfSprite_setTexture(boardWater, boardWaterTx, sfFalse);
	sfTexture* boardTerraTx = sfTexture_createFromFile("../images/board3.jpeg", NULL);
	sfSprite* boardTerra = sfSprite_create(); sfSprite_setTexture(boardTerra, boardTerraTx, sfFalse);
	sfSprite* boards[4] = { NULL, boardLava, boardWater, boardTerra };

	sfSprite* snakes[4] = { NULL, NULL, NULL, NULL };

	sfTexture* head0Tx = sfTexture_createFromFile("../images/head0.png", NULL);
	sfSprite* head0 = sfSprite_create(); sfSprite_setTexture(head0, head0Tx, sfFalse);
	sfTexture* head1Tx = sfTexture_createFromFile("../images/head1.png", NULL);
	sfSprite* head1 = sfSprite_create(); sfSprite_setTexture(head1, head1Tx, sfFalse);
	sfTexture* head2Tx = sfTexture_createFromFile("../images/head2.png", NULL);
	sfSprite* head2 = sfSprite_create(); sfSprite_setTexture(head2, head2Tx, sfFalse);
	sfTexture* head3Tx = sfTexture_createFromFile("../images/head3.png", NULL);
	sfSprite* head3 = sfSprite_create(); sfSprite_setTexture(head3, head3Tx, sfFalse);
	sfTexture* tailLVertTx = sfTexture_createFromFile("../images/tailLVert.png", NULL);
	sfSprite* tailLVert = sfSprite_create(); sfSprite_setTexture(tailLVert, tailLVertTx, sfFalse);
	sfTexture* tailRVertTx = sfTexture_createFromFile("../images/tailRVert.png", NULL);
	sfSprite* tailRVert = sfSprite_create(); sfSprite_setTexture(tailRVert, tailRVertTx, sfFalse);
	sfTexture* tailLHorTx = sfTexture_createFromFile("../images/tailLHor.png", NULL);
	sfSprite* tailLHor = sfSprite_create(); sfSprite_setTexture(tailLHor, tailLHorTx, sfFalse);
	sfTexture* tailRHorTx = sfTexture_createFromFile("../images/tailRHor.png", NULL);
	sfSprite* tailRHor = sfSprite_create(); sfSprite_setTexture(tailRHor, tailRHorTx, sfFalse);
	sfTexture* edge0Tx = sfTexture_createFromFile("../images/edge0.png", NULL);
	sfSprite* edge0 = sfSprite_create(); sfSprite_setTexture(edge0, edge0Tx, sfFalse);
	sfTexture* edge1Tx = sfTexture_createFromFile("../images/edge1.png", NULL);
	sfSprite* edge1 = sfSprite_create(); sfSprite_setTexture(edge1, edge1Tx, sfFalse);
	sfTexture* edge2Tx = sfTexture_createFromFile("../images/edge2.png", NULL);
	sfSprite* edge2 = sfSprite_create(); sfSprite_setTexture(edge2, edge2Tx, sfFalse);
	sfTexture* edge3Tx = sfTexture_createFromFile("../images/edge3.png", NULL);
	sfSprite* edge3 = sfSprite_create(); sfSprite_setTexture(edge3, edge3Tx, sfFalse);
	sfTexture* tailB0Tx = sfTexture_createFromFile("../images/tailB0.png", NULL);
	sfSprite* tailB0 = sfSprite_create(); sfSprite_setTexture(tailB0, tailB0Tx, sfFalse);
	sfTexture* tailB1Tx = sfTexture_createFromFile("../images/tailB1.png", NULL);
	sfSprite* tailB1 = sfSprite_create(); sfSprite_setTexture(tailB1, tailB1Tx, sfFalse);
	sfTexture* tailB2Tx = sfTexture_createFromFile("../images/tailB2.png", NULL);
	sfSprite* tailB2 = sfSprite_create(); sfSprite_setTexture(tailB2, tailB2Tx, sfFalse);
	sfTexture* tailB3Tx = sfTexture_createFromFile("../images/tailB3.png", NULL);
	sfSprite* tailB3 = sfSprite_create(); sfSprite_setTexture(tailB3, tailB3Tx, sfFalse);
	sfSprite* snake[16] = { head0, head1, head2, head3, tailLVert, tailRVert, tailLHor, tailRHor, edge0, edge1, edge2, edge3, tailB0, tailB1, tailB2, tailB3 };
	

	////// text //////
	sfFont* game = sfFont_createFromFile("../fonts/manaspc.ttf");
	sfText* length = sfText_create(); sfText_setFont(length, game); sfText_setFillColor(length, sfWhite); sfText_setStyle(length, style);
	sfText* start = sfText_copy(length); sfText_setString(start, "Press Enter to continue"); sfText_setCharacterSize(start, 40);
	sfVector2f pos; pos.x = 400.0; pos.y = 800.0; sfText_setPosition(start, pos);

	////// time //////
	sfClock* clock = sfClock_create();
	double timer = 0.0;

	resetStats();
	loadStartScreen(window, start, boards, snakes);
	while (sfRenderWindow_isOpen(window)) {
		sfEvent e;

		sfTime t = sfClock_getElapsedTime(clock);
		float time = sfTime_asSeconds(t);

		sfClock_restart(clock);
		timer += time;

		while (sfRenderWindow_pollEvent(window, &e)) { if (e.type == sfEvtClosed) sfRenderWindow_close(window); }

		if (sfKeyboard_isKeyPressed(sfKeyA) && dir != 2) dir = 1;
		else if (sfKeyboard_isKeyPressed(sfKeyD) && dir != 1) dir = 2;
		else if (sfKeyboard_isKeyPressed(sfKeyW) && dir != 0) dir = 3;
		else if (sfKeyboard_isKeyPressed(sfKeyS) && dir != 3) dir = 0;

		if (timer > 0.1 - delay * 0.025) { // run Tick
			timer = 0;
			if (!Tick()) {
				loadEndScreen(window, boards, snakes);
				resetStats();
			}
			loadBoard(window, boards[b], square, snake, length);
		}
	}
	return 0;
}
