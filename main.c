/** Made by Guilherme Viegas
*   Instituto Superior Técnico - MEEC
*           - ISTDots -
*   Goal: add points of the same color placed on a board eliminating them
*   With the possibility of returning back by pressing the 'n' key
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX(a,b)    (((a)>(b))?(a):(b))
#define SQR(a)      (a)*(a)
#define M_PI 3.14159265
#define STRING_SIZE 100       // max size for some strings
#define TABLE_SIZE 850        // main game space size
#define LEFT_BAR_SIZE 150     // left white bar size
#define WINDOW_POSX 200       // initial position of the window: x
#define WINDOW_POSY 200       // initial position of the window: y
#define SQUARE_SEPARATOR 8    // square separator in px
#define BOARD_SIZE_PER 0.7f   // board size in % wrt to table size
#define MAX_BOARD_POS 15      // maximum size of the board
#define MAX_COLORS 5
#define MARGIN 5

// declaration of the functions related to graphical issues
void InitEverything(int , int , TTF_Font **, SDL_Surface **, SDL_Window ** , SDL_Renderer ** , TTF_Font **, TTF_Font **);
void InitSDL();
void InitFont();
SDL_Window* CreateWindow(int , int );
SDL_Renderer* CreateRenderer(int , int , SDL_Window *);
int RenderText(int, int, const char *, TTF_Font *, SDL_Color *, SDL_Renderer *);
int RenderLogo(int, int, SDL_Surface *, SDL_Renderer *);
int RenderTable(int, int, int [], TTF_Font *, SDL_Surface **, SDL_Renderer *, int, int);
void ProcessMouseEvent(int , int , int [], int , int *, int * );
void RenderPoints(int [][MAX_BOARD_POS], int, int, int [], int, SDL_Renderer *, int, int);
void RenderStats( SDL_Renderer *, TTF_Font *, int [], int , int );
void filledCircleRGBA(SDL_Renderer * , int , int , int , int , int , int );

void InitRandomBoard(int, int, int [][MAX_BOARD_POS], int);
void GetUserParams(char [], char [], int [MAX_COLORS], int *, int *, int *, int *);
int ValidatePlay(int [MAX_BOARD_POS*MAX_BOARD_POS][2], int [MAX_BOARD_POS][MAX_BOARD_POS], int);
void DestroyCircles(int [MAX_BOARD_POS][MAX_BOARD_POS], int [MAX_BOARD_POS][MAX_BOARD_POS], int , int , int , int);
int CheckSquares(int [MAX_BOARD_POS][MAX_BOARD_POS], int [MAX_BOARD_POS][MAX_BOARD_POS], int [MAX_BOARD_POS*MAX_BOARD_POS][2], int [MAX_COLORS], int, int, int, int);
void UpdateStats(int [MAX_COLORS], int [MAX_COLORS], int * );
void EndGame(SDL_Renderer * , TTF_Font *, TTF_Font *, int );
int Shuffle(int [MAX_BOARD_POS][MAX_BOARD_POS], int, int );
void RenderShuffleRect(SDL_Renderer * , TTF_Font *);
void WriteFile(FILE *, char [], char [], int, int, int []);
void RenderSelectedSquares(SDL_Renderer *, int, int, int [][2], int, int, int [], int);
void PressedUKey(int [][MAX_BOARD_POS], int [][MAX_BOARD_POS], int [MAX_COLORS], int [MAX_COLORS], int , int , int *, int );

// definition of some strings: they cannot be changed when the program is executed !
const char myName[] = "Guilherme Viegas";
const char myNumber[] = "IST90090";
const int colors[3][MAX_COLORS] = {{91, 170, 54, 255, 253},{32, 240, 61, 174, 118},{94, 183, 105, 3, 144}};

/**
 * main function: entry point of the program
 * only to invoke other functions !
 */
