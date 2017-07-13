#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <limits.h>

/*
 *  Variable for -f (# of available frames, Default: 5)
 */
int frame_num = 5;
int* frame;

/*
 *  Variable for -r (replacement-policy, Default: FIFO)
 *  [ FIFO, LFU, LRU-STACK, LRU-CLOCK, LRU-REF8 ]
 */
char* policy;

// Input File
char input_file[20];
bool file_exist = false;

// Input page number array, and a size of the array.
int* page_num;
int page_num_count;


// Optimal Algorithm Variables
int opt_pagefault;
double opt_start_time;
double opt_end_time;

// Other Algorithm variables
int pagefault = 0;
double start_time;
double end_time;

// Core Methods
void preset(void);
void perform_replacement(void);
bool check_frame(int*, int);

// sort algo
int compare(const void*, const void*);

// Algorithm Methods
void fifo_algorithm(void);
void optimal_algorithm(void);
void lfu_algorithm(void);
void lru_stack_algorithm(void);
void lru_clock_algorithm(void);
void lru_ref8_algorithm(void);

// Debugging Methods
void print_int_array(int[], int);

void print_usage(){
        printf("Option\tArgument\t\tExplanation\n");
        printf("-----------------------------------------------------------------------------------------\n");
        printf("-h\t\t\t\tPrint a usage summary with all options and exit.\n");
        printf("-f\tavailable-frames\tSet the number of available frames. By default it is 5.\n");
        printf("-r\treplacement-policy\tSet the page replacement policy. (Default: 5)\n");
        printf("\t\t\t\t {FIFO, LFU, LRU-STACK, LRU-CLOCK, LRU-REF8}\n");
        printf("-i\tinput_file\t\tRead the page reference sequence from a specified file. \n\t\t\t\tIf not given, read from STDIN(ended with ENTER).\n\n");

}


int main(int argc, char* argv[])
{
    bool policy_specified = false;

	if(argc != 1){	
    
	    char command = argv[1][1];
        int counter = 1;
        while(counter < argc){
    	    switch(argv[counter][1]) {    
	          	case 'h' :
                    printf("[ Usage Summary ]\n");
			        print_usage();
		    	    exit(0);
		            break;

	    	    case 'f' :
	    		    printf("*** Frame Number Received: %s\n",argv[counter+1]);
                    frame_num = atoi(argv[counter+1]);
                    counter++;
	    	        break;
		
	    	    case 'r' :
					counter++;
	    		    printf("*** Replacement Policy Received: %s \n", argv[counter]);
                    policy = malloc(sizeof(argv[counter]));
                    strcpy(policy, argv[counter]);
                    policy_specified = true;
    		        break;

    		    case 'i' :
					counter++;
	    		    printf("*** Input File Received: %s\n", argv[counter]);
                    strcpy(input_file, argv[counter]);
                    file_exist = true;
    	    	    break;
        
    	    	default :
	    	    	printf("ERROR: Wrong Command.\n");
		            break;
    	    }
            counter++;
        }
    }

    if(!policy_specified){
        policy=malloc(4*sizeof(char));
        policy="FIFO";
    }
    
    preset();   

    perform_replacement();

	return 0;
	
}

void preset(){
    
    /*
     *  Gets Page string
     */
    char num_str[254];
    strcpy(num_str,"");
    if(file_exist){
        FILE *opened_file = fopen(input_file,"r");
        char temp_c[254];
        if(opened_file != NULL) {
            while(fgets(temp_c,sizeof(temp_c), opened_file)) {
                strcat(num_str, temp_c);
            }
            fclose(opened_file);
        }  
    }else{
        if(fgets(num_str,1024,stdin)==NULL){
        };
    }


    /*
     *  Put page string into int list
     */
    int temp_page_num[254];
    page_num_count = 0;
    char* split_num_str = strtok(num_str, " ");
    while(split_num_str != NULL){
        char temp_c[254];
        strcpy(temp_c,split_num_str);
        int temp_int = atoi(temp_c);

        /*  Capturing Non-integer Type
         *
        if(temp_int==0){
            printf("Non integer-type input captured. (0 is not allowed)\n");
            exit(0);
        }
        */
        temp_page_num[page_num_count] = atoi(temp_c);
        split_num_str=strtok(NULL," ");
        page_num_count++;
    }
    
    page_num = (int*) malloc(page_num_count * sizeof(int));
    int index;
    for(index = 0; index < page_num_count; index++){
        page_num[index] = temp_page_num[index];
    }
    
    frame = (int*) malloc(frame_num * sizeof(int));

    printf("*** Input Page Numbers\n");
    print_int_array(page_num, page_num_count);


}

