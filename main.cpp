/*
 * MAZE
 * Camron Kelly
 * CSCI 115
 * May 11, 2018
 */

#include <string.h>
#include <CommonThings.h>
#include <Maze.h>
#include <iostream>
#include <Timer.h>
#include <player.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <Enemies.h>

#include <wall.h>
#include <math.h>
#include <fstream>
#include <string.h>
#include <sstream>
#include <queue>
#include <algorithm>
/* GLUT callback Handlers */

using namespace std;


Maze *M = new Maze();                         // Set Maze grid size
Player *P = new Player();                       // create player

wall W[400];                                    // wall with number of bricks
int numWalls;
Enemies E[10];                                  // create number of enemies
int numEnemies;
int liveEnemies;
Timer *T0 = new Timer();                        // animation timer
string endState;
float wWidth, wHeight;                          // display window width and Height
float xPos,yPos;                                // Viewpoar mapping


void display(void);                             // Main Display : this runs in a loop
void displayEnding(void);

void init(string mazeChoice)
{


    glEnable(GL_COLOR_MATERIAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0,0.0,0.0,0.0);
    gluOrtho2D(0, wWidth, 0, wHeight);

    T0->Start();                                        // set timer to 0

    glEnable(GL_BLEND);                                 //display images with transparent
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    string line;
    ifstream myfile(mazeChoice);
    int gSize;

    getline(myfile,line);
    stringstream ss(line);
    ss >> line >> gSize;

    if(line == "gridsize")
    {
        M->setGridSize(gSize);
    }

    M->loadBackgroundImage("images/bak.jpg");           // Load maze background image
    M->loadChestImage("images/chest.png");              // load chest image
    //code to extract chest location
    int x,y;

    do{

        getline(myfile,line);

        stringstream ss(line);
        ss >> line >> x >> y;
        if(myfile.eof())
            break;
    }while(line != "chest");
    myfile.clear();
    myfile.seekg(0,myfile.beg);
    getline(myfile,line);
    M->placeChest(x,y);                                 // place chest in a grid

    M->loadSetOfArrowsImage("images/arrwset.png");      // load set of arrows image
    //code to extract arrow location
    do{
        getline(myfile,line);
        stringstream ss(line);
        ss >> line >> x >> y;
        if(myfile.eof())
            break;
    }while(line != "arrows");

    myfile.clear();
    myfile.seekg(0,myfile.beg);
    getline(myfile,line);
    M->placeStArrws(x,y);                               // place set of arrows

    P->initPlayer(M->getGridSize(),6,"images/k.png");   // initialize player pass grid size,image and number of frames
    P->loadArrowImage("images/arr.png");                // Load arrow image

    //code to extract player location
    do{

        getline(myfile,line);
        stringstream ss(line);
        ss >> line >> x >> y;
        if(myfile.eof())
            break;
    }while(line != "player");

    myfile.clear();
    myfile.seekg(0,myfile.beg);
    getline(myfile,line);
    P->placePlayer(x,y);                                // Place player

    for(int i=0; i< 400;i++)
    {
      W[i].wallInit(M->getGridSize(),"images/wall.png");// Load walls
    }

    numWalls = 0;
    while(!myfile.eof())
    {
        getline(myfile,line);
        stringstream ss(line);
        ss >> line >> x >> y;
        if(line == "wall")
        {

            W[numWalls].placeWall(x,y);                              // place each brick

            numWalls++;
            //cout << line << x << y << endl;
        }
    }
    myfile.clear();
    myfile.seekg(0,myfile.beg);
    getline(myfile,line);

    for(int i=0; i<10;i++)
    {
        E[i].initEnm(M->getGridSize(),4,"images/e.png"); //Load enemy image
        //E[i].placeEnemy(float(rand()%(M->getGridSize())),float(rand()%(M->getGridSize())));
        //place enemies random x,y
    }

    numEnemies = 0;
    while(!myfile.eof())
    {
        getline(myfile,line);
        stringstream ss(line);
        ss >> line >> x >> y;

        if(line == "enemy")
        {
            E[numEnemies].placeEnemy(x,y);                              // place each brick
            numEnemies++;
            //cout <<line << x << y << endl;
        }
    }
        //E[0].moveEnemy("left");
        for(int i = 0; i < numEnemies;i++)
            //cout << E[i].getEnemyLoc().x << "   " << E[i].getEnemyLoc().y << endl;

    //initialize matrix for maze
    liveEnemies = numEnemies;
}
int m = M->getGridSize();
int n = M->getGridSize();





