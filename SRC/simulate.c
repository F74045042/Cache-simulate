#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "simulate.h"


/* Read file and save it to cache. 
*  Get cache_size, block_size, associate, method.
*
*  offset bit width = log2(block size)
*  index bit width = log2(number of block)
*  tag bit width = 32-offset-index
*/

int count=0;

FILE* cache_init(FILE* fp, char* location){
	cache = (Cache) malloc(sizeof(struct Cache_));
	if(cache == NULL){
		printf("<Error> Can't allocate memory to cache.\n");
	}
	cache->hit = 0;
	cache->miss = 0;
	
	int cache_size, block_size, associate, method;

	fp = fopen(location, "r");
	if(fp == NULL){
		printf("<Error> File opening error.\n");
	}else{
		fscanf(fp, "%d ", &cache_size);
		fscanf(fp, "%d ", &block_size);
		fscanf(fp, "%d ", &associate);
		fscanf(fp, "%d ", &method);
		cache->cache_size = cache_size;
		cache->block_size = block_size;
		cache->associate = associate;
		cache->method = method;
		if(DEBUG){
			printf("<DEBUG>cache_size: %d\n",cache->cache_size);
			printf("<DEBUG>block_size: %d\n",cache->block_size);
			printf("<DEBUG>associate: %d\n",cache->associate);
			printf("<DEBUG>method: %d\n",cache->method);
		}
	}	

	cache->numBlocks = (cache->cache_size*1024*8)/(cache->block_size*8);	//convert cache_size and block_size to bit.
	if(cache->associate == 0){
		cache->INDEX = (int)(log(cache->numBlocks)/log(2.0));
	}else if(cache->associate == 1){
		cache->INDEX = (int)(log(cache->numBlocks/4)/log(2.0));
	}else{
		cache->INDEX = 0;
	}
	cache->OFFSET = (int)(log(cache->block_size)/log(2.0));
	cache->TAG = 32 - cache->OFFSET - cache->INDEX;

	cache->block = (Block*)malloc(sizeof(Block)*cache->numBlocks);

	if(DEBUG){
		printf("<DEBUG>numBlocks: %d\n", cache->numBlocks);
		printf("<DEBUG>TAG: %d\n", cache->TAG);
		printf("<DEBUG>INDEX: %d\n", cache->INDEX);
		printf("<DEBUG>OFFSET: %d\n", cache->OFFSET);
	}

	for(int i=0; i<cache->numBlocks; i++){
		cache->block[i] = (Block)malloc(sizeof(struct Block_));
		cache->block[i]->tag = NULL;
		cache->block[i]->valid = 0;
	}

	return fp;
}

/* Convert Hex to Int. */
unsigned int htoi(char *str){
	int i = 0, result = 0;
	if(str[i] == '0' && str[i+1] == 'x'){
		i = i+2;
	}
	while(str[i] != '\0'){
		result = result * 16;
		if(str[i] >= '0' && str[i] <= '9')
		{
			result = result + (str[i] - '0');
		}
		else if(tolower(str[i]) >= 'a' && tolower(str[i]) <= 'f')
		{
			result = result + (tolower(str[i]) - 'a') + 10;
		}
		i++;
	}

	return result;
}

/* Convert Int to binary. */
char *getBinary(unsigned int num)
{
	char* bstring;
	int i;
	
	bstring = (char*) malloc(sizeof(char) * 33);
	bstring[32] = '\0';
	
	for( i = 0; i < 32; i++ )
	{
		bstring[32 - 1 - i] = (num == ((1 << i) | num)) ? '1' : '0';
	}

	return bstring;
}

/*Convert binary to int*/
int btoi(char* bin)
{
	int  b=0, k=0, m=0, n=0;
	int  len, sum;

	sum = 0;
	len = strlen(bin) - 1;

	for(k = 0; k <= len; k++)
	{
		n = (bin[k] - '0'); 
		if ((n > 1) || (n < 0))
		{
			return 0;
		}
		for(b = 1, m = len; m > k; m--)
		{
			b *= 2;
		}
		sum = sum + n * b;
	}
	return(sum);
}

