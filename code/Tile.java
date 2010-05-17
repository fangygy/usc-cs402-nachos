/**
 * @(#)Tile.java
 *
 *
 * @author 
 * @version 1.00 2010/5/5
 */


public class Tile {

    private String question;
    private String answer;
    private int value;
    private boolean isAnswered;
    
    public Tile(String q, String a, int v) {
    	
    	isAnswered = false;
    	question = q;
    	answer = a;
    	value = v;
    }
    
    public String toString ()
    {
    	String s = question;
    	s+= "\n" + answer;
    	s+= "\nScore: " + value;
    	return s;
    }
    
    
}
