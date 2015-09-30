/*
 * RCSID $Id: sha1.c,v 1.1.1.1 2009/05/13 00:14:25 root Exp $
 */

/*
 * The rest of the code is derived from sha1.c by Steve Reid, which is
 * public domain.
 * Minor cosmetic changes to accomodate it in the Linux kernel by ji.
 */

//#include </byteorder.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "sha1.h"

#if defined(rol)
#undef rol
#endif

#define SHA1HANDSOFF

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifdef __LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


/* Hash a single 512-bit block. This is the core of the algorithm. */

void SHA1Transform(uint32_t state[5], uint8_t buffer[64])
{
uint32_t a, b, c, d, e;
typedef union {
    unsigned char c[64];
    uint32_t l[16];
} CHAR64LONG16;
CHAR64LONG16* block;
#ifdef SHA1HANDSOFF
static unsigned char workspace[64];
    block = (CHAR64LONG16*)workspace;
    memcpy(block, buffer, 64);
#else
    block = (CHAR64LONG16*)buffer;
#endif
    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;
}


/* SHA1Init - Initialize new context */

void SHA1Init(void *vcontext)
{
   SHA1_CTX* context=(SHA1_CTX *)vcontext;

    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}


/* Run your data through this. */

void SHA1Update(void *vcontext, unsigned char* data, uint32_t len)
{
    SHA1_CTX* context = vcontext;
    uint32_t i, j;

    j = context->count[0];
    if ((context->count[0] += len << 3) < j)
	context->count[1]++;
    context->count[1] += (len>>29);
    j = (j >> 3) & 63;
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64-j));
        SHA1Transform(context->state, context->buffer);
        for ( ; i + 63 < len; i += 64) {
            SHA1Transform(context->state, &data[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

void SHA1Final(unsigned char digest[20], void *vcontext)
{
  uint32_t i, j;
  unsigned char finalcount[8];
  SHA1_CTX* context = vcontext;
    
    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }
    SHA1Update(context, (unsigned char *)"\200", 1);
    while ((context->count[0] & 504) != 448) {
        SHA1Update(context, (unsigned char *)"\0", 1);
    }
    SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
    for (i = 0; i < 20; i++) {
        digest[i] = (unsigned char)
         ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }
    /* Wipe variables */
    i = j = 0;
    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(&finalcount, 0, 8);
#ifdef SHA1HANDSOFF  /* make SHA1Transform overwrite its own static vars */
    SHA1Transform(context->state, context->buffer);
#endif
}

int calculate_sha1(char* filename,unsigned char *digest)
{
	int fd1;
	int result;
	int bytes_to_copy = 0;
	int filesize = 0;
	struct stat attribute;
	SHA1_CTX  index;
	char *sha1buffer;
	sha1buffer = (char *) malloc(4096);
	fd1=open(filename,O_RDONLY);

	if(fd1 < 0)
	{
		printf("Error opening file\n");
		return -1;
	}
	stat(filename,&attribute);
	filesize = attribute.st_size;
	/*result =*/
	SHA1Init(&index);
/*	if(result)
	{
		close(fd1);
		free(sha1buffer);
		return -1;
	}
*/
	bytes_to_copy = filesize;
	while(bytes_to_copy > 4096)
	{
		read(fd1,sha1buffer,4096);
		/*result =*/
		 SHA1Update(&index,sha1buffer,4096);
		/*if(result)
		{
			close(fd1);
			free(sha1buffer);
			return -1;
		}
		*/
		bytes_to_copy = bytes_to_copy - 4096;
	}
	memset(sha1buffer,0,4096);
	read(fd1,sha1buffer,bytes_to_copy);
	/*result =*/
	SHA1Update(&index,sha1buffer,bytes_to_copy);
/*	if(result)
	{
		close(fd1);
		free(sha1buffer);
		return -1;
	}
*/
	close(fd1);
	free(sha1buffer);
	/*result =*/ 
	SHA1Final(digest,&index);
/*	if(result)
		return -1;
	return 0;
*/

	
}
int calculate_context_sha1(char *context,int context_size,uint32_t *SM3_hash)
{
	int result;
	SHA1_CTX index;
	SHA1Init(&index);
	SHA1Update(&index,context,context_size);
	SHA1Final(SM3_hash,&index);
	return 0;
}
int calculate_pathsha1(char* filepath,unsigned char *digest)
{
	int result;
	SHA1_CTX index;
	//result = 
	SHA1Init(&index);
	//if(result)
	//	return -1;
	//result = 
	SHA1Update(&index,filepath,strlen(filepath));
	//if(result)
	//	return -1;
	//result = 
	SHA1Final(digest,&index);
	//if(result)
	//	return -1;
	return 0;
}
/*
 * $Log: sha1.c,v $
 * Revision 1.1.1.1  2009/05/13 00:14:25  root
 * linux-2-6-26
 *
 * Revision 1.1.2.1  2006/08/10 08:11:38  sunyu
 *
 * 2006/8/10 by sunyu modified for porting from freeswan to openswan.
 *
 * Revision 1.9  2004/04/06 02:49:26  mcr
 * 	pullup of algo code from alg-branch.
 *
 * Revision 1.8  2002/09/10 01:45:14  mcr
 * 	changed type of MD5_CTX and SHA1_CTX to void * so that
 * 	the function prototypes would match, and could be placed
 * 	into a pointer to a function.
 *
 * Revision 1.7  2002/04/24 07:55:32  mcr
 * 	#include patches and Makefiles for post-reorg compilation.
 *
 * Revision 1.6  2002/04/24 07:36:30  mcr
 * Moved from ./klips/net/ipsec/ipsec_sha1.c,v
 *
 * Revision 1.5  1999/12/13 13:59:13  rgb
 * Quick fix to argument size to Update bugs.
 *
 * Revision 1.4  1999/04/11 00:29:00  henry
 * GPL boilerplate
 *
 * Revision 1.3  1999/04/06 04:54:27  rgb
 * Fix/Add RCSID Id: and Log: bits to make PHMDs happy.  This includes
 * patch shell fixes.
 *
 * Revision 1.2  1999/01/22 06:55:50  rgb
 * 64-bit clean-up.
 *
 * Revision 1.1  1998/06/18 21:27:50  henry
 * move sources from klips/src to klips/net/ipsec, to keep stupid
 * kernel-build scripts happier in the presence of symlinks
 *
 * Revision 1.2  1998/04/23 20:54:04  rgb
 * Fixed md5 and sha1 include file nesting issues, to be cleaned up when
 * verified.
 *
 * Revision 1.1  1998/04/09 03:06:11  henry
 * sources moved up from linux/net/ipsec
 *
 * Revision 1.1.1.1  1998/04/08 05:35:05  henry
 * RGB's ipsec-0.8pre2.tar.gz ipsec-0.8
 *
 * Revision 0.4  1997/01/15 01:28:15  ji
 * New transform
 *
 *
 */