/*Write data to cache.
*  ________________________
*  | tag | index | offset |
*  ------------------------
*  return 1 => hit
*  return 0 => miss
*  associative => 0 = 1-set, 1 = 4-way, 2 = fully
*  method => 0 = FIFO, 1 = LRU
*/
int writeCache(char* address)
{
	char *tag, *index, *offset;
	int i_index, set;
	Block block, tmp;
	count++;	// count for how many write
	// if(count == 100)
	// 	while(1);

	if(address == NULL){
		printf("<Error> Must input a valid address.\n");
	}else{
		tag = (char*)malloc(sizeof(char)*cache->TAG+1);
		tag[cache->TAG] = '\0';
		index = (char*)malloc(sizeof(char)*cache->INDEX+1);
		index[cache->INDEX] = '\0';
		offset = (char*)malloc(sizeof(char)*cache->OFFSET+1);
		offset[cache->OFFSET] = '\0';
		for(int i=0; i<32; i++){
			if(i < cache->TAG){
				tag[i] = address[i];
			}else if(i >= cache->TAG && i < cache->TAG + cache->INDEX){
				index[i-cache->TAG] = address[i];
			}else{
				offset[i-cache->TAG-cache->INDEX] = address[i];
			}
		}
		if(DEBUG){
			printf("<DEBUG>address:\t%s\n", address);
			printf("<DEBUG>tag:\t%s\n", tag);
			printf("<DEBUG>index:\t%s\n", index);
			printf("<DEBUG>offset:\t%s\n", offset);
			printf("<DEBUG>btoi(index):%d\n", btoi(index));
		}
		//Direct-Mapping
		if(cache->associate == 0){
			block = cache->block[btoi(index)];
			if(block->valid == 1 && !strcmp(tag, block->tag)){
				cache->hit++;
				free(tag);
				free(index);
				free(offset);
				return 1;
			}else{
			//FIFO or LRU
				cache->miss++;
				block->valid = 1;
				if(block->tag != NULL){
					free(block->tag);
				}
				block->tag = tag;
				free(index);
				free(offset);
				return 0;
			}
		//4-way set 
		}else if(cache->associate == 1){
			set = (int)((float)cache->numBlocks/4.0);
			/*Hit*/
			i_index = btoi(index);
			// set = (int)((float)i_index/4.0);
			for(int i=0; i<4; i++){
				// printf("%d %d\n", set, i_index);
				block = cache->block[i_index];
				if(block->valid == 1 && !strcmp(tag, block->tag)){
					if(DEBUG){
						printf("<DEBUG>i_index:%d\n", i_index);
						printf("<DEBUG>set:%d\n", set);
					}
					cache->hit++;
					if(cache->method == 1)
						block->cnt = count;
					free(tag);
					free(index);
					free(offset);
					return 1;
				}
				i_index += set;
				if(i_index > cache->numBlocks){
					i_index =  i_index%set;
					if(i_index == 0){
						i_index = cache->numBlocks;
					}
				}
			}
			/*Miss*/
			//FIFO or LRU
			cache->miss++;
			//set not full
			i_index = btoi(index);
			for(int i=0; i<4; i++){
				block = cache->block[i_index];
				if(block->tag == NULL){
					if(DEBUG){
						printf("<DEBUG>i_index:%d\n", i_index);
						printf("<DEBUG>set:%d\n", set);
					}
					block->tag = tag;
					block->cnt = count;
					block->valid = 1;
					free(index);
					free(offset);
					return 0;
				}
				i_index += set;
				if(i_index > cache->numBlocks){
					i_index =  i_index%set;
					if(i_index == 0){
						i_index = cache->numBlocks;
					}
				}
			}
			//set full
			i_index = btoi(index);
			tmp = cache->block[i_index];
			for(int i=0; i<4; i++){
				block = cache->block[i_index];
				if(tmp->cnt > block->cnt){ // smallest
					tmp = cache->block[i_index];
				}
				i_index += set;
				if(i_index > cache->numBlocks){
					i_index =  i_index%set;
					if(i_index == 0){
						i_index = cache->numBlocks;
					}
				}
			}
			free(tmp->tag);
			tmp->tag = tag;
			tmp->cnt = count;
			tmp->valid = 1;
			free(index);
			free(offset);
			return 0;
		//fully
		}else if(cache->associate == 2){
			/*Hit*/
			for(int i=0; i<cache->numBlocks; i++){
				block = cache->block[i];
				if(block->valid == 1 && !strcmp(block->tag, tag)){
					cache->hit++;
					if(cache->method == 1)
						block->cnt = count;
					free(tag);
					free(index);
					free(offset);
					return 1;
				}
			}
			/*Miss*/
			//FIFO or LRU
			cache->miss++;
			//set not full
			for(int i=0; i<cache->numBlocks; i++){
				block = cache->block[i];
				if(block->tag == NULL){
					block = cache->block[i];
					block->tag = tag;
					block->cnt = count;
					block->valid = 1;
					free(index);
					free(offset);
					return 0;
				}
			}
			//set is full
			tmp = cache->block[0];
			for(int i=0; i<cache->numBlocks; i++){
				block = cache->block[i];
				if(tmp->cnt > block->cnt){
					tmp = cache->block[i];
				}
			}
			free(tmp->tag);
			tmp->tag = tag;
			tmp->cnt = count;
			tmp->valid = 1;
			free(index);
			free(offset);
			return 0;
		}
	}
	return -1;
}


