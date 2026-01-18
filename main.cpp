#include <cmath>
#include <vector>
#include "constants.h"
#include <ctime>
#include <iostream>
#include <raymath.h>
#include <raylib.h>

//colors
Color randRainbow() //random rgb with max saturation function
{
    // should return an rgba value where
    // a=255, out of rgb one is 255, one is 0, and one is somewhere between 0 and 255
    unsigned char vals[] = {0,0,0}; //all 0
    int idx = rand() % 3; //select which to set to 255
    vals[idx] = 255; //set it to 255
    idx += rand() % 2 + 1; // switch to another index, by adding to it,
    idx = idx % 3; // then taking the remainder
    vals[idx] = rand() % 256; //set it to a random value inside the range
    return {vals[0],vals[1],vals[2],255}; //return a color struct {r,g,b,a}

}

//textures
Texture2D textures[3]; //holds the frames of the jellyfish sprite
Texture2D glow; //holds the glow effect which is jsut a transluscent png
void loadTextures() //loads textures that will be needed
{
    char filename[15]="assets/jfn.png"; //the convenient naming allows for a simple for loop
    for(int i = 0; i < 3; i++) //the for loop
    {
        filename[9]=(char)(i+48); //int to char 
        textures[i] = LoadTexture(filename); //load texture
    }
    glow = LoadTexture("assets/glow.png"); //the glow effect

;
}
void unloadTextures() //unload used textures
{
    for(Texture2D & t : textures) //loop thorugh jellyfish frames
    {
        UnloadTexture(t); //unload frame
    }
    UnloadTexture(glow); //unload glow effect
}

//boid class and functions' predeclaration
class Boid; 
float distance(Vector2 v, Vector2 w); 
Vector2 cohesion(Boid &b); 
Vector2 alignment(Boid &b);
Vector2 separation(Boid &b);
Vector2 pull(Boid &b);


class Boid //boid class implementation
{
    public:        
    Vector2 pos = {(float)(rand() % SCREEN_WIDTH),(float)(rand() % SCREEN_HEIGHT)}; //position, initially random on screen
    Vector2 vel = {(float)pow(-1,rand() % 2),(float)pow(-1,rand() % 2),}; //velocity, xy initially random between -1 and 1
    Vector2 acc; //acceleration

    Color color = randRainbow(); //color of boid, initially randomm but fully saturated

    int spriteIdx = rand() % 3; //start frame
    int timer = 0; //animation timer

    void draw() //render boid
    {
        Rectangle src = {0,0,16,14}; //source rectangle for pro
        Rectangle dest = {pos.x,pos.y,16*2,14*2}; //destination rectangle for pro
        DrawTexturePro(textures[spriteIdx],src,dest,{16,14},0,color); //draw sprite
        DrawTexture(glow,pos.x-32,pos.y-32,{255,255,255,200}); //add glow effect
    }

    void step() //update boid every frame
    {
        acc=Vector2Add(Vector2Add(cohesion(*this),alignment(*this)),Vector2Add(separation(*this),pull(*this))); 
        //calculates: acc=cohesion*cFactor+alignment*aFactor+separation*sFactor+pull*pFactor
    }
    void endStep() //after update, every frame
    {
        vel=Vector2Add(vel,acc); //acccelerate
        vel=Vector2ClampValue(vel,-1,1); //cap velocity

        pos=Vector2Add(pos,vel);  //move

        //bounce of edges
        if(pos.x > SCREEN_WIDTH){pos.x=SCREEN_WIDTH; vel.x=-vel.x;}
        if(pos.x < 0){pos.x=0; vel.x=-vel.x;}
        if(pos.y > SCREEN_HEIGHT){pos.y=SCREEN_HEIGHT; vel.y=-vel.y;}
        if(pos.y < 0){pos.y=0; vel.y=-vel.y;}

        //animation
        if(timer % 15 == 0) //every 15 frames(4/s):
        {
            spriteIdx += 1; //move to next frame
            spriteIdx = spriteIdx % 3; //loop back to start if >= 3
        }
        timer++; //increment timer
        timer = timer % 60; //loop back to start if timer >= 60
    }
};
std::vector<Boid> boids; //array holding boids on screen

