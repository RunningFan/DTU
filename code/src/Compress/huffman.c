#include "huffman.h"


typedef struct huffman_node_tag
{
    unsigned char isLeaf;//是否为树叶
    unsigned long count;//节点代表的符号加权和
    struct huffman_node_tag *parent;//父节点指针

    union//共同体：如果是叶节点，则只能有symbol，如果是非叶节点，只能有左右孩子指针
    {
        struct
        {
            struct huffman_node_tag *zero, *one;//左右孩子指针
        };
        unsigned char symbol;//符号，叶节点带有的字符
    };
} huffman_node;


typedef struct huffman_code_tag
{
    /*以比特为单位的代码的长度。 */
    unsigned long numbits;
    /* The bits that make up this code. The first
       bit is at position 0 in bits[0]. The second
       bit is at position 1 in bits[0]. The eighth
       bit is at position 7 in bits[0]. The ninth
       bit is at position 0 in bits[1]. */
    unsigned char *bits;//指向该码比特串的指针

} huffman_code;

int huffman_encode_file(FILE *in, FILE *out, FILE *out_Table)//step1:changed by yzhang for huffman statistics from (FILE *in, FILE *out) to (FILE *in, FILE *out, FILE *out_Table)表示输出文件结果的指针
{
    SymbolFrequencies sf;//
    SymbolEncoder *se;
    huffman_node *root = NULL;
    int rc;
    unsigned int symbol_count;
    //step2:add by yzhang for huffman statistics
    huffman_stat hs;
    //end by yzhang

/*第一次扫描，得到输入文件每个信源符号出现的频率*/
    symbol_count = get_symbol_frequencies(&sf, in); //演示扫描完一遍文件后，SF指针数组的每个元素的构成

    //step3:add by yzhang for huffman statistics,...  get the frequency of each symbol 
    huffST_getSymFrequencies(&sf,&hs,symbol_count);
    //end by yzhang

    /*从symbolCount构建最佳表。即构建霍夫曼树 */
    se = calculate_huffman_codes(&sf);
    root = sf[0];

    //step3:add by yzhang for huffman statistics... output the statistics to file
    huffST_getcodeword(se, &hs);
    output_huffman_statistics(&hs,out_Table);
    //end by yzhang

    /*再次扫描文件，并使用之前构建的表将其编码到输出文件中。 */
    rewind(in);
    rc = write_code_table(out, se, symbol_count);//在输出文件中写入码表
    if(rc == 0)
        rc = do_file_encode(in, out, se);//第二次扫描，写完码表后对文件字节按照码表进行编码

    /* 释放霍夫曼树 */
    free_huffman_tree(root);
    free_encoder(se);
    return rc;
}


static unsigned int
get_symbol_frequencies(SymbolFrequencies *pSF, FILE *in)//统计文件中各个字符出现频率
{
    int c;
    unsigned int total_count = 0;//扫描的总信源符号数，初始化为0

    /* 将所有信源符号地址初始化为NULL，使得所有字符频率为0 */
    init_frequencies(pSF);

    /* 计算输入文件中每个符号的频率。 */
    while((c = fgetc(in)) != EOF)//挨个读取字符
    {
        unsigned char uc = c;//将读取的字符赋给uc
        if(!(*pSF)[uc])//如果uc不存在对应的空间，即uc是一个新的符号
            (*pSF)[uc] = new_leaf_node(uc);//产生该字符的一个新的叶节点。
        ++(*pSF)[uc]->count;//如果uc不是一个新的字符，则当前字符出现的频数累加1
        ++total_count;//总计数值加1
    }

    return total_count;//返回值为总计数值
}

static huffman_node*
new_leaf_node(unsigned char symbol)/*新建一个叶节点*/
{
    huffman_node *p = (huffman_node*)malloc(sizeof(huffman_node));
    p->isLeaf = 1;//1表示是叶节点
    p->symbol = symbol;//将新的符号的值存入symbol中
    p->count = 0;//该节点的频数为初始化0
    p->parent = 0;//该节点父节点初始化为0
    return p;
}