int main( void )
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *serif = NULL;
    TTF_Font *serifGrande = NULL;
    TTF_Font *serifSmall = NULL;
    SDL_Surface *imgs[2];
    SDL_Event event;
    int delay = 100;
    int quit = 0;
    int width = (TABLE_SIZE + LEFT_BAR_SIZE);
    int height = TABLE_SIZE;
    int square_size_px = 0, board_size_px[2] = {0};
    int board_pos_x = 0, board_pos_y = 0;
    int board[MAX_BOARD_POS][MAX_BOARD_POS] = {{0}};
    int pt_x = 0, pt_y = 0;

    //User Params
    char tmp[STRING_SIZE] = {0}; //For the combination of fgets and sscanf to restringe the user to only input numbers
    char username[STRING_SIZE] = {0};
    int numColors = 0;
    int colorPoints[MAX_COLORS] = {0};
    int maxPlays = 0;

    int currentPointsPerColor[MAX_COLORS] = {0};
    int currentPlays = 0;
    int lastBoard[MAX_BOARD_POS][MAX_BOARD_POS] = {{0}};

    int gameState = 0; //1 if Player won, -1 if Player lost, and 0 if it remains in game
    int wins = 0;
    int defeats = 0;
    int gameStats[200] = {0}; //Each position is a different game, if defeat: value=-1 and if victory: value=currentPlays (Number of plays)
    int index = 0; //Index of gameStats array

    int isPlaying = 0; //If user is making a move
    int lastXPos = 0;
    int lastYPos = 0;
    int gamePlay[MAX_BOARD_POS * MAX_BOARD_POS][2] = {{0}};
    int gamePlayIndex=0;
    int circlesToDestroy[MAX_BOARD_POS][MAX_BOARD_POS] = {{0}}; //1 on the circles coordinates to destroy and 0 on the rest
    int gamePlayColor = -1;
    int pointsMadePerColor[MAX_COLORS] = {0}; //Number of circles, by color, on each play
    int isSquare = 0;

    FILE *statistics;
    char filePath[] = "stats.txt";

    //--------------------------

    GetUserParams(username, tmp, colorPoints, &board_pos_x, &board_pos_y, &numColors, &maxPlays);

    currentPlays = maxPlays;
    for(int i=0; i < MAX_COLORS; i++) {
        currentPointsPerColor[i] = colorPoints[i];
    }

    // initialize graphics
    InitEverything(width, height, &serif, imgs, &window, &renderer, &serifGrande, &serifSmall);

    //initialize board table array for random colors
    InitRandomBoard(board_pos_x, board_pos_y, board, numColors);

    while( quit == 0 )
    {
        // while there's events to handle
        while( SDL_PollEvent( &event ) )
        {

            if( event.type == SDL_QUIT )
            {
                WriteFile(statistics, filePath, username, wins, defeats, gameStats);
                quit = 1;
            }
            else if ( event.type == SDL_KEYDOWN )
            {
                switch ( event.key.keysym.sym )
                {
                    case SDLK_n:
                        if(gameState == 0) {
                            defeats++;
                            gameStats[index] = -1;
                            index++;
                        }


                        //Reset to the game variables
                        currentPlays = maxPlays;
                        for(int i=0; i < MAX_COLORS; i++) {
                            currentPointsPerColor[i] = colorPoints[i];
                        }

                        for(int i=0; i < MAX_COLORS; i++) {
                            pointsMadePerColor[i] = 0;
                        }

                        for(int i = 0; i < MAX_BOARD_POS; i++) {
                            for(int j=0; j < MAX_BOARD_POS; j++) {
                                circlesToDestroy[i][j] = 0;
                            }
                        }

                        for(int i = 0; i < MAX_BOARD_POS*MAX_BOARD_POS; i++) {
                            gamePlay[i][0] = 0;
                            gamePlay[i][1] = 0;
                        }
                        gamePlayColor = -1;
                        gamePlayIndex = 0;

                        gameState = 0;
                        InitRandomBoard(board_pos_x, board_pos_y, board, numColors);
                        break;
                    case SDLK_q:

                        WriteFile(statistics, filePath, username, wins, defeats, gameStats);
                        quit=1;
                        break;
                    case SDLK_u:
                        PressedUKey(board, lastBoard, currentPointsPerColor, pointsMadePerColor, board_pos_x, board_pos_y, &currentPlays, numColors);

                    default:
                        break;
                }
            }
            else if ( event.type == SDL_MOUSEBUTTONDOWN )
            {
                ProcessMouseEvent(event.button.x, event.button.y, board_size_px, square_size_px, &pt_x, &pt_y);
                //printf("Button down: %d %d\n", pt_x, pt_y);

                isPlaying = 1;
                lastXPos = pt_x;
                lastYPos = pt_y;
                //The first circle of the play is not counted on mouse movement functions
                gamePlay[gamePlayIndex][0] = pt_x;
                gamePlay[gamePlayIndex][1] = pt_y;
                gamePlayIndex++;

                //Save the current board to the lastBoard array
                for(int i=0; i < board_pos_x; i++) {
                    for(int j=0; j < board_pos_y; j++) {
                        lastBoard[i][j] = board[i][j];
                    }
                }


                //Reset to the pointsPerCOlor array
                for(int i=0; i < MAX_COLORS; i++) {
                    pointsMadePerColor[i] = 0;
                }


            }
            else if ( event.type == SDL_MOUSEBUTTONUP )
            {
                ProcessMouseEvent(event.button.x, event.button.y, board_size_px, square_size_px, &pt_x, &pt_y);
                //printf("\nButton up: %d %d ", pt_x, pt_y);
                if(ValidatePlay(gamePlay, board, gamePlayIndex)) {
                    for(int i=0; i < gamePlayIndex; i++) {
                        circlesToDestroy[gamePlay[i][0]][gamePlay[i][1]] = 1;
                    }

                    gamePlayColor = board[gamePlay[0][0]][gamePlay[0][1]]; //The general play color is equal to, for example, the first circle of the user play

                    //On the case of the gameplay being a line that the user returned back, the points have to be counted has half + 1
                    if(gamePlay[0][0] == gamePlay[gamePlayIndex-1][0] && gamePlay[0][1] == gamePlay[gamePlayIndex-1][1] && gamePlay[1][0] == gamePlay[gamePlayIndex-2][0] && gamePlay[1][1] == gamePlay[gamePlayIndex-2][1]) {
                        pointsMadePerColor[gamePlayColor] = gamePlayIndex/2 +1;
                    } else {
                        pointsMadePerColor[gamePlayColor] = gamePlayIndex;
                    }
                    isSquare = CheckSquares(board, circlesToDestroy, gamePlay, pointsMadePerColor, gamePlayColor, gamePlayIndex, board_pos_x, board_pos_y);
                    if(isSquare) {
                        pointsMadePerColor[gamePlayColor]--;     //Being a square the first circle is equal to the last so only counted as one
                    }
                    DestroyCircles(board, circlesToDestroy, board_pos_x, board_pos_y, gamePlayIndex, numColors);
                }

                //Score values update
                if(gameState==0)
                    UpdateStats(pointsMadePerColor, currentPointsPerColor, &currentPlays);


                if(currentPlays ==0) {
                    gameState = -1;
                    defeats++;
                    gameStats[index] = -1;
                    index++;
                }else if(currentPointsPerColor[0] <=0 && currentPointsPerColor[1] <=0 && currentPointsPerColor[2] <=0 && currentPointsPerColor[3] <=0 && currentPointsPerColor[4] <=0 ) {
                    gameState = 1;
                    wins++;
                    gameStats[index] = currentPlays;
                    index++;
                }


                //Variables reset
                for(int i = 0; i < MAX_BOARD_POS; i++) {
                    for(int j=0; j < MAX_BOARD_POS; j++) {
                        circlesToDestroy[i][j] = 0;
                    }
                }

                for(int i = 0; i < MAX_BOARD_POS*MAX_BOARD_POS; i++) {
                    gamePlay[i][0] = 0;
                    gamePlay[i][1] = 0;
                }
                isPlaying = 0;
                gamePlayIndex = 0;
            }
            else if ( event.type == SDL_MOUSEMOTION )
            {
                ProcessMouseEvent(event.button.x, event.button.y, board_size_px, square_size_px, &pt_x, &pt_y);
                //printf("Moving Mouse: %d %d\n", pt_x, pt_y);

                //Retrieving the user game-play
                if(isPlaying && gameState==0) {
                    if((lastXPos != pt_x || lastYPos != pt_y) && pt_x != -1 && pt_y != -1) {
                        gamePlay[gamePlayIndex][0] = pt_x;
                        gamePlay[gamePlayIndex][1] = pt_y;
                        lastXPos = pt_x;
                        lastYPos = pt_y;
                        gamePlayIndex++;
                    }
                }
            }
        }
        // render game table
        square_size_px = RenderTable( board_pos_x, board_pos_y, board_size_px, serif, imgs, renderer, isSquare, gamePlayColor);
        isSquare = 0;
        //Render with a different color(red) the selected squares
        RenderSelectedSquares(renderer, 3, 3, gamePlay, board_pos_x, board_pos_y, board_size_px, gamePlayIndex);
        // render board
        RenderPoints(board, board_pos_x, board_pos_y, board_size_px, square_size_px, renderer, pt_x, pt_y);
        //Render points and goals
        RenderStats(renderer, serifGrande, currentPointsPerColor, numColors, currentPlays);
        //If it is needed to shuffle:
        if(Shuffle(board, board_pos_x, board_pos_y)) {
            RenderShuffleRect(renderer, serifGrande);
            SDL_RenderPresent(renderer);
            SDL_Delay( 1500 );
            InitRandomBoard(board_pos_x, board_pos_y, board, numColors);
        }
        //If player won or lost:
        if(gameState != 0) {
            EndGame(renderer, serifGrande, serifSmall, gameState);
        }
        // render in the screen all changes above
        SDL_RenderPresent(renderer);
        // add a delay
        SDL_Delay( delay );
    }

    // free memory allocated for images and textures and closes everything including fonts
    TTF_CloseFont(serif);
    SDL_FreeSurface(imgs[0]);
    SDL_FreeSurface(imgs[1]);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_SUCCESS;
}

