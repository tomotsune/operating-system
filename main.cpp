#include <iostream>
#include <queue>
#include <list>
#include "PCB.h"

#define MIN_PAGE 2
#define PAGE_FORM_SIZE 50
#define WORD_SIZE 8
std::list<PCB> ready_queue;
std::list<PCB> run_queue;
std::list<PCB> block_queue;
std::vector<bitset<WORD_SIZE>> bitmap;
int u_bound = 0, l_bound = 0;
int page_form_num = 0;
int ID_INDEX = 1;

std::vector<PCB::page_table_item> allocate_memory(int length);

int free_memory(int pid);

void monitor();

int create_process(int length);

int block_process(int pid);

int end_process(int pid);

int suspend_process(int pid);

int main() {
    std::string instruction;
    std::cout << "Input two integer: upper and lower of memory " << std::endl;
    INPUT:
    std::cin >> l_bound >> u_bound;
    std::cin.get();
    if (u_bound <= l_bound) {
        std::cout << "***Upper bound must be bigger than lower***" << std::endl;
        goto INPUT;
    }
    page_form_num = (u_bound - l_bound) / PAGE_FORM_SIZE;
    int left_bit = page_form_num % WORD_SIZE;
    int word_num = left_bit == 0 ? page_form_num / WORD_SIZE : page_form_num / WORD_SIZE + 1;
    bitmap.resize(word_num);
    if (left_bit != 0) {
        for (int i = left_bit; i < WORD_SIZE; ++i) {
            bitmap[word_num - 1].set(i);
        }
    }
    std::cout << "-------------INSTRUCTIONS------------" << std::endl;
    std::cout << "ONE: OperationCode + PID/SPACE" << std::endl;
    std::cout << "  1-->Create a new process" << std::endl;
    std::cout << "  3-->Suspend a process" << std::endl;
    std::cout << "  3-->Block a process" << std::endl;
    std::cout << "  4-->End a process" << std::endl;
    std::cout << "TWO: monitor" << std::endl;
    std::cout << "--------------------------------------" << std::endl;
    LOOP:
    std::cout << "_>";
    getline(std::cin, instruction);
    if (instruction == "monitor") {
        monitor();
        goto LOOP;
    } else {
        try {
            int oc = stoi(instruction.substr(0, 1));
            if (oc == 1) {
                int flag = create_process(std::stoi(instruction.substr(2)));
                if (flag == -1) {
                    std::cout << "***memory allocation failed***" << std::endl;
                }
            } else if (oc == 2) {
                suspend_process(std::stoi(instruction.substr(2)));
            } else if (oc == 3) {
                block_process(stoi(instruction.substr(2)));
            } else if (oc == 4) {
                end_process(stoi(instruction.substr(2)));
            }
            goto LOOP;
        } catch (std::exception e) {
            std::cout << "***Illegal instruction***" << std::endl;
            goto LOOP;
        }
        return 0;
    }
}

void monitor() {
    std::cout << "<process>" << std::endl;
    auto whole_processes{ready_queue};
    auto temp_run_queue{run_queue};
    auto temp_block_queue{block_queue};
    whole_processes.merge(temp_run_queue);
    whole_processes.merge(temp_block_queue);
    for (const auto &process : whole_processes) {
        std::cout << process << std::endl;
    }
    std::cout << "<memory>" << std::endl;
    for (const auto &item : bitmap) {
        std::cout << item << std::endl;
    }
}

/**
 * This method schedules the process to run on the processor.
 */
void dispatch() {
    // It can simulate 4-core CPU, and can be executed by 4 processes simultaneously
    if (run_queue.size() < 4 && !ready_queue.empty()) {
        // According to the process scheduling algorithm, select the appropriate process to run on the processor.
        PCB pcb = ready_queue.front();
        pcb.status = RUN;
        run_queue.emplace_back(pcb);
        ready_queue.pop_front();
    }
    monitor();
}

int create_process(int length) {
    auto page_table = allocate_memory(length);
    if (page_table.empty())return -1;
    ready_queue.emplace_back(PCB{
            ID_INDEX++,
            0,
            READY,
            page_table,
            length
    });
    dispatch();
}

int block_process(int pid) {
    free_memory(pid);
    std::list<PCB>::iterator target;
    for (target = run_queue.begin(); target != run_queue.end(); ++target) {
        if (target->pid == pid)break;
    }
    target->status = BLOCK;
    block_queue.emplace_back(*target);
    run_queue.erase(target);
    dispatch();
    monitor();
}

int end_process(int pid) {
    free_memory(pid);
    std::list<PCB>::iterator target;
    int target_pid;
    for (target = run_queue.begin(); target != run_queue.end(); ++target) {
        if (target->pid == pid) {
            target_pid = pid;
            run_queue.erase(target);
            break;
        }
    }
    if (target->pid != pid)
        for (target = ready_queue.begin(); target != ready_queue.end(); ++target) {
            if (target->pid == pid) {
                target_pid = pid;
                ready_queue.erase(target);
                break;
            }
        }
    if (target->pid != pid)
        for (target = block_queue.begin(); target != block_queue.end(); ++target) {
            if (target->pid == pid) {
                target_pid = pid;
                block_queue.erase(target);
                break;
            }
        }

    monitor();
}

int suspend_process(int pid) {
    free_memory(pid);
    std::list<PCB>::iterator target;
    for (target = run_queue.begin(); target != run_queue.end(); ++target) {
        if (target->pid == pid)break;
    }
    target->status = READY;
    ready_queue.emplace_back(*target);
    run_queue.erase(target);
    dispatch();
    monitor();
}

std::vector<PCB::page_table_item> allocate_memory(int length) {
    int page_num = length % PAGE_FORM_SIZE == 0 ? length / PAGE_FORM_SIZE : length / PAGE_FORM_SIZE + 1;
    int live_page = page_num > MIN_PAGE ? MIN_PAGE : page_num;

    std::vector<PCB::page_table_item> page_table;
    auto temp_bitmap{bitmap};
    for (int i = 0; i < page_form_num, live_page > 0; ++i) {
        for (int j = 0; j < WORD_SIZE, live_page > 0; ++j) {
            if (temp_bitmap[i][j] != 1) {
                temp_bitmap[i].set(j);
                page_table.emplace_back(PCB::page_table_item{i * WORD_SIZE + j, ON});
                --live_page;
            }
        }
    }
    if (live_page == 0) {
        bitmap = temp_bitmap;
        for (int i = 0; i < page_num - live_page; ++i) {
            page_table.emplace_back(PCB::page_table_item{-1, OFF});
        }
        return page_table;
    } else {
        page_table.clear();
        return page_table;
    }
}

/**
 * free memory, according to the specific situation to choose if combine or not.
 * @param pid :The ID of a process, which is stored in the PCB.
 * @return -1 :false, 1 :success.
 */

int free_memory(int pid) {
    // According to pid to locate the staring place of it.
    std::vector<PCB::page_table_item> page_table;

    for (const auto &item : run_queue) {
        if (item.pid == pid) {
            page_table = item.page_table;
            break;
        }
    }
    // There is not the process.
    if (page_table.empty())return -1;

    for (const auto &item : page_table) {
        if (item.status == ON) {
            bitmap[item.mem_block / WORD_SIZE].reset(item.mem_block % WORD_SIZE);
        }
    }
}