int countline(char* location){
	// count the number of lines in the file called filename                                    
	FILE *fp = fopen(location,"r");
	int ch=0;
	int lines=0;

	if (fp == NULL){
		printf("<Error> File open for count line error.\n");
	}

	lines++;
	while ((ch = fgetc(fp)) != EOF)
	{
		if (ch == '\n')
			lines++;
	}
	fclose(fp);
	return lines;
}

/*Output the result to file*/
void output(char* out, char* location, int* isHit){
	FILE* fp;
	float hit_cnt=0, miss_cnt=0;
	int count = 1;
	int size = countline(location)-5;

	fp = fopen(out, "w");

	printf("%d %d\n", cache->hit, cache->miss);
	fprintf(fp, "Hits instructions: ");
	for (int i = 0; i < size; i++)
	{
		if(isHit[i] == 1){
			fprintf(fp, "%d", i+1);
			hit_cnt++;
			if(count < cache->hit){
				fprintf(fp, ",");
			}
			count++;
		}
	}
	count = 1;
	fprintf(fp, "\nMisses instructions: ");
	for (int i = 0; i < size; i++)
	{
		if(isHit[i] == 0){
			fprintf(fp, "%d", i+1);
			miss_cnt++;
			if(count < cache->miss){
				fprintf(fp, ",");
			}
			count++;
		}
	}
	fprintf(fp, "\nMiss rate: %f\n", miss_cnt/(hit_cnt+miss_cnt));
}

/*method not done yet*/
int main(int argc, char* argv[]){
	FILE* fp;
	char address[32];
	char *in, *out;
	int tmp;
	int count=0;

	for(int i=1; i<argc; i++){
		if(!strcmp(argv[i], "-input")){
			in = argv[i+1];
		}
		if(!strcmp(argv[i], "-output")){
			out = argv[i+1];
		}
	}

	int size = countline(in)-5;
	int isHit[size];

	fp = cache_init(fp, in);

	while(fscanf(fp, "%s ", address) != EOF){
		count++;
		if(DEBUG){
			printf("<DEBUG>address_HEX:%s\n", address);
		}
		tmp = writeCache(getBinary(htoi(address)));
		if(tmp == 1){
			if(DEBUG)
				printf("%d hit\n", count);
			isHit[count-1] = 1;
		}
		else if(tmp == 0){
			if(DEBUG)
				printf("%d miss\n", count);
			isHit[count-1] = 0;
		}
		else{
			printf("<Error> writeCache is error.(Associative)\n");
		}
	}
	output(out, in, isHit);
}