/**
 *InitRandomBoard: inicializes the board array with random colors on each "square"
 * \param _boardX board size on x axis
 * \param _boardY board size on y axis
 * \param _boardTable[] array that stores the random values for choose the color
 * \param _numColors Number of colors chosen by the user to be used in the game
*/

void InitRandomBoard(int _boardX, int _boardY, int _boardTable[][MAX_BOARD_POS], int _numColors) {
    srand(time(NULL));
    for(int i=0; i < _boardX; i++) {
        for(int j=0; j < _boardY; j++) {
            _boardTable[i][j] = rand() % _numColors;
        }
    }
}

/**GetUserParams To get the initial params before starting the game
* \param _username The username string array
* \param _tmp A temporary array to combine fgets and sscanf, for restricting the user to the needed characters
* \param _colorPoints Array that stores the goals for each color
* \param _board_pos_x and _board_pos_y The size of the board on the respective axis
* \param _numColors The maximum number of colors to appear on the game
* \param _maxPlays Maximum number of plays to make on each game before loosing
*/

void GetUserParams(char _username[STRING_SIZE], char _tmp[STRING_SIZE], int _colorPoints[MAX_COLORS], int *_board_pos_x, int *_board_pos_y, int *_numColors, int *_maxPlays) {
    do {
    printf("Enter your username (No more than 8 letters): ");
    fgets(_username, STRING_SIZE, stdin);
    } while (strlen(_username) <1 || strlen(_username) >8);
    //Para não guardar o caractere \n no array (o \n poderia dar problemas + para a frente)
    int len = strlen(_username);
    if (len > 0 && _username[len-1] == '\n')
        _username[len-1] = '\0';

    do {
        printf("\nEnter the VERTICAL game-table size(between 5 and 15): ");
        fgets(_tmp, STRING_SIZE, stdin);
        sscanf(_tmp, " %d",  _board_pos_y);
    } while(* _board_pos_y<5 || * _board_pos_y >15 );

    do {
        printf("\nEnter the HORIZONTAL game-table size(between 5 and 15): ");
        fgets(_tmp, STRING_SIZE, stdin);
        sscanf(_tmp, " %d", _board_pos_x);
    } while(*_board_pos_x<5 || *_board_pos_x >15);

    do {
        printf("\nEnter the number of colors to appear on the game (1-5): ");
        fgets(_tmp, STRING_SIZE, stdin);
        sscanf(_tmp, " %d", _numColors);
    } while(*_numColors<1 || *_numColors >5);

    //Escolha do numero de pontos PARA CADA uma das cores
    for(int i = 0; i < *_numColors; i++) {
        do {
            printf("\nEnter number of points to collect for the color %d (1-99): ", i+1);
            fgets(_tmp, STRING_SIZE, stdin);
            sscanf(_tmp, " %d", &_colorPoints[i]);
        } while(_colorPoints[i]<1 || _colorPoints[i] >99);
    }

    do {
        printf("\nAnd finally...enter the maximum number of plays to obtain victory(1-99): ");
        fgets(_tmp, STRING_SIZE, stdin);
        sscanf(_tmp, " %d", _maxPlays);
    } while(*_maxPlays<1 || *_maxPlays >99);

}

