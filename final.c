// Final Project
// Fund Comp - Final Project - Rocket Game
// Danny Jasek and Eric Krebs

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "gfx5.h"
#define WID 800
#define HT 600
#define NUMLINES 2

// struct for spaceship
struct Ship
{
    double x;
    double y;
    double theta;
    int radius;
    double x_vel;
    double y_vel;
    double accel;
    double max_vel;
    double drag;
    double scrollSpeed;
    double gravity;
    XPoint body[10]; //the 10 points of the spaceship body (double up first)
    XPoint fin1[5];
    XPoint fin2[5];
    XPoint keyPt[9]; // points to check in case of a collision
};

// struct for each line
struct Rect
{
    int ht;
    int xPos; // the x coordinate of the center of the gap
    double yPos;
    double vel;
};

// Function Prototypes:

// lines:
void initLines(struct Rect lines[NUMLINES]);
void drawLines(struct Rect lines[NUMLINES]);

// ship:
void setUpShip(struct Ship* ship);
void keyboardProcessing(int event, char *arr, int *sp, int clear);
void rotateShip(struct Ship* ship, char key);
void advanceShip(struct Ship* ship, int space, struct Rect lines[NUMLINES]);
void updateShipPoints(struct Ship* ship);
void drawShip(struct Ship ship);
void bring_in_bounds(struct Ship* ship);

// collision detection:
int collision(struct Rect lines[NUMLINES], struct Ship* ship);

// score tracking
void dispScore(int score);
char *num2str(int n);
void getScoreList(int scores[10], int *listLen);
void updateScoreList(int score, int scores[10], int *listLen);
void writeScoreList(int scores[10], int *listLen);
void dispScoresGFX(int scores[10]);

// smoke
void drawSmoke(int smokePts[2][400], struct Ship ship, int space); 
void clearSmoke(int smokePts[2][400]);

// stars
void initStars(int starPts[2][100]);
void drawStars(int starPts[2][100]);

// end of game procedure
int endGame();

int main()
{
   gfx_open(WID, HT, "Spaceship Game");
   struct Rect lines[NUMLINES];
   struct Ship ship; // creates instance of Ship : "ship"

   int quit = 0;
   int scores[10] = {0}; // array of scores
   int listLen = 0;
   int starPts[2][100]; // locations of stars
   int smokePts[2][400] = {0}; // locations of smoke points

   getScoreList(scores, &listLen); // opens and reads in scores file

   while(!quit)
   {
      // creates array of line structs
      initLines(lines);
      setUpShip(&ship);

      // seeds random number generator
      srand(time(NULL));

      // for ship:
      int event=0; // event returned by gfx_event_waiting
      char key = 0;
      int space = 0;
      char arrow = 0;
      int gameOver = 0;
      double score = 0;

      // sets up array of star points
      initStars(starPts);
  
      while(!gameOver)
      {
         event = gfx_event_waiting();
         keyboardProcessing(event, &arrow, &space, 0);
         if(arrow)
         {
            rotateShip(&ship, arrow);
         }
         advanceShip(&ship, space, lines);

         // if the ship hits a line, the game is over
         if(collision(lines, &ship))
         {
            gameOver = 1;
         }

         score += .01;
         
         // draws everything
         gfx_clear();
         drawStars(starPts);
         drawSmoke(smokePts, ship, space);
         drawLines(lines);
         updateShipPoints(&ship);
         drawShip(ship);  
         dispScore(score);
         gfx_flush();

         usleep(10000);
      }

      updateScoreList(score, scores, &listLen);
      dispScoresGFX(scores);
      quit = endGame();
      keyboardProcessing(2, &arrow, &space, 1); // clear static variables in func
      clearSmoke(smokePts);
   }
   writeScoreList(scores, &listLen); // writes new high scores to file when the user quits the game
}


// set initial properties of each line
void initLines(struct Rect lines[NUMLINES])
{
   int i, edge;
   for(i=0; i< NUMLINES; i++)
   {
      lines[i].xPos =  rand()%(WID*3/4) + WID/8; 
      lines[i].ht = 10;
      lines[i].yPos =(i*HT/NUMLINES)-(HT*(NUMLINES-1)/NUMLINES); // each line begins off the screen
      lines[i].vel = 1.0;
   }
}