static SymbolEncoder*
calculate_huffman_codes(SymbolFrequencies * pSF)
{
    unsigned int i = 0;
    unsigned int n = 0;
    huffman_node *m1 = NULL, *m2 = NULL;
    SymbolEncoder *pSE = NULL;

#if 0
    printf("BEFORE SORT\n");
    print_freqs(pSF);
#endif

    /* 按升序对符号频率数组进行排序 */
    qsort((*pSF), MAX_SYMBOLS, sizeof((*pSF)[0]), SFComp);//数组的起始地址，数组的元素数，每个元素的大小，比较函数的指针
    //将所有的节点按照字符概率小到大排序，可使用qsort函数对节点结构体进行排序。排序的依据是SFComp，即根据每个字符发生的概率进行排序。
#if 0   
    printf("AFTER SORT\n");
    print_freqs(pSF);
#endif

    /*得到文件出现的字符种类数   */
    for(n = 0; n < MAX_SYMBOLS && (*pSF)[n]; ++n)
        ;

    /*
     * Construct a Huffman tree. This code is based
     * on the algorithm given in Managing Gigabytes
     * by Ian Witten et al, 2nd edition, page 34.
     * Note that this implementation uses a simple
     * count instead of probability.
     构建霍夫曼树
     */
    for(i = 0; i < n - 1; ++i)
    {
        /* 将m1和m2设置为最小概率的两个子集。 */
            m1 = (*pSF)[0];
        m2 = (*pSF)[1];

        /* 将m1和m2替换为一个集合{m1，m2}，其概率是m1和m2之和的概率。*/

        //合并m1、m2为非叶节点，count为二者count之和  
        //并将该非叶节点的左右孩子设为m1、m2  
        //将左右孩子的父节点指向该非叶节点  
        //将(*pSF)[0]指向该非叶节点
        (*pSF)[0] = m1->parent = m2->parent =
            new_nonleaf_node(m1->count + m2->count, m1, m2);//
        (*pSF)[1] = NULL;//1节点置空    
        /* 由于最小的两个频率数，进行了合并，频率大小发生改变，所以重新排序 */
        qsort((*pSF), n, sizeof((*pSF)[0]), SFComp);
    }

    /* Build the SymbolEncoder array from the tree. */
    pSE = (SymbolEncoder*)malloc(sizeof(SymbolEncoder));
    //定义一个指针数组，数组中每个元素是指向码节点的指针
    memset(pSE, 0, sizeof(SymbolEncoder));
    build_symbol_encoder((*pSF)[0], pSE);
    return pSE;
}


static int
SFComp(const void *p1, const void *p2)
{
    const huffman_node *hn1 = *(const huffman_node**)p1;
    const huffman_node *hn2 = *(const huffman_node**)p2;

    /* 用于将所有NULL排到最后 */
    if(hn1 == NULL && hn2 == NULL)
        return 0;//若两者都为空，则返回相等
    if(hn1 == NULL)
        return 1;//若返回值为1，大于0，则hn1排到hn2后
    if(hn2 == NULL)
        return -1;////若返回值为-1，小于0，则hn2排到hn1后
    /*由小到大排列*/
    if(hn1->count > hn2->count)
        return 1;
    else if(hn1->count < hn2->count)
        return -1;

    return 0;
}

/*
 * build_symbol_encoder builds a SymbolEncoder by walking
 * down to the leaves of the Huffman tree and then,
 * for each leaf, determines its code.
 */
static void
build_symbol_encoder(huffman_node *subtree, SymbolEncoder *pSF)
{
    if(subtree == NULL)
        return;//判断是否是空树, 是则说明编码结束，

    if(subtree->isLeaf)//判断是否为树叶节点，是则产生新的码字
        (*pSF)[subtree->symbol] = new_code(subtree);
    else
    {//
        build_symbol_encoder(subtree->zero, pSF);//遍历左子树，调用build_symbol_encoder函数自身
        build_symbol_encoder(subtree->one, pSF);//遍历右子数
    }
}

