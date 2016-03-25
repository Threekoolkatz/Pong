/**************************************************************************************************
Author:  Mackenzie Smith

Project:  Pong Game

Due Date:  September 30th

Professor:  John Weiss

Course:  CSC 433 - Computer Graphics

Description:

	This is a very basic Pong game using keyboard input to control both left and right paddles, 
keeping track of each players score and exiting upon either player reaching 10 points.  The 
"ASDW" keys control the left player, while the arrow keys control the right player.  Both paddles
as well as the pong ball are stored as global structs to allow the callback functions to access and 
modify their x and y coords and velocities.  This program implements a timer function to handle the
animations.  

I did not split this program into multiple files unfortunately.  

Bugs:

 - Sometimes the paddles will not register collision with the court boundaries or net and will move
   through them.  MOST of the time they dont, but every once in a while they will clip through them

 - The ball will hit the top or bottom of each paddle and decide whether it wants to do what its 
   supposed to.  Sometimes when it hits the top it will bounce back up like its supposed to, other 
   times it wont.  Same with hitting the bottom of the paddles.

******************************************************************************************************/

#include <GL/freeglut.h>
#include <cmath>

using namespace std;

// world coordinate window extents: -1000 to +1000 in smaller dimension
const float ViewplaneSizeX = 2000.0;
const float ViewplaneSizeY = 1000.0;
const int FRAMES_PER_MILLISECOND = 1;

//Initial window size
int Screenwidth = 1000;
int Screenheight = 500;

void initProgram ( );

// Score tracker
int rightScore = 48;
int leftScore = 48;

// Serve alternator
bool serve = false;

// Callback function prototypes
void display ( );
void keyboard ( unsigned char key, int x, int y );
void reshape ( int w, int h );
void idle ( );
void specialInput ( int key, int x, int y );
void specialUp ( int key, int x, int y );
void keyboardUp ( unsigned char key, int x, int y );
void anim ( int val );

//Object display functions
void displayNet ( );
void displayBorder ( );
void displayPaddle ( float x1, float y1, float x2, float y2 );
void displayBall ( float x, float y, float radius );
void displayScore ( );

/*********************************************
Author: Mackenzie Smith

Global struct to hold the x,y coords and the 
x and y velocities of both paddles.  Uses a 
constructor for easy initialization.
**********************************************/
struct Paddle
{
	//stores dimensions and coordinates
	float x, y, w, h;

	//velocity for paddle
	float vx, vy;

	//constructor
	Paddle ( float px, float py, float pw, float ph, float pvx, float pvy ) : x( px ), y( py ), w( pw ), 
																			  h( ph ), vx( pvx ), vy ( pvy ){}
};

/*********************************************
Author: Mackenzie Smith

Global struct to hold the x,y coords and the 
x and y velocities of the ball.  Uses a 
constructor for easy initialization.
**********************************************/
struct Ball
{
	//X and Y coordinates and radius
	float x, y;
	float radius;

	//X and Y velocity coordinates
	float vx, vy;

	//sees if game is paused or not
	bool pause;

	//constructor
	Ball ( float px, float py, float pradius, float pvx, float pvy ) : x ( px ), y ( py ), radius ( pradius ), vx (pvx), vy (pvy){}
};

//Global structs for keeping track of positions
Paddle left ( -1500, -150, 50, 500, 0, 0 );
Paddle right ( 1500, -150, 50, 500, 0, 0 );
Ball ball ( 0, 0, 50, 0.5, 0.5 ); 

/*********************************************
Author: Mackenzie Smith

Where the initializing function is called
and where the glutMainloop is called to never
return from.
**********************************************/
int main ( int argc, char **argv )
{
	glutInit ( &argc, argv );
	initProgram ( );

	//Enters glut main loop. the point of no return
	glutMainLoop ();

	return 0;
}

/****************************************************
Author:  Mackenzie Smith

Displays the various polygons and lines in the game.
The ball, paddles, scores, net, and court borders.
*****************************************************/
void display ( )
{
	glClear ( GL_COLOR_BUFFER_BIT );

	displayNet ();
	displayBorder ();

	displayPaddle ( left.x, left.y, (left.x + left.w), (left.y + left.h) );
	displayPaddle ( right.x, right.y, (right.x + right.w), (right.y + right.h) );
	displayBall ( ball.x, ball.y, ball.radius );
	displayScore (  );

	glutSwapBuffers ();
	glFlush();
}

