//
// Created by haipinHu on 2021/5/28.
//

#ifndef OS_1_OS_H
#define OS_1_OS_H
#include <iostream>
#include <queue>
#include <list>
#include <functional>
#include "PCB.h"

#define MIN_PAGE 2
#define MAX_PAGE 5
#define PAGE_FORM_SIZE 50
#define WORD_SIZE 8

std::list<PCB> ready_queue;
std::list<PCB> run_queue;
std::list<PCB> blocked_queue;
std::vector<bitset<WORD_SIZE>> bitmap;
int u_bound = 0, l_bound = 0;
int page_frame_num = 0;
int request[10]{};
int ID_INDEX = 1;

std::deque<PCB::page_table_item> allocate_memory(int length);

int free_memory(int pid);
/**
 * Printing overall info.
 */
void monitor();

void monitor(double);

int create_process(int length);

int block_process(int pid);

int end_process(int pid);

int suspend_process(int pid);

/**
 * The function accesses the corresponding page according to its logical address.
 * If the page is not in memory, the page replacement algorithm selects the appropriate page to be swapped into memory,
 * and gives the out-of-page rate.
 * @param Process id
 * @param Logical address of the process
 * @param Page replacement algorithm functions.
 * @return -1 or out-of-page rate.
 */
double access(int pid, int addr, std::function<void(std::deque<PCB::page_table_item> &, int, int)> alg);

void FIFO(std::deque<PCB::page_table_item> &page_table, int i, int j);

void LRU(std::deque<PCB::page_table_item> &page_table, int i, int j);

#endif //OS_1_OS_H