struct Node
{
    int x, y, dist;
};
//all 4 directions
int row[] = {-1, 0, 0, 1};
int col[] = {0, -1, 1, 0};

bool canVisit(int** mat, int **visited,int row, int col)
{
    return (row >= 0) && (row < M->getGridSize()) && (col >= 0) && (col < M->getGridSize())
        && mat[row][col]!=1 && !visited[row][col];
}

bool canMove(int** mat, int **visited,int row, int col)
{
    return (row >= 0) && (row < M->getGridSize()) && (col >= 0) && (col < M->getGridSize())
        && mat[row][col]!=1 && mat[row][col]!=2 && !visited[row][col];
}

int BFS(int **mat, int i, int j, int x, int y)
{

    if(i == x && y == j)
        return 0;
    int** visited = new int*[M->getGridSize()];
    for(int i = 0; i < M->getGridSize(); ++i)
        visited[i] = new int[M->getGridSize()];

   for(int i = 0;i < M->getGridSize(); i ++)
        for(int j = 0;j < M->getGridSize(); j ++)
            visited[i][j] = 0;
    queue<Node> q;

    // mark source cell as visited and enqueue the source node
    visited[i][j] = true;
    q.push({i, j, 0});

    // stores length of longest path
    int min_dist = INT_MAX;

    while (!q.empty())
    {
        // pop front node from queue and process it
        Node node = q.front();
        q.pop();

        int i = node.x, j = node.y, dist = node.dist;

        // if destination is found, update min_dist
        if (i == x && j == y)
        {
            min_dist = dist;
            break;
        }

        // check for all 4 possible movements from current cell
        for (int k = 0; k < 4; k++)
        {
            // check if it is possible to go to position
            if (canVisit(mat, visited, i + row[k], j + col[k]))
            {
                // mark next cell as visited
                visited[i + row[k]][j + col[k]] = true;
                q.push({ i + row[k], j + col[k], dist + 1 });
            }
        }
    }

    return min_dist;
}

string pickShortestDirection(int **mat, int i, int j, int x, int y)
{
    int min_dist_up = INT_MAX;
    int min_dist_down = INT_MAX;
    int min_dist_right= INT_MAX;
    int min_dist_left = INT_MAX;
    int min_dist_still = INT_MAX;
    int** visited = new int*[M->getGridSize()];
    for(int i = 0; i < M->getGridSize(); ++i)
        visited[i] = new int[M->getGridSize()];

   for(int i = 0;i < M->getGridSize(); i ++)
        for(int j = 0;j < M->getGridSize(); j ++)
            visited[i][j] = 0;

    if(canMove(mat,visited,i-1,j))
        min_dist_up = BFS(mat, i-1, j, x, y);

    if(canMove(mat,visited,i+1,j))
        min_dist_down = BFS(mat, i+1, j, x, y);

    if(canMove(mat,visited,i,j-1))
         min_dist_left = BFS(mat, i, j-1, x, y);
    if(canMove(mat,visited,i,j+1))
        min_dist_right = BFS(mat, i, j+1, x, y);

    min_dist_still = BFS(mat, i, j, x, y);


    //cout <<"still" << min_dist_still << endl;
   // cout << "up" << min_dist_up << endl;
    //cout << "left" << min_dist_left << endl;
    //cout << "right" << min_dist_right << endl;
    //cout << "down" << min_dist_down << endl << endl;

    //if( min_dist_down == min_dist_left == min_dist_up == min_dist_right==INT_MAX)
        //cout << min_dist_down <<"   "<<min_dist_left <<"    "<<min_dist_up <<"  "<<min_dist_right<< endl;

    if(min_dist_still== min(min_dist_up,min(min_dist_down,min(min_dist_right,min(min_dist_left,min_dist_still)))))
        return "still";
    else if(min_dist_down == min(min_dist_up,min(min_dist_down,min(min_dist_right,min(min_dist_left,min_dist_still)))))
       return "down";
    else if(min_dist_up == min(min_dist_up,min(min_dist_down,min(min_dist_right,min(min_dist_left,min_dist_still)))))
       return "up";
    else if(min_dist_right == min(min_dist_up,min(min_dist_down,min(min_dist_right,min(min_dist_left,min_dist_still)))))
       return "right";
    else if(min_dist_left == min(min_dist_up,min(min_dist_down,min(min_dist_right,min(min_dist_left,min_dist_still)))))
       return "left";

}


