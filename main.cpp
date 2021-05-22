#include <iostream>
#include <queue>
#include <list>
#include <regex>
#include "PCB.h"

#define PAGE_SIZE 4
std::list<PCB> ready_queue;
std::list<PCB> run_queue;
std::list<PCB> block_queue;
std::list<partition> memory_list;
int u_bound = 0, l_bound = 0;
bitmap bitmap;
int ID_INDEX = 1;


int allocate_memory(int length);

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
    bitmap = bitmap(u_bound - l_bound);
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
    for (const auto &item : memory_list) {
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
    int begin = allocate_memory(length);
    if (begin == -1)return -1;
    ready_queue.emplace_back(PCB{
            ID_INDEX++,
            0,
            READY,
            begin,
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

/**
 *
 * @param length : needed length of a process.
 * @return -1 :false, a integer :staring location.
 */
int allocate_memory(int length) {
    int pageNum = length / PAGE_SIZE + 1;


    if (length <= 0 && u_bound - l_bound < length)return -1;
    if (memory_list.empty()) {
        memory_list.emplace_back(
                l_bound,
                u_bound - l_bound,
                UNOCCUPIED
        );
    }

    //
    std::list<partition>::iterator iter;
    for (iter = memory_list.begin(); iter != memory_list.end(); iter++) {
        if (iter->status == UNOCCUPIED && iter->length >= length) {
            break;
        }
    }
    if (iter->length < length) {
        return -1;
    } else if (iter->length == length) {
        iter->status = OCCUPIED;
        return iter->begin;
    } else if (iter->length > length) {
        int left_length = iter->length - length;
        int begin = iter->begin;

        iter->status = OCCUPIED;
        iter->length = length;

        memory_list.insert(++iter, partition{
                begin + length,
                left_length,
                UNOCCUPIED
        });
        return begin;
    }
}

/**
 * free memory, according to the specific situation to choose if combine or not.
 * @param pid :The ID of a process, which is stored in the PCB.
 * @return -1 :false, 1 :success.
 */
int free_memory(int pid) {
    // According to pid to locate the staring place of it.
    int begin = INT_MAX;
    for (const auto &item : run_queue) {
        if (item.pid == pid) {
            begin = item.memory_segment_pointer;
            break;
        }
    }
    // There is not the process.
    if (begin == INT_MAX)return -1;

    // Locate the partition in memory_list.
    std::list<partition>::iterator iter;

    for (iter = memory_list.begin(); iter != memory_list.end(); iter++) {
        if (iter->begin == begin) {
            break;
        }
    }
    if (iter->begin != begin) { // 分配失败
        return -1;
    }
    // free memory (and combine memory)
    iter->status = UNOCCUPIED;

    // It's of importance to save the origin iter.

    auto temp_iter{iter};
    std::list<partition>::iterator prior_iter;
    if (memory_list.begin() == temp_iter) {
        prior_iter = temp_iter;
    } else {
        prior_iter = --temp_iter;
        temp_iter++;
    }

    auto next_iter = memory_list.end() == ++temp_iter ? iter : temp_iter;


    if (iter != prior_iter &&
        prior_iter->begin + prior_iter->length == iter->begin &&
        prior_iter->status == UNOCCUPIED) {
        iter->length += prior_iter->length;
        iter->begin = prior_iter->begin;
        memory_list.erase(prior_iter);
    }

    if (iter != next_iter &&
        iter->begin + iter->length == next_iter->begin &&
        next_iter->status == UNOCCUPIED) {
        iter->length += next_iter->length;
        memory_list.erase(next_iter);
    }
}