/**ValidatePlay: Checks if its a valid play or not
 *\param _gamePlay array with the coordenates of the gamed play in sequence
 *\param _board board of the game with the colors of circles in each coordinate
 *\param _numberOfCircles The number of circles that the player selected
*/

int ValidatePlay(int _gamePlay[MAX_BOARD_POS*MAX_BOARD_POS][2], int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _numberOfCircles) {
    int colorNow = 0;
    int colorNext = 0;
    if(_numberOfCircles > 1) {

    //Verificar se foram feitas jogadas na diagonal
    for(int i=0; i < _numberOfCircles-1; i++) {
        if(_gamePlay[i][0] == _gamePlay[i+1][0]+1 && _gamePlay[i][1] == _gamePlay[i+1][1]-1) //Se for para a diagonal superior direita
            return 0;
        else if(_gamePlay[i][0] == _gamePlay[i+1][0]-1 && _gamePlay[i][1] == _gamePlay[i+1][1]-1) //diagonal superior esquerda
            return 0;
        else if(_gamePlay[i][0] == _gamePlay[i+1][0]-1 && _gamePlay[i][1] == _gamePlay[i+1][1]+1) //diagonal inferior esquerda
            return 0;
        else if(_gamePlay[i][0] == _gamePlay[i+1][0]+1 && _gamePlay[i][1] == _gamePlay[i+1][1]+1) //diagonal inferior direita
            return 0;

    }

    //Verificar se todos os circulos são da mesma cor
    for(int i = 0; i< _numberOfCircles-1; i++) {
        colorNow = _board[_gamePlay[i][0]][_gamePlay[i][1]];
        colorNext = _board[_gamePlay[i+1][0]][_gamePlay[i+1][1]];
        if(colorNow != colorNext) {
            return 0;
        }
    }
    } else
        return 0;

    return 1;
}

/**DestroyCircles: Changes the selected circle colors for the color of the circle above
 *\param _board Board with the circle colors in the respective coordinates
 * \param _circlesToDestroy Array with value 1 on the selected circles coordinates and 0 on the non-selected
 * \param _sizeX, _sizeY Board sizes, respectively on the xx axis and the yy axis
 * \param _numCircles Number of circles selected by the player
 * \param _numColors Number of colors used in this game
*/

void DestroyCircles(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _circlesToDestroy[MAX_BOARD_POS][MAX_BOARD_POS], int _sizeX, int _sizeY, int _numCircles, int _numColors) {
    int i=0, j=0, d=0;
    srand(time(NULL));
    for(i=0; i<_sizeX; i++) {
        for(j=0; j < _sizeY; j++) {
            if(_circlesToDestroy[i][j] == 1) {
                for(d=0; d<j; d++) {
                    _board[i][j-d] = _board[i][j-d-1];
                }
                _board[i][0] = (int) (rand() % (_numColors));
            }
        }
    }
}

/**CheckSquares: Checks if the play is a square and if it is makes the inner circles on the array circlesToDestroy = 1 and all the circles on the board with the selected color
*\param _board Saves the circles colors of the board on the respective coordinates
*\param _circlesToDestroy Value 1 on the position of a selected circle and 0 on a non-selected one
*\param _gamePlay Has the coordinates of the game play in sequence
*\param _pointsMadePerColor Number of points made in that game play per color
*\param _currentColor Color of the general selected circles on that game play (not the inner circles in a square)
*\param _numberOfCircles Number of circles selected on the game play by the mouse movement
*\param _board_pos_x Board size on the xx axis
*\param _board_pos_y Board size on the yy axis
*\return 1 if its a square and 0 if not
*/
int CheckSquares(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _circlesToDestroy[MAX_BOARD_POS][MAX_BOARD_POS], int _gamePlay[MAX_BOARD_POS*MAX_BOARD_POS][2], int _pointsMadePerColor[MAX_COLORS], int _currentColor, int _numberOfCircles, int _board_pos_x, int _board_pos_y) {
    int minX=_board_pos_x, minY=_board_pos_y, maxX=0, maxY=0;

    //No caso de o jogador ter feito uma jogada em linha em que voltou para trás
    if(_gamePlay[0][0] == _gamePlay[_numberOfCircles-1][0] && _gamePlay[0][1] == _gamePlay[_numberOfCircles-1][1] && _gamePlay[1][0] == _gamePlay[_numberOfCircles-2][0] && _gamePlay[1][1] == _gamePlay[_numberOfCircles-2][1]) {
        printf("Foi uma linha!!!");
        return 0;
    }

    //For getting the vertices of the square(or rectangle)
    for(int i = 0; i < _board_pos_x; i++) {
        for(int j=0; j < _board_pos_y; j++) {
            if(_circlesToDestroy[i][j]==1) {
                if(i<minX)
                    minX = i;
                if(i>maxX)
                    maxX = i;
                if(j<minY)
                    minY = j;
                if(j>maxY)
                    maxY = j;
            }
        }
    }

    if((_gamePlay[0][0] == _gamePlay[_numberOfCircles-1][0] && _gamePlay[0][1] == _gamePlay[_numberOfCircles-1][1]) && _numberOfCircles>1) { //Se o primeiro e ultimo indices da jogada coincidirem(linha fechada)
        for(int i=0; i < _board_pos_x; i++) {
            for(int j=0; j < _board_pos_y; j++) {
                if((i>minX && i<maxX && j > minY && j < maxY) || (_board[i][j] == _currentColor && _circlesToDestroy[i][j] != 1)) {
                    _circlesToDestroy[i][j] = 1;
                    _pointsMadePerColor[_board[i][j]]++;
                }
            }
        }
        return 1;
    } else {
        return 0;
    }

}