void resize(int width, int height)              // resizing case on the window
{
    wWidth = width;
    wHeight = height;

    if(width<=height)
        glViewport(0,(GLsizei) (height-width)/2,(GLsizei) width,(GLsizei) width);
    else
        glViewport((GLsizei) (width-height)/2 ,0 ,(GLsizei) height,(GLsizei) height);
}

void display(void)
{
  glClear (GL_COLOR_BUFFER_BIT);        // clear display screen
        glPushMatrix();
         M->drawBackground();
        glPopMatrix();
        for(int i=0; i<numWalls;i++)
        {
           W[i].drawWall();
        }
        /**/
        glPushMatrix();
            M->drawGrid();
        glPopMatrix();



        for(int i=0; i<numEnemies;i++)
        {
        E[i].drawEnemy();
        //cout << E[0].getEnemyLoc().x << "   " << E[0].getEnemyLoc().y << endl;
        }

        glPushMatrix();
        for(int i = 0; i < numEnemies; i ++)
                if(P->getPlayerLoc().x == E[i].getEnemyLoc().x && P->getPlayerLoc().y == E[i].getEnemyLoc().y && E[i].live)
                {
                    P->livePlayer = false;
                    endState = "you lost by getting hit by an enemy";
                }
            P->drawplayer();
        glPopMatrix();

        glPushMatrix();
            for(int i = 0; i < numWalls; i ++)
                if(P->getArrowLoc().x == W[i].getWallLoc().x && P->getArrowLoc().y == W[i].getWallLoc().y)
                    P->arrowStatus = false;
            for(int i = 0; i < numEnemies; i ++)
                if(P->getArrowLoc().x == E[i].getEnemyLoc().x && P->getArrowLoc().y == E[i].getEnemyLoc().y && E[i].live)
                {
                    P->arrowStatus = false;
                    E[i].live = false;
                    liveEnemies--;
                }

            P->drawArrow();
        glPopMatrix();

         glPushMatrix();
           M->drawChest();
           if(M->liveChest == false)
                endState = "you won by collecting the chest";
        glPopMatrix();

        glPushMatrix();
           M->drawArrows();
        glPopMatrix();
        if(liveEnemies == 0)
            endState = "you won by killing all the enemies";

        if(M->liveChest == false || P->livePlayer == false ||  liveEnemies == 0)
        {
            cout << endState << endl;
           glutHideWindow();
        }
    glutSwapBuffers();
}




void key(unsigned char key, int x, int y)
{
    switch (key)
    {
        case ' ':
             if(!M->liveSetOfArrws)      // if setof arrows were picked by player
                P->shootArrow();
        break;
        case 27 :                       // esc key to exit
        case 'q':
            exit(0);
            break;
    }

    glutPostRedisplay();
}


 void GetOGLPos(int x, int y)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;

    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

    xPos =posX ;
    yPos =posY ;
}

 void idle(void)
{

    //Your Code here

    glutPostRedisplay();
}


void mouse(int btn, int state, int x, int y){

    switch(btn){
        case GLUT_LEFT_BUTTON:

        if(state==GLUT_DOWN){

              GetOGLPos(x,y);
            }
            break;


      case GLUT_RIGHT_BUTTON:

        if(state==GLUT_DOWN){

              GetOGLPos(x,y);
            }
            break;
    }
     glutPostRedisplay();
};



