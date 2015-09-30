#define _GNU_SOURCE


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "sm3.h"
#define DIGEST_SIZE 32

int SM3_init(SM3_context *index)
{
  if ( index == NULL )
  {
    return -1;
  }

  index->total_bytes_High = 0;
  index->total_bytes_Low = 0;
  index->vector[0] = 0x7380166f;
  index->vector[1] = 0x4914b2b9;
  index->vector[2] = 0x172442d7;
  index->vector[3] = 0xda8a0600;
  index->vector[4] = 0xa96f30bc;
  index->vector[5] = 0x163138aa;
  index->vector[6] = 0xe38dee4d;
  index->vector[7] = 0xb0fb0e4e;

  return 0;
}

static void SM3_CF(SM3_context *index, uint8_t *byte_64_block )
{
uint32_t j,temp,W[68];

#ifdef DEBUG
  for(j=0;j<64;j++)
	printf("%d:%8x\n",j,byte_64_block[j]);
  printf("\n--------------------\n");
#endif

uint32_t A,B,C,D,E,F,G,H,SS1,SS2,TT1,TT2;
  CONCAT_4_BYTES( W[0],  byte_64_block,  0 );
  CONCAT_4_BYTES( W[1],  byte_64_block,  4 );
  CONCAT_4_BYTES( W[2],  byte_64_block,  8 );
  CONCAT_4_BYTES( W[3],  byte_64_block, 12 );
  CONCAT_4_BYTES( W[4],  byte_64_block, 16 );
  CONCAT_4_BYTES( W[5],  byte_64_block, 20 );
  CONCAT_4_BYTES( W[6],  byte_64_block, 24 );
  CONCAT_4_BYTES( W[7],  byte_64_block, 28 );
  CONCAT_4_BYTES( W[8],  byte_64_block, 32 );
  CONCAT_4_BYTES( W[9],  byte_64_block, 36 );
  CONCAT_4_BYTES( W[10], byte_64_block, 40 );
  CONCAT_4_BYTES( W[11], byte_64_block, 44 );
  CONCAT_4_BYTES( W[12], byte_64_block, 48 );
  CONCAT_4_BYTES( W[13], byte_64_block, 52 );
  CONCAT_4_BYTES( W[14], byte_64_block, 56 );
  CONCAT_4_BYTES( W[15], byte_64_block, 60 );
#ifdef DEBUG
  for(j=0;j<16;j++)
	{
	printf("%d:%8x\n",j,W[j]);
	}
#endif
  for(j=16;j<68;j++)
	{
// waitting to modified 
// there is something strange here,"P1(W[j-16]^W[j-9]^rol(W[j-3],15))" will get a error result
	temp = W[j-16]^W[j-9]^rol(W[j-3],15);
	W[j] = P1(temp)^rol(W[j-13],7)^(W[j-6]);
//	W[j] = P1((W[j-16]^W[j-9]^rol(W[j-3],15)))^rol(W[j-13],7)^(W[j-6]);

#ifdef DEBUG

	printf("%d::(*)=%8x :temp=%8x :P1((*))=%8x :P1(*)=%8x :p1(temp)=%8x\n",
		j,(W[j-16]^W[j-9]^rol(W[j-3],15)),P1((W[j-16]^W[j-9]^rol(W[j-3],15))),temp,P1(W[j-16]^W[j-9]^rol(W[j-3],15)),P1(temp));

#endif
	}
  A = index->vector[0];
  B = index->vector[1];
  C = index->vector[2];
  D = index->vector[3];
  E = index->vector[4];
  F = index->vector[5];
  G = index->vector[6];
  H = index->vector[7];
#ifdef DEBUG
	printf(" :A:%08x,B:%08x,C:%08x,D:%08x,E:%08x,F:%08x,G:%08x,H:%08x\n",A,B,C,D,E,F,G,H);
#endif

#define T 0x79cc4519
#define FF(X,Y,Z) (X^Y^Z)
#define GG(X,Y,Z) (X^Y^Z)

  for(j=0;j<16;j++)
	{
	SS1 = rol(rol(A,12) + E + rol(T,j),7);
	SS2 = SS1^(rol(A,12));
	TT1 = FF(A,B,C) + D + SS2 + (W[j]^W[j+4]);
	TT2 = GG(E,F,G) + H + SS1 + W[j];
	D = C;
	C = rol(B,9);
	B = A;
	A = TT1;
	H = G;
	G = rol(F,19);
	F = E;
	E = P0(TT2);
#ifdef DEBUG
	printf("%d: A:%08x,B:%08x,C:%08x,D:%08x,E:%08x,F:%08x,G:%08x,H:%08x\n",j,A,B,C,D,E,F,G,H);
#endif
	}

#undef T
#undef FF 
#undef GG


#define T 0x7a879d8a 
#define FF(X,Y,Z) ((X&Y)|(X&Z)|(Y&Z))
#define GG(X,Y,Z) ((X&Y)|(~X&Z))


  for(j=16;j<64;j++)
	{
	SS1 = rol(rol(A,12) + E + rol(T,j),7);
	SS2 = SS1^(rol(A,12));
	TT1 = FF(A,B,C) + D + SS2 + (W[j]^W[j+4]);
	TT2 = GG(E,F,G) + H + SS1 + W[j];
	D = C;
	C = rol(B,9);
	B = A;
	A = TT1;
	H = G;
	G = rol(F,19);
	F = E;
	E = P0(TT2);
#ifdef DEBUG
	printf("%d: A:%08x,B:%08x,C:%08x,D:%08x,E:%08x,F:%08x,G:%08x,H:%08x\n",j,A,B,C,D,E,F,G,H);
#endif
	}
#undef T
#undef FF 
#undef GG

index->vector[0] ^= A;
index->vector[1] ^= B;
index->vector[2] ^= C;
index->vector[3] ^= D;
index->vector[4] ^= E;
index->vector[5] ^= F;
index->vector[6] ^= G;
index->vector[7] ^= H;

}