void perform_replacement(){
    if(strcmp(policy,"FIFO")==0){
        printf("FIFO Starts.\n");
        // Perform FIFO
		start_time = clock();
		fifo_algorithm();
		end_time = clock();

    }else if(strcmp(policy,"LFU")==0){
        printf("LFU starts.\n");
		// Perform LFU
		start_time = clock();
		lfu_algorithm();
		end_time = clock();


    }else if(strcmp(policy,"LRU-STACK")==0){
        printf("LRU-STACK starts.\n");
		// Perform LRU-STACK
		start_time = clock();
		lru_stack_algorithm();
		end_time = clock();

    }else if(strcmp(policy,"LRU-CLOCK")==0){
        printf("LRU-CLOCK starts.\n");
		// Perform LRU-CLOCK
		start_time = clock();
		lru_clock_algorithm();
		end_time = clock();


    }else if(strcmp(policy,"LRU-REF8")==0){
        printf("LRU-REF8 starts.\n");
		// Perform LRU-REF8
        start_time = clock();
        lru_ref8_algorithm();
        end_time = clock();

    }else{
		printf("ERROR: Policy name is Invalid -> %s\n",policy);	
		exit(0);
	}

    double msec = (end_time - start_time) * 1000 / CLOCKS_PER_SEC;
    
    opt_start_time = clock();
    optimal_algorithm();
    opt_end_time = clock();

    int replacement = pagefault - frame_num;
    int opt_replacement = opt_pagefault - frame_num;

    double opt_msec = (opt_end_time - opt_start_time) * 1000 / CLOCKS_PER_SEC;
    printf("\n# of page replacements with %s\t: %d\n", policy, replacement);
    printf("# of page replacements with Optimal\t: %d\n", opt_replacement);
    double penalty = ((double)(replacement - opt_replacement) / opt_replacement) * 100;
    printf("%% page replacement penalty using %s\t: %.1f%%\n",policy, penalty);

    printf("\nTotal time to run %s algorithm\t: %f msec\n", policy, msec);
    printf("Total time to run Optimal algorithm\t: %f msec\n", opt_msec);
    double t_diff_percent = (opt_msec-msec)/opt_msec*100;
    printf("%s is %.1f%% faster than Optimal algorithm.\n", policy, t_diff_percent);

}



void lru_clock_algorithm() {
	int* lru_clock_frame = (int*) calloc(frame_num, sizeof(int));
	int* time_frame = (int*) calloc(frame_num, sizeof(int));
	int* bit_frame = (int*) calloc(frame_num, sizeof(int));
	
	int empty_frame = 0;

	int i;
	for(i = 0; i < page_num_count; i++){
		int cur_page = page_num[i];
	
		// HIT
		if(check_frame(lru_clock_frame, cur_page) && !(empty_frame < frame_num)) {
	//		printf("HIT!! \n");
			int j;
			for(j = 0; j < frame_num; j++) {
				if(cur_page == lru_clock_frame[j]) {
					bit_frame[j] = 1;
					break;
				}
			}
			continue;
		}
	
		// Empty exists
		if(empty_frame < frame_num) {
			// Empty with HIT
			if(check_frame(lru_clock_frame, cur_page) == true) {
	//			printf("hit empty\n");
				int j;
				for(j = 0; j < frame_num; j++) {
					if(cur_page == lru_clock_frame[j]) {
						bit_frame[j] = 1;
						break;
					}
				}
			// Empty normal
			} else {
	//			printf("empty!!\n");
				lru_clock_frame[empty_frame] = cur_page;		
				time_frame[empty_frame] = i;
				empty_frame++;
			//	pagefault++;			
			}
			continue;
		}
		
		int sorted_time_frame[frame_num];
		// copy time frame to sort
		int j;
		for(j = 0; j < frame_num; j++) {
			sorted_time_frame[j] = time_frame[j];
		}

		qsort(sorted_time_frame, sizeof(sorted_time_frame)/sizeof(*sorted_time_frame), sizeof(*sorted_time_frame), compare);				
	//	printf("sorted : "); print_int_array(sorted_time_frame, frame_num);

		int target = -1; 
		for(j = 0; j < frame_num; j++) {
			int k;
			for(k = 0; k < frame_num; k++) {
				if(sorted_time_frame[j] == time_frame[k]){
					if(bit_frame[k] == 1) {
						bit_frame[k] = 0;
						break;
					} else {
						target = k;
						break;		
					} 
				}
			}	
			if( target != -1) {
				break;
			}	
		}
		lru_clock_frame[target] = cur_page;
		time_frame[target] = i;
		bit_frame[target] = 0;
		pagefault++;

	}
	//printf("LRU-CLOCK : "); print_int_array(lru_clock_frame, frame_num);
	//printf("Final bit : "); print_int_array(bit_frame, frame_num);
}