// loops through array of lines, draws them and updates their position
void drawLines(struct Rect lines[NUMLINES])
{
   int j;
   for(j=0; j<NUMLINES; j++)
   {
      gfx_color(0, 200, 0);
      if(lines[j].yPos > 0) // only displays lines that are within the window
      {
         // draws the line on both sides of the gap
         gfx_fill_rectangle(0, lines[j].yPos, lines[j].xPos-WID/16, lines[j].ht);
         gfx_fill_rectangle(lines[j].xPos+WID/16, lines[j].yPos, WID-lines[j].xPos+WID/16, lines[j].ht);
      }

      lines[j].yPos+=lines[j].vel; // advances lines
      lines[j].vel += .0002; // lines gradually move faster

      // if a line goes off the screen, bring it back to the top
      if(lines[j].yPos > HT)
      {
         lines[j].xPos = rand()%(WID*3/4) + WID/8;
         lines[j].yPos=0;
      }
   }
}

// funciton that detects of the spaceship has hit any of the lines on the screen
int collision(struct Rect lines[NUMLINES], struct Ship* ship)
{
   int n;
   int i;
   for(n=0; n<9; n++)
   {
      for(i=0; i<NUMLINES; i++)
      {
         double top = lines[i].yPos;
         double bottom = lines[i].yPos + lines[i].ht;

         // if ship is above the bottom of a line and below the top of the line
         // -- checks when ship comes from both the top and the bottom
         if(ship->keyPt[n].y < bottom && ship->keyPt[n].y > top) {
            // if ship is not within the gap
            if(ship->keyPt[n].x <= lines[i].xPos - WID/16 ||
            ship->keyPt[n].x >= lines[i].xPos + WID/16)
            {
               return 1; // collision detected
            }
         }
      }
   }
   return 0;
}

// displays end of game messages, returning 1 if the user wants to quit, 0 if not
int endGame()
{
   gfx_color(255, 255, 255); 
   gfx_text(WID/2-50, HT/2, "Game Over");
   gfx_text(WID/2-130, HT/2 + 50, "Would you like to play again? (Y/N)");
   gfx_flush(); 

   // waits until user chooses to play again or not
   while(1)
   {
      char c = gfx_wait();
      if(c=='Y' || c=='y')
      { 
         return 0; // play again
      }
      else if(c=='N' || c=='n')
      {
         return 1; // end program
      }
   }
}

// brings ship back in bounds if it is out of bounds
void bring_in_bounds(struct Ship* ship)
{
   if(ship->x - ship->radius < 0)      
   {
      ship->x = 0 + ship->radius;
   }
   if(ship->x + ship->radius > WID)
   {
      ship->x = WID - ship->radius;
   }
   if(ship->y - ship->radius < 0)
   {
      ship->y = 0 + ship->radius;
   }
   if(ship->y + ship->radius > HT)
   {
      ship->y = HT - ship->radius;
   }
}

// displays score to user in the corner of the screen
void dispScore(int score)
{  
   gfx_color(255, 255, 255);
   gfx_text(25, 25, "Score:");
   gfx_text(75, 25, num2str(score));
}

// sets up pointer to file and reads in array of high scores, updating the length of the list variable
void getScoreList(int scores[10], int *listLen)
{
   FILE *ifp;
   ifp = fopen("scores.txt", "ab+");
   char string [10][10]; // array of strings to store file input at first
   int i=0;

   // reads in scores from file
   while(!feof(ifp))
   {
      fgets(string[i], 9, ifp);
      scores[i] = atoi(string[i]);
      i++;
   }

   *listLen = i; // "listLength" of file (overcounted by 1)
}

// function that stores new scores in array and sorts it properly
void updateScoreList(int score, int scores[10], int *listLen)
{
   int i, j, greater = 0;

   // if the scores array is not full yet
   if(*listLen < 11)
   {
      *listLen+=1;
      // an insertion type sort
      for(i=0; i<*listLen-2; i++)
      {
         if(score > scores[i])
         {
            for(j=*listLen-2; j > i; j--)
            {
               scores[j] = scores[j-1];
            }
            scores[i] = score;
            greater = 1; // flag variable that keeps track if the new score belongs on the list
            break;
         }
      }

      // if new score is not greater than any old score, puts it at the end
      if(!greater)
      {
         scores[*listLen-2] = score;
      }
   }

   // if array is already full
   else
   {
      // slightly different insertion type sort
      for(i=0; i<*listLen-1; i++)
      {
         if(score > scores[i])
         {
            for(j=*listLen-2; j > i; j--)
            {
               scores[j] = scores[j-1];
            }
            scores[i] = score;
            break;
         }
      }
   }
}

// at the end of the program, this function writes all of the sorted scores
// to the file and displays them to the user
void writeScoreList(int scores[10], int *listLen)
{
   FILE *ofp;
   int i;

   // writes scores array to file
   ofp = fopen("scores.txt", "w");
   for(i=0; i<*listLen-1; i++)
   {
      fprintf(ofp, "%d\n", scores[i]);
   }
}

