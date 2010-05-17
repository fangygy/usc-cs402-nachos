//********************************************************************
// Created by: 211263
// Last Date of Modification: 5/16/10
// GameBoardTester.java
//********************************************************************
import java.io.IOException;

public class GameBoardTester {
        
    public static void main(String[] args) throws IOException {
        
        Tile theTile;
        GameBoard gb = new GameBoard ("Tile.txt");
        for (int i = 0; i < 5; i++)
        {
        System.out.println("\nNEW COLUMN\n");
        for (int j = 0; j < 5; j++)
        {
          theTile = gb.getTile(i,j);
          System.out.println ("\n" + theTile); 
        }
        
        }
        
    }
}
