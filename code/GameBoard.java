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
    public String askQuestion(int cat, int points) {
	/* Parse the string to get the question */
	Scanner scanner = new Scanner(gameTiles[cat][(points/100)-100].toString());
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
