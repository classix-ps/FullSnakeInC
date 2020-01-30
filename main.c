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
double totalTime;

int b = 0;
int skin = 0;
int mode = 0;

int offset = 0;

struct Snake
{
	int x, y;
}  s[600];

struct Fruit
{
	int x, y;
} f;

struct Powerup
{
	int x, y;
	int time;
} p;

struct Portal
{
	int x, y;
	int time;
	bool open;
} pPort;

struct Save
{
	int lenScores[9][3]; // classic, classicHardcore, lava, lavaHardcore, water, waterHardcore, terra, terraHardcore, shuffle
	float timeScores[9][3];
	int completedMaps[4]; // 1 for normal, 2 for hardcore, 3 for both (if all maps on 3, offer easter egg snake skin)
} save;
void loadSaveData() {
	FILE* high = fopen("../save/highScores.txt", "r");
	if (high == NULL) {
		perror("Error");
		return;
	}
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 3; j++) {
			if (fscanf(high, "%d", &save.lenScores[i][j]) != 1) {
				printf("Error reading length high score\n");
			}
			if (fscanf(high, "%f", &save.timeScores[i][j]) != 1) {
				printf("Error reading time high score\n");
			}
		}
	}
	fclose(high);

	FILE* maps = fopen("../save/completedMaps.txt", "r");
	if (maps == NULL) {
		perror("Error");
		return;
	}
	for (int i = 0; i < 4; i++) {
		if (fscanf(maps, "%d", &save.completedMaps[i]) != 1) {
			printf("Error reading completed maps\n");
		}
	}
	fclose(maps);
}
bool updateSaveData() {
	if (mode != 2 && num > 19) { // map is beaten after length of 20 is reached
		if (save.completedMaps[b] != mode + 1 && save.completedMaps[b] != 3) {
			save.completedMaps[b] += mode + 1;
		}
	}

	bool newHighScore = false;
	int openPage = mode == 2 ? 8 : 2 * b + mode;
	for (int i = 0; i < 3; i++) {
		if (num >= save.lenScores[openPage][i]) {
			if (num == save.lenScores[openPage][i] && totalTime > save.timeScores[openPage][i]) continue;
			for (int j = 2; j > i; j--) {
				save.lenScores[openPage][j] = save.lenScores[openPage][j - 1];
				save.timeScores[openPage][j] = save.timeScores[openPage][j - 1];
			}
			save.lenScores[openPage][i] = num;
			save.timeScores[openPage][i] = (float)totalTime;
			newHighScore = true;
			break;
		}
	}

	return newHighScore;
}
void uploadSaveData() {
	FILE* high = fopen("../save/highScores.txt", "w");
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 3; j++) {
			fprintf(high, "%d %f ", save.lenScores[i][j], save.timeScores[i][j]);
		}
		fprintf(high, "\n");
	}
	fclose(high);

	FILE* maps = fopen("../save/completedMaps.txt", "w");
	for (int i = 0; i < 4; i++) {
		fprintf(maps, "%d ", save.completedMaps[i]);
	}
	fclose(maps);
}

