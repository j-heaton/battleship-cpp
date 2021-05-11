#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <string>
#include <sstream>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SIZE 10
#define SA struct sockaddr
#define backlog 10

const int NUM_ROWS = 10;
const int NUM_COLS = 10;
const int CARRIER_LEN = 5;
const int BATTLESHIP_LEN = 4;
const int CRUISER_LEN = 3;
const int SUB_LEN = 3;
const int DESTROYER_LEN = 2;
const int NUM_SHIPS = 5;

struct Node { 					//nodes hold data for each turn
    char row_target;			//the coordinates entered by the player for that turn
	char col_target;
	char saved_result[5];		//the result of the turn (hit or miss)
	char ship_hit[11];			//the type of ship hit (empty string if no ship hit)
    struct Node* next; 			//pointer to the node for the next turn
};

struct Ship {
	char identifier;
	int length;
	int orientation;
	int x;
	int y;
	bool sunk;
};

char ipAddress[200], port[200];
int ourSocket, listenSocket;

void place_ships(char state[NUM_ROWS][NUM_COLS], Ship ships[NUM_SHIPS]);									//prototype for place_ships

struct Node accept_input(char& row, char& col);																//prototype for accept_input

void update_state(char& row, char& col, char state[NUM_ROWS][NUM_COLS], Node& temp);						//prototype for update_state

int check_ship(char state[NUM_ROWS][NUM_COLS], Ship ship);													//prototype for check_ship

void display_state(char state[NUM_ROWS][NUM_COLS]);															//prototype for display_state

int createSendingSocket();

int createListenSocket();


int main(int argc, char **argv){
	if (argc == 1) { 
	}
	if (argc == 3) {
        // if there are two command line arguments, where
        // first is the ipaddress and 
        // second is the port, then we initialize 
        // the client side in initialization() function
		strcpy(ipAddress,argv[1]);
		strcpy(port,argv[2]);
	}
	else {
        // if there is only one command line argument, 
        // then we initialize the server side in initialization
        // function
		memset(ipAddress,0,200);
		strcpy(port,argv[1]);
	}
	srand(time(NULL));						//seeds rand
	if (ipAddress[0] == 0){
        std::cout << "create listen socket";
        listenSocket = createListenSocket();

    }
	else{
        std::cout << "create sending socket";
        ourSocket = createSendingSocket();

    }
	char state[NUM_ROWS][NUM_COLS];			//the game board
	char empty_square = '-';				//the char '-' represents an empty square on the game board
	for(int i = 0; i < NUM_ROWS; i++) {
		for(int j = 0; j < NUM_COLS; j++){
			state[i][j] = empty_square;
		}
	}
	
	Ship ships[5] = {	{'C', CARRIER_LEN, 		-1, 0, 0, false}, 
						{'B', BATTLESHIP_LEN, 	-1, 0, 0, false}, 
						{'c', CRUISER_LEN, 		-1, 0, 0, false}, 
						{'S', SUB_LEN, 			-1, 0, 0, false}, 
						{'D', DESTROYER_LEN, 	-1, 0, 0, false}
					};	
	place_ships(state, ships);
	
	display_state(state);
	
	Node head;						//head of the turn-list
	Node temp;
	
	char row;
	char col;
	int n = 0;
	int counter = 0;						//counts number of turns
	int end_flag = 0;						//flag to check if the game is over
	while(end_flag == 0){       			//loops while the game is not over 
		if(counter == 0){									//for the first turn, data is recorded in the head node
			head = accept_input(row, col);			//accepts input from the user and stores that input as row and col, the user's input is recorded in a node
			/*send our move to the other player*/
			n = send(ourSocket, &head, sizeof(head), 0);
			if (n < 0){
				std::cout << "error sending move\n";
			}		
			/*receive the state of our move from the other player*/
			n = recv(ourSocket, &head, sizeof(head), 0);
			if (n < 0){
				std::cout << "error receiving move\n";
			}
			temp = head;
		}else{
			Node temp2 = accept_input(row, col);
			n = send(ourSocket, &temp2, sizeof(head), 0);
			if (n < 0){
				std::cout << "error sending move\n";
			}
			n = recv(ourSocket, &temp2, sizeof(head), 0);
			if (n < 0){
				std::cout << "error receiving move\n";
			}
			temp.next = &temp2;
		}
		struct Node *theirMove;
        /*receive theirMove from the other player*/
        n = recv(ourSocket, theirMove, sizeof(theirMove), 0);
		if (n < 0){
			std::cout << "error receiving move\n";
		}
        /*modify the update_state function to check theirMove is HIT or MISS
         * and send the state back to the other player */
		update_state(row, col, state, *theirMove);
		n = send(ourSocket, theirMove, sizeof(theirMove), 0);
		if (n < 0){
			std::cout << "error sending move\n";
		}	
		//check if carrier was sunk
		if (ships[0].sunk == false){
			if (check_ship(state, ships[0]) == 1){
				std::cout << "you sunk my carrier!\n";
				ships[0].sunk = true;
			}
		}
		//check if battleship was sunk
		if (ships[1].sunk == false){
			if (check_ship(state, ships[1]) == 1){
				std::cout << "you sunk my battleship!\n";
				ships[1].sunk = true;
			}
		}
		//check if cruiser was sunk
		if (ships[2].sunk == false){
			if (check_ship(state, ships[2]) == 1){
				std::cout << "you sunk my cruiser!\n";
				ships[2].sunk = true;
			}
		}
		//check if submarine was sunk
		if (ships[3].sunk == false){
			if (check_ship(state, ships[3]) == 1){
				std::cout << "you sunk my submarine!\n";
				ships[3].sunk = true;
			}
		}
		//check if destroyer was sunk
		if (ships[4].sunk == false){
			if (check_ship(state, ships[4]) == 1){
				std::cout << "you sunk my destroyer!\n";
				ships[4].sunk = true;
			}
		}
		display_state(state);
		if (ships[0].sunk == true && ships[1].sunk == true && ships[2].sunk == true && ships[3].sunk == true && ships[4].sunk == true){
			end_flag = 1;
		}
		counter++;
	}
	std::cout << "you win!";
}