/**UpdateStats: Updates the current points for each color and the current number of plays until loose
* \param _pointsPerColor points made in each play holding the destroyed circles for each color
* \param _currentPointsPerColor total number of points for each color until victory
* \param _numPlays Total number of plays until loose made in that game
*/
void UpdateStats(int _pointsPerColor[MAX_COLORS], int _currentPointsPerColor[MAX_COLORS], int * _numPlays) {
    *_numPlays = *_numPlays - 1;

    for(int i=0; i < MAX_COLORS; i++) {
        _currentPointsPerColor[i] = _currentPointsPerColor[i] - _pointsPerColor[i];
    }

    for(int i=0; i < MAX_COLORS;i++) {
        if(_currentPointsPerColor[i]< 0)
            _currentPointsPerColor[i] = 0;
    }
}

/**EndGame: For displaying the boxes and text of VICTORY and DEFEAT
* \param _font The text font
* \param _winOrLoose If the player won or not
*/

void EndGame(SDL_Renderer * _renderer, TTF_Font *_font, TTF_Font *_smallFont, int _winOrLoose ) {
    SDL_Color blue = {30,144,255};
    SDL_SetRenderDrawColor( _renderer,  250,250,210 , 0.5 );

    //O quadrado paara mostrar vitoria ou derrota
    SDL_Rect endGame = {200, 200, 500, 250};
    SDL_RenderFillRect(_renderer, &endGame);
    if(_winOrLoose == 1) {
        RenderText(360, 275, "VICTORY!", _font, &blue, _renderer);
    } else if(_winOrLoose == -1) {
        RenderText(360, 275, "DEFEAT!", _font, &blue, _renderer);
    }

    RenderText(375, 350, "Press n key to play again", _smallFont, &blue, _renderer);

}

/**RenderShuffleRect For rendering the box and text if needed to shuffle
*  When there no more possible moves
*/

void RenderShuffleRect(SDL_Renderer * _renderer, TTF_Font *_font) {
    SDL_Color blue = {30,144,255};
    SDL_SetRenderDrawColor( _renderer,  250,250,210 , 0.5 );

    SDL_Rect shuffle = {75, 200, 750, 250};
    SDL_RenderFillRect(_renderer, &shuffle);
    RenderText(100, 300, "NO MORE MOVES! SHUFFLING", _font, &blue, _renderer);

}

/**Shuffle: Checks if there is no more moves to do
* \param _board The current circle colors for each coordinate
* \param _board_size_x size of the board on the xx axis
* \param _board_size_y size of the board on the yy axis
* \return returns 1 if it needs to shuffle and 0 if not needed
*/


int Shuffle(int _board[MAX_BOARD_POS][MAX_BOARD_POS], int _board_size_x, int _board_size_y) {
    for(int i=0; i < _board_size_x-1; i++) {
        for(int j=0; j < _board_size_y; j++) {
            if(j != _board_size_y-1 && i != _board_size_x-1) {
                if(_board[i][j] == _board[i][j+1] || _board[i][j] ==_board[i+1][j]) {
                    return 0;
                }
            } else if(i != _board_size_x-1){
                if(_board[i][j] == _board[i+1][j]) {
                    return 0;
                }
            } else {
                if(_board[i][j] == _board[i][j+1]) {
                    return 0;
                }
            }
    }
    }
    return 1;
}

/**WriteFile For writing the statistics file when exiting the game
*\param _statistics The file variable for writing
*\param _filePath The stats.txt desired filepath
*\param _username The username the user entered initially
*\param _wins and _defeats The total number of wins and defeats for all games made for that app execution
*\param _gameStats Array that stores on each position(index of a game) '-1' if Defeated and 'number of plays made' if Victory
*/
void WriteFile(FILE *_statistics, char _filePath[], char _username[], int _wins, int _defeats, int _gameStats[]) {
    _statistics = fopen(_filePath, "w");
    if(_statistics == NULL) {
        printf("Can't open stats file\n");
        exit(EXIT_FAILURE);
    }

    fprintf(_statistics, "%s\n", _username);
    fprintf(_statistics, "Total number of games: %d\nNumber of Victories: %d\nNumber of Defeats: %d\n", _defeats+_wins, _wins, _defeats);
    for(int i=0; i<_wins+_defeats; i++) {
        if(_gameStats[i] == -1) {
            fprintf(_statistics, "Game %d: Defeat\n", i+1);
        } else {
            fprintf(_statistics, "Game %d: %d Victory\n", i+1, _gameStats[i]);
        }
    }

    fclose(_statistics);
}

/**RenderSelectedSquares
*
*/

void RenderSelectedSquares(SDL_Renderer * _renderer, int _pt_x, int _pt_y, int _gamePlay[][2], int _board_pos_x, int _board_pos_y, int _board_size_px[], int _numberOfCircles) {
    SDL_Color red = { 247, 56, 56 };
    SDL_Rect board_square;
    int board_size, square_size_px, max_pos;

    //Computes the size adjustements for the selected squares
    max_pos = MAX(_board_pos_x, _board_pos_y);
    board_size = (int)(BOARD_SIZE_PER*TABLE_SIZE);
    square_size_px = (board_size - (max_pos+1)*SQUARE_SEPARATOR) / max_pos;

    SDL_SetRenderDrawColor(_renderer, red.r, red.g, red.b, red.a );

    for(int i=0; i<_numberOfCircles; i++) {
        if(_gamePlay[i][0] !=-1) {
            board_square.x = ((TABLE_SIZE - _board_size_px[0]) >> 1) + (_gamePlay[i][0]+1)*SQUARE_SEPARATOR + _gamePlay[i][0]*square_size_px;
            board_square.y = (TABLE_SIZE - _board_size_px[1] - 15) + (_gamePlay[i][1]+1)*SQUARE_SEPARATOR + _gamePlay[i][1]*square_size_px;
            board_square.w = square_size_px;
            board_square.h = square_size_px;
            SDL_RenderFillRect(_renderer, &board_square);
        }
    }
}