void lru_stack_algorithm() {
	int* lru_stack_frame = (int*) calloc(frame_num, sizeof(int));

	int empty_frame = 0;
	
	int i = 0;
	for(i = 0; i < page_num_count; i++) {
		int cur_page = page_num[i];

//		print_int_array(lru_stack_frame, frame_num);
		
		// When HIT happens
		if(check_frame(lru_stack_frame, cur_page) && !(empty_frame < frame_num)) {
			int j = 0;
			for(j = 0; j < frame_num; j++) {
				if(lru_stack_frame[j] == cur_page) {
					int k;
					for(k = j + 1; k < frame_num; k++) {
						lru_stack_frame[k-1] = lru_stack_frame[k];
					}
					lru_stack_frame[frame_num-1] = cur_page;
				}
			}			
			continue;
		}

		// Empty exists
		if(empty_frame < frame_num) {
			//Empty with HIT
			if(check_frame(lru_stack_frame, cur_page) == true){
				int j = 0;
				for(j = 0; j < frame_num; j++) {
					if(lru_stack_frame[j] == cur_page) {
						int k = 0;
						for(k = j + 1; k < empty_frame; k++){
							lru_stack_frame[k-1] = lru_stack_frame[k];
						}
						lru_stack_frame[empty_frame-1] = cur_page;
						break;						
					}
				}
			//Empty normal
			} else {
				lru_stack_frame[empty_frame] = cur_page;						 empty_frame++;
				//pagefault++;
			}
			continue;
		}		
		int j;
		for(j = 1; j < frame_num; j++){
			lru_stack_frame[j-1] = lru_stack_frame[j];
		}
		pagefault++;
		lru_stack_frame[frame_num-1] = cur_page;
	}
//	printf("Finished : ");print_int_array(lru_stack_frame, frame_num);
//	printf("pagefault : %d\n", pagefault);
}