// displays what is in the scores array to the screen via gfx
void dispScoresGFX(int scores[10])
{
   int i;
   char string[4];

   gfx_text(WID-90, HT-220, "High Scores");

   for(i=0; i<9; i++)
   {
      gfx_text(WID-75, HT-200+(20*i), num2str(i+1));
      gfx_text(WID-70, HT-200+(20*i), ".");
      gfx_text(WID-50, HT-200+(20*i), num2str(scores[i]));
   }

   // slightly different procedure for the number 10, because it is 2 digits long
   gfx_text(WID-80, HT-200+(20*9), num2str(10));
   gfx_text(WID-70, HT-200+(20*9), ".");
   gfx_text(WID-50, HT-200+(20*9), num2str(scores[9]));
}

// function Prof Bualuan provided that turns integer into a string
char *num2str(int n)
{
  static char a[10], *p = a;
  snprintf(p, 10, "%d", n);
  return p;
}

// constants of the ship when the game starts:
void setUpShip(struct Ship* ship)
{
    ship->x = 300;
    ship->y = 400;
    ship->theta = -M_PI/2;
    ship->radius = 10;
    ship->x_vel = 0;
    ship->y_vel = 0;
    ship->accel = .2;
    ship->max_vel = 2;
    ship->drag = 1.02; // factor at which rocket slows down
    ship->gravity = 0.02;
}

// allows for continual return of space and arrow values even when gfx_event_
// waiting is returning only one or the other:
void keyboardProcessing(int event, char *arr, int *sp, int clear)
{
    char key;
    static char arrow; // static so they don't turn off if return is to other
    static int space;
    if(clear)
    {
        arrow = 0;
        space =0;
    }
    else if(event==1) // key pressed
    {
            key = gfx_wait();
            if(key == ' ')
              space = 1;
            if(key >= 'Q' && key <= 'T')
              arrow = key;
    }
    else if(event==2) // key released
    {
            key = gfx_wait();
            if(key == ' ')
              space = 0;
            if(key == 'Q' || key == 'S')
              arrow = 0;
    }
    else if(event) // clears junk input
    {
            gfx_wait();
    }
        *arr = arrow; // updates pointers to variables accessible in main
        *sp = space;
} 

// rotates ship based off theta value
void rotateShip(struct Ship* ship, char key)
{
    switch(key) // arrow key
    {
        case 'Q':
            ship->theta = ship->theta - .03;//reverse to account for y being (-)
            break;
        case 'S':
            ship->theta = ship->theta + .03;
            break;
    }   
}

// moves the ship, applies max speed and gravity and drag
void advanceShip(struct Ship* ship, int space, struct Rect lines[NUMLINES])
{
    double vel;
    ship->max_vel = 1 + lines[0].vel; // max_vel increase with game progression
    if(space) // when accelerating
    {
          ship->x_vel = ship->x_vel + ship->accel*cos(ship->theta);
          ship->y_vel = ship->y_vel + ship->accel*sin(ship->theta);
          vel = pow(pow(ship->x_vel, 2) + pow(ship->y_vel,2), .5);
          if(vel > ship->max_vel) // apply max_vel limit
            {
              ship->x_vel = ship->max_vel/vel*ship->x_vel;
              ship->y_vel = ship->max_vel/vel*ship->y_vel;
            }
    }
    else // deccelating
    {
            ship->x_vel = ship->x_vel/ship->drag;
            ship->y_vel = ship->y_vel/ship->drag;
    }
    ship->y_vel += ship->gravity; // apply gravity
    ship->x += ship->x_vel; // new x-position
    ship->y += ship->y_vel + lines[0].vel; // new y-position
    bring_in_bounds(ship); // if out of bounds, place in bounds
}
 