void PressedUKey(int _board[][MAX_BOARD_POS], int _lastBoard[][MAX_BOARD_POS], int _currentPointsPerColor[MAX_COLORS], int _pointsMadePerColor[MAX_COLORS], int _board_pos_x, int _board_pos_y, int *_currentPlays, int _numColors) {
    *_currentPlays = *_currentPlays + 1;
    for(int i=0; i<_numColors; i++) {
        _currentPointsPerColor[i] = _currentPointsPerColor[i] + _pointsMadePerColor[i];
    }


    for(int i=0; i < _board_pos_x; i++) {
        for(int j=0; j < _board_pos_y; j++) {
            _board[i][j] = _lastBoard[i][j];
        }
    }
}

//----------------------

/**
 * ProcessMouseEvent: gets the square pos based on the click positions !
 * \param _mouse_pos_x position of the click on pixel coordinates
 * \param _mouse_pos_y position of the click on pixel coordinates
 * \param _board_size_px size of the board !
 * \param _square_size_px size of each square
 * \param _pt_x square nr
 * \param _pt_y square nr
 */
void ProcessMouseEvent(int _mouse_pos_x, int _mouse_pos_y, int _board_size_px[], int _square_size_px,
        int *_pt_x, int *_pt_y )
{
    int sq_x = 0, sq_y = 0;
    int circleX = 0, circleY=0, circleR=0, dist=0;

    // corner of the board
    int x_corner = (TABLE_SIZE - _board_size_px[0]) >> 1;
    int y_corner = (TABLE_SIZE - _board_size_px[1] - 15);

    // verify if valid cordinates
    if (_mouse_pos_x < x_corner || _mouse_pos_y < y_corner || _mouse_pos_x > (x_corner + _board_size_px[0])
        || _mouse_pos_y > (y_corner + _board_size_px[1]) )
    {
        *_pt_x = -1;
        *_pt_y = -1;
        return;
    }

    // computes the square where the mouse position is
    ///_mouse_pos_x = _mouse_pos_x - x_corner;
    ///_mouse_pos_y = _mouse_pos_y - y_corner;

    sq_x = ( _mouse_pos_x - x_corner ) / ( _square_size_px + SQUARE_SEPARATOR );
    sq_y = ( _mouse_pos_y - y_corner ) / ( _square_size_px + SQUARE_SEPARATOR );

    circleX = x_corner + (sq_x+1)*SQUARE_SEPARATOR + sq_x*(_square_size_px)+(_square_size_px>>1);
    circleY = y_corner + (sq_y+1)*SQUARE_SEPARATOR + sq_y*(_square_size_px)+(_square_size_px>>1);
    circleR = (int)(_square_size_px*0.4f);

    dist = (int)floor( sqrt( SQR(_mouse_pos_x - circleX) + SQR(_mouse_pos_y - circleY) ) );

    if(dist < circleR) {
        *_pt_x = sq_x;
        *_pt_y = sq_y;
    }else {
        *_pt_x = -1;
        *_pt_y = -1;
    }
}

/**
 * RenderPoints: renders the board
 * \param _board 2D array with integers representing board colors
 * \param _board_pos_x number of positions in the board (x axis)
 * \param _board_pos_y number of positions in the board (y axis)
 * \param _square_size_px size of each square
 * \param _board_size_px size of the board
 * \param _renderer renderer to handle all rendering in a window
 */
void RenderPoints(int _board[][MAX_BOARD_POS], int _board_pos_x, int _board_pos_y,
        int _board_size_px[], int _square_size_px, SDL_Renderer *_renderer, int _selectedPosX, int _selectedPosY)
{
    srand(time(NULL));

    int clr, x_corner, y_corner, circleX, circleY, circleR;
    int i, j;
    float raio =  (rand()%100)/500.0 + 0.25;

    // corner of the board
    x_corner = (TABLE_SIZE - _board_size_px[0]) >> 1;
    y_corner = (TABLE_SIZE - _board_size_px[1] - 15);

    // renders the squares where the dots will appear

    for (i = 0; i < _board_pos_x; i++ )
    {
        for (j = 0; j < _board_pos_y; j++ )
        {
                // define the size and copy the image to display
                circleX = x_corner + (i+1)*SQUARE_SEPARATOR + i*(_square_size_px)+(_square_size_px>>1);
                circleY = y_corner + (j+1)*SQUARE_SEPARATOR + j*(_square_size_px)+(_square_size_px>>1);

                if(i == _selectedPosX && j == _selectedPosY) {
                    circleR = (int)(_square_size_px* raio);
                } else {
                    circleR = (int)(_square_size_px*0.4f);
                }


                // draw a circle
                clr = _board[i][j];
                filledCircleRGBA(_renderer, circleX, circleY, circleR, colors[0][clr], colors[1][clr], colors[2][clr]);
        }
    }
}

/**
 * filledCircleRGBA: renders a filled circle
 * \param _circleX x pos
 * \param _circleY y pos
 * \param _circleR radius
 * \param _r red
 * \param _g gree
 * \param _b blue
 */