void lfu_algorithm() {
	int* lfu_frame = (int*) calloc(frame_num, sizeof(int));
	int* time_frame = (int*) calloc(frame_num, sizeof(int));
	int* freq_frame = (int*) calloc(frame_num, sizeof(int));
	
	int empty_frame = 0;

	int i = 0;
	for(i = 0; i< page_num_count; i++) {
//		printf("lfu_frame : ");
	//	printf("in>>> %d\n", page_num[i]);

		//print_int_array(lfu_frame, frame_num);

		//printf("freq_frame : ");
		//print_int_array(freq_frame, frame_num);
		int cur_page = page_num[i];
		
		// When HIT happens + time frame update required 
		if(check_frame(lfu_frame, cur_page) && !(empty_frame < frame_num)){
			int j = 0;
			for(j = 0; j < page_num_count; j++){
				if(lfu_frame[j] == cur_page) {
					// update index
					//time_frame[j] = i;
					freq_frame[j]++;
					break;
				}
			}
			//printf("HIT !!\n");
	//		print_int_array(lfu_frame, frame_num);
	//		print_int_array(freq_frame, frame_num);
	//		printf("pagefault : %d\n", pagefault);
			continue;			
		}
		// When there exists empty frame
		if(empty_frame < frame_num){
			if(check_frame(lfu_frame, cur_page)) {
				int j;
				for(j = 0; j< frame_num; j++) {
					if(lfu_frame[j] == cur_page) {
						freq_frame[j]++;
						break;
					}
				}
			} else {
				lfu_frame[empty_frame] = cur_page;
				time_frame[empty_frame] = i;
				freq_frame[empty_frame] = 1;
				empty_frame++;
				
				//print_int_array(lfu_frame, frame_num);
				//print_int_array(freq_frame, frame_num);
				//printf("pagefault : %d\n", pagefault);
			}
			continue;
		}
		
		// Replace a frame
		// 1. Get the least freq
		int least_freq = freq_frame[0];
		int oldest;
		//printf("Least_freq : %d\n", least_freq);
		for(int j = 0; j < frame_num; j++){
		//	printf("j : %d  /  freq_frame : %d\n", j, freq_frame[j]);
			if(freq_frame[j] < least_freq) {
				least_freq = freq_frame[j];
				oldest = j;
			}
		}
	//	printf("picked least : %d\n", least_freq);
	
	//	printf("time_frame : ");
		for(int j = 0; j < frame_num; j++) {
			if(least_freq == freq_frame[j] ) {
	//			printf("time_frame[j] : %d <= time_frame[oldest] : %d\n", time_frame[j], time_frame[oldest]);
				if(time_frame[j] <= time_frame[oldest]) {
					oldest = j;
				}
			}
		}
//		print_int_array(freq_frame, frame_num);
//		print_int_array(time_frame, frame_num);
		lfu_frame[oldest] = cur_page;
		time_frame[oldest] = i;
		freq_frame[oldest] = 1;
		pagefault++;
	//	print_int_array(lfu_frame, frame_num);
	//	print_int_array(freq_frame, frame_num);
		//printf("pagefault : %d\n", pagefault);
		//printf("%d\n",pagefault);
	}
	printf("*** LFU Algo ***\n");
	print_int_array(lfu_frame, frame_num);
}


/*
 *	FIFO Algorithm
 * 	- Replace oldest frame
 */
void fifo_algorithm(){
    int* fifo_frame = (int*) calloc(frame_num, sizeof(int));    
    int i;
    int empty_frame = 0;
    int* time_frame = (int*) calloc(frame_num, sizeof(int));

    for(i = 0; i < page_num_count; i++){
        int cur_page = page_num[i];

        // When it contains the page
        if(check_frame(fifo_frame, cur_page)){
//            printf("*** FIFO_frame: %dth page, %d ***\n",i+1,cur_page);
//            print_int_array(fifo_frame,frame_num);  
            continue;
        }

        // When empty space is available
        if(empty_frame < frame_num){
            fifo_frame[empty_frame] = cur_page;
	    time_frame[empty_frame] = i;	
            empty_frame++;
            pagefault++;
//            printf("*** FIFO_frame: %dth page, %d ***\n",i+1,cur_page);
//            print_int_array(fifo_frame, frame_num);
            continue;
        }

        // Replace a frame
        int j;
	int oldest=0;
	for(j=0; j < frame_num; j++){
		if(time_frame[j] < time_frame[oldest]){
			oldest = j;
		}
	}		

	fifo_frame[oldest] = cur_page;
	time_frame[oldest] = i;
	pagefault++;
//	printf("*** FIFO_frame: %dth page, %d ***\n",i+1,cur_page);
//	print_int_array(fifo_frame,frame_num);
    }    
}


/*
 *  Optimal Algorithm
 *  - Replace page that will not be used for longest period of time.
 */