static huffman_code*
new_code(const huffman_node* leaf)
{
    /* 通过走到根节点然后反转位来构建huffman代码，
    因为霍夫曼代码是通过走下树来计算的。*/
    //采用向上回溯的方法
    unsigned long numbits = 0;//表示码长，以位为单位
    unsigned char* bits = NULL;//表示指向码字的指针
    huffman_code *p;

    while(leaf && leaf->parent)//用来判断节点和父节点是否存在，leaf为NULL时，不进行编码；parent为NULL时，已经到达树根不在编码
    {
        huffman_node *parent = leaf->parent;
        unsigned char cur_bit = (unsigned char)(numbits % 8);//current_bit为当前在bits[]的第几位

        unsigned long cur_byte = numbits / 8;//current_byte

        /* 如果码字长度超过一个字节，那么就在分配一个字节 */
        if(cur_bit == 0)
        {
            size_t newSize = cur_byte + 1;
            bits = (char*)realloc(bits, newSize);
            /*realloc()函数先判断当前的指针是否有足够的连续空间，如果有，扩大bits指向的地址，并且将bits返回，如果空间不够，先按照newsize指定的大小分配空间，将原有数据从头到尾拷贝到新分配的内存区域，而后释放原来bits所指内存区域(注意:原来指针是自动释放，不需要使用free)，同时返回新分配的内存区域的首地址。即重新分配存储器块的地址。*/
            bits[newSize - 1] = 0; /* Initialize the new byte. */
        }

//如果是左孩子，则不用改变数值，因为初始化为0。如果是右孩子，则将该位置1
        if(leaf == parent->one)
            bits[cur_byte] |= 1 << cur_bit;//将1左移至cur_bit，再将其与bits[cur_byte]进行或的操作

        ++numbits;//码字位数加1
        leaf = parent;//下一位的码字在当前码字的父节点一级
    }

    if(bits)//将现有的码字进行反转
        reverse_bits(bits, numbits);

    p = (huffman_code*)malloc(sizeof(huffman_code));
    p->numbits = numbits;//码长赋给节点的numbits
    p->bits = bits;//码字付给节点的bits
    return p;//返回值为码字
}

static void
reverse_bits(unsigned char* bits, unsigned long numbits)
{
    unsigned long numbytes = numbytes_from_numbits(numbits);//将numbits除8后上取整得到numbytes
    unsigned char *tmp =
        (unsigned char*)alloca(numbytes);//alloca()是内存分配函数，在栈上申请空间，用完后马上就释放
    unsigned long curbit;
    long curbyte = 0;//记录即将要反转的二进制码所在的的数组下标

    memset(tmp, 0, numbytes); //将数组tmp[numbytes]所有元素置为为0

    for(curbit = 0; curbit < numbits; ++curbit)
    {
        unsigned int bitpos = curbit % 8;//表示curbit不是8的倍数时需要左移的位数

        if(curbit > 0 && curbit % 8 == 0)//curbit为8的倍数时，进入下一个字节
            ++curbyte;

        tmp[curbyte] |= (get_bit(bits, numbits - curbit - 1) << bitpos);
    }

    memcpy(bits, tmp, numbytes);//将tmp临时数组内容拷贝到bits数组中
}

//第i位在第 i/8 字节的第 i%8 位，把这一位移到字节最低位，和 0000 0001 做与，从而只留下这一位，
static unsigned char
get_bit(unsigned char* bits, unsigned long i)
{
    return (bits[i / 8] >> i % 8) & 1;
}

