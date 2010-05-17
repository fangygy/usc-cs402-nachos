//********************************************************************
// Created by: 211263
// Last Date of Modification: 5/16/10
// GameBoard.java
//********************************************************************

import java.io.FileReader;
import java.io.FileWriter;
import java.io.BufferedReader;
import java.io.PrintWriter;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.lang.*;
import java.util.*;

public class GameBoard {
                    
    private BufferedReader inputStream; 
        
    private newTile [][] gameTiles;
    String[] categories;
        
    public GameBoard (String fileName) throws IOException {
        
	inputStream = new BufferedReader(new FileReader("Tile.txt"));
	String rowCount = inputStream.readLine();
	String colCount = inputStream.readLine();
	gameTiles = new newTile [5][5];
	String[] data = new String[105];
	categories = new String[5];

	/* Read in the first 5 lines to get the category names */
	for(int i = 0; i < 5; i++) {
	    categories[i] = inputStream.readLine();
	}
	
	/* Read in the question and answers and points */
	for (int col = 0; col < 5; col++) {
	    for (int row = 0; row < 5; row++) {
		int numPoints       = Integer.parseInt(inputStream.readLine());
		String question     = inputStream.readLine();
		int bullshit        = Integer.parseInt(inputStream.readLine());
		String answer       = inputStream.readLine();
		newTile temp        = new newTile (question, answer, numPoints);
		gameTiles[row][col] = temp;
	    }
	}
	
	try {
	    
	    String line;
	    int lineCount = 0;
	    /*
	      while ((line = inputStream.readLine()) != null) {
	      System.out.println ("\nLine: " + lineCount + ": " + line);                
	      String thisLine = line;
	      data[lineCount] = line;
	      lineCount++;
	      }
	    */
	} 
	
	finally {
	    if (inputStream != null) {
		inputStream.close();
	    }
	    /* 
	    System.out.println ("\n\nQuestion: " + data[0]);
	    System.out.println ("Answer: " + data[1]);
	    System.out.println ("Value: " + data[2]);
	    */
	}
	
    } // End Constructor



    /*
     * Application Code 
     *
     *
     */

    /* When the user selects a category and points, show them the question */
    public void askQuestion(int row, int column) {
	
	/* Prints the question to console */
	System.out.println(gameTiles[row][column].getQuestion());
    }

    /* Get the answer from the user and see if they answered it correctly */
    public boolean answerQuestion(String answer, int row, int column) {
	String correctAnswer = gameTiles[row][column].getAnswer();

	if(compareAnswers(correctAnswer,answer)) {
	    System.out.println("That's correct");
	    return true;
	} else {
	    System.out.println("I'm sorry that's wrong");
	    return false;
	}
    }

    /* Compares the answer the user gave and the answer to the question */
    public boolean compareAnswers(String bonaFideAnswer, String userAnswer){
	if (userAnswer.equals(bonaFideAnswer))
	    return true;
	return false;
    }
    
    /* Prompt user for the row */
    public int usersDesiredQuestionROW(String name){
	//need to import scann and instantiate scanner in game board
	int row;
	Scanner scan = new Scanner(System.in);
	System.out.print("Enter points:");
	row = scan.nextInt();
	row = (row-100)/100;
	return row;
    }
    
    /* Prompt user for the column */
    public int usersDesiredQuestionCOLUMN (){
	String category;
	int column;
	column = 0;
	Scanner scan = new Scanner(System.in);
	System.out.print("Enter category:");
	category = scan.nextLine();
	for(int i = 0; i < 5; i++) {
	    if(categories[i].equals(category)) {
		column = i;
		break;
	    }
	}
	return column;
    } 

    /* Use the function for formatting the string so the grid appears organized */
    public void formatString(String string) {
	int length = string.length();
	System.out.print(string);
	for(int i = 0; i < (20-length);i++) {
	   System.out.print(" ");
	}
    }
    
    /* Show the grid of questions to the user */
    public void showGrid() {
	System.out.println();
	String[] grid = new String[5];	
	for(int i = 0; i < 5; i++) {
	    formatString(categories[i]);
	}
	System.out.println();
	for(int i = 0; i < 5; i++) {
	    for(int g = 0; g < 5; g++) {
		if(!gameTiles[i][g].isAnswered()) {
		    formatString(gameTiles[i][g].getPointsString());
		} else {
		    formatString("Done");
		}
	    } 
	    System.out.println();
	}
	System.out.println();
    }

    /* Sets the question to has been answered */
    public void setAnswered(int row, int column) {
	gameTiles[row][column].setIsAnswered(true);
    }

