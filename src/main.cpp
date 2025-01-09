#include <raylib.h>
#include<iostream>
#include<deque>
#include<raymath.h>
#include<unordered_set>
#include <functional>

using namespace std;

Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};
Color black = {0, 0, 0, 255};

int cellSize = 30;
int cellCount = 25;
int offset = 75;
double lastUpdateTime = 0;
double startTime = 0;

struct Vector2Hash {
    size_t operator()(const Vector2& v) const {
        return hash<float>()(v.x) ^ (hash<float>()(v.y) << 1);
    }
};

struct Vector2Equal {
    bool operator()(const Vector2& lhs, const Vector2& rhs) const {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }
};

bool elementInSnakeBody(Vector2& position, unordered_set<Vector2, Vector2Hash, Vector2Equal>&snakeBody){
    return snakeBody.find(position) != snakeBody.end();
}

bool eventTriggered(double interval){
    double currTime = GetTime();
    if(currTime - lastUpdateTime >= interval){
        lastUpdateTime = currTime;
        return true;
    }
    return false;
}

bool scoreTimeUpdate(double interval){
    double currTime = GetTime();
    if(currTime - startTime >= interval){
        startTime = currTime;
        return true;
    }
    return false;
}

class Snake{
public:
    deque<Vector2> body = {Vector2{7,9}, Vector2{6,9}, Vector2{5,9}};
    unordered_set<Vector2, Vector2Hash, Vector2Equal> bodySet = {Vector2{5,9}, Vector2{6,9}, Vector2{7,9}};
    Vector2 direction = Vector2{1, 0};
    void draw(){
        for(unsigned int i = 0;i<body.size(); i++){
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x*cellSize, offset +y*cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment, 0.8, 6, darkGreen);
        }
    }
    void updateFront(){
        Vector2 newElement = Vector2Add(body[0], direction);  
        bodySet.insert(newElement);
        body.push_front(newElement);
    }
    void updateBack(){
        Vector2 lastElement = body.back();
        bodySet.erase(lastElement);
        body.pop_back();
    }
};

class Food{
public:
    Vector2 position;
    Texture texture;
    Food(unordered_set<Vector2, Vector2Hash, Vector2Equal>&snakeBody){
        texture = LoadTexture("Graphics/food.png");
        position = generateRandomPos(snakeBody);
    }

    ~Food(){
        UnloadTexture(texture);
    }

    void draw(){
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }
    
    Vector2 generateRandomCell(){
        float y = GetRandomValue(0, cellCount-1);
        float x = GetRandomValue(0, cellCount-1);
        return Vector2{x, y};
    }

    Vector2 generateRandomPos(unordered_set<Vector2, Vector2Hash, Vector2Equal>&snakeBody){
        Vector2 position = generateRandomCell();
        while(elementInSnakeBody(position, snakeBody)){
            position = generateRandomCell();
        }
        return position;
    }

};

class Game{
public:
    Snake snake = Snake();
    Food food = Food(snake.bodySet);
    bool isRunning = false;
    bool overScreen = false;
    int score = 0;
    double gameSpeed = 0.2;
    double gameStartTime = 0;
    Sound eatSound;
    Sound gameOverSound;
    Music backgroundMusic;

    Game(){
        InitAudioDevice();
        eatSound = LoadSound("Sounds/eat.mp3");
        gameOverSound = LoadSound("Sounds/wall.mp3");
        backgroundMusic = LoadMusicStream("Sounds/backgroundMusic.mp3");
        PlayMusicStream(backgroundMusic);
    }

    ~Game(){
        UnloadSound(eatSound);
        UnloadSound(gameOverSound);
        UnloadMusicStream(backgroundMusic);
    }

    void draw(){
        food.draw();
        snake.draw();
    }

    void toggleDifficulty(){
        double currentTime = GetTime();
        if(gameSpeed != 0.1 && currentTime - gameStartTime > 100){
            cout<<"100 CROSSED";
            gameSpeed = 0.1;
        }
        else if(gameSpeed!= 0.15 && currentTime - gameStartTime >50){
            cout<<"50 CROSSED";
            gameSpeed = 0.15;
        }
    }
    
    void checkCollisionWithFood(){
        if(snake.body[0] == food.position){
            food.position = food.generateRandomPos(snake.bodySet);
            score+= 5;
            PlaySound(eatSound);
        }
        else{
            snake.updateBack();
        }
    }

    void checkCollisionWithEdges(){
        if(snake.body[0].x ==-1 || snake.body[0].x==cellCount || snake.body[0].y==-1 || snake.body[0].y==cellCount){
            gameOver();
        }
    }

    void checkCollisionWithBody(){
        for (size_t i = 1; i < snake.body.size(); i++) {
        if (snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y){
            gameOver();
            break;
            }
        }
    }

    void update(){
        if(isRunning){
            snake.updateFront();
            checkCollisionWithFood();
            checkCollisionWithEdges();
            checkCollisionWithBody();
        }
    }

    void gameOver(){
        overScreen = true;
        isRunning = false;
        PlaySound(gameOverSound);
    }

    void restartGame(){
        overScreen = false;
        snake.body = {Vector2{7,9}, Vector2{6,9}, Vector2{5,9}};
        snake.bodySet = {Vector2{7,9}, Vector2{6,9}, Vector2{5,9}};
        snake.direction = {1, 0};
        food.position = food.generateRandomPos(snake.bodySet);
        score = 0;
        StopMusicStream(backgroundMusic);
        PlayMusicStream(backgroundMusic);
        gameStartTime = GetTime();
    }
};

int main() 
{
    InitWindow(2*offset + cellSize * cellCount, 2*offset + cellSize * cellCount, "Snake Game");

    Game game = Game();

    while(WindowShouldClose() == false){
        BeginDrawing();
        
        if(game.isRunning){
            game.toggleDifficulty();
        }

        if(game.overScreen){
            DrawText("GAME OVER", cellCount*cellSize/2 - 100, cellCount*cellSize/2 - offset/2, 60, black);
            DrawText("Press Spacebar to Restart", cellCount*cellSize/2 -130, cellCount*cellSize/2 +30, 30, black);
            if(IsKeyPressed(KEY_SPACE)){
                game.restartGame();
            }
        }

        if(eventTriggered(game.gameSpeed)){
            game.update();
        }
        
        if(game.isRunning && scoreTimeUpdate(5)){
            game.score++;
        }

        if(IsKeyPressed(KEY_UP) && game.snake.direction.y !=1){
            game.isRunning = true;
            game.snake.direction = {0,-1};
        }
        if(IsKeyPressed(KEY_DOWN) && game.snake.direction.y !=-1){
            game.isRunning = true;
            game.snake.direction = {0,1};
        }
        if(IsKeyPressed(KEY_LEFT) && game.snake.direction.x !=1){
            game.isRunning = true;
            game.snake.direction = {-1,0};
        }
        if(IsKeyPressed(KEY_RIGHT) && game.snake.direction.x !=-1){
            game.isRunning = true;
            game.snake.direction = {1,0};
        }

        ClearBackground(green);
        DrawRectangleLinesEx(Rectangle{(float)offset-5, (float)offset-5, (float)cellCount*cellSize + 10, (float)cellCount * cellSize + 10}, 5, darkGreen);
        DrawText("Snake Game", 65, 30, 40, darkGreen);
        DrawText(TextFormat("Score: %d", game.score), cellCount * cellSize - 100, 30, 40, darkGreen);
        game.draw();
        
        UpdateMusicStream(game.backgroundMusic);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}