void filledCircleRGBA(SDL_Renderer * _renderer, int _circleX, int _circleY, int _circleR, int _r, int _g, int _b)
{
    int off_x = 0;
    int off_y = 0;
    float degree = 0.0;
    float step = M_PI / (_circleR*8);

    SDL_SetRenderDrawColor(_renderer, _r, _g, _b, 255);

    while (_circleR > 0)
    {
        for (degree = 0.0; degree < M_PI/2; degree+=step)
        {
            off_x = (int)(_circleR * cos(degree));
            off_y = (int)(_circleR * sin(degree));
            SDL_RenderDrawPoint(_renderer, _circleX+off_x, _circleY+off_y);
            SDL_RenderDrawPoint(_renderer, _circleX-off_y, _circleY+off_x);
            SDL_RenderDrawPoint(_renderer, _circleX-off_x, _circleY-off_y);
            SDL_RenderDrawPoint(_renderer, _circleX+off_y, _circleY-off_x);
        }
        _circleR--;
    }
}

/**
 * Shows some information about the game:
 * - Goals of the game
 * - Number of moves
 * \param _renderer renderer to handle all rendering in a window
 * \param _font font to display the scores
 * \param _goals goals of the game
 * \param _ncolors number of colors
 * \param _moves number of moves remaining
 */
void RenderStats( SDL_Renderer *_renderer, TTF_Font *_font, int _goals[], int _ncolors, int _moves)
{
    /* To Be Done */
    SDL_Color black = { 0, 0, 0 };
    int rect_x_pos = 170;
    SDL_SetRenderDrawColor( _renderer,  205, 193, 181 , 255 );
    char numberPlays[5];
    sprintf(numberPlays, "%d", _moves); //Convert int _moves to string

    //Conversoes de inteiros para chars


    //O quadrado do numero de jogadas para perder
    SDL_Rect jogadas = {50, 50, 80, 60};
    SDL_RenderFillRect(_renderer, &jogadas);
    RenderText(65, 45, numberPlays, _font, &black, _renderer);



    //Os quadrados da pontuacao por cor
    for(int i = 0; i < _ncolors; i++) {
        SDL_SetRenderDrawColor( _renderer,  205, 193, 181 , 255 );
        SDL_Rect rect = {rect_x_pos, 50, 120, 60};
        SDL_RenderFillRect(_renderer, &rect);
        char colorPoints[5];
        sprintf(colorPoints, "%d", _goals[i]);
        filledCircleRGBA(_renderer, rect_x_pos + 25, 75, 15, colors[0][i], colors[1][i], colors[2][i]);
        RenderText(rect_x_pos + 50, 45, colorPoints, _font, &black, _renderer);


        rect_x_pos += 135;
}
}

/**
 * RenderTable: Draws the table where the game will be played, namely:
 * -  some texture for the background
 * -  the right part with the IST logo and the student name and number
 * -  the grid for game board with squares and seperator lines
 * \param _board_pos_x number of positions in the board (x axis)
 * \param _board_pos_y number of positions in the board (y axis)
 * \param _board_size_px size of the board
 * \param _font font used to render the text
 * \param _img surfaces with the table background and IST logo (already loaded)
 * \param _renderer renderer to handle all rendering in a window
 */
int RenderTable( int _board_pos_x, int _board_pos_y, int _board_size_px[],
        TTF_Font *_font, SDL_Surface *_img[], SDL_Renderer* _renderer, int _isSquare, int _gamePlayColor )
{
    SDL_Color black = { 0, 0, 0 }; // black
    SDL_Color light = { 205, 193, 181 };
    SDL_Color dark = { 120, 110, 102 };
    SDL_Texture *table_texture;
    SDL_Rect tableSrc, tableDest, board, board_square;
    int height, board_size, square_size_px, max_pos;

    int i, j;

    // set color of renderer to some color
    SDL_SetRenderDrawColor( _renderer, 255, 255, 255, 255 );

    // clear the window
    SDL_RenderClear( _renderer );

    tableDest.x = tableSrc.x = 0;
    tableDest.y = tableSrc.y = 0;
    tableSrc.w = _img[0]->w;
    tableSrc.h = _img[0]->h;
    tableDest.w = TABLE_SIZE;
    tableDest.h = TABLE_SIZE;

    // draws the table texture
    table_texture = SDL_CreateTextureFromSurface(_renderer, _img[0]);
    SDL_RenderCopy(_renderer, table_texture, &tableSrc, &tableDest);

    // render the IST Logo
    height = RenderLogo(TABLE_SIZE, 0, _img[1], _renderer);

    // render the student name
    height += RenderText(TABLE_SIZE+3*MARGIN, height, myName, _font, &black, _renderer);

    // this renders the student number
    RenderText(TABLE_SIZE+3*MARGIN, height, myNumber, _font, &black, _renderer);

    // compute and adjust the size of the table and squares
    max_pos = MAX(_board_pos_x, _board_pos_y);
    board_size = (int)(BOARD_SIZE_PER*TABLE_SIZE);
    square_size_px = (board_size - (max_pos+1)*SQUARE_SEPARATOR) / max_pos;
    _board_size_px[0] = _board_pos_x*(square_size_px+SQUARE_SEPARATOR)+SQUARE_SEPARATOR;
    _board_size_px[1] = _board_pos_y*(square_size_px+SQUARE_SEPARATOR)+SQUARE_SEPARATOR;

    // renders the entire board background
    SDL_SetRenderDrawColor(_renderer, dark.r, dark.g, dark.b, dark.a );
    board.x = (TABLE_SIZE - _board_size_px[0]) >> 1;
    board.y = (TABLE_SIZE - _board_size_px[1] - 15);
    board.w = _board_size_px[0];
    board.h = _board_size_px[1];
    SDL_RenderFillRect(_renderer, &board);

    // renders the squares where the numbers will appear
    if(_isSquare ) {
        SDL_SetRenderDrawColor(_renderer, colors[0][_gamePlayColor], colors[1][_gamePlayColor], colors[2][_gamePlayColor], 255 );
    }else {
        SDL_SetRenderDrawColor(_renderer, light.r, light.g, light.b, light.a );
    }
    // iterate over all squares
    for ( i = 0; i < _board_pos_x; i++ )
    {
        for ( j = 0; j < _board_pos_y; j++ )
        {
            board_square.x = board.x + (i+1)*SQUARE_SEPARATOR + i*square_size_px;
            board_square.y = board.y + (j+1)*SQUARE_SEPARATOR + j*square_size_px;
            board_square.w = square_size_px;
            board_square.h = square_size_px;
            SDL_RenderFillRect(_renderer, &board_square);
        }
    }

    // destroy everything
    SDL_DestroyTexture(table_texture);
    // return for later use
    return square_size_px;
}

