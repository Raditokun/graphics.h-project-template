#include <graphics.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <vector>
#include <mmsystem.h>
#include "D:/RADIT Files/code/prolanvs/.vscode/Game/include/SpriteLoader.h"
#pragma comment(lib, "winmm.lib")


using namespace std;

enum SpriteType {
    STATIC,      // Non-moving sprite (objects, NPCs, etc)
    DYNAMIC      // Moving sprite (player, enemies, etc)
};

class Sprite{
    vector<void*> frames;
    int currentFrame;
    int x, y;
    int frameWidth, frameHeight;
    SpriteType type;
    int animationCounter;
    int animationSpeed;
    int velocityX, velocityY;
    string spriteID;
    
public:
    Sprite(const string &filepath, int numFrames, int startX, int startY, 
           const string &id, SpriteType spriteType = DYNAMIC, int animSpeed = 10)
        : currentFrame(0), x(startX), y(startY), type(spriteType), 
          animationCounter(0), animationSpeed(animSpeed), velocityX(0), velocityY(0), spriteID(id)
    {
        SpriteLoader loader;
        frames = loader.LoadStrip(filepath, numFrames);
        frameWidth = loader.GetFrameW();
        frameHeight = loader.GetFrameH();
    }
    
    void Draw()
    {
        if(!frames.empty()){
            putimage(x, y, frames[currentFrame], COPY_PUT);
        }
    }
    
    void Update()
    {
        // Update position if dynamic
        if(type == DYNAMIC)
        {
            x += velocityX;
            y += velocityY;
            
            // Clamp to screen bounds
            if(x < 0) x = 0;
            if(x + frameWidth > 1280) x = 1280 - frameWidth;
            if(y < 0) y = 0;
            if(y + frameHeight > 720) y = 720 - frameHeight;
        }
        
        // Update animation
        animationCounter++;
        if(animationCounter >= animationSpeed)
        {
            currentFrame = (currentFrame + 1) % frames.size();
            animationCounter = 0;
        }
    }
    
    void SetPosition(int newX, int newY)
    {
        x = newX;
        y = newY;
    }
    
    void SetVelocity(int vx, int vy)
    {
        velocityX = vx;
        velocityY = vy;
    }
    
    int GetX() const { return x; }
    int GetY() const { return y; }
    int GetWidth() const { return frameWidth; }
    int GetHeight() const { return frameHeight; }
    string GetID() const { return spriteID; }
    SpriteType GetType() const { return type; }
    
    // Simple AABB collision detection
    bool IsCollidingWith(const Sprite &other) const
    {
        return !(x + frameWidth < other.GetX() || 
                 x > other.GetX() + other.GetWidth() ||
                 y + frameHeight < other.GetY() || 
                 y > other.GetY() + other.GetHeight());
    }
    
    ~Sprite()
    {
        SpriteLoader::Free(frames);
    }
};

class SpriteManager {
    vector<Sprite*> sprites;
    
public:
    void AddSprite(Sprite* sprite)
    {
        sprites.push_back(sprite);
    }
    
    void RemoveSprite(const string &id)
    {
        for(auto it = sprites.begin(); it != sprites.end(); ++it)
        {
            if((*it)->GetID() == id)
            {
                delete *it;
                sprites.erase(it);
                break;
            }
        }
    }
    
    Sprite* GetSprite(const string &id)
    {
        for(auto sprite : sprites)
        {
            if(sprite->GetID() == id)
                return sprite;
        }
        return nullptr;
    }
    
    void UpdateAll()
    {
        for(auto sprite : sprites)
            sprite->Update();
    }
    
    void DrawAll()
    {
        for(auto sprite : sprites)
            sprite->Draw();
    }
    
    void CheckCollisions(const string &dynamicSpriteID)
    {
        Sprite* player = GetSprite(dynamicSpriteID);
        if(!player) return;
        
        for(auto sprite : sprites)
        {
            if(sprite->GetID() != dynamicSpriteID && player->IsCollidingWith(*sprite))
            {
                // Handle collision
                printf("Collision detected with: %s\n", sprite->GetID().c_str());
            }
        }
    }
    
    ~SpriteManager()
    {
        for(auto sprite : sprites)
            delete sprite;
        sprites.clear();
    }
};


int main(){

    initwindow(1280, 720, "Save My Wife");
    
    SpriteManager manager;
    
    // Add player sprite (moving)
    manager.AddSprite(new Sprite("./media/down.bmp", 4, 640, 360, "player", DYNAMIC, 10));
    
    // Add static sprites (NPCs, objects, etc)
    manager.AddSprite(new Sprite("./media/down.bmp", 4, 200, 200, "npc1", STATIC, 15));
    manager.AddSprite(new Sprite("./media/down.bmp", 4, 1000, 500, "npc2", STATIC, 15));
    
    int playerSpeed = 10;
    
    while(true)
    {
        cleardevice();
        
        // Update all sprites
        manager.UpdateAll();
        
        // Draw all sprites
        manager.DrawAll();
        
        // Check collisions with player
        manager.CheckCollisions("player");
        
        // Handle input
        if(kbhit())
        {
            char key = getch();
            Sprite* player = manager.GetSprite("player");
            
            if(key == 'w')
                player->SetVelocity(0, -playerSpeed);
            else if(key == 's')
                player->SetVelocity(0, playerSpeed);
            else if(key == 'a')
                player->SetVelocity(-playerSpeed, 0);
            else if(key == 'd')
                player->SetVelocity(playerSpeed, 0);
            else if(key == ' ')
                player->SetVelocity(0, 0); // Stop moving
            else if(key == 'q') 
                break;
        }
        
        delay(30);
    }
    
    closegraph();
    return 0;

}