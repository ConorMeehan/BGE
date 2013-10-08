#include<sdl.h>
#include <windows.h>
#include <GL/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include<glm.hpp>
#include "Game.h"
#include "TriangleTest.h"
#include "TexturedCube.h"
#include "CubeTest.h"
#include "ModelTest.h"
#include "SteeringGame.h"
#include "Lab4.h"

using namespace BGE;

int main(int argc, char *argv[])
{
	
	//SteeringGame game;

	// Uncomment this line to run the Rift/Connect/Physics demo as per 
	// http://www.youtube.com/watch?v=EEbVHxOkTxw
	//SteeringGame game;

	Lab4 game;

	game.Run();

	return 0;
}