void Specialkeys(int key, int x, int y)
{

    // Your Code here
    bool wallBlocking = false;

    //matrix representing the maze
    int** maze = new int*[M->getGridSize()];
    for(int i = 0; i < M->getGridSize(); ++i)
        maze[i] = new int[M->getGridSize()];
    //maze[0][0] = 0;

    for(int i = 0;i < M->getGridSize(); i ++)
        for(int j = 0;j < M->getGridSize(); j ++)
            maze[i][j] = 0;

    for(int i = 0; i < numWalls; i++)
        maze[M->getGridSize()-W[i].getWallLoc().y-1][W[i].getWallLoc().x] = 1;

    switch(key)
    {
    case GLUT_KEY_UP:

        for(int i = 0; i < numWalls; i ++)
        if(W[i].getWallLoc().x == (P->getPlayerLoc().x) && (W[i].getWallLoc().y == P->getPlayerLoc().y+1))
            wallBlocking = true;
        if(!wallBlocking)
            P->movePlayer("up");
        //cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
    break;

    case GLUT_KEY_DOWN:

         for(int i = 0; i < numWalls; i ++)
            if(W[i].getWallLoc().x == (P->getPlayerLoc().x) && (W[i].getWallLoc().y == P->getPlayerLoc().y-1))
               wallBlocking = true;
         if(!wallBlocking)
            P->movePlayer("down");
        //cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
    break;

    case GLUT_KEY_LEFT:
         for(int i = 0; i < numWalls; i ++)
            if(W[i].getWallLoc().x == (P->getPlayerLoc().x-1) && (W[i].getWallLoc().y == P->getPlayerLoc().y))
               wallBlocking = true;
         if(!wallBlocking)
            P->movePlayer("left");
        //cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
    break;

    case GLUT_KEY_RIGHT:

        for(int i = 0; i < numWalls; i ++)
            if(W[i].getWallLoc().x == (P->getPlayerLoc().x+1) && (W[i].getWallLoc().y == P->getPlayerLoc().y))
               wallBlocking = true;
         if(!wallBlocking)
            P->movePlayer("right");
        //cout<< P->getPlayerLoc().x<< "    "<<P->getPlayerLoc().y<<endl;
    break;

   }
    if((P->getPlayerLoc().x == M->GetStArrwsLoc().x) && (P->getPlayerLoc().y == M->GetStArrwsLoc().y))
      M->liveSetOfArrws = false;
    if((P->getPlayerLoc().x == M->GetChestLoc().x) && (P->getPlayerLoc().y == M->GetChestLoc().y))
      M->liveChest = false;

    for(int i = 0; i < numEnemies; i++)
    {
        if(!wallBlocking){
        //cout << "moving enemy" << i << endl;
        string direction = pickShortestDirection( maze, M->getGridSize()-E[i].getEnemyLoc().y-1, E[i].getEnemyLoc().x,M->getGridSize()- P->getPlayerLoc().y-1, P->getPlayerLoc().x);

        if(direction == "up")
            maze[M->getGridSize()-E[i].getEnemyLoc().y-1-1][E[i].getEnemyLoc().x] = 2;
        if(direction == "down")
            maze[M->getGridSize()-E[i].getEnemyLoc().y-1+1][E[i].getEnemyLoc().x] = 2;
        if(direction == "left")
            maze[M->getGridSize()-E[i].getEnemyLoc().y-1][E[i].getEnemyLoc().x-1] = 2;
        if(direction == "right")
            maze[M->getGridSize()-E[i].getEnemyLoc().y-1][E[i].getEnemyLoc().x+1] = 2;
        E[i].moveEnemy(direction);
        //cout << E[i].getEnemyLoc().x << "   " << E[i].getEnemyLoc().y << endl;
        }
    }

    maze[M->getGridSize()-P->getPlayerLoc().y-1][P->getPlayerLoc().x] = 3;

    //for(int i = 0; i < numEnemies; i++)
        //maze[M->getGridSize()-E[i].getEnemyLoc().y-1][E[i].getEnemyLoc().x] = 2;

/*
    for(int i = 0;i < M->getGridSize(); i ++)
        {
        for(int j = 0;j < M->getGridSize(); j ++)
            cout << maze[i][j] << " ";
        cout << endl;
        }
        cout << endl << endl;
    //BFS(maze, M->getGridSize()-E[0].getEnemyLoc().y-1,E[0].getEnemyLoc().x, M->getGridSize()-P->getPlayerLoc().y-1,P->getPlayerLoc().x);
    /**/
  glutPostRedisplay();
}


/* Program entry point */

int main(int argc, char *argv[])
{
    int user_choice;
    cout << "which maze would you like to run?" << endl;
    cout << "Enter 1, 2, or the name of your maze text file" << endl;

    string userMaze;
    cin >> userMaze;

    if (userMaze == "1")
        userMaze = "maze.txt";
    if (userMaze == "2")
        userMaze = "maze2.txt";

   glutInit(&argc, argv);

   glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
   glutInitWindowSize (800, 800);                //window screen
   glutInitWindowPosition (100, 100);            //window position
   glutCreateWindow ("Maze");                    //program title
   init(userMaze);

   glutDisplayFunc(display);                     //callback function for display
   glutReshapeFunc(resize);                      //callback for reshape
   glutKeyboardFunc(key);                        //callback function for keyboard
   glutSpecialFunc(Specialkeys);
   glutMouseFunc(mouse);
   glutIdleFunc(idle);
   glutMainLoop();

   return EXIT_SUCCESS;

}