    /* Gets the points for the question */
    public int getQPoints(int row, int column) {
	return gameTiles[row][column].getPoints();
    }

    /* Returns true if there are still answers left */
    public boolean questionsLeft() {
	for(int i = 0; i < 5; i++) {
	    for(int g = 0; g < 5; g++) {
		if(!gameTiles[i][g].isAnswered) {
		    /* There exists a question that has not been answered */
		    return true;
		}
	    }
	}
	/* If we get to the end that means all questions have been answered */
	return false;
    }

    /* Starts the game */
    public void startGame(int numPlayers) {
	Player[] players = new Player[numPlayers];
	for(int i=0; i < numPlayers; i++) {
	    players[i] = new Player();
	}

	Scanner scan = new Scanner(System.in);
        
        /* Welcome players to the game */
	System.out.println("Why hello there.....");
	System.out.println("WELCOME TO 211263's JEOPARDY EXTRA CREDIT ASSIGNMENT AMIGO!");
	System.out.println("READY TO PLAY SOME JEOPARDY?");
	System.out.println("WELL I DON'T REALLY CARE IF YOU'RE READY OR NOT YOU'RE GOING TO PLAY!");
	System.out.println("Here we go.................");
	
        /* (Optional) Set Players Names */
	for(int i = 0; i < numPlayers; i++) {
	    System.out.println("What would you like to call yourself player" + (i+1) + "?");
	    players[i].setName(scan.nextLine());
	}
	
        /* Begin asking questions */
	while (questionsLeft()) {
	    for(int i = 0; i < numPlayers; i++){
		showGrid();
		int column = usersDesiredQuestionCOLUMN();
		int row = usersDesiredQuestionROW(players[i].getName());
		
		askQuestion(row, column);
		System.out.print("Your answer in the form of a question:");
		String answer = scan.nextLine();
		boolean didYouGuessRight = answerQuestion(answer, row, column);
		setAnswered(row, column);
		
		if (didYouGuessRight)
		    players[i].addScore(getQPoints(row, column));
		else
		    players[i].subScore(getQPoints(row, column));
		
	    }
	}
        /* Tabulate the score */
	//Do I need anything?
        /* Determine the winner */
	int max = 0;
	for(int i = 1; i < numPlayers; i++) {
	    if(players[i].getScore() > players[max].getScore()) {
		max = i;
	    }
	}
	
	System.out.println( players[max].getName() + " IS THE WINNER!!!!!");
	
        /* Exit the program now */
	System.out.println("This game of jeopardy is over!");
	System.out.print("ADIOS");
    }
    
    
    /*
     * NEW CLASSES 
     * User Class and newTile Class
     *
     */


    /* Create a new class that has appropriate accessors and mutators for data */
    public class newTile {
	private String question;
	private String answer;
	private Integer value;
	private boolean isAnswered;
	
	public newTile(String q, String a, int v) {
	    
	    isAnswered = false;
	    question = q;
	    answer = a;
	    value = v;
	}
	
	public String getQuestion() {
	    return question;
	}
	
	public String getAnswer() {
	    return answer;
	}

	public int getPoints() {
	    return value; 
	}
	
	public String getPointsString() {
	    return value.toString();
	}
	
	public boolean isAnswered() {
	    return isAnswered;
	}
	public void setIsAnswered(boolean yesNo) {
	    isAnswered = yesNo;
	}
	
	public String toString() {
	    String s = question;
	    s+= "\n" + answer;
	    s+= "\nScore: " + value;
	    return s;
	}
    }
    
    public newTile getTile (int r, int c) {
	return gameTiles [r] [c];
    }
    
    /* Create a new class that stores information for each player */
    public class Player {

	private int score;
	private String name;
	
	public Player() {
	    score = 0;
	}

	/* Return the players score */
	public int getScore() {
	    return score;
	}
	/* When a player gets the answer right, add points */
	public void addScore(int points) {
	    score+=points;
	}

	/* When a player gets the answer wrong, subtract points */
	public void subScore(int points) {
	    score-=points;
	} 
	
	public void setName(String playerName) {
	    name = playerName;
	}

	public String getName() {
	    return name;
	}
	
    } // End Player 
    

    /*
     * MAIN METHOD 
     *
     *
     */ 
    
    public static void main(String args[]) {
	int numPlayers = 4;
	
	/* Create the game */
	try {
	    GameBoard newGame = new GameBoard("newGame");
	    newGame.startGame(numPlayers);
	} catch (Exception e) {
	    e.printStackTrace();
	}
    }    
}