/***************************************************
Author: Mackenzie Smith

Initiallizes the various callback functions as well
as the intial windowsize and viewport.
****************************************************/
void initProgram ( )
{
	glutInitDisplayMode ( GLUT_RGBA | GLUT_DOUBLE ); // 32 - bit color and double buffering
	
	//Initial window and viewport sizes
	glutInitWindowSize ( Screenwidth, Screenheight );
	glutInitWindowPosition ( 100, 150 );
	glutCreateWindow ( "Pong" );
	glViewport ( 0, 0, Screenwidth, Screenheight );

	glClearColor ( 0, 0, 0, 1 );

	//Callback initializers
	glutDisplayFunc ( display );
	glutReshapeFunc ( reshape );
	glutKeyboardFunc ( keyboard );
	glutSpecialFunc ( specialInput );
	glutKeyboardUpFunc ( keyboardUp );
	glutSpecialUpFunc ( specialUp );

	glutTimerFunc ( FRAMES_PER_MILLISECOND, anim, 0 );
	//glutIdleFunc ( idle );  Didnt end up using this one because timer functioned more smoothly



}

/*****************************************************************************
Author:  John Weiss - tinkered a little bit to reshape rectangle by Mackenzie Smith

Handles the reshaping of the view plane by changing the ViewplaneSize x and y 
coords when the user clicks on the sides of the glut window.
******************************************************************************/
void reshape ( int w, int h )
{
	// store new window dimensions globally
    Screenwidth = w;
    Screenheight = h;

    
    glMatrixMode( GL_PROJECTION );      
    glLoadIdentity();                   
    if ( w > ViewplaneSizeX / ViewplaneSizeY * h )                       
        gluOrtho2D( -ViewplaneSizeY * w / h, ViewplaneSizeY * w / h, -ViewplaneSizeY, ViewplaneSizeY );
    else
        gluOrtho2D( -ViewplaneSizeX, ViewplaneSizeX, -ViewplaneSizeX * h / w, ViewplaneSizeX * h / w );
    glViewport( 0, 0, w, h );           

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

}

/***************************************************
Author: Mackenzie Smith

Processes down-keypresses from the ascii keyboard keys
ESC exits program, space pauses the ball velocity, and
'aswd' moves the left paddle.  Right paddle is controlled
by specialInput.
***************************************************/
void keyboard ( unsigned char key, int x, int y )

{
	float xVelocity = 1.0, yVelocity = 1.0;


	//processes key presses
	switch ( key )
	{
		case 27:	// when escape key is pressed
			exit ( 0 );
			break;

		case 32:
			if ( ball.pause == false )
				ball.pause = true;
			else 
				ball.pause = false;
			break;

		case 97:	// when 'a' is pressed, if on edge doesnt move
			if ( left.x + left.vx <= -1900 )
				left.vx = 0;
			else
				left.vx = -xVelocity;
			break;

		case 115:	// when 's' is pressed
			if ( left.y + left.vy <= -950 )
				left.vy = 0;
			else
				left.vy = -yVelocity;
			break;

		case 100:	// when 'd' is pressed
			if ( (left.x + left.w) + left.vx >= -10 )
				left.vx = 0;
			else
				left.vx = xVelocity;
			break;

		case 119:	//when 'w' is pressed
			if ( (left.y + left.h) + left.vy >= 950 )	// if paddle reaches top of court
				left.vy = 0;
			else
				left.vy = yVelocity;
			break;

	}
}

/***********************************************
Author: Mackenzie Smith

The callback function that gets input from arrow
key down presses controlling the right paddle.
************************************************/
void specialInput ( int key, int x, int y )
{
	float moveFactor = 1.0;

	switch (key)
	{
			//up key press, stops at ceiling
		case GLUT_KEY_UP:
			if ( (right.y + right.h) + right.vy  >= 950  )
				right.vy = 0;
			else
				right.vy = moveFactor;
			break;

			//left key press, stops at net
		case GLUT_KEY_LEFT:
			if ( right.x + right.vx <= 10 )
				right.vx = 0;
			else
				right.vx = -moveFactor;
			break;

			//right key press, stops on right edge
		case GLUT_KEY_RIGHT:
			if ( (right.x + right.w) + right.vx > 1900 )
				right.vx = 0;
			else
				right.vx = moveFactor;
			break;

			//down key press, stops on bottom edge
		case GLUT_KEY_DOWN:
			if ( right.y + right.vy <= -950 )
				right.vy = 0;
			else
				right.vy = -moveFactor;
			break;

			//page up key increases ball speed
		case GLUT_KEY_PAGE_UP:
			ball.vx *= 1.1;
			break;
	}
}

