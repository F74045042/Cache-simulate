<H3>Command Line Format</H3>
./simulate –input <trace.txt> -output <trace.out><br>
<br><br>
<H3>Specification</H3><br>
a. Width of memory (line size) : 32bits<br>
b. Unit of memory : byte.<br>
c. Cache size : Read from test cases. With variable cache size from 1KB up to 256KB.<br>
d. Block size : Read from test cases. From 16B to 256B.<br>
e. Associativity : Read from test cases. Various associativity from direct-mapped to fully associative .<br>
f. Replaced Algorithm : Read from test cases. Will be FIFO or LRU.<br>
g. All cache is initialized 0.<br>
<br><br>
<H3>Input file format</H3><br>
The 1st line specifies the cache size.<br>
The 2nd line specifies the block size.<br>
The 3rd line specifies the associativity. 0 represents direct-mapped, 1 represents four-way set associative, 2 represents fully associative.<br>
The 4th line specifies the Replace algorithm. 0 represents FIFO, 1 represents LRU.<br>
The rest of the test case is a trace of memory accesses executed from some benchmark program.<br>

Q1. How do you know the number of block from input file?<br>
	number of block = cache size/block size<br>

Q2. How do you know how many set in this cache?<br>
	According to the input file. <br>
	0 = Direct Mapping (1-way set)<br>
	1 = 4-way set<br>
	2 = Fully (number of block-way set)<br>

Q3. How do you know the bits of the width of the Tag ?<br>
	Input file will give us cache size and block size, according to those information, we can calculate tag bit width using following statement.<br>
	Tag bit width = 32 - offset - index<br>
	offset bit width = log2(block size)<br>
	index bit width = log2(number of block)<br>
  
  這次的使用c code去實作硬體概念，覺得特別有趣。
