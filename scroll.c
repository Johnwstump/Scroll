// John Stump
// COS 350
// Project 4
// April 9, 2016


// This program presents a custom 'more' file
// reader with automatic scrolling capabilities.

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <sys/time.h>
#include <math.h>
#include <limits.h>

short winCols;
short winRows;

char *buffer;
struct termios oldInfo;

int scrolling = 0;
int scrollSpeed = 2000000;

void showFile(int file);
int printRow();
void getWindowSize();
int getFileSize(int file);
int openFile(char fileName[]);
void readArguments(int argc, char *argv[]);
void oops(char message[], char entity[]);
void printPrompt();
void erasePrompt();
void printScreen(int hasPrompt);
void configureTerminal();
void startScrolling();
void stopScrolling();
void revertTerminal();
void getInput();
void toggleScrolling();
int getFile(int argc, char *argv[]);
int getNumDigits(double num);
void changeSpeed(double factor);
static void intHandler(int param);
static void timerHandler(int param);


main(int argc, char *argv[]){
	signal(SIGINT, intHandler);
	configureTerminal();
	readArguments(argc, argv);
	getWindowSize();
	showFile(getFile(argc, argv));
	revertTerminal();
	exit(EXIT_SUCCESS);
}

// Alters configures the terminal
// to disable echo and canonical mode.
void configureTerminal(){
  struct termios info;
  tcgetattr(1, &oldInfo);
  tcgetattr(1, &info);
  info.c_lflag &= ~ECHO ;
  info.c_lflag &= ~ICANON ;
  info.c_cc[VMIN] = 1;       
  tcsetattr(1,TCSANOW,&info);
}

// Reverts the terminal to its 
// pre-program state.
void revertTerminal(){
	tcsetattr(1,TCSANOW,&oldInfo);
}

// Reads in and displays the first screen
// of provided file, gets user input, then
// closes the file.
void showFile(int file){
	int fileSize = getFileSize(file);
	buffer = malloc(sizeof(char) * fileSize);
	if (read(file, buffer, fileSize) == -1){
		oops("Cannot read file.", "");
	}
	printScreen(0);
  	signal(SIGALRM, timerHandler);   // install time handler
	
	getInput();	
	
	if (file != 0 && close(file) == -1){
		oops("Cannot close file:", ""+file);
	}
}

// Gets the user input. 
void getInput(){
	FILE *tty;
	char buf;
	
	if ((tty = fopen("/dev/tty", "r" )) == NULL){
			oops("Problem reading from keyboard.", "/dev/tty");
	}
	while (1){
		buf = getc(tty);
		if (buf == ' '){
			stopScrolling();
			printScreen(1);
		}
		if (buf == '\n'){
			toggleScrolling();	
		}
		if (buf == 'q'){
			erasePrompt();
			break;	
		}
		if (buf == 'f'){
			changeSpeed(0.8);
		}
		if (buf == 's'){
			changeSpeed(1.2);
		}
	}
}

// Changes the current scrolling speed by
// altering timer interval value.
void changeSpeed(double factor){
	int newSpeed = scrollSpeed;
	newSpeed *= factor;
	
	erasePrompt();
	
	// This handles overflows
	if (factor > 1 && newSpeed < scrollSpeed){
		scrollSpeed = INT_MAX;
	}
	else if (newSpeed < 0){
		scrollSpeed = 0;
	}
	else {
		scrollSpeed = newSpeed;			
	}
	
	printPrompt();
	
	// If already scrolling, update
	// scrolling speed.
	if (scrolling){
		startScrolling();
	}
}

// Toggles timer on or off,
// as appropriate.
void toggleScrolling(){
	erasePrompt();
	if (scrolling){
		stopScrolling();	
	}
	else {
		startScrolling();	
	}
	printPrompt();
}

// Enables timer
void startScrolling(){
	struct itimerval timer;
	int sec = scrollSpeed / 1000000;
	int usec = scrollSpeed % 1000000;
  	timer.it_value.tv_sec = sec;
  	timer.it_value.tv_usec = usec; 
  	timer.it_interval.tv_sec = sec;
  	timer.it_interval.tv_usec = usec;  
  	setitimer(ITIMER_REAL, &timer, NULL);
	scrolling = 1;
}

// Disable timer
void stopScrolling(){
  struct itimerval timer;
  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 0;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 0;  
  setitimer(ITIMER_REAL, &timer, NULL);
  scrolling = 0;
}

// Prints a screen full of lines.
void printScreen(int hasPrompt){
	if (hasPrompt){
		erasePrompt();
	}
	int numRows = 0;
	while (numRows < winRows - 1){
		if (!printRow()){
			printf("End of file.\n");
			break;
		}
		numRows++;
	}
	
	printPrompt();
}

// Erases the prompt for this program.
void erasePrompt(){
	// Length of prompt minus numbers
	int n = 51;
	double interval = (scrollSpeed / 1000000.0);
	n += getNumDigits(interval);
    if (scrolling){
		n += 2;	
	}
	else {
		n += 3;	
	}
	
	while (n > 0){
		printf("\b");
		n--;
	}
	printf("\033[0K"); 
	fflush(stdout);
}