/************************************************
Author:  Mackenzie Smith

Callback for input from 'aswd' upstrokes
*************************************************/
void keyboardUp ( unsigned char key, int x, int y )
{
	switch ( key )
	{
		case 97:
		case 100:
			left.vx = 0;
			break;
			
		case 115:
		case 119:
			left.vy = 0;
	}
}

/*****************************************************
Author:  Mackenzie Smith

The callback that takes input from the
arrow keys on upstrokes
******************************************************/
void specialUp ( int key, int x, int y)
{
	switch ( key )
	{

		case GLUT_KEY_UP:
		case GLUT_KEY_DOWN:
			right.vy = 0;

		case GLUT_KEY_LEFT:
		case GLUT_KEY_RIGHT:
			right.vx = 0;

	}
}

/*********************************************************************
Author:  Mackenzie Smith

The animation callback i used before i implemented the timer function
**********************************************************************/
void idle ( )
{
	// if game is paused, ball ceases to move 
	if ( ball.pause == false )
	{
		// adds velocity to ball coordinates to create movement
		ball.x += ball.vx;
		ball.y += ball.vy;

		// adds velocity to left paddle to move it
		left.x += left.vx;
		left.y += left.vy;

		// if ball hits side walls, changes directions
		if ( ball.x == 1975 || ball.x == -1975 )
			ball.vx = -ball.vx; // will eventually count points

		// if ball hits top or bottom, changes direction
		if ( ball.y == 975 || ball.y == -975 )
			ball.vy = -ball.vy;

		// if ball hits the right paddle, reverses x direction
		if ( ball.x == right.x && ball.y >= right.y && ball.y <= ( right.y + right.h ) )
			ball.vx = -ball.vx;

		// if ball hits left paddle, reverses x direction
		if ( ball.x == left.x && ball.y >= left.y && ball.y <= ( left.y + left.h ) )
			ball.vx = -ball.vx;
			
		
		//need to implement reversing y direction on the top and bottom of paddles
	}


	glutPostRedisplay();
} // turns out i didnt need this 

/************************************************************************
Author: Mackenzie Smith

Description:  

This function covers all of the animation.  It moves the
paddles and ball according to each of their velocities, stored in global
variables.  It registers goals for each side and increments the scores.
Calls timer callback function
*************************************************************************/
void anim ( int val )
{
	// if either score reaches 10
	if ( leftScore >= 58 || rightScore >= 58 )
		exit (0);

	// adds velocity to left paddle to move it
	left.x += left.vx;
	left.y += left.vy;

	// adds velocity to right paddle to move it
	right.x += right.vx;
	right.y += right.vy;

	
	// if game is paused, ball ceases to move 
	if ( ball.pause == false )
	{
		// adds velocity to ball coordinates to create movement
		ball.x += ball.vx;
		ball.y += ball.vy;

		// if ball hits right goal, counts score
		if ( ball.x + ball.vx >= 1975 )
		{
			leftScore++;
			ball.x = 0;
			ball.y = 0;

			right.h = 500;
			left.h = 500;
			
			//resets paddles to original points
			right.x = 1500;
			right.y = -150;
			left.x = -1500;
			left.y = -150;

			if ( serve == true )
			{
				ball.vx = 0.5;
				serve = false;
			}
			else 
			{
				ball.vx = -0.5;
				serve = true;
			}
			ball.vy = 0.5;
			ball.pause = true; // resets the ball coordinates and pauses game
		}

		// if ball reaches left goal
		if ( ball.x + ball.vx <= -1975 )
		{
			rightScore++;
			ball.x = 0;
			ball.y = 0;

			right.h = 500;
			left.h = 500;

			//resets paddles to original points
			right.x = 1500;
			right.y = -150;
			left.x = -1500;
			left.y = -150;

			if ( serve = true )
			{
				ball.vx = 0.5;
				serve = false;
			}
			else 
			{
				ball.vx = -0.5;
				serve = true;			
			}
			ball.vy = 0.5;
			ball.pause = true; // resets ball coords and pauses game
		}

		// if ball hits top or bottom, changes direction
		if ( ball.y + ball.vy == 975 || ball.y + ball.vy <= -975 )
			ball.vy = -ball.vy;

		// if ball hits the right paddle, reverses x direction
		if ( right.x + right.vx >= ball.x + ball.vx && ball.x + ball.vx >= right.x - right.w + right.vx )
		{
			//  if ball hits bottom stil -vy
			if ( ball.y + ball.vy >= right.y + right.vy && ball.y + ball.vy <= ( right.y + right.h ) / 3.0 ) 
			{
				ball.vy = abs( ball.vy ) *  -1;
				right.h -= 50;
				ball.vx = -ball.vx * 1.5;
			}
			// if ball hits top of paddle change vy to positive
			if ( ball.y + ball.vy >= (right.y + right.h) / 1.5 && ball.y + ball.vy <= right.y + right.h ) 
			{
				ball.vy = abs ( ball.vy );
				right.h -= 50;
				ball.vx = -ball.vx * 1.5;
			}
			// if ball hits middle changes vy to pos
			if ( ball.y + ball.vy >= ( right.y + right.h ) / 3.0  && ball.y + ball.vy <= (right.y + right.h) / 1.5 ) 
			{
				ball.vx = -ball.vx;
				right.h -= 50;
			}

		}

		// if ball hits left paddle, reverses x direction
		if ( left.x + left.vx <= ball.x + ball.vx && ball.x + ball.vx <= (left.x + left.w) + left.vx )
		{
			// if ball hits bottom of paddle -vy
			if ( ball.y + ball.vy >= left.y + left.vy && ball.y + ball.vy <= (left.y + left.h) / 3.0 ) 
			{
				ball.vy = abs(ball.vy) * -1;
				left.h -= 50;
				ball.vx = -ball.vx * 1.5;
			}
			
			// if ball hits middle of paddle
			if ( ball.y + ball.vy >= ( left.y + left.h ) / 3.0 && ball.y + ball.vy <= (left.y + left.h) / 1.5 )
			{
				left.h -= 50;
				ball.vx = -ball.vx * 1.5;
			}

			// if ball hits top of paddle vy is positive
			if ( ball.y + ball.vy >= ( left.y + left.h ) / 1.5 && ball.y + ball.vy <= (left.y + left.h) )
			{
				left.h -= 50;
				ball.vx = -ball.vx * 1.5;
				ball.vy = abs(ball.vy);
			}
		}
			
	}
	glutPostRedisplay();
	glutTimerFunc ( FRAMES_PER_MILLISECOND, anim, 0);

}