/**
 * places the ships on the game board
 * @param state array that represents the game board
 * @param ships array of ships to be placed on the game board
 */
void place_ships(char state[NUM_ROWS][NUM_COLS], Ship ships[NUM_SHIPS]){
	int orientation_val;			//int that represents ship orientation
	int rand_row, rand_col;			//these will hold random coordinates that are used to place the ship
	for (int i=0; i<NUM_SHIPS; i++){
		orientation_val = rand()%2;		//random number [0,1]
		rand_row = rand()%(11-ships[i].length);	//random row that ensures the ship will not go off the board
		rand_col = rand()%(11-ships[i].length);	//random column that ensures the ship will not go off the board
		int count = 0;
		switch(orientation_val){	
		//horizontal
		case 0:
			do{
			count = 0;
				//checks if there is enough empty space for the ship at (rand_row, rand_col)
				for (int j=0;j<ships[i].length;j++){
					if (state[rand_row][rand_col+j] != '-'){
						count++;
					}
				}
				//get new rand_row and rand_col if there isn't enough space
				if (count != 0){
					rand_row = rand()%(11-ships[i].length);
					rand_col = rand()%(11-ships[i].length);
				}
			}while (count != 0);
			break;
		//vertical
		case 1:
			do{
			count = 0;
				//checks if there is enough empty space for the ship at (rand_row, rand_col)
				for (int j=0;j<ships[i].length;j++){
					if (state[rand_row+j][rand_col] != '-'){
						count++;
					}
				}
				//get new rand_row and rand_col if there isn't enough space
				if (count != 0){
					rand_row = rand()%(11-ships[i].length);
					rand_col = rand()%(11-ships[i].length);
				}
			}while (count != 0);
			break;
		default:
			std::cout << "error";
		}
		//place the ship
		switch(orientation_val){
		//horizontal
		case 0:
			for (int j=0;j<ships[i].length;j++){
				state[rand_row][rand_col+j] = ships[i].identifier;		
			}
			break;
		//vertical
		case 1:
			for (int j=0;j<ships[i].length;j++){
				state[rand_row+j][rand_col] = ships[i].identifier;		
			}
			break;
		default:
			std::cout << "error";
	}
	//store the ship's position
	ships[i].x = rand_col;
	ships[i].y = rand_row;
	//store the ship's orientation
	ships[i].orientation = orientation_val;
	}
}

/**
 * Prints the game board for the user.
 * @param state array that represents the game board
  */
void display_state(char state[NUM_ROWS][NUM_COLS]){
	printf("  0 1 2 3 4 5 6 7 8 9\n");
	for (int i = 0; i < NUM_ROWS; i++){
		printf("%c ", i+65);
		for (int j = 0; j < NUM_COLS; j++){
			printf("%c ", state[i][j]);
		}printf("\n");
	}printf("\n");
}

/**
 * Prompts the user to enter a letter and a number. this pair represents the row and column in the game world.
 * @param row reference to the address of the row value
 * @param col reference to the address of the col value
 * @return a node that contains the user's input
 */
struct Node accept_input(char& row, char& col){
	std::string str;
	std::cout << "enter a letter (A-J) and a number (0-9):";  		//prompts the user for coordinates
	getline(std::cin, str);												//stores the user's input into a string
	row = str.at(0);                								//extracts the row and col from the string
	col = str.at(1);
	//check length of input, check if row is a letter A-J, check if col is a number 0-9
	while (!isalpha(row) || !isdigit(col) || !(((row >= 65) && (row <= 74)) || ((row >= 97) && (row <= 106)))){		
		std::cout << "invalid entry, please enter a letter (A-J) and a number (0-9) - e.g., J7:";
		//get input if it was invalid
		getline(std::cin, str);												//stores the user's input into a string
		row = str.at(0);                								//extracts the row and col from the string
		col = str.at(1);
	}
	if (islower(row)){       									//makes the row an uppercase letter
		row = toupper(row);
	}
	printf("row is %c, col is %c\n", row, col);			//print the user's input
	
