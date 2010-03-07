int findShortestLine(int line[], int length) {
  /* Create a variable to get the shortest line */
  int shortest_line[2];
  int i;
  /* Setting the shortest line to the first line, to compare later on
   shortest_line[0] is the length of the shortest length 
   shortest_line[1] is which line is the shortest line (lines 0-4)
  */
  shortest_line[0] = line[0];
  shortest_line[1] = 0;

  for( i = 1; i < length; i++) {
    if(shortest_line[0] > line[i]) {
      shortest_line[0] = line[i];
      shortest_line[1] = i;
    } /* end if */
  }  /* end for */
  return shortest_line[1];
}
int findCISShortestLine(int line[], int start, int stop) {
  /* Create a variable to get the shortest line */
  int shortest_line[2];
  int i;
  /* Setting the shortest line to the first line, to compare later on
     shortest_line[0] is the length of the shortest length 
     shortest_line[1] is which line is the shortest line (lines start-stop)
  */
  shortest_line[0] = line[start];
  shortest_line[1] = start;

  for(i = start; i <= stop; i++) {
    if(shortest_line[0] > line[i]) {
      shortest_line[0] = line[i];
      shortest_line[1] = i;
    } /* end if */
  } /* end for */
  return shortest_line[1];
}
