/**
 * Copyright [2015] Tianfu Ma (matianfu@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * File: buddy.h
 *
 * Created on: Jun 5, 2015
 * Author: Tianfu Ma (matianfu@gmail.com)
 */

#ifndef BUDDY_H_
#define BUDDY_H_


/******************************************************************************
 *
 * Definitions
 *
 ******************************************************************************/
#define MAX_ORDER       20   // 2 ** 26 == 64M bytes
#define MIN_ORDER       4   // 2 ** 4 == 16 bytes
#define PAGE_SIZE       4096

typedef struct buddy {
  int order;
  int poolsize;
  void ** freelist;  // one more slot for first block in pool
  BYTE * pool;
} buddy_t;

void * T_mem_struct;
void * G_mem_struct;
void * C_mem_struct;

#define BLOCKSIZE(i)    (1 << (i))

/* the address of the buddy of a block from freelists[i]. */
static inline int offset(void * b,buddy_t * buddy) 
{return (BYTE *)b-buddy->pool; };
static inline void * buddyof(void * b, int i,buddy_t * buddy)
{
	int off=offset(b,buddy)^(1<<i);
	return (void *)(off+buddy->pool);
};

/* the address of the buddy of a block from freelists[i]. */

// not used yet, for higher order memory alignment
#define ROUND4(x)       ((x % 4) ? (x / 4 + 1) * 4 : x)

/******************************************************************************
 *
 * Types & Globals
 *
 ******************************************************************************/

void * bmalloc(int size, buddy_t * buddy);
void * bmalloc0(int size, buddy_t * buddy);
void bfree(void * block,buddy_t * buddy);
void bfree0(void * block,buddy_t * buddy);


void * buddy_init(int order);
void buddy_clear(buddy_t * buddy);
void buddy_reset(buddy_t * buddy);
void buddy_destroy(buddy_t * buddy);

void print_buddy(buddy_t * buddy);
int total_free(buddy_t * buddy);
int ispointerinbuddy(void * pointer,buddy_t * buddy);

#endif /* BUDDY_H_ */