void optimal_algorithm(){
    int* opt_frame = (int*) calloc(frame_num, sizeof(int));   
    opt_pagefault=0;
    int i;
    int empty_frame = 0;

    for(i = 0; i < page_num_count; i++){
        int cur_page = page_num[i];

        // When it contains the page
        if(check_frame(opt_frame, cur_page)){
//            printf("*** Opt_frame: %dth page, %d ***\n",i+1,cur_page);
//            print_int_array(opt_frame,frame_num);  
            continue;
        }

        // When empty space is available
        if(empty_frame < frame_num){
            opt_frame[empty_frame] = cur_page;
            empty_frame++;
            opt_pagefault++;
//            printf("*** Opt_frame: %dth page, %d ***\n",i+1,cur_page);
//            print_int_array(opt_frame, frame_num);
            continue;
        }

        // If last item, just replace 1st frame
        if(i == page_num_count-1){
            opt_frame[0] = cur_page;
            continue;    
        }

        // Replace a frame
        int* flag_frame = (int*) calloc(frame_num, sizeof(int));
        int flag_count = 0;
        int j;
        for(j=i+1; j < page_num_count; j++){
            int k;
            if(flag_count == frame_num-1) break;
            for(k=0; k < frame_num; k++){
                if(page_num[j] == opt_frame[k]){
                    flag_frame[k] = 1;
                    flag_count++;
                }
            }
        }
        for(j=0; j < frame_num; j++){
            if(flag_frame[j]==0){
                opt_frame[j] = cur_page;
                opt_pagefault++;
//                printf("*** Opt_frame: %dth page, %d ***\n",i+1,cur_page);
//                print_int_array(opt_frame,frame_num);
                break;
            }
        }
    }    

}


/*
 *	LRU_REF8 Algorithm
 *	- Using 8 reference bits.
 */
void lru_ref8_algorithm(){
    int* ref8_frame = (int*) calloc(frame_num, sizeof(int));    
    int i;
    int empty_frame = 0;
    int* bit_frame = (int*) calloc(frame_num, sizeof(int));

    for(i = 0; i < page_num_count; i++){
        int cur_page = page_num[i];

//        printf("*** REF8_frame: %dth page, %d ***\n",i+1,cur_page);
//        print_int_array(ref8_frame, frame_num);
//        int temp_i;
//        for(temp_i=0; temp_i < frame_num; temp_i++){
//            printf("bit_frame %dth: %d \n",temp_i,bit_frame[temp_i]);
//        }


        // When it contains the page                
        int t;
        bool found=false;
        for(t=0; t < frame_num; t++){
            bit_frame[t] = bit_frame[t]/10;
            if(ref8_frame[t]==cur_page){
                bit_frame[t] += 10000000;
                found = true;
            }    
        }

        if(found) continue;

        // When empty space is available
        if(empty_frame < frame_num){
            ref8_frame[empty_frame] = cur_page;
            bit_frame[empty_frame] = 10000000;
            empty_frame++;
            pagefault++;
//            printf("*** REF8_frame: %dth page, %d ***\n",i+1,cur_page);
//            print_int_array(ref8_frame, frame_num);
            continue;
        }

        // Replace a frame
        int j;
        int least=0;
        for(j=0; j < frame_num; j++){
            if(bit_frame[j] < bit_frame[least]){
                least = j;
            }
        }       

        ref8_frame[least] = cur_page;
        bit_frame[least] = 10000000;
        pagefault++;
        //printf("*** REF8_frame: %dth page, %d ***\n",i+1,cur_page);
        //print_int_array(ref8_frame,frame_num);
    }    


}


// Check if page is in the frame
bool check_frame(int* frame, int page_num){
    int i;
    for(i=0; i < frame_num; i++){
        if(frame[i]==page_num){
            return true;
        }    
    }
    return false;
}

// compare algo for qsort
int compare(const void* e1, const void* e2) {
	int first = *((int*)e1);
	int second = *((int*)e2);

	if( first > second){
		return 1;
	} else if (first < second) {
		return -1;
	} else {
		return 0;
	}
}

/*
 *  (Debugging) 1st param: int array, 2nd param: size of the array
 */
void print_int_array(int* int_array, int size){
    int index;
    printf("[ ");
    for(index = 0; index < size; index++){
        printf("%d ",int_array[index]);
    }
    printf("]\n");
}


