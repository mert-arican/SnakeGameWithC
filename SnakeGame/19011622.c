//
//  main.c
//  SnakeGame
//
//  Created by Mert Arıcan on 30.12.2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// MARK: - Data structures

enum SquareType { empty, food, snakeBody, deadHead };

typedef struct Position {
    int x;
    int y;
} Position;

typedef struct Square {
    Position position;
    enum SquareType type;
    int snakePieceCount;
} Square;

typedef struct Board {
    int rowCount;
    int colCount;
    Square** squares;
} Board;

typedef struct Snake {
    int size;
    Position lastTailPosition;
    bool shouldGrowInNextStep;
} Snake;

// MARK: - Function prototypes

void assignRandomPosition(Position* position, int numberOfRows, int numberOfColumns);
void initializeSnake(Snake *snake, Position initialPosition);
void drawSquare(Square square, bool fancyModeIsOn);
void drawBoard(Board *board, int *numberOfMoves, bool fancyModeIsOn);
void _moveSnakesBodyExceptHeadHelper(Board *board, Snake *snake, int snakeBodyNumber);
void moveSnakesBodyExceptHead(Board *board, Snake *snake);
void freeBoard(Board *board, int numberOfRows, int numberOfColumns);
bool snakeIsOutOfBounds(Board *board, Position positionOfSnakesHead);
bool arrayContainsPosition(Position *array, int numberOfElements, Position position);
Position getPositionOfSnakeBodyWithNumber(Board *board, Snake *snake, int bodyNumber);
Position initializeBoardAndReturnInitialPositionOfSnake(Board* board, int numberOfRows, int numberOfColumns, int *numberOfFoods);

// MARK: - Free the board...

void freeBoard(Board *board, int numberOfRows, int numberOfColumns) {
    int i;
    for (i=0; i<numberOfRows; i++) {
        free((void *)board->squares[i]);
    }
    free((void *)board->squares);
    free(board);
}

// MARK: - Functions to initialize the board and snake...

bool arrayContainsPosition(Position *array, int numberOfElements, Position position) {
    int i = 0;
    while (i < numberOfElements) {
        if (array[i].x == position.x && array[i].y == position.y) {
            return true;
        }
        i++;
    }
    return false;
}

void assignRandomPosition(Position* position, int numberOfRows, int numberOfColumns) {
    position->x = rand() % numberOfColumns;;
    position->y = rand() % numberOfRows;
}

Position initializeBoardAndReturnInitialPositionOfSnake(Board* board, int numberOfRows, int numberOfColumns, int *numberOfFoods) {
    int i, j;
    board->rowCount = numberOfRows;
    board->colCount = numberOfColumns;
    
    // Safety check...
    while (numberOfRows*numberOfColumns-1 < *numberOfFoods) {
        printf("Invalid number of foods!\nPlease re-enter valid number of foods: \n");
        scanf("%d", numberOfFoods);
    }
    
    // Spread food on board...
    int numberOfRandomlySpreadedFoods = 0;
    Position *randomlySpreadedFoodPositions = malloc(sizeof(Position *) * (*numberOfFoods));
    
    while (numberOfRandomlySpreadedFoods < *numberOfFoods) {
        Position randomFoodPosition;
        assignRandomPosition(&randomFoodPosition, numberOfRows, numberOfColumns);
        
        if (!arrayContainsPosition(randomlySpreadedFoodPositions, numberOfRandomlySpreadedFoods, randomFoodPosition)) {
            randomlySpreadedFoodPositions[numberOfRandomlySpreadedFoods] = randomFoodPosition;
            numberOfRandomlySpreadedFoods++;
        }
    }
    
    // Put snake on board...
    bool snakePositionIsDetermined = false;
    Position snakePosition = { -1, -1 }; // Weird initial value so that if anything goes wrong I can detect...
    
    while (!snakePositionIsDetermined) {
        assignRandomPosition(&snakePosition, numberOfRows, numberOfColumns);
        if (!arrayContainsPosition(randomlySpreadedFoodPositions, numberOfRandomlySpreadedFoods, snakePosition)) {
            snakePositionIsDetermined = true;
        }
    }
    
    for (i=0; i < numberOfRows; i++) {
        for (j=0; j < numberOfColumns; j++) {
            Square square;
            Position position = { j, i };
            square.position = position;
            square.snakePieceCount = 0;
            square.type = empty;
            
            if (snakePosition.y == i && snakePosition.x == j) {
                square.type = snakeBody;
                square.snakePieceCount = 1;
            }
            else if (arrayContainsPosition(randomlySpreadedFoodPositions, numberOfRandomlySpreadedFoods, position)) {
                square.type = food;
            }
            
            board->squares[i][j] = square;
        }
    }
    return snakePosition;
}