// Gets the number of digits to the 
// left of the decimal place.
int getNumDigits(double num){
    if(num >= 1000000000) return 10;
    if(num >= 100000000) return 9;
    if(num >= 10000000) return 8;
    if(num >= 1000000) return 7;
    if(num >= 100000) return 6;
    if(num >= 10000) return 5;
    if(num >= 1000) return 4;
    if(num >= 100) return 3;
    if(num >= 10) return 2;
    return 1;
}	

// Prints the prompt for this program.
void printPrompt(){
	double interval = (scrollSpeed / 1000000.0);
	printf("\033[7m");
	printf("Current Speed: 1 line per %.3f seconds  Scrolling: ", interval);
	if (scrolling){
		printf("On");
	}
	else{
		printf("Off");
	} 
	printf("\033[0m");
	fflush(stdout);
}

// Cleans up the program prior to exit.
static void intHandler(int param){
	erasePrompt();
	revertTerminal();	
	exit(0);
}

// Prints another row of text each time
// timer is triggered.
static void timerHandler(int param){
  erasePrompt();
  if (!printRow()){
		printf("End of file.\n");
		stopScrolling();
  }
  printPrompt();
  fflush(stdout);
}

// Prints a single row of text from 
// the input.
int printRow(){
	
	char *rowBuffer = malloc(sizeof(char) * winRows);
	
	// Track our leading iterator
	int i = 0;
	// Track our trailing iterator, which
	// points to the last thing which we know 
	//will be printed.
	int k = 0;
	// Keeps track of the current row length.
	int rowLength = 0;
	// Keeps track of the current word length, so
	// we can make sure it won't overflow the row.
	int wordLength;
	// Psuedo-bool indicating whether we have a word.
	int hasWord;
	// Psuedo-bool indicating whether we have a complete
	// line
	int hasLine = 0;
	// The total size of input.
	int len = strlen(buffer);
	
	while (rowLength < winCols && !hasLine){
		
		hasWord = 0;
		wordLength = 0;
		
		if (i >= len){
			hasLine = 1;	
		}
		
		// While we don't have a complete word.
		while (i < len && !hasWord){
			// Handle tabs.
			if (buffer[i] == '\t'){
				wordLength += ((wordLength + rowLength) % 8);
				hasWord = 1;
			}
			// Handle newlines.
			else if (buffer[i] == '\n'){
				hasWord = 1;
				hasLine = 1;
			}
			// Handle spaces.
			else if (buffer[i] == ' '){
				wordLength++;
				hasWord = 1;	
			}
			else {
				wordLength++;
			}
			i++;
		}
				
		if ((rowLength + wordLength) < winCols){
			rowLength += wordLength;	
			k = i;
		}
		else {
			hasLine = 1;	
		}
	}
	
	printf("%.*s", k, buffer);
	
	if (buffer[k - 1] != '\n' && buffer[k] != '\r'){
		printf("\n");
	}
	
	// Pointer math to keep track of our
	// place in the buffer.
	buffer = buffer + k;
	
	// Returns whether we've reached the end of
	// the file.
	return i < len;
}

// Reads the current window size and sets
// global fields representing the length
// and width of the current window.
void getWindowSize(){
	struct winsize *winSizeStruct = malloc(sizeof(struct winsize));
	ioctl(1, TIOCGWINSZ, winSizeStruct);
	winCols = winSizeStruct->ws_col;
	winRows = winSizeStruct->ws_row;
}
			 
 // Returns the size of a provide file
int getFileSize(int file){
	int fileSize = lseek(file, 0, SEEK_END) + 1;
	// Place cursor back at beginning of file.	
	lseek(file, 0, SEEK_SET);
	return fileSize;
}

// Calls open on the provide filename and handles
// errors.
int openFile(char fileName[]){
	int rfile = open(fileName, O_RDONLY);	

	if (rfile == -1){
		oops("Cannot open file:", fileName);
	}
	
	return rfile;
}

// Handles errors and presents them in a generic format.
// Takes a message to be printed, and a string description
// of the problem entity. Concludes by exiting program.
void oops(char message[], char entity[]){
	erasePrompt();
	fprintf(stderr, "scroll: %s %s \n scroll: Operation "
			"could not be completed.\n", message, entity);
	revertTerminal();
	exit(EXIT_FAILURE);
}

// Gets the file to be read from
// Returns the fd obtained from opening
// the file or 0 if no file is provided and
// text is to be read from stdin.
int getFile(int argc, char *argv[]){
	if (argc > 1){
		return openFile(argv[argc - 1]);	
	}
	else{
		return 0;	
	}

}

// Reads arguments and calls methods to handle any present.
void readArguments(int argc, char *argv[]){	
	int i;
	// Loop through all arguments aside from first
	for(i = 1; i < argc - 1; i++){
		// If presented in argument format, switch through
		// and handle possible arguments.
		// None currently implemented.
		if (argv[i][0] == '-'){
			switch (argv[i][1]){
			default:
				oops("Unrecognized Option:", argv[i]);
			}
		}
		else {
			oops("Unrecognized input:", argv[i]);
		}
	}
}