// update ship coordinates:
void updateShipPoints(struct Ship* ship)
{
    int n;
    int t;
    double bodyPt[] = {20.0, 15.24, 8.94, 8.94, 14.56, 
                      14.56, 8.94, 8.94, 15.24, 20.0}; // dist from center
    double bodyAngle[] = {0.00, 0.40, 1.11, 2.03,  2.86, 
                         -2.86, -2.03, -1.11, -0.40, 0.0}; // angle from center
    double finPt[] = {8.94, 14.42, 19.70, 14.56, 8.94}; // dist
    double finAngle[] = {2.03, 2.15, 2.72, 2.86, 2.03}; // angle
    int key[5] = {0, 1, 2, 7, 8}; // checked for collisions
    // update body points
    for(n=0; n<10; n++) // from cone down left side up right side
    {
        ship->body[n].x = ship->x + bodyPt[n]*cos(ship->theta + bodyAngle[n]);
        ship->body[n].y = ship->y + bodyPt[n]*sin(ship->theta + bodyAngle[n]);
    }
    // update points on fin1 and fin2:
    for(n=0; n<5; n++)
    {
        ship->fin1[n].x = ship->x + finPt[n]*cos(ship->theta + finAngle[n]);
        ship->fin1[n].y = ship->y + finPt[n]*sin(ship->theta + finAngle[n]);
        ship->fin2[n].x = ship->x + finPt[n]*cos(ship->theta - finAngle[n]);
        ship->fin2[n].y = ship->y + finPt[n]*sin(ship->theta - finAngle[n]);
    }
    // update keyPts (which are checked for collisions)
    for(n=0; n<5; n++) // 5 keyPts in body
    {
        ship->keyPt[n].x = ship->body[key[n]].x;
        ship->keyPt[n].y = ship->body[key[n]].y;
    }
    // 4 keyPoints in fins:
    ship->keyPt[5].x = ship->fin1[1].x;
    ship->keyPt[5].y = ship->fin1[1].y;
    ship->keyPt[6].x = ship->fin1[2].x;
    ship->keyPt[6].y = ship->fin1[2].y;
    ship->keyPt[7].x = ship->fin2[1].x;
    ship->keyPt[7].y = ship->fin2[1].y;
    ship->keyPt[8].x = ship->fin2[2].x;
    ship->keyPt[8].y = ship->fin2[2].y;
}

// draws the ship around the center point
void drawShip(struct Ship ship)
{
    double x;
    double y;
    double r;
    double theta;
    x = ship.x;
    y = ship.y;
    r = ship.radius;
    theta = ship.theta;
    // classic red:
    gfx_color(255, 0, 0);
    gfx_fill_polygon(ship.body, 10);
    // white:
    gfx_color(255, 255, 255);
    gfx_polygon(ship.body, 10);
    gfx_fill_polygon(ship.fin1, 5);
    gfx_fill_polygon(ship.fin2, 5);
}

// sets up array of 100 random star points within an array
void initStars(int starPts[2][100])
{
   int i;
   for(i=0; i<100; i++)
   {
      starPts[0][i] = rand()%WID;
      starPts[1][i] = rand()%HT;
   }
}

// draws smoke trail:
void drawSmoke(int smokePts[2][400], struct Ship ship, int space)
{
    static int n=0;
    int i;
    int r;
    if(n%4 == 0) // only add point every 4 times
    {
        if(n>399) // keep n in boundary of array
        {
            n=0;
        }

        if(space) // more smoke when accelerating
        {
            smokePts[0][n] = ship.x + 14.5*cos(ship.theta + 3.14); // draws central
            smokePts[1][n] = ship.y + 14.5*sin(ship.theta + 3.14); // points
            for(i=1; i<4; i++) // draws 3 random point
            {
                smokePts[0][n+i] = smokePts[0][n] + rand()%11 - 5;
                smokePts[1][n+i] = smokePts[1][n] + rand()%11 - 5;
            }
        }
        else // draws 1 random point behind rocket
        {
            smokePts[0][n] = ship.x + 14.5*cos(ship.theta+3.14) + rand()%7 - 3;
            smokePts[1][n] = ship.y + 14.5*sin(ship.theta+3.14) + rand()%7 - 3;
            for(i=1; i<4; i++) // and 3 blank points
            {
                smokePts[0][n+i] = 0;
                smokePts[1][n+i] = 0;
            }
        }
    }
    n++;
    gfx_color(100, 100, 100); // grey
    for(i=0; i<400; i++)
    {
        if(smokePts[0][i]>0) // blank points not drawn
        {
            gfx_point(smokePts[0][i], smokePts[1][i]);
            smokePts[1][i] += 1; // moves smoke at constant speed
        }
    }
}

// clear smoke when game is reset:
void clearSmoke(int smokePts[2][400])
{
    int i;
    int j;
    for(i=0; i<2; i++)
    {
        for(j=0; j<400; j++)
        {
            smokePts[i][j] = 0;
        }
    }
}

// draws stars (white point) at each point in the starPts array
void drawStars(int starPts[2][100])
{
   int i;
   gfx_color(255, 255, 255);
   for(i=0; i<100; i++)
   {
      gfx_point(starPts[0][i], starPts[1][i]);
   }
}

