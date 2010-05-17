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
        
    private Tile [][] gameTiles;
        
    public GameBoard (String fileName) throws IOException {
        
	inputStream = new BufferedReader(new FileReader("Tile.txt"));
	String rowCount = inputStream.readLine();
	String colCount = inputStream.readLine();
	gameTiles = new Tile [5][5];
	String[] data = new String[105];
	String[] categories = new String[5];

	/* Read in the first 5 lines to get the category names */
	for(int i = 0; i < 5; i++) {
	    categories[i] = inputStream.readLine();
	}
	
	/* Read in the question and answers and points */
	for (int row = 0; row < 5; row++) {
	    for (int col = 0; col < 5; col++) {
		int numPoints       = Integer.parseInt(inputStream.readLine());
		String question     = inputStream.readLine();
		int bullshit        = Integer.parseInt(inputStream.readLine());
		String answer       = inputStream.readLine();
		Tile temp           = new Tile (question, answer, numPoints);
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

    /* When the user selects a category and points, show them the question */
    public String askQuestion(int row, int column) {
	/* Parse the string to get the question */
	Scanner scanner = new Scanner(gameTiles[row][column].toString());
	/* Return the question */
	return scanner.next();
    }
    /* Get the answer from the user and see if they answered it correctly */
    public void answerQuestion(String ques, String answer) {
	/* Prompt the user for the answer */
	System.out.println( "That's correct");
    }
    /* Compares */
    public boolean compareAnswers(String bonaFideAnswer, String userAnswer){
	if (userAnswer.equals(bonaFideAnswer))
	    return true;
	return false;
    }

    public int usersDesiredQuestionROW(String name){
	//need to import scann and instantiate scanner in game board
	int row;
	Scanner scan = new Scanner(System.in);
	System.out.println("So " + name + " what question would you like to answer or at least attempt to answer?");
	System.out.println("Give your desired question in coorindates.");
	System.out.println("How to figure out the coordinates of your desired question");
	System.out.println("1.) The Categories at the top are not part of the grid.");
	System.out.println("2.) The columns start with 0 at the most left column and increment by 1 integer each column to the right.");
	System.out.println("3.) The rows start with 0 at the top of the grid and increment by 1 integer each row down.");
	System.out.println(" ");
	System.out.print("Enter row of desired question:");
	row = scan.nextInt();
	return row;
    }
    
    public int usersDesiredQuestionCOLUMN (){
	int column;
	Scanner scan = new Scanner(System.in);
	System.out.print("Enter column of desired question:");
	column = scan.nextInt();
	return column;
    }    
    
    public void showGrid() {
	
    }

    public void startGame(int numPlayers) {
	Player[] players = new Player[numPlayers];
	for(int i=0; i < numPlayers; i++) {
	    players[i] = new Player();
	}

	/* Welcome players to the game */
	
	/* (Optional) Set Players Names */

	/* Begin asking questions */

	/* Tabulate the score */

	/* Determine the winner */

	/* Exit the program now */
    }


    /* Create a new class that stores information for each player */
    public class Player {

	private int score;

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
	
    } // End Player 
    
    public Tile getTile (int r, int c) {
	return gameTiles [r] [c];
    }

    
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
