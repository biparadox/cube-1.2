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
 * File: cube_static.h
 *
 * Created on: Jun 5, 2015
 * Author: Tianfu Ma (matianfu@gmail.com)
 */

#ifndef CUBE_STATIC_H_
#define CUBE_STATIC_H_


/******************************************************************************
 *
 * Definitions
 *
 ******************************************************************************/
#define MAX_ORDER       20   // 2 ** 26 == 64M bytes
#define MIN_ORDER       4   // 2 ** 4 == 16 bytes
#define PAGE_SIZE       4096
#define PAGE_ORDER      12

UINT32 static_init(UINT32 addr);

UINT32 salloc(int size);

#endif /* BUDDY_H_ */