void initializeSnake(Snake *snake, Position initialPosition) {
    snake->size = 1;
    snake->lastTailPosition = initialPosition;
    snake->shouldGrowInNextStep = false;
}

// MARK: - Drawing the board and snake...

void drawSquare(Square square, bool fancyModeIsOn) {
    switch (square.type) {
        case empty: fancyModeIsOn ? printf("⬜") : printf(" - "); break;
        case food: fancyModeIsOn ? printf("🍎") : printf(" 0 "); break;
        case snakeBody: fancyModeIsOn ? (square.snakePieceCount == 1 ? printf("🔵") : printf("🟢")) : printf(" %d ", square.snakePieceCount); break;
        case deadHead: fancyModeIsOn ? printf("🔴") : printf(" X "); break;
    }
}

void drawBoard(Board *board, int *numberOfMoves, bool fancyModeIsOn) {
    int i, j;
    for (i = 0; i < board->rowCount; i++) {
        printf("\t\t\t");
        for (j = 0; j < board->colCount; j++) {
            drawSquare(board->squares[i][j], fancyModeIsOn);
        }
        printf("\n");
    }
    printf("\n\t\t\tNumber of moves: %d\n\n", *numberOfMoves);
}

// MARK: - Functions for moving the snake...

Position getPositionOfSnakeBodyWithNumber(Board *board, Snake *snake, int bodyNumber) {
    int i, j;
    Position position = { -1, -1 }; // end execution if something fails.
    for (i = 0; i < board->rowCount; i++) {
        for (j = 0; j < board->colCount; j++) {
            if (board->squares[i][j].type == snakeBody && board->squares[i][j].snakePieceCount == bodyNumber) {
                position.y = i ; position.x = j;
            }
        }
    }
    return position;
}

bool snakeIsOutOfBounds(Board *board, Position positionOfSnakesHead) {
    int x = positionOfSnakesHead.x; int y = positionOfSnakesHead.y;
    return (x == -1 || y == -1 || x == board->colCount || y == board->rowCount);
}

// !!! Do not directly call this. Only intended to use within 'moveSnakesBodyExceptHead' function. !!!
void _moveSnakesBodyExceptHeadHelper(Board *board, Snake *snake, int snakeBodyNumber) {
    if (snakeBodyNumber == 1 || snakeBodyNumber > snake->size) { return; }
    
    Position positionBeforeMove = getPositionOfSnakeBodyWithNumber(board, snake, snakeBodyNumber);
    Position nextStepPosition = getPositionOfSnakeBodyWithNumber(board, snake, 1);
    
    // Swap squares with bodyNumber 1 and parameter 'snakeBodyNumber'
    board->squares[nextStepPosition.y][nextStepPosition.x].type = snakeBody;
    board->squares[nextStepPosition.y][nextStepPosition.x].snakePieceCount = snakeBodyNumber;
    
    board->squares[positionBeforeMove.y][positionBeforeMove.x].type = snakeBody;
    board->squares[positionBeforeMove.y][positionBeforeMove.x].snakePieceCount = 1;
    
    // If this is the last part of the snake, update board and snake accordingly...
    if (snakeBodyNumber == snake->size) {
        snake->lastTailPosition = positionBeforeMove;
        board->squares[positionBeforeMove.y][positionBeforeMove.x].type = empty;
    }
    
    // Call procedure for 'the piece after'...
    _moveSnakesBodyExceptHeadHelper(board, snake, snakeBodyNumber+1);
}

void moveSnakesBodyExceptHead(Board *board, Snake *snake) {
    _moveSnakesBodyExceptHeadHelper(board, snake, 2);
}

