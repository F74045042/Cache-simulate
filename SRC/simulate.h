struct Block_{
	char* tag;
	int valid;
	int cnt;	//variable for counting LRU or FIFO
};

typedef struct Block_* Block;

struct Cache_{
	int associate;
	int method;
	int hit;
	int miss;
	int cache_size;
	int block_size;
	int numBlocks;
	int INDEX;
	int OFFSET;
	int TAG;
	Block* block;
};

typedef struct Cache_* Cache;

Cache cache;

FILE* cache_init(FILE*, char*);
unsigned int htoi(char *str);
char *getBinary(unsigned int num);
int btoi(char* bin);
int writeCache(char* address);
int countline(char* location);
void output(char* out, char* location, int* isHit);

#define DEBUG 0