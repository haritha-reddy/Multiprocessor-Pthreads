/**********************************************************************************
*
* Gaussian elimination - Pthreads parallel implementation
*
* Instructions to compile and run the source code
*
**********************************************************************************/

-> Upload source code and make file 
-> In command prompt, type:
	make
-> Execution:
/usr/bin/time ./gauss -n [matrix size] -I [init_type] -m [max_random_num] -P [print_matrix]
Example: /usr/bin/time ./gauss -n 2048 -I rand -m 15 -P 0

// Manual compilation and execution

compile: gcc -pthread -w -o gauss gaussian.c
run: /usr/bin/time ./gauss -n 2048 -I rand -m 15 -P 0