int main(int argc, const char * argv[]) {
    srand((unsigned int) time(NULL));
    bool fancyModeIsOn = false;
    if (argc > 1) {
        fancyModeIsOn = argv[1][0] == '1';
    }
    int i, N, M, numberOfFoods;
    int numberOfMoves = 0;
    bool isOutOfBounds = false;
    bool hasBittenItself = false;
    bool hasEatenAllTheFood = false;
    char move;
    
    // Take necessary information from user...
    printf("Enter the number of rows: \n");
    scanf("%d", &N);
    printf("Enter the number of columns: \n");
    scanf("%d", &M);
    printf("Enter the number of foods: \n");
    scanf("%d", &numberOfFoods);
    
    // Initialize board...
    Board *board = malloc(sizeof(board));
    board->squares = (Square **) malloc(sizeof(Square *) * N);
    for (i = 0; i < N; i++) {
        board->squares[i] = (Square *) malloc(sizeof(Square) * M);
    }
    
    Position initialPositionOfTheSnake;
    initialPositionOfTheSnake = initializeBoardAndReturnInitialPositionOfSnake(board, N, M, &numberOfFoods);
    
    // Initialize snake...
    Snake *snake = malloc(sizeof(snake));
    initializeSnake(snake, initialPositionOfTheSnake);
    
    drawBoard(board, &numberOfMoves, fancyModeIsOn);
    printf("Press U to go up\nPress D to go down\nPress L to go left\nPress R to go right.\n");
    
    while (!hasEatenAllTheFood) {
        printf("Enter your move: ");
        scanf(" %c", &move);
        printf("\n");
        
        Position positionOfSnakesHeadBeforeMove = getPositionOfSnakeBodyWithNumber(board, snake, 1);
        Position positionOfSnakesHeadAfterMove = positionOfSnakesHeadBeforeMove;
        
        if (move == 'U' || move == 'u') {
            positionOfSnakesHeadAfterMove.y--;
        }
        else if (move == 'D' || move == 'd') {
            positionOfSnakesHeadAfterMove.y++;
        }
        else if (move == 'L' || move == 'l') {
            positionOfSnakesHeadAfterMove.x--;
        }
        else if (move == 'R' || move == 'r') {
            positionOfSnakesHeadAfterMove.x++;
        }
        else {
            printf("Invalid move!\n");
            continue;
        }
        
        numberOfMoves++;
        
        if (snakeIsOutOfBounds(board, positionOfSnakesHeadAfterMove)) {
            isOutOfBounds = true; break;
        }
        
        // move snakes body except the head--this call mutates the board. Only effective when snake's size >= 2...
        moveSnakesBodyExceptHead(board, snake);
        
        // make snake grow if necessary...
        if (snake->shouldGrowInNextStep) {
            // Create new square and assign it at the right position in the table...
            Square square;
            square.type = snakeBody;
            square.snakePieceCount = (snake->size) + 1;
            
            // Update board...
            if (snake->size == 1) {
                square.position = positionOfSnakesHeadBeforeMove;
                board->squares[positionOfSnakesHeadBeforeMove.y][positionOfSnakesHeadBeforeMove.x] = square;
            } else {
                square.position = snake->lastTailPosition;
                board->squares[snake->lastTailPosition.y][snake->lastTailPosition.x] = square;
            }
            
            // Update snake...
            snake->size++;
            snake->shouldGrowInNextStep = false;
        }
        
        // Check whether snake collides with itself...
        if (board->squares[positionOfSnakesHeadAfterMove.y][positionOfSnakesHeadAfterMove.x].type == snakeBody) {
            board->squares[positionOfSnakesHeadAfterMove.y][positionOfSnakesHeadAfterMove.x].type = deadHead;
            hasBittenItself = true; break;
        }
        
        // Check whether snake reached a food...
        if (board->squares[positionOfSnakesHeadAfterMove.y][positionOfSnakesHeadAfterMove.x].type == food) {
            snake->shouldGrowInNextStep = true;
            if (snake->size == numberOfFoods) {
                hasEatenAllTheFood = true;
            }
        }
        
        // Put head at new position in board...
        board->squares[positionOfSnakesHeadAfterMove.y][positionOfSnakesHeadAfterMove.x].type = snakeBody;
        board->squares[positionOfSnakesHeadAfterMove.y][positionOfSnakesHeadAfterMove.x].snakePieceCount = 1;
        
        // If snake's size is 1, then this branch handles the movement.
        if (snake->size == 1) {
            board->squares[positionOfSnakesHeadBeforeMove.y][positionOfSnakesHeadBeforeMove.x].type = empty;
            board->squares[positionOfSnakesHeadBeforeMove.y][positionOfSnakesHeadBeforeMove.x].snakePieceCount = 0;
            snake->lastTailPosition = positionOfSnakesHeadBeforeMove;
        }
        drawBoard(board, &numberOfMoves, fancyModeIsOn);
    }
    
    if (hasEatenAllTheFood) {
        printf("You won! You have eaten all the food in %d moves.\n\n", numberOfMoves);
    }
    else {
        if (hasBittenItself) {
            printf("Game over! You've bitten yourself.\n");
        }
        else if (isOutOfBounds) {
            printf("Game over! You're out of bounds.\n");
        }
        printf("Number of moves: %d\n", numberOfMoves);
        printf("Snake's size: %d\n", snake->size);
        printf("Number of remaining foods on board: %d\n\n", numberOfFoods - snake->size + 1);
    }
    
    drawBoard(board, &numberOfMoves, fancyModeIsOn);
    
    // free board and snake...
    freeBoard(board, N, M);
    free(snake);
    return 0;
}