int SM3_update(SM3_context *index, uint8_t *chunk_data, uint32_t chunk_length)
{

  uint32_t left, fill;
  uint32_t i;


  if ( (index == NULL) || (chunk_data == NULL) || (chunk_length < 1) )
       {
           return -1;
       }

  left = index->total_bytes_Low & 0x3F;
  fill = 64 - left;
  index->total_bytes_Low += chunk_length;
  index->total_bytes_Low &= 0xFFFFFFFF;

  if ( index->total_bytes_Low < chunk_length )
       {
           index->total_bytes_High++;
       }

  if ( (left > 0) && (chunk_length >= fill) )
       {
           for ( i = 0; i < fill; i++ )
  	     {
                 index->buffer[left + i] = chunk_data[i];
             }
           SM3_CF( index, index->buffer );
           chunk_length -= fill;
           chunk_data  += fill;
           left = 0;
        }

  while( chunk_length >= 64 )
        {
           SM3_CF( index, chunk_data );
           chunk_length -= 64;
           chunk_data  += 64;
        }

  if ( chunk_length > 0 )
        {
            for ( i = 0; i < chunk_length; i++ )
               {
                     index->buffer[left + i] = chunk_data[i];
               }
        }
  return 0;
  }


int SM3_final(SM3_context *index, uint32_t *SM3_hash)
{

  uint32_t last, padn;
  uint32_t high, low;
  uint8_t  msglen[8];
  int   ret;
  if ( (index == NULL) || (SM3_hash == NULL) )
       {
           *SM3_hash = 0;
           return -1;
       }
  high = ( index->total_bytes_Low >> 29 ) | ( index->total_bytes_High <<  3 );
  low  = ( index->total_bytes_Low <<  3 );
  SPLIT_INTO_4_BYTES( high, msglen, 0 );
  SPLIT_INTO_4_BYTES( low,  msglen, 4 );

  last = index->total_bytes_Low & 0x3F;
  padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );
  ret = SM3_update( index, SM3_padding, padn );
  ret = SM3_update( index, msglen, 8 );
  SM3_hash[0] = index->vector[0];
  SM3_hash[1] = index->vector[1];
  SM3_hash[2] = index->vector[2];
  SM3_hash[3] = index->vector[3];
  SM3_hash[4] = index->vector[4];
  SM3_hash[5] = index->vector[5];
  SM3_hash[6] = index->vector[6];
  SM3_hash[7] = index->vector[7];
  //SM3_hash[8] = index->vector[8];
  return 0;

}

