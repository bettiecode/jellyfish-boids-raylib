#include "include/raylib.h"

const int SCREEN_WIDTH = 1024; 
const int SCREEN_HEIGHT = 768;
const Color BG_COLOR = {0,0,60,255};
const Color ICE_COLOR = {160,160,255,255};

float aFactor = 1/1600; //aligmnent factor
float cFactor = 1/6400; //cohesion factor
float sFactor = 0.1; //separation factor
float sDist = 16; //collison range
float pFactor = 4; //pull factor