struct Danger
{
	int x, y;
} d[156];
void setDanger() {
	int i = 0;

	if (b == 0) {
		for (int y = 8; y < 12; y++) {
			for (int x = 13; x < 17; x++) {
				if ((x == 13 || x == 16) && (y == 8 || y == 11)) continue;
				d[i].x = x; d[i].y = y; i++;
			}
		}
		while (i < 156) {
			d[i].x = -1; d[i].y = -1; i++;
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
		for (int x = 2; x < 7; x += 4) {
			for (int y = 2; y < 7; y += 4) {
				d[i].x = x; d[i].y = y; i++;
				d[i].x = 29 - x; d[i].y = y; i++;
				d[i].x = x; d[i].y = 19 - y; i++;
				d[i].x = 29 - x; d[i].y = 19 - y; i++;
			}
		}
		for (int y = 4; y < 9; y += 4) {
			d[i].x = 4; d[i].y = y; i++;
			d[i].x = 4; d[i].y = 19 - y; i++;
			d[i].x = 29 - 4; d[i].y = y; i++;
			d[i].x = 29 - 4; d[i].y = 19 - y; i++;
		}
		while (i < 156) {
			d[i].x = -1; d[i].y = -1; i++;
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
	f.x = 14;
	f.y = 6;
	p.x = -1;
	p.y = -1;
	p.time = 0;
	pPort.x = -2;
	pPort.x = -2;
	pPort.time = 0;
	dir = 0;
	num = 1;
	totalTime = 0.0;
}

/* Draws Window */
void loadBoard(sfRenderWindow* window, sfSprite* board, sfSprite* fruit, sfSprite* power, sfSprite* portal[2], sfSprite* snake[16], sfText* length) {
	sfRenderWindow_clear(window, sfBlack);
	sfVector2f pos;
	offset = (offset + 1) % 2;

	////// board //////
	sfRenderWindow_drawSprite(window, board, NULL);

	////// snake //////
	pos.x = s[0].x * size; pos.y = s[0].y * size; // head
	sfSprite_setPosition(snake[16* skin + dir], pos);
	sfRenderWindow_drawSprite(window, snake[16 * skin + dir], NULL);

	int snakeMapped[600];
	int currentDir = dir, lastDir = dir;
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
		if (mode == 2 && i == num - pPort.time) {
			break;
		}
		pos.x = s[i].x * size; pos.y = s[i].y * size;
		sfSprite_setPosition(snake[16 * skin + snakeMapped[i]], pos);
		sfRenderWindow_drawSprite(window, snake[16 * skin + snakeMapped[i]], NULL);
	}

	////// food //////
	pos.x = f.x * size; pos.y = f.y * size;
	sfSprite_setPosition(fruit, pos); // food
	sfRenderWindow_drawSprite(window, fruit, NULL);

	////// powerup //////
	if (mode == 2 && p.time) {
		pos.x = p.x * size; pos.y = p.y * size;
		sfSprite_setPosition(power, pos);
		sfRenderWindow_drawSprite(window, power, NULL);
	}

	////// portal //////
	if (mode == 2 && (pPort.open || pPort.time)) {
		pos.x = pPort.x * size; pos.y = pPort.y * size;
		sfSprite_setPosition(portal[offset], pos);
		sfRenderWindow_drawSprite(window, portal[offset], NULL);
	}

	////// length count //////
	pos.x = 1420.0f - 20.0f * floorf(log10f((float)num)); pos.y = 0.0;
	char numStr[4]; snprintf(numStr, 4, "%d", num); sfText_setString(length, numStr); sfText_setPosition(length, pos); // length
	sfRenderWindow_drawText(window, length, NULL);

	sfRenderWindow_display(window);
}

/* HIGH SCORES */
void drawHighScores(sfRenderWindow* window, sfSprite* highScore, sfText* highScores, sfText* mapMode, sfText* lenTime[3], int openPage) {
	sfRenderWindow_clear(window, sfBlack);

	sfRenderWindow_drawSprite(window, highScore, NULL);
	sfRenderWindow_drawText(window, highScores, NULL);
	sfRenderWindow_drawText(window, mapMode, NULL);
	for (int i = 0; i < 3; i++) {
		sfRenderWindow_drawText(window, lenTime[i], NULL);
	}

	sfText* score[3];
	for (int i = 0; i < 3; i++) {
		score[i] = sfText_copy(lenTime[i]);
		char scoreStr[30];
		float digits = floorf(log10f(((float)save.lenScores[openPage][i])));
		if ((int)digits == 0 || save.lenScores[openPage][i] == 0) snprintf(scoreStr, 30, "\t   %d\t    %.2f", save.lenScores[openPage][i], save.timeScores[openPage][i]);
		else if ((int)digits == 1) snprintf(scoreStr, 30, "\t   %d\t   %.2f", save.lenScores[openPage][i], save.timeScores[openPage][i]);
		else snprintf(scoreStr, 30, "\t   %d\t  %.2f", save.lenScores[openPage][i], save.timeScores[openPage][i]);
		sfText_setString(score[i], scoreStr);
		sfRenderWindow_drawText(window, score[i], NULL);
		sfText_destroy(score[i]);
	}

	sfRenderWindow_display(window);
}
bool loadHighScores(sfRenderWindow* window, int firstOpen) {
	int openPage = firstOpen;

	sfTexture* highScoreTx = sfTexture_createFromFile("../images/highScore.jpeg", NULL);
	sfSprite* highScore = sfSprite_create(); sfSprite_setTexture(highScore, highScoreTx, sfFalse);

	sfVector2f pos;
	sfFont* game = sfFont_createFromFile("../fonts/arcade.ttf");
	sfFont* data = sfFont_createFromFile("../fonts/lunchds.ttf");

	sfText* highScores = sfText_create();
	sfText_setFont(highScores, game);
	sfText_setCharacterSize(highScores, 150);
	pos.x = 340.0; pos.y = 60.0; sfText_setPosition(highScores, pos);
	sfText_setString(highScores, "High Scores");

	sfText* lenTime[3];
	for (int i = 0; i < 3; i++) {
		lenTime[i] = sfText_create();
		sfText_setFont(lenTime[i], data);
		sfText_setColor(lenTime[i], sfWhite);
		sfText_setCharacterSize(lenTime[i], 50);
		sfText_setString(lenTime[i], "Length:\tTime:");
		pos.x = 375.0; pos.y = 255.0f + 145.0f * i; sfText_setPosition(lenTime[i], pos);
	}

	sfText* mapMode[9];
	for (int i = 0; i < 9; i++) {
		mapMode[i] = sfText_create();
		sfText_setFont(mapMode[i], game);
		sfText_setCharacterSize(mapMode[i], 40);
		sfText_setFillColor(mapMode[i], sfGreen);
		pos.x = 550.0; pos.y = 20.0; sfText_setPosition(mapMode[i], pos);
	}
	sfText_setString(mapMode[0], " Classic - Normal");
	sfText_setString(mapMode[1], "Classic - Hardcore");
	sfText_setString(mapMode[2], "  Lava - Normal  ");
	sfText_setString(mapMode[3], " Lava - Hardcore ");
	sfText_setString(mapMode[4], "  Water - Normal ");
	sfText_setString(mapMode[5], " Water - Hardcore");
	sfText_setString(mapMode[6], "   Mine - Normal ");
	sfText_setString(mapMode[7], "  Mine - Hardcore");
	sfText_setString(mapMode[8], "     Shuffle     ");


	sfEvent e;
	while (1) {
		drawHighScores(window, highScore, highScores, mapMode[openPage], lenTime, openPage);

		while (sfRenderWindow_pollEvent(window, &e)) {
			if (e.type == sfEvtClosed) { sfRenderWindow_close(window); return false; }

			else if (e.type == sfEvtKeyPressed) {
				if (sfKeyboard_isKeyPressed(sfKeyD)) { // right
					openPage = (openPage + 1) % 9;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyA)) { // left
					openPage = ((openPage - 1) % 9 + 9) % 9;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyTab)) { // back
					sfFont_destroy(game); sfFont_destroy(data); sfTexture_destroy(highScoreTx); sfSprite_destroy(highScore); sfText_destroy(highScores); for (int i = 0; i < 9; i++) sfText_destroy(mapMode[i]); for (int i = 0; i < 3; i++) sfText_destroy(lenTime[i]);

					return true;
				}
			}
		}
	}
}