/**
 * RenderLogo function: Renders the IST logo on the app window
 * \param x X coordinate of the Logo
 * \param y Y coordinate of the Logo
 * \param _logoIST surface with the IST logo image to render
 * \param _renderer renderer to handle all rendering in a window
 */
int RenderLogo(int x, int y, SDL_Surface *_logoIST, SDL_Renderer* _renderer)
{
    SDL_Texture *text_IST;
    SDL_Rect boardPos;

    // space occupied by the logo
    boardPos.x = x;
    boardPos.y = y;
    boardPos.w = _logoIST->w;
    boardPos.h = _logoIST->h;

    // render it
    text_IST = SDL_CreateTextureFromSurface(_renderer, _logoIST);
    SDL_RenderCopy(_renderer, text_IST, NULL, &boardPos);

    // destroy associated texture !
    SDL_DestroyTexture(text_IST);
    return _logoIST->h;
}

/**
 * RenderText function: Renders some text on a position inside the app window
 * \param x X coordinate of the text
 * \param y Y coordinate of the text
 * \param text string with the text to be written
 * \param _font TTF font used to render the text
 * \param _color color of the text
 * \param _renderer renderer to handle all rendering in a window
 */
int RenderText(int x, int y, const char *text, TTF_Font *_font, SDL_Color *_color, SDL_Renderer* _renderer)
{
    SDL_Surface *text_surface;
    SDL_Texture *text_texture;
    SDL_Rect solidRect;

    solidRect.x = x;
    solidRect.y = y;
    // create a surface from the string text with a predefined font
    text_surface = TTF_RenderText_Blended(_font,text,*_color);
    if(!text_surface)
    {
        printf("TTF_RenderText_Blended: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
    // create texture
    text_texture = SDL_CreateTextureFromSurface(_renderer, text_surface);
    // obtain size
    SDL_QueryTexture( text_texture, NULL, NULL, &solidRect.w, &solidRect.h );
    // render it !
    SDL_RenderCopy(_renderer, text_texture, NULL, &solidRect);
    // clear memory
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);
    return solidRect.h;
}

/**
 * InitEverything: Initializes the SDL2 library and all graphical components: font, window, renderer
 * \param width width in px of the window
 * \param height height in px of the window
 * \param _font font that will be used to render the text
 * \param _img surface to be created with the table background and IST logo
 * \param _window represents the window of the application
 * \param _renderer renderer to handle all rendering in a window
 * \param _font font that will be used to render the big text of Victory or Defeat
 */
void InitEverything(int width, int height, TTF_Font **_font, SDL_Surface *_img[], SDL_Window** _window, SDL_Renderer** _renderer, TTF_Font **_fontBig, TTF_Font **_fontSmall)
{
    InitSDL();
    InitFont();
    *_window = CreateWindow(width, height);
    *_renderer = CreateRenderer(width, height, *_window);

    // load the table texture
    _img[0] = IMG_Load("table_texture.png");
    if (_img[0] == NULL)
    {
        printf("Unable to load image: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // load IST logo
    _img[1] = SDL_LoadBMP("ist_logo.bmp");
    if (_img[1] == NULL)
    {
        printf("Unable to load bitmap: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    // this opens (loads) a font file and sets a size
    *_font = TTF_OpenFont("FreeSerif.ttf", 16);
    *_fontBig = TTF_OpenFont("OpenSans.ttf", 46);
    *_fontSmall = TTF_OpenFont("FreeSerif.ttf", 20);
    if(!*_font)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

/**
 * InitSDL: Initializes the SDL2 graphic library
 */
void InitSDL()
{
    // init SDL library
    if ( SDL_Init( SDL_INIT_EVERYTHING ) == -1 )
    {
        printf(" Failed to initialize SDL : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

/**
 * InitFont: Initializes the SDL2_ttf font library
 */
void InitFont()
{
    // Init font library
    if(TTF_Init()==-1)
    {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(EXIT_FAILURE);
    }
}

/**
 * CreateWindow: Creates a window for the application
 * \param width width in px of the window
 * \param height height in px of the window
 * \return pointer to the window created
 */
SDL_Window* CreateWindow(int width, int height)
{
    SDL_Window *window;
    // init window
    window = SDL_CreateWindow( "ISTDots", WINDOW_POSX, WINDOW_POSY, width, height, 0 );
    // check for error !
    if ( window == NULL )
    {
        printf("Failed to create window : %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    return window;
}

/**
 * CreateRenderer: Creates a renderer for the application
 * \param width width in px of the window
 * \param height height in px of the window
 * \param _window represents the window for which the renderer is associated
 * \return pointer to the renderer created
 */
SDL_Renderer* CreateRenderer(int width, int height, SDL_Window *_window)
{
    SDL_Renderer *renderer;
    // init renderer
    renderer = SDL_CreateRenderer( _window, -1, 0 );

    if ( renderer == NULL )
    {
        printf("Failed to create renderer : %s", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    // set size of renderer to the same as window
    SDL_RenderSetLogicalSize( renderer, width, height );

    return renderer;
}