static int
write_code_table(FILE* out, SymbolEncoder *se, unsigned int symbol_count)
{
    unsigned long i, count = 0;

    /* 计算se中的字符种类数. */
    for(i = 0; i < MAX_SYMBOLS; ++i)
    {
        if((*se)[i])
            ++count;
    }

    /* Write the number of entries in network byte order. */
    i = htonl(count);    //在网络传输中，采用big-endian序，对于0x0A0B0C0D ，传输顺序就是0A 0B 0C 0D ，
    //因此big-endian作为network byte order，little-endian作为host byte order。
    //little-endian的优势在于unsigned char/short/int/long类型转换时，存储位置无需改变
    if(fwrite(&i, sizeof(i), 1, out) != 1)
        return 1;//将字符种类的个数写入文件

    /* Write the number of bytes that will be encoded. */
    symbol_count = htonl(symbol_count);
    if(fwrite(&symbol_count, sizeof(symbol_count), 1, out) != 1)
        return 1;//将字符数写入文件

    /* Write the entries. */
    for(i = 0; i < MAX_SYMBOLS; ++i)
    {
        huffman_code *p = (*se)[i];
        if(p)
        {
            unsigned int numbytes;
            /* 写入1字节的符号 */
            fputc((unsigned char)i, out);
            /* 写入一字节的码长 */
            fputc(p->numbits, out);
            /* 写入numbytes字节的码字*/
            numbytes = numbytes_from_numbits(p->numbits);
            if(fwrite(p->bits, 1, numbytes, out) != numbytes)
                return 1;
        }
    }

    return 0;
}

static int
do_file_encode(FILE* in, FILE* out, SymbolEncoder *se)
{
    unsigned char curbyte = 0;
    unsigned char curbit = 0;
    int c;

    while((c = fgetc(in)) != EOF)//遍历文件的每一个字符
    {
        unsigned char uc = (unsigned char)c;
        huffman_code *code = (*se)[uc];//查表
        unsigned long i;
        /*将码字写入文件*/
        for(i = 0; i < code->numbits; ++i)
        {
            /* Add the current bit to curbyte. */
            curbyte |= get_bit(code->bits, i) << curbit;

            /* If this byte is filled up then write it
             * out and reset the curbit and curbyte. */
            if(++curbit == 8)
            {
                fputc(curbyte, out);
                curbyte = 0;
                curbit = 0;
            }
        }
    }


	typedef struct huffman_statistics_result
	{
		float freq[256];//用于记录每个信源符号出现的频次
		unsigned long numbits[256];
		unsigned char bits[256][100];//用来存放码字，规定每个码字的最大长度为100
	}huffman_stat;

	int huffST_getSymFrequencies(SymbolFrequencies *SF, huffman_stat *st,int total_count)
	{
		int i,count =0;
		for(i = 0; i < MAX_SYMBOLS; ++i)
		{	
			if((*SF)[i])
			{
				st->freq[i]=(float)(*SF)[i]->count/total_count;
				count+=(*SF)[i]->count;
			}
			else 
			{
				st->freq[i]= 0;
			}
		}
		if(count==total_count)
			return 1;
		else
			return 0;
	}
	
	int huffST_getcodeword(SymbolEncoder *se, huffman_stat *st)
	{
		unsigned long i,j;
	
		for(i = 0; i < MAX_SYMBOLS; ++i)
		{
			huffman_code *p = (*se)[i];
			if(p)
			{
				unsigned int numbytes;
				st->numbits[i] = p->numbits;
				numbytes = numbytes_from_numbits(p->numbits);
				for (j=0;j<numbytes;j++)
					st->bits[i][j] = p->bits[j];
			}
			else
				st->numbits[i] =0;
		}
	
		return 0;
	}
	
	void output_huffman_statistics(huffman_stat *st,FILE *out_Table)
	{
		int i,j;
		unsigned char c;
		fprintf(out_Table,"symbol\t   freq\t   codelength\t   code\n");
		for(i = 0; i < MAX_SYMBOLS; ++i)
		{	
			fprintf(out_Table,"%d\t   ",i);
			fprintf(out_Table,"%f\t   ",st->freq[i]);
			fprintf(out_Table,"%d\t    ",st->numbits[i]);
			if(st->numbits[i])
			{
				for(j = 0; j < st->numbits[i]; ++j)
				{
					c =get_bit(st->bits[i], j);
					fprintf(out_Table,"%d",c);
				}
			}
			fprintf(out_Table,"\n");
		}
	}