/********************************
Author: Mackenzie Smith

Displays the centerline net
*********************************/
void displayNet ( )
{

	glColor3f ( 1, 1, 1 );
	glLineStipple ( 2, 0xAAAA );
	glEnable ( GL_LINE_STIPPLE );

	glBegin ( GL_LINES );
		glVertex2i ( 5, -975 );
		glVertex2i ( 5, 975 );
	glEnd ();

	glFlush ();
}

/*****************************************
Author: Mackenzie Smith

Displays both left and right side paddles
******************************************/
void displayPaddle ( float x1, float y1, float x2, float y2 )
{
	glDisable(GL_LINE_STIPPLE);

	glColor3f ( 1, 1, 1 );
	glBegin ( GL_POLYGON );
		glVertex2f ( x1, y1 );
		glVertex2f ( x1, y2 );
		glVertex2f ( x2, y2 );
		glVertex2f ( x2, y1 );
	glEnd ();
}

/*******************************
Author: Mackenzie Smith

Displays the court boundaries
********************************/
void displayBorder ( )
{
	glDisable ( GL_LINE_STIPPLE );
	glColor3f ( 1, 1, 1 );

	glBegin ( GL_LINE_LOOP );
		glVertex2i ( -1975, -975 );
		glVertex2i ( -1975, 975 );
		glVertex2i ( 1975, 975 );
		glVertex2i ( 1975, -975 );
	glEnd ();
	
}

/**************************************
Author: John Weiss copied from anim.cpp

Displays the ball

***************************************/
void displayBall ( float x, float y, float radius )
{
	glColor3f ( 1, 1, 1 );
	glPushMatrix();
    glTranslatef( x, y, 0 );
	GLUquadricObj *disk = gluNewQuadric();
    gluDisk( disk, 0, radius, int( radius ), 1 );
    gluDeleteQuadric( disk );
    glPopMatrix();
}

/*************************************
Author: Mackenzie Smith

Displays the score for both sides

*************************************/
void displayScore ( )
{
	char left, right;

	left = leftScore;
	right = rightScore;

	glRasterPos2f ( -1000, 900 );
	glutBitmapCharacter ( GLUT_BITMAP_HELVETICA_18, left );

	glRasterPos2f ( 1000, 900 );
	glutBitmapCharacter ( GLUT_BITMAP_HELVETICA_18, right );

}