	Node temp = {row, col, 0, 0, 0};									
	return temp;													//returns the node
}

/**
 * Updates the state of the game using the row and column input by the user. if the user hit a ship, indicate that.
 * @param row reference to the row value
 * @param col reference to the col value
 * @param state array that represents the game board
 * @param temp reference to the current node in the game log
 */
void update_state(char& row, char& col, char state[NUM_ROWS][NUM_COLS], Node& temp){
	int irow, icol;
	//convert the row coordinate into a number
	switch(row){
		case 'A':
			irow = 0;
			break;
		case 'B':
			irow = 1;
			break;
		case 'C':
			irow = 2;
			break;
		case 'D':
			irow = 3;
			break;
		case 'E':
			irow = 4;
			break;
		case 'F':
			irow = 5;
			break;
		case 'G':
			irow = 6;
			break;
		case 'H':
			irow = 7;
			break;
		case 'I':
			irow = 8;
			break;
		case 'J':
			irow = 9;
			break;
		default:
			std::cout << "error\n";
	}
	icol = col - '0';							//cast the column coordinate to an int
	//check if the user made a hit
	if (state[irow][icol] != '-'){
		std::cout << "hit\n";
		strcpy(temp.saved_result, "hit");			//record the result in the game log
		switch(state[irow][icol]){					//record the ship type hit in the game log
			case 'C':
				strcpy(temp.ship_hit, "carrier");
				break;
			case 'B':
				strcpy(temp.ship_hit, "battleship");
				break;
			case 'c':
				strcpy(temp.ship_hit, "cruiser");
				break;
			case 'S':
				strcpy(temp.ship_hit, "submarine");
				break;
			case 'D':
				strcpy(temp.ship_hit, "destroyer");
				break;
			default:
				std::cout << "error\n";
		}
		state[irow][icol] = 'X';					//indicate the hit on the game board
	}else{
		std::cout << "miss\n";
		strcpy(temp.saved_result, "miss");			//record the result in the game log
	}	
}

/**
 * Checks if a ship was sunk
 * @param state array that represents the game board
 * @param ship the ship to be checked
 * @return 1 if the ship was sunk, 0 otherwise
 */
int check_ship(char state[NUM_ROWS][NUM_COLS], Ship ship){
	int count = 0;
	if (ship.orientation == 0){
		//horizontal ship
		for (int i=0;i<ship.length;i++){
			//increment count for every 'X' in the ship's location
			if (state[ship.y][ship.x+i] == 'X'){
				++count;
			}
		}
	}else{
		//vertical ship
		for (int i=0;i<ship.length;i++){
			//increment count for every 'X' in the ship's location
			if (state[ship.y+i][ship.x] == 'X'){
				++count;
			}
		}
	}
	if (count == ship.length){
		return 1;	//return 1 if the whole ship is 'X'd out
	}else{
		return 0;	//otherwise return 0
	}
}

int createSendingSocket() {
    int sockfd, connfd; 
    struct sockaddr_in servaddr;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);  //initialize socket
	if (sockfd == -1){ 
		std::cout << "error creating socket\n";		
        exit(1); 
    }
    bzero(&servaddr, sizeof(servaddr));
	//fill out serv_addr's fields
	servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(ipAddress); 
    servaddr.sin_port = htons(*port);
	//connect the socket to the desired address and port
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		std::cout << "error connecting\n";	
        exit(1); 
    }
	std::cout << "socket conneted.\n";
	return sockfd;
}


int createListenSocket() {
    int sockfd, numbytes, newFd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
 
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 
 
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		std::cerr << "getaddrinfo: \n" << gai_strerror(rv) << std:: endl;
		exit(1);
    }
 

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            std::cout << "server: socket\n";
            continue;
        }
 
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            std::cout << "setsockopt\n";
            exit(1);
        }
 
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            std::cout << "server: bind\n";
            continue;
        }
 
        break;
    }
 
    freeaddrinfo(servinfo); 
 
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
		std::cerr << "server: failed to bind\n";
        exit(0);
    }
 
    if (listen(sockfd, backlog) == -1) {
        std::cout << "listen";
        exit(0);
    }

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        std::cout << "sigaction";
        exit(0);
    }
 
    printf("server: waiting for connections...\n");
 
    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        newFd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (newFd == -1) {
            std::cout << "accept";
            continue;
        }
    }

    int connfd;
	socklen_t len;
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        std::cout << "socket creation failed...\n"; 
        exit(0); 
    } 
    else
        std::cout << "Socket successfully created..\n"; 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(inet_addr(ipAddress)); 
    servaddr.sin_port = htons(*port); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        std::cout << "socket bind failed...\n"; 
        exit(0); 
    } 
    else
        std::cout << "Socket successfully binded..\n"; 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        std::cout << "Listen failed...\n"; 
        exit(0); 
    } 
    else
        std::cout << "Server listening..\n"; 
    len = sizeof(cli); 
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (SA*)&cli, &len); 
    if (connfd < 0) { 
        std::cout << "server acccept failed...\n"; 
        exit(0); 
    } 
    else
        std::cout << "server acccept the client...\n"; 
	return sockfd;
}