/* SETTINGS */
void drawSettingsText(sfRenderWindow* window, sfSprite* boards[4], sfSprite* snakes[4], sfText* modeTexts[3], sfText* settings, sfText* types[3], sfText* bTexts[4], sfText* sTexts[4], sfText* dTexts[3], sfText* completed[4], int selects[3], int select) {
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

	// map
	sfSprite* displayMap = sfSprite_create();
	sfSprite_setTexture(displayMap, sfSprite_getTexture(boards[selects[0]]), sfFalse);
	sfVector2f scale; scale.x = 0.3f; scale.y = 0.3f; sfSprite_setScale(displayMap, scale);
	sfVector2f pos; pos.x = 750.0; pos.y = 230.0; sfSprite_setPosition(displayMap, pos);
	sfRenderWindow_drawSprite(window, displayMap, NULL);
	sfSprite_destroy(displayMap);
	
	// completed
	sfText* normal = sfText_copy(dTexts[0]);
	sfText_setCharacterSize(normal, 30);
	sfText_setString(normal, "Normal - ");
	pos.x = 300.0; pos.y = 420.0; sfText_setPosition(normal, pos);
	sfRenderWindow_drawText(window, normal, NULL);
	if (save.completedMaps[selects[0]] == 1 || save.completedMaps[selects[0]] == 3) sfRenderWindow_drawText(window, completed[0], NULL);
	else sfRenderWindow_drawText(window, completed[1], NULL);
	sfText* hardcore = sfText_copy(normal);
	sfText_setString(hardcore, "Hardcore - ");
	pos.y = 450.0; sfText_setPosition(hardcore, pos);
	sfRenderWindow_drawText(window, hardcore, NULL);
	if (save.completedMaps[selects[0]] == 2 || save.completedMaps[selects[0]] == 3) sfRenderWindow_drawText(window, completed[2], NULL);
	else sfRenderWindow_drawText(window, completed[3], NULL);
	sfText_destroy(normal); sfText_destroy(hardcore);

	// snake
	if (save.completedMaps[selects[1]] < 1 && selects[1] != 0) {
		sfText* notOwned = sfText_copy(sTexts[0]);
		sfText_setCharacterSize(notOwned, 50);
		sfText_setFillColor(notOwned, sfRed);
		pos.x = 820.0; pos.y = 570.0; sfText_setPosition(notOwned, pos);
		sfText_setString(notOwned, "Not Unlocked");
		sfRenderWindow_drawText(window, notOwned, NULL);
		sfText_destroy(notOwned);
	}
	else sfRenderWindow_drawSprite(window, snakes[selects[1]], NULL);

	// mode
	sfRenderWindow_drawText(window, modeTexts[selects[2]], NULL);
	
	sfRenderWindow_display(window);
}
bool loadSettings(sfRenderWindow* window, sfSprite* boards[4], sfSprite* snakes[4], sfText* modeTexts[3]) {
	sfVector2f pos;
	sfFont* game = sfFont_createFromFile("../fonts/arcade.ttf");
	unsigned int textSize = 50;

	sfText* settings = sfText_create();
	sfText_setFont(settings, game);
	sfText_setCharacterSize(settings, 200);
	pos.x = 375.0; pos.y = 20.0; sfText_setPosition(settings, pos);
	sfText_setString(settings, "Settings");

	sfText* types[3];
	for (int i = 0; i < 3; i++) {
		types[i] = sfText_create();
		sfText_setFont(types[i], game);
		sfText_setCharacterSize(types[i], textSize);
		pos.x = 100.0; pos.y = 360.0f + 210.0f * i; sfText_setPosition(types[i], pos);
	}
	sfText_setString(types[0], "Map");
	sfText_setString(types[1], "Snake");
	sfText_setString(types[2], "Mode");

	sfText* boardTexts[4]; pos.x = 300.0; pos.y = 360.0;
	for (int i = 0; i < 4; i++) {
		boardTexts[i] = sfText_create();
		sfText_setFont(boardTexts[i], game);
		sfText_setCharacterSize(boardTexts[i], textSize);
		sfText_setPosition(boardTexts[i], pos);
	}
	sfText_setString(boardTexts[0], "<  Classic  >");
	sfText_setString(boardTexts[1], "<   Lava   >");
	sfText_setString(boardTexts[2], "<   Water   >");
	sfText_setString(boardTexts[3], "<   Mine    >");

	sfText* snakeTexts[4]; pos.y = 570.0;
	for (int i = 0; i < 4; i++) {
		snakeTexts[i] = sfText_create();
		sfText_setFont(snakeTexts[i], game);
		sfText_setCharacterSize(snakeTexts[i], textSize);
        sfText_setPosition(snakeTexts[i], pos);
	}
	sfText_setString(snakeTexts[0], "<  Classic  >");
	sfText_setString(snakeTexts[1], "<   Lava   >"); if (save.completedMaps[1] < 1) sfText_setFillColor(snakeTexts[1], sfRed);
	sfText_setString(snakeTexts[2], "<   Water   >"); if (save.completedMaps[2] < 1) sfText_setFillColor(snakeTexts[2], sfRed);
	sfText_setString(snakeTexts[3], "<   Mine    >"); if (save.completedMaps[3] < 1) sfText_setFillColor(snakeTexts[3], sfRed);

	sfText* difficultyTexts[3]; pos.y = 780.0;
	for (int i = 0; i < 3; i++) {
		difficultyTexts[i] = sfText_create();
		sfText_setFont(difficultyTexts[i], game);
		sfText_setCharacterSize(difficultyTexts[i], textSize);
		sfText_setPosition(difficultyTexts[i], pos);
	}
	sfText_setString(difficultyTexts[0], "<  Normal  >");
	sfText_setString(difficultyTexts[1], "< Hardcore >");
	sfText_setString(difficultyTexts[2], "<  Shuffle  >");

	sfText* completed[4];
	for (int i = 0; i < 4; i++) {
		completed[i] = sfText_create();
		sfText_setFont(completed[i], game);
		sfText_setCharacterSize(completed[i], 30);
		if (i < 2) { pos.x = 430.0; pos.y = 420.0; }
		else { pos.x = 465.0; pos.y = 450.0; }
		sfText_setPosition(completed[i], pos);
		if (i % 2) {
			sfText_setString(completed[i], "Not Completed");
			sfText_setFillColor(completed[i], sfRed);
		}
		else {
			sfText_setString(completed[i], "Completed");
			sfText_setFillColor(completed[i], sfGreen);
		}
	}

	int selects[3] = { b, skin, mode };
	int select = 0;

	sfEvent e;
	while (1) {
		drawSettingsText(window, boards, snakes, modeTexts, settings, types, boardTexts, snakeTexts, difficultyTexts, completed, selects, select);

		while (sfRenderWindow_pollEvent(window, &e)) {
			if (e.type == sfEvtClosed) { sfRenderWindow_close(window); return false; }

			else if (e.type == sfEvtKeyPressed) {
				if (sfKeyboard_isKeyPressed(sfKeyS)) { // down
					select = (select + 1) % 3;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyW)) { // up
					select = ((select - 1) % 3 + 3) % 3;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyD)) { // right
					if (select != 2) { // map or snake
						selects[select] = (selects[select] + 1) % 4;
					}
					else {
						selects[select] = (selects[select] + 1) % 3;
					}
				}
				else if (sfKeyboard_isKeyPressed(sfKeyA)) { // left
					if (select != 2) { // map or snake
						selects[select] = ((selects[select] - 1) % 4 + 4) % 4;
					}
					else {
						selects[select] = ((selects[select] - 1) % 3 + 3) % 3;
					}
				}
				else if (sfKeyboard_isKeyPressed(sfKeyTab) || sfKeyboard_isKeyPressed(sfKeyEnter)) { // back
					sfFont_destroy(game); sfText_destroy(settings); for (int i = 0; i < 3; i++) sfText_destroy(types[i]); for (int i = 0; i < 4; i++) { sfText_destroy(boardTexts[i]); sfText_destroy(snakeTexts[i]); } for (int i = 0; i < 2; i++) sfText_destroy(difficultyTexts[i]); for (int i = 0; i < 4; i++) sfText_destroy(completed[i]);

					b = selects[0]; skin = selects[1]; mode = selects[2];
					setDanger();
					return true;
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
bool loadStartScreen(sfRenderWindow* window, sfText* startText, sfSprite* boards[4], sfSprite* snakes[4], sfText* modeTexts[3]) {
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
			if (e.type == sfEvtClosed) { sfRenderWindow_close(window); return false; }

			else if (e.type == sfEvtKeyPressed) {
				if (sfKeyboard_isKeyPressed(sfKeyEnter)) {
					sfClock_destroy(clock);
					startText = NULL;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyEscape)) {
					sfRenderWindow_close(window);
					return false;
				}
			}
		}
	}

	int select = 0;
	while (1) {
		drawStartSprites(window, snake, startScreenSprites, select);
		
		while (sfRenderWindow_pollEvent(window, &e)) {
			if (e.type == sfEvtClosed) { sfRenderWindow_close(window); return false; }

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
					case 0: sfTexture_destroy(snakeTx); sfTexture_destroy(startTx); sfTexture_destroy(settingsTx); sfTexture_destroy(highScoresTx); sfSprite_destroy(snake); sfSprite_destroy(start); sfSprite_destroy(settings); sfSprite_destroy(highScores); return true; // start
					case 1: if (!loadSettings(window, boards, snakes, modeTexts)) return false; break; // settings
					case 2: if (!loadHighScores(window, 0)) return false; break; // highscores
					}
				}
				else if (sfKeyboard_isKeyPressed(sfKeyEscape)) { // quit
					sfRenderWindow_close(window);
					return false;
				}
			}
		}
	}
}