float distance(Vector2 v, Vector2 w) //distance function
{
    return sqrt(pow(v.x-w.x,2)+pow(v.y-w.y,2)); //pythagorean theorem
}
Vector2 cohesion(Boid &b) //calc. cohesion, offset from avg. pos of other boids
{
    Vector2 result = {0,0}; //result var.
    for(Boid & b2 : boids) //loop through boids
    {
        if(&b2!=&b) //skip self
        {
            result=Vector2Add(result,b2.pos); //add to sum
        }
    }
    result=Vector2Scale(result,1/(boids.size()-1)); //divide by size-1 to get the avg.
    result=Vector2Subtract(b.pos,result); //get diff
    result=Vector2Scale(result,cFactor); //scale by cFactor
    return result; 
}
Vector2 alignment(Boid &b) //calc. alingment, offset from avg. vel of other boids
{
    Vector2 result = {0,0}; //result var.
    for(Boid & b2 : boids) //loop through boids
    {
        if(&b2!=&b) //skip self
        {
            result=Vector2Add(result,b2.vel); //add to sum
        }
    }
    result=Vector2Scale(result,1/(boids.size()-1)); //divide by size-1 to get the avg.
    result=Vector2Subtract(b.vel,result); //get diff
    result=Vector2Scale(result,aFactor); //scale by aFactor
    return result; 
}
Vector2 separation(Boid &b) //calc. separation, force acting as collision
{
    Vector2 result = {0,0}; //result var.
    for(Boid & b2 : boids) //loop through boids
    {
        if(&b2!=&b) //skip self
        {
            if(distance(b.pos,b2.pos)< sDist) //if close enough:
            {
                result=Vector2Subtract(result,Vector2Subtract(b2.pos,b.pos)); //add pos. diff. to sum
            }
        }
    }
    result=Vector2Scale(result,sFactor); //apply sFactor
    return result; 
}
Vector2 pull(Boid &b) //pull, force pulling towards mouse
{
    Vector2 m = GetMousePosition(); //mouse
    return Vector2Scale({(m.x-b.pos.x),(m.y-b.pos.y)},pow(distance(b.pos,m),-2)*pFactor); 
    //formula: |p|=pFactor*r^-2 p points from boid to mouse, r: distance between mouse and boid, inspired by Coulomb's law (f=kQq/r^2)
}

class Game //game class, systemizes the simulation
{
    public:
        void draw() //render
        {
            BeginDrawing();
                ClearBackground({0,0,50,255}); //clear
                for(Boid & boid : boids) //loop through boids
                {
                    boid.draw(); //rendering of boid defined inside method
                    //DrawLine(boid.pos.x,boid.pos.y,GetMousePosition().x,GetMousePosition().y,WHITE); //debug thing
                }
            EndDrawing();  
        }
        void step() //update
        {   
            for(Boid & boid : boids) //loop through boids
            {
                boid.step(); //update boid
            }
        }
        void endStep() //post-update, pre-render
        {
            for(Boid & boid : boids) //loop through boids
            {
                boid.endStep(); //post-update method
            }
        }
};

int main() //main
{
    srand(time(NULL)); //set seed of rand()
    
    InitWindow(SCREEN_WIDTH,SCREEN_HEIGHT,"Jellyfishes"); //make window
    SetTargetFPS(60); //set fps to 60
    Game game; //init. simulation

    loadTextures(); //load textures

    for(int i = 0; i<128; i++) //make boids
    {
        boids.push_back(Boid{}); //add boid
    }
    boids.shrink_to_fit(); //boids array is static after this, free pre-allocated memory that won't be needed
    //std::cout << sizeof(Boid)*128 << ": size of boids array.\n"; //debug, check if array has been made correctly
    
    while(!WindowShouldClose()) //game loop
    {
        game.step(); //update
        game.endStep(); //post-update
        game.draw(); //render
    }

    unloadTextures(); //unload textures

    CloseWindow(); //close
    return 0; //end
}

/* 
TODO //i planned to add these, but i never did
-controls
--add jellyfish
--remove jellyfish
--tweak factors
*/