//int calculate_by_context(char buffer[][DIGEST_SIZE],int countline,uint32_t *SM3_hash)
int calculate_sm3(char* filename, uint32_t *SM3_hash)
{
    int fd1;
    int result;
    int bytes_to_copy = 0;
    int filesize = 0;
    struct stat attribute;
        SM3_context index;
    char *sm3buffer;
    sm3buffer = (char*) malloc(4096);

    /* Open the input file */
    fd1 = open(filename,O_RDONLY);

    if (fd1 < 0)
    {
        printf("Error opening file %s\n",filename);
        return -1;
    }
    stat(filename,&attribute);
    filesize = attribute.st_size;
        //printf("filesize:%d\n",filesize);

    /* Initialise sm3-Context */
    result = SM3_init(&index);
    if (result)
    {
        close (fd1);
        free(sm3buffer);
        return -1;
    }


	 bytes_to_copy = filesize;
	    while (bytes_to_copy > 4096)
	    {

		read(fd1,sm3buffer,4096);
    		result = SM3_update(&index,sm3buffer,4096);
		if (result)
	        {
		    close (fd1);
		    free(sm3buffer);
    		    return -1;
		}
    		bytes_to_copy = bytes_to_copy - 4096;
		
	    }

	



            // And the last one
            memset(sm3buffer,0,4096);
            read(fd1,sm3buffer,bytes_to_copy);
            result = SM3_update(&index,sm3buffer,bytes_to_copy);
            if (result)
            {
                close (fd1);
                free(sm3buffer);
                return -1;
            }

    close (fd1);
    free(sm3buffer);
    result = SM3_final(&index,SM3_hash);
    if (result)
        return -1;
    return 0;
}

//int calculate_sm3(char* filename, uint32_t *SM3_hash)
//int calculate_by_context(char buffer[][DIGEST_SIZE],int countline,uint32_t *SM3_hash)
int calculate_by_context(char **buffer,int countline,uint32_t *SM3_hash)
{
    int fd1;
    int result;
    int bytes_to_copy = 0;
    int filesize = 0;
    struct stat attribute;
	SM3_context index;
    char *sm3buffer;
    sm3buffer = (char*) malloc(4096);

    /* Open the input file */
    /*fd1 = open(filename,O_RDONLY);

    if (fd1 < 0)
    {
	printf("Error opening file\n");
	return -1;
    }
    stat(filename,&attribute);
    filesize = attribute.st_size;*/
	//printf("filesize:%d\n",filesize);

    /* Initialise sm3-Context */
    result = SM3_init(&index);
    if (result)
    {
	close (fd1);
	free(sm3buffer);
        return -1;
    }

       // bytes_to_copy = filesize;
	int i=0;
	    while (i < countline)
	    {

		//read(fd1,sm3buffer,4096);
    		result = SM3_update(&index,buffer[i],DIGEST_SIZE);
		if (result)
	        {
		    //close (fd1);
		    //free(sm3buffer);
    		    return -1;
		}
    		//bytes_to_copy = bytes_to_copy - 4096;
		i++;
		
	    }

	    // And the last one
	    //memset(sm3buffer,0,4096);
	    //read(fd1,sm3buffer,bytes_to_copy);
    	    /*result = SM3_update(&index,sm3buffer,bytes_to_copy);
    	    if (result)
	    {
		close (fd1);
		free(sm3buffer);
    		return -1;
	    }

    close (fd1);
    free(sm3buffer);*/
    result = SM3_final(&index,SM3_hash);
    if (result)
	return -1;
    return 0;
}
#ifdef calculate_pathsm3
int calculate_pathsm3(char* filepath, uint32_t *SM3_hash)
{
   
    int result;
	 SM3_context index;
	 result = SM3_init(&index);
    if (result)
    	 {
      return -1;
    	 }
    result = SM3_update(&index,filepath,strlen(filepath));
    if (result)
	   {
    	return -1;
	   }
    result = SM3_final(&index,SM3_hash);
    if (result)
		return -1;
    return 0;
}
#endif
int calculate_context_sm3(char* context, int context_size, uint32_t *SM3_hash)
{
   
    int result;
     SM3_context index;
     result = SM3_init(&index);
    if (result)
    {
           return -1;
    }
    result = SM3_update(&index,context,context_size);
    if (result)
    {
    	   return -1;
    }
    result = SM3_final(&index,SM3_hash);
    if (result)
		return -1;
    return 0;
}