/* END MENU */
void drawEndSprites(sfRenderWindow* window, sfSprite* background, sfSprite* sprites[3], sfSprite* newHighScore, sfText* newHighScoreText, int select) {
	sfRenderWindow_clear(window, sfBlack);
	sfRenderWindow_drawSprite(window, background, NULL);
	if (newHighScore != NULL) {
		sfRenderWindow_drawSprite(window, newHighScore, NULL);
		sfRenderWindow_drawText(window, newHighScoreText, NULL);
	}

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
bool loadEndScreen(sfRenderWindow* window, sfSprite* boards[4], sfSprite* snakes[4], sfText* modeTexts[3], sfSprite* newHighScore, sfText* newHighScoreText) {
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
		drawEndSprites(window, gameOver, endScreenSprites, newHighScore, newHighScoreText, select);

		while (sfRenderWindow_pollEvent(window, &e)) {
			if (e.type == sfEvtClosed) { sfRenderWindow_close(window); return false; }

			else if (e.type == sfEvtKeyPressed) {
				if (sfKeyboard_isKeyPressed(sfKeyS)) { // down
					select = (select + 1) % 3;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyW)) { // up
					select = ((select -1) % 3 + 3) % 3;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyEnter)) { // confirm
					switch (select) {
					case 0: sfTexture_destroy(gameOverTx); sfTexture_destroy(retryTx); sfTexture_destroy(menuTx); sfTexture_destroy(highScoresTx); sfSprite_destroy(gameOver); sfSprite_destroy(retry); sfSprite_destroy(menu); sfSprite_destroy(highScores); return true; // retry
					case 1: sfTexture_destroy(gameOverTx); sfTexture_destroy(retryTx); sfTexture_destroy(menuTx); sfTexture_destroy(highScoresTx); sfSprite_destroy(gameOver); sfSprite_destroy(retry); sfSprite_destroy(menu); sfSprite_destroy(highScores); if (loadStartScreen(window, NULL, boards, snakes, modeTexts)) return true; else return false; // menu
					case 2: if (!loadHighScores(window, mode == 2 ? 8 : 2 * b + mode)) return false; break; // highscores
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
		while (onSnake(f.x, f.y, 0) || onDanger(f.x, f.y) || (p.x == f.x && p.y == f.y)) {
			f.x = rand() % N;
			f.y = rand() % M;
		}

		if (mode == 2) {
			srand((unsigned int)time(NULL));
			if (!(rand() % 5) && !p.time && !(pPort.open || pPort.time)) { // spawn powerup at 20% chance per fruit, as long as no portal is open
				p.x = rand() % N;
				p.y = rand() % M;
				while (onSnake(p.x, p.y, 0) || onDanger(p.x, p.y) || (p.x == f.x && p.y == f.y)) {
					p.x = rand() % N;
					p.y = rand() % M;
				}
				p.time = 80; // 8 seconds
			}
		}
	}

	if (mode == 2) {
		////// powerup //////
		if ((s[0].x == p.x) && (s[0].y == p.y)) {
			p.x = -1; p.y = -1;
			p.time = 0;

			pPort.open = true;
			pPort.x = rand() % (N - 1);
			pPort.y = rand() % (M - 1);
			while (onSnake(pPort.x, pPort.y, 0) || onSnake(pPort.x + 1, pPort.y, 0) || onSnake(pPort.x, pPort.y + 1, 0) || onSnake(pPort.x + 1, pPort.y + 1, 0) ||
				onDanger(pPort.x, pPort.y) || onDanger(pPort.x + 1, pPort.y) || onDanger(pPort.x, pPort.y + 1) || onDanger(pPort.x + 1, pPort.y + 1) ||
				(pPort.x == f.x && pPort.y == f.y) || (pPort.x + 1 == f.x && pPort.y == f.y) || (pPort.x == f.x && pPort.y + 1 == f.y) || (pPort.x + 1 == f.x && pPort.y + 1 == f.y)) {
				pPort.x = rand() % (N - 1);
				pPort.x = rand() % (M - 1);
			}
		}
		if (p.time) p.time--;

		////// portal //////
		if ((s[0].x == pPort.x && s[0].y == pPort.y) || (s[0].x == pPort.x + 1 && s[0].y == pPort.y) || (s[0].x == pPort.x && s[0].y == pPort.y + 1) || (s[0].x == pPort.x + 1 && s[0].y == pPort.y + 1)) {
			int xOffset = 0, yOffset = 0;
			if (dir == 0) {
				if (s[0].x == pPort.x + 1) {
					xOffset = 1;
				}
				yOffset = 2;
			}
			else if (dir == 3) {
				if (s[0].x == pPort.x + 1) {
					xOffset = 1;
				}
				yOffset = -1;
			}
			else if (dir == 2) {
				if (s[0].y == pPort.y + 1) {
					yOffset = 1;
				}
				xOffset = 2;
			}
			else {
				if (s[0].y == pPort.y + 1) {
					yOffset = 1;
				}
				xOffset = -1;
			}

			int newB = rand() % 4;
			while (b == newB) {
				newB = rand() % 4;
			}
			b = newB;
			setDanger();
			if (b == 0) {
				pPort.x = 8; pPort.y = 6;
			}
			else if (b == 1) {
				pPort.x = 14; pPort.y = 9;
			}
			else if (b == 2) {
				pPort.x = 5; pPort.y = 6;
			}
			else {
				pPort.x = 14; pPort.y = 9;
			}
			pPort.time = num;
			pPort.open = false;

			s[0].x = pPort.x + xOffset;
			s[0].y = pPort.y + yOffset;

			for (int i = 1; i < num; i++) { // set rest of body outside of board
				s[i].x = -1; s[i].y = -1;
			}

			while (onSnake(f.x, f.y, 0) || onDanger(f.x, f.y) || (p.x == f.x && p.y == f.y)) {
				f.x = rand() % N;
				f.y = rand() % M;
			}
		}
		if (!pPort.open) {
			if (pPort.time) pPort.time--;
			else { pPort.x = -2; pPort.y = -2; }
		}
	}

	return true;
}

void runSnake() {
	loadSaveData();
	srand((unsigned int)time(NULL));
	sfVector2f pos;

	////// window //////
	sfVideoMode video; video.width = w; video.height = h; video.bitsPerPixel = 4;
	sfRenderWindow* window = sfRenderWindow_create(video, "Snake", sfDefaultStyle, NULL);
	sfRenderWindow_setTitle(window, "Snake");

	////// sprites //////
	sfTexture* fruitTx = sfTexture_createFromFile("../images/fruit.png", NULL);
	sfSprite* fruit = sfSprite_create(); sfSprite_setTexture(fruit, fruitTx, sfFalse);

	sfTexture* powerTx = sfTexture_createFromFile("../images/power.png", NULL);
	sfSprite* power = sfSprite_create(); sfSprite_setTexture(power, powerTx, sfFalse);
	
	sfTexture* portal0Tx = sfTexture_createFromFile("../images/portal0.png", NULL);
	sfSprite* portal0 = sfSprite_create(); sfSprite_setTexture(portal0, portal0Tx, sfFalse);
	sfTexture* portal1Tx = sfTexture_createFromFile("../images/portal1.png", NULL);
	sfSprite* portal1 = sfSprite_create(); sfSprite_setTexture(portal1, portal1Tx, sfFalse);
	sfSprite* portal[2] = { portal0, portal1 };

	sfTexture* boardClassicTx = sfTexture_createFromFile("../images/board0.jpg", NULL);
	sfSprite* boardClassic = sfSprite_create(); sfSprite_setTexture(boardClassic, boardClassicTx, sfFalse);
	sfTexture* boardLavaTx = sfTexture_createFromFile("../images/board1.jpeg", NULL);
	sfSprite* boardLava = sfSprite_create(); sfSprite_setTexture(boardLava, boardLavaTx, sfFalse);
	sfTexture* boardWaterTx = sfTexture_createFromFile("../images/board2.jpeg", NULL);
	sfSprite* boardWater = sfSprite_create(); sfSprite_setTexture(boardWater, boardWaterTx, sfFalse);
	sfTexture* boardTerraTx = sfTexture_createFromFile("../images/board3.jpeg", NULL);
	sfSprite* boardTerra = sfSprite_create(); sfSprite_setTexture(boardTerra, boardTerraTx, sfFalse);
	sfSprite* boards[4] = { boardClassic, boardLava, boardWater, boardTerra };

	sfTexture* snakeClassicTx = sfTexture_createFromFile("../images/snake0.png", NULL);
	sfSprite* snakeClassic = sfSprite_create(); sfSprite_setTexture(snakeClassic, snakeClassicTx, sfFalse);
	sfTexture* snakeLavaTx = sfTexture_createFromFile("../images/snake1.png", NULL);
	sfSprite* snakeLava = sfSprite_create(); sfSprite_setTexture(snakeLava, snakeLavaTx, sfFalse);
	sfTexture* snakeWaterTx = sfTexture_createFromFile("../images/snake2.png", NULL);
	sfSprite* snakeWater = sfSprite_create(); sfSprite_setTexture(snakeWater, snakeWaterTx, sfFalse);
	sfTexture* snakeTerraTx = sfTexture_createFromFile("../images/snake3.png", NULL);
	sfSprite* snakeTerra = sfSprite_create(); sfSprite_setTexture(snakeTerra, snakeTerraTx, sfFalse);
	sfSprite* snakes[4] = { snakeClassic, snakeLava, snakeWater, snakeTerra };
	sfVector2f scale; scale.x = 1.5; scale.y = 1.5; pos.x = 740.0; pos.y = 570.0;
	for (int i = 0; i < 4; i++) {
		sfSprite_setScale(snakes[i], scale);
		sfSprite_setPosition(snakes[i], pos);
	}

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

	sfTexture* head0LTx = sfTexture_createFromFile("../images/head0L.png", NULL);
	sfSprite* head0L = sfSprite_create(); sfSprite_setTexture(head0L, head0LTx, sfFalse);
	sfTexture* head1LTx = sfTexture_createFromFile("../images/head1L.png", NULL);
	sfSprite* head1L = sfSprite_create(); sfSprite_setTexture(head1L, head1LTx, sfFalse);
	sfTexture* head2LTx = sfTexture_createFromFile("../images/head2L.png", NULL);
	sfSprite* head2L = sfSprite_create(); sfSprite_setTexture(head2L, head2LTx, sfFalse);
	sfTexture* head3LTx = sfTexture_createFromFile("../images/head3L.png", NULL);
	sfSprite* head3L = sfSprite_create(); sfSprite_setTexture(head3L, head3LTx, sfFalse);
	sfTexture* tailLVertLTx = sfTexture_createFromFile("../images/tailLVertL.png", NULL);
	sfSprite* tailLVertL = sfSprite_create(); sfSprite_setTexture(tailLVertL, tailLVertLTx, sfFalse);
	sfTexture* tailRVertLTx = sfTexture_createFromFile("../images/tailRVertL.png", NULL);
	sfSprite* tailRVertL = sfSprite_create(); sfSprite_setTexture(tailRVertL, tailRVertLTx, sfFalse);
	sfTexture* tailLHorLTx = sfTexture_createFromFile("../images/tailLHorL.png", NULL);
	sfSprite* tailLHorL = sfSprite_create(); sfSprite_setTexture(tailLHorL, tailLHorLTx, sfFalse);
	sfTexture* tailRHorLTx = sfTexture_createFromFile("../images/tailRHorL.png", NULL);
	sfSprite* tailRHorL = sfSprite_create(); sfSprite_setTexture(tailRHorL, tailRHorLTx, sfFalse);
	sfTexture* edge0LTx = sfTexture_createFromFile("../images/edge0L.png", NULL);
	sfSprite* edge0L = sfSprite_create(); sfSprite_setTexture(edge0L, edge0LTx, sfFalse);
	sfTexture* edge1LTx = sfTexture_createFromFile("../images/edge1L.png", NULL);
	sfSprite* edge1L = sfSprite_create(); sfSprite_setTexture(edge1L, edge1LTx, sfFalse);
	sfTexture* edge2LTx = sfTexture_createFromFile("../images/edge2L.png", NULL);
	sfSprite* edge2L = sfSprite_create(); sfSprite_setTexture(edge2L, edge2LTx, sfFalse);
	sfTexture* edge3LTx = sfTexture_createFromFile("../images/edge3L.png", NULL);
	sfSprite* edge3L = sfSprite_create(); sfSprite_setTexture(edge3L, edge3LTx, sfFalse);
	sfTexture* tailB0LTx = sfTexture_createFromFile("../images/tailB0L.png", NULL);
	sfSprite* tailB0L = sfSprite_create(); sfSprite_setTexture(tailB0L, tailB0LTx, sfFalse);
	sfTexture* tailB1LTx = sfTexture_createFromFile("../images/tailB1L.png", NULL);
	sfSprite* tailB1L = sfSprite_create(); sfSprite_setTexture(tailB1L, tailB1LTx, sfFalse);
	sfTexture* tailB2LTx = sfTexture_createFromFile("../images/tailB2L.png", NULL);
	sfSprite* tailB2L = sfSprite_create(); sfSprite_setTexture(tailB2L, tailB2LTx, sfFalse);
	sfTexture* tailB3LTx = sfTexture_createFromFile("../images/tailB3L.png", NULL);
	sfSprite* tailB3L = sfSprite_create(); sfSprite_setTexture(tailB3L, tailB3LTx, sfFalse);

	sfTexture* head0WTx = sfTexture_createFromFile("../images/head0W.png", NULL);
	sfSprite* head0W = sfSprite_create(); sfSprite_setTexture(head0W, head0WTx, sfFalse);
	sfTexture* head1WTx = sfTexture_createFromFile("../images/head1W.png", NULL);
	sfSprite* head1W = sfSprite_create(); sfSprite_setTexture(head1W, head1WTx, sfFalse);
	sfTexture* head2WTx = sfTexture_createFromFile("../images/head2W.png", NULL);
	sfSprite* head2W = sfSprite_create(); sfSprite_setTexture(head2W, head2WTx, sfFalse);
	sfTexture* head3WTx = sfTexture_createFromFile("../images/head3W.png", NULL);
	sfSprite* head3W = sfSprite_create(); sfSprite_setTexture(head3W, head3WTx, sfFalse);
	sfTexture* tailLVertWTx = sfTexture_createFromFile("../images/tailLVertW.png", NULL);
	sfSprite* tailLVertW = sfSprite_create(); sfSprite_setTexture(tailLVertW, tailLVertWTx, sfFalse);
	sfTexture* tailRVertWTx = sfTexture_createFromFile("../images/tailRVertW.png", NULL);
	sfSprite* tailRVertW = sfSprite_create(); sfSprite_setTexture(tailRVertW, tailRVertWTx, sfFalse);
	sfTexture* tailLHorWTx = sfTexture_createFromFile("../images/tailLHorW.png", NULL);
	sfSprite* tailLHorW = sfSprite_create(); sfSprite_setTexture(tailLHorW, tailLHorWTx, sfFalse);
	sfTexture* tailRHorWTx = sfTexture_createFromFile("../images/tailRHorW.png", NULL);
	sfSprite* tailRHorW = sfSprite_create(); sfSprite_setTexture(tailRHorW, tailRHorWTx, sfFalse);
	sfTexture* edge0WTx = sfTexture_createFromFile("../images/edge0W.png", NULL);
	sfSprite* edge0W = sfSprite_create(); sfSprite_setTexture(edge0W, edge0WTx, sfFalse);
	sfTexture* edge1WTx = sfTexture_createFromFile("../images/edge1W.png", NULL);
	sfSprite* edge1W = sfSprite_create(); sfSprite_setTexture(edge1W, edge1WTx, sfFalse);
	sfTexture* edge2WTx = sfTexture_createFromFile("../images/edge2W.png", NULL);
	sfSprite* edge2W = sfSprite_create(); sfSprite_setTexture(edge2W, edge2WTx, sfFalse);
	sfTexture* edge3WTx = sfTexture_createFromFile("../images/edge3W.png", NULL);
	sfSprite* edge3W = sfSprite_create(); sfSprite_setTexture(edge3W, edge3WTx, sfFalse);
	sfTexture* tailB0WTx = sfTexture_createFromFile("../images/tailB0W.png", NULL);
	sfSprite* tailB0W = sfSprite_create(); sfSprite_setTexture(tailB0W, tailB0WTx, sfFalse);
	sfTexture* tailB1WTx = sfTexture_createFromFile("../images/tailB1W.png", NULL);
	sfSprite* tailB1W = sfSprite_create(); sfSprite_setTexture(tailB1W, tailB1WTx, sfFalse);
	sfTexture* tailB2WTx = sfTexture_createFromFile("../images/tailB2W.png", NULL);
	sfSprite* tailB2W = sfSprite_create(); sfSprite_setTexture(tailB2W, tailB2WTx, sfFalse);
	sfTexture* tailB3WTx = sfTexture_createFromFile("../images/tailB3W.png", NULL);
	sfSprite* tailB3W = sfSprite_create(); sfSprite_setTexture(tailB3W, tailB3WTx, sfFalse);

	sfTexture* head0MTx = sfTexture_createFromFile("../images/head0M.png", NULL);
	sfSprite* head0M = sfSprite_create(); sfSprite_setTexture(head0M, head0MTx, sfFalse);
	sfTexture* head1MTx = sfTexture_createFromFile("../images/head1M.png", NULL);
	sfSprite* head1M = sfSprite_create(); sfSprite_setTexture(head1M, head1MTx, sfFalse);
	sfTexture* head2MTx = sfTexture_createFromFile("../images/head2M.png", NULL);
	sfSprite* head2M = sfSprite_create(); sfSprite_setTexture(head2M, head2MTx, sfFalse);
	sfTexture* head3MTx = sfTexture_createFromFile("../images/head3M.png", NULL);
	sfSprite* head3M = sfSprite_create(); sfSprite_setTexture(head3M, head3MTx, sfFalse);
	sfTexture* tailLVertMTx = sfTexture_createFromFile("../images/tailLVertM.png", NULL);
	sfSprite* tailLVertM = sfSprite_create(); sfSprite_setTexture(tailLVertM, tailLVertMTx, sfFalse);
	sfTexture* tailRVertMTx = sfTexture_createFromFile("../images/tailRVertM.png", NULL);
	sfSprite* tailRVertM = sfSprite_create(); sfSprite_setTexture(tailRVertM, tailRVertMTx, sfFalse);
	sfTexture* tailLHorMTx = sfTexture_createFromFile("../images/tailLHorM.png", NULL);
	sfSprite* tailLHorM = sfSprite_create(); sfSprite_setTexture(tailLHorM, tailLHorMTx, sfFalse);
	sfTexture* tailRHorMTx = sfTexture_createFromFile("../images/tailRHorM.png", NULL);
	sfSprite* tailRHorM = sfSprite_create(); sfSprite_setTexture(tailRHorM, tailRHorMTx, sfFalse);
	sfTexture* edge0MTx = sfTexture_createFromFile("../images/edge0M.png", NULL);
	sfSprite* edge0M = sfSprite_create(); sfSprite_setTexture(edge0M, edge0MTx, sfFalse);
	sfTexture* edge1MTx = sfTexture_createFromFile("../images/edge1M.png", NULL);
	sfSprite* edge1M = sfSprite_create(); sfSprite_setTexture(edge1M, edge1MTx, sfFalse);
	sfTexture* edge2MTx = sfTexture_createFromFile("../images/edge2M.png", NULL);
	sfSprite* edge2M = sfSprite_create(); sfSprite_setTexture(edge2M, edge2MTx, sfFalse);
	sfTexture* edge3MTx = sfTexture_createFromFile("../images/edge3M.png", NULL);
	sfSprite* edge3M = sfSprite_create(); sfSprite_setTexture(edge3M, edge3MTx, sfFalse);
	sfTexture* tailB0MTx = sfTexture_createFromFile("../images/tailB0M.png", NULL);
	sfSprite* tailB0M = sfSprite_create(); sfSprite_setTexture(tailB0M, tailB0MTx, sfFalse);
	sfTexture* tailB1MTx = sfTexture_createFromFile("../images/tailB1M.png", NULL);
	sfSprite* tailB1M = sfSprite_create(); sfSprite_setTexture(tailB1M, tailB1MTx, sfFalse);
	sfTexture* tailB2MTx = sfTexture_createFromFile("../images/tailB2M.png", NULL);
	sfSprite* tailB2M = sfSprite_create(); sfSprite_setTexture(tailB2M, tailB2MTx, sfFalse);
	sfTexture* tailB3MTx = sfTexture_createFromFile("../images/tailB3M.png", NULL);
	sfSprite* tailB3M = sfSprite_create(); sfSprite_setTexture(tailB3M, tailB3MTx, sfFalse);
	sfSprite* snake[64] = {
		head0, head1, head2, head3, tailLVert, tailRVert, tailLHor, tailRHor, edge0, edge1, edge2, edge3, tailB0, tailB1, tailB2, tailB3,
		head0L, head1L, head2L, head3L, tailLVertL, tailRVertL, tailLHorL, tailRHorL, edge0L, edge1L, edge2L, edge3L, tailB0L, tailB1L, tailB2L, tailB3L,
		head0W, head1W, head2W, head3W, tailLVertW, tailRVertW, tailLHorW, tailRHorW, edge0W, edge1W, edge2W, edge3W, tailB0W, tailB1W, tailB2W, tailB3W,
		head0M, head1M, head2M, head3M, tailLVertM, tailRVertM, tailLHorM, tailRHorM, edge0M, edge1M, edge2M, edge3M, tailB0M, tailB1M, tailB2M, tailB3M };

	sfTexture* newHighScoreTx = sfTexture_createFromFile("../images/newHighScore.jpeg", NULL);
	sfSprite* newHighScore = sfSprite_create(); sfSprite_setTexture(newHighScore, newHighScoreTx, sfFalse);
	scale.x = 0.6f; scale.y = 0.6f; sfSprite_setScale(newHighScore, scale);
	pos.x = 1100.0; pos.y = 250.0; sfSprite_setPosition(newHighScore, pos);

	////// text //////
	sfFont* game = sfFont_createFromFile("../fonts/manaspc.ttf");
	sfText* length = sfText_create(); sfText_setFont(length, game);
	sfText* start = sfText_copy(length); sfText_setString(start, "Press Enter to continue"); sfText_setCharacterSize(start, 40);
	pos.x = 400.0; pos.y = 800.0; sfText_setPosition(start, pos);
	sfText* newHighScoreText = sfText_create(); sfText_setFont(newHighScoreText, game);
	sfText_setString(newHighScoreText, "Fabuloussssss!\nNew High Score\nfor this map!"); sfText_setCharacterSize(newHighScoreText, 18);
	pos.x = 1135.0; pos.y = 325.0; sfText_setPosition(newHighScoreText, pos); sfText_setFillColor(newHighScoreText, sfBlack);

	sfFont* arcade = sfFont_createFromFile("../fonts/arcade.ttf");
	sfText* modeTexts[3]; pos.x = 750.0; pos.y = 700.0;
	for (int i = 0; i < 3; i++) {
		modeTexts[i] = sfText_create();
		sfText_setFont(modeTexts[i], arcade);
		sfText_setCharacterSize(modeTexts[i], 40);
		sfText_setPosition(modeTexts[i], pos);
	}
	sfText_setString(modeTexts[0], "Classic Snake at normal\nSpeed. Try reaching the\nhighest length you can!");
	sfText_setString(modeTexts[1], "Snake at an extra high\nSpeed. Please play only\nunder users own risk!");
	sfText_setString(modeTexts[2], "Collect an occasional\npower-up which spawns\na Portal. It teleports\nyou to a different map!");

	////// time //////
	sfClock* clock = sfClock_create();
	double timer = 0.0;

	if (!loadStartScreen(window, start, boards, snakes, modeTexts)) return;
	resetStats();
	setDanger();
	while (sfRenderWindow_isOpen(window)) {
		sfEvent e;

		sfTime t = sfClock_getElapsedTime(clock);
		float time = sfTime_asSeconds(t);

		sfClock_restart(clock);
		timer += time;

		while (sfRenderWindow_pollEvent(window, &e)) { if (e.type == sfEvtClosed) { sfRenderWindow_close(window); return; } }

		if (sfKeyboard_isKeyPressed(sfKeyA) && dir != 2) dir = 1;
		else if (sfKeyboard_isKeyPressed(sfKeyD) && dir != 1) dir = 2;
		else if (sfKeyboard_isKeyPressed(sfKeyW) && dir != 0) dir = 3;
		else if (sfKeyboard_isKeyPressed(sfKeyS) && dir != 3) dir = 0;

		if (timer > 0.1 - 0.02 * (mode == 1)) { // run Tick
			totalTime += timer;
			timer = 0.0;
			if (!Tick()) {
				if (updateSaveData()) {
					if (!loadEndScreen(window, boards, snakes, modeTexts, newHighScore, newHighScoreText)) return;
				}
				else {
					if (!loadEndScreen(window, boards, snakes, modeTexts, NULL, NULL)) return;
				}
				resetStats();
			}
			loadBoard(window, boards[b], fruit, power, portal, snake, length);
		}
	}
}

int main() {
	runSnake();

	uploadSaveData();

	return 0;
}