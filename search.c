#include<stdio.h> 
#include<string.h>
#include<stdlib.h>
#include <sys/stat.h>
#include<mpi.h>

// char_num is the number of characters in the input alphabet 
#define char_num 256
double mintime = -1;
struct stat st;
double myelapsed;

/* pattern -> pattern 
    text -> string text 
    q -> A prime number 
*/

void search(char pattern[], char text[], int q, int start, int end) 
{ 
    int M = strlen(pattern);
    int i, j; 
    int p = 0; // hash value for pattern 
    int t = 0; // hash value for text 
    int h = 1; //initial hash value
      
    // The value of initial hash value(h) is "pow(char_num, M-1)%q" 
    for (i = 0; i < M-1; i++) 
        h = (h*char_num)%q; 
  
    // Calculate the hash value of pattern  
    for (i = 0; i < M; i++) 
    { 	
        p = (char_num*p + pattern[i])%q; 
    } 
    // Calculate the hash value of first window of text
    for( i= start; i<start+M; i++){
 	    t=(char_num*t + text[i])%q;
    }
    // Glide the pattern over text one by one 
    for (i = start; i <= end - M+1; i++) 
    { 
        // the hash values of current window of text and pattern is checked.  
        if ( p == t ) 
        { 
            //If the hash values match then only check for characters on by one
            for (j = 0; j < M; j++) 
            { 
          
                if (text[i+j] != pattern[j]) 
                    break; 
            } 
  
            if (j == M) 
                printf("Pattern found at index %d \n", i); 
        } 
  
        // Calculate hash value for next window of text by removing the 
        // Most significant character and adding the least significant character
           if ( i < end-M ) 
        { 
            t = (char_num*(t - text[i]*h) + text[i+M])%q; 
            if (t < 0) 
            t = (t + q); 
        } 
    } 
} 
  
int main() 
{      
    FILE *fp ;
    fp = fopen("text3.txt", "r"); // reading the text file
    stat("text3.txt", &st); // size of the text file
    char c;
    char *text;
    long lsize;
    fseek( fp , 0L , SEEK_END);
    lsize = ftell( fp );
    rewind( fp );
    char pattern[] = "information"; // pattern to search in the text file
    size_t k=0;
    int comm_sz;// total processes
    int my_rank;// particular process rank
    int start, end;
    /* allocate memory for entire text file */
    text = calloc( 1, lsize+1 );
    if( !text ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

    /* copy the file into the buffer */
    if( 1!=fread( text , lsize, 1 , fp) )
    fclose(fp),free(text),fputs("entire read fails",stderr),exit(1);
    int q = 101; //suitable prime number for characters
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
 
    if(lsize < comm_sz-1)
    {
	if(my_rank == 0){
         printf("Rerun the program with single task as the size of input file is small");
	 return 0;
	}
	return 0;
    }
    
    if(my_rank !=0)
    {
    	double mystart = MPI_Wtime();
    	start = (my_rank-1)*(lsize/(comm_sz-1));
    	if(my_rank!=comm_sz-1){ 
    		end = start +(lsize/(comm_sz-1)) - 1 + strlen(pattern);
     	}
    	else
    	{
     		end = start + (lsize/(comm_sz-1)) -1+(lsize%(comm_sz-1)) ;
    	}
    // search function call 
    search(pattern, text, q,start,end);
    double myfinish = MPI_Wtime();
    myelapsed = myfinish - mystart;
    MPI_Send(&myelapsed, 1,MPI_DOUBLE,0,0,MPI_COMM_WORLD);
    }
    /* Find the max time elapsed among all processes/threads */
    else{
       	for(int q=1; q < comm_sz; q++){
           MPI_Recv(&myelapsed,1, MPI_DOUBLE, q,0,MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	  
	   if(mintime <  myelapsed){
           	mintime = myelapsed ;
	   }
	 }	
       printf("\ntotal time for parallel search is %f\n",mintime);
    }
 
    MPI_Finalize(); 
 
    return 0; 
}
