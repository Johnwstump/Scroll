Scroll is a custom implementation of more with automatic page scrolling. Written as part of a course on systems programming.

NAME  
	
    Scroll - read files in console with automatic scrolling

SYNOPSIS  
	
    Scroll [fileName]

DESCRIPTION  
	
>    Scroll is a program which displays text input in both screenful increments and single-row increments with variable-speed automatic scrolling. Input can be provided by specifying a given file or by using Scroll as a filter and piping/redirecting text input. Upon reaching the end of provided text, Scroll informs the user that the end of the file has been reached and ceases scrolling.

OPTIONS  
	
    None currently implemented.

COMMANDS 

    SPACE			Display the full screen of text and cease scrolling.

    RETURN			Toggle scrolling.

    F                       Increase scrolling speed by 20%

    S                       Decrease scrolling speed by 20%
	
    Q or INTERRUPT		Exit
	
DIAGNOSTICS  
	
    Exit status is normally 0; if an error occurs, exit status is 1.  

	Scroll: Cannot read file  
		Indicates that either the file in question could not be read.
	
	Scroll: Cannot close file  
		Indicates that a file could not be closed

	Scroll: Problem reading from keyboard  
		Indicates that there was a problem reading from the keyboard.
	
	Scroll: Cannot open file  
		Indicates that file could not be opened, likely because of an incorrect filename or incorrect permissions.  

	Scroll: Unrecognized option  
		Indicates that an option was indicated which is not implemented/supported at this time.  

	Scroll: Unrecognized input  
		Indicates that input was provided prior to the file name which was not in -option format.  

BUGS  
	
    None currently documented
