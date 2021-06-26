#include <iostream>
#include <queue>
#include <list>
#include <regex>
#include "PCB.h"

std::list<PCB> ready_queue;
std::list<PCB> run_queue;
std::list<PCB> block_queue;
std::list<partition> memory_list;
int u_bound = 0, l_bound = 0;
int ID_INDEX = 1;

int allocate_memory(int length);

int free_memory(int pid);

void monitor();

int create_process(int length, int arrival, int burst);

int block_process(int pid);

int end_process(int pid);

int suspend_process(int pid);

void dispatch(int alg);

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
    std::cout << "-------------INSTRUCTIONS------------" << std::endl;
    std::cout << "ONE: OperationCode + PID/SPACE" << std::endl;
    std::cout << "  1-->Create a new process" << std::endl;
    std::cout << "  3-->Suspend a process" << std::endl;
    std::cout << "  3-->Block a process" << std::endl;
    std::cout << "  4-->End a process" << std::endl;
    std::cout << "  5-->Dispatch [alg]" << std::endl;
    std::cout << "TWO: monitor" << std::endl;
    std::cout << "--------------------------------------" << std::endl;
    LOOP:
    std::cout << "_>";
    //getline(std::cin, instruction);
    std::cin >> instruction;
    if (instruction == "monitor") {
        monitor();
        goto LOOP;
    } else {
        try {
            int oc = stoi(instruction.substr(0, 1));
            if (oc == 1) {
                //int flag = create_process(std::stoi(instruction.substr(2)));
                int size, arrival, burst;
                std::cin >> size >> arrival >> burst;
                int flag = create_process(size, arrival, burst);
                if (flag == -1) {
                    std::cout << "***memory allocation failed***" << std::endl;
                }
            } else if (oc == 2) {
                suspend_process(std::stoi(instruction.substr(2)));
            } else if (oc == 3) {
                block_process(stoi(instruction.substr(2)));
            } else if (oc == 4) {
                end_process(stoi(instruction.substr(2)));
            } else if (oc == 5) {
                int alg;
                std::cin >> alg;
                dispatch(alg);
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
void dispatch(int alg) {

    // It can simulate 4-core CPU, and can be executed by 4 processes simultaneously
////////////////////////////////////////////////////////////////////
//    if (run_queue.size() < 4 && !ready_queue.empty()) {
//        // According to the process scheduling algorithm, select the appropriate process to run on the processor.
//        PCB pcb = ready_queue.front();
//        pcb.status = RUN;
//        run_queue.emplace_back(pcb);
//        ready_queue.pop_front();
//    }
    if (alg == 1) {
        // FCFS
        ready_queue.sort([](PCB a, PCB b) {
            return a.arrival_time < b.arrival_time;
        });
        //記錄運行完成時間
        int finished = ready_queue.front().arrival_time;
        //記錄各個進程的周轉時間 [完成時間-到達時間]
        double total_turnarounds = 0;
        double total_turnarounds_with_rights = 0;
        for (const auto &item : ready_queue) {
            //std::cout << item << std::endl;
            finished += item.burst_time;
            // 如果在前一个进程结束后到达..
            if (item.arrival_time > finished)
                finished = item.arrival_time + item.burst_time;
            double turnaround = finished - item.arrival_time;
            total_turnarounds += turnaround;
            double turnaround_with_rights = turnaround / item.burst_time;
            total_turnarounds_with_rights += turnaround_with_rights;

            std::cout << "pid: " << item.pid << "turnaround_time:" << turnaround
                      << " Turnaround time with rights:" << turnaround_with_rights << std::endl;
        }
        std::cout << "average:" << total_turnarounds / ready_queue.size()
                  << "average_with_rights:" << total_turnarounds_with_rights / ready_queue.size();

    }
    if (alg == 2) {
        //SJF
        ready_queue.sort([](PCB a, PCB b) {
            return a.arrival_time < b.arrival_time;
        });
        int finished = ready_queue.front().burst_time + ready_queue.front().arrival_time;
        double total_turnarounds = finished - ready_queue.front().arrival_time;
        double total_turnarounds_with_rights = total_turnarounds / ready_queue.front().burst_time;
        std::cout << "pid: " << ready_queue.front().pid << "turnaround_time:" << total_turnarounds
                  << " Turnaround time with rights:" << total_turnarounds_with_rights << std::endl;
        ready_queue.pop_front();
        ready_queue.sort([](PCB a, PCB b) {
            return a.burst_time < b.burst_time;
        });
        for (const auto &item : ready_queue) {
            finished += item.burst_time;
            // 如果在前一个进程结束后到达..
            if (item.arrival_time > finished)
                finished = item.arrival_time + item.burst_time;
            double turnaround = finished - item.arrival_time;
            total_turnarounds += turnaround;
            double turnaround_with_rights = turnaround / item.burst_time;
            total_turnarounds_with_rights += turnaround_with_rights;

            std::cout << "pid: " << item.pid << "turnaround_time:" << turnaround
                      << " Turnaround time with rights:" << turnaround_with_rights << std::endl;
        }
        std::cout << "average:" << total_turnarounds / (ready_queue.size() + 1)
                  << "average_with_rights:" << total_turnarounds_with_rights / (ready_queue.size() + 1);
    }
    if (alg == 3) {
        //RR
        ready_queue.sort([](PCB a, PCB b) {
            return a.arrival_time < b.arrival_time;
        });
        int num = 0;
        while (num < ready_queue.size()) {
            if (ready_queue.front().runned_time < ready_queue.front().burst_time) {
                ready_queue.front().runned_time += 2;
                std::cout << "pid: " << ready_queue.front().pid << " run 2ms" << std::endl;
            } else num++;


            ready_queue.push_back(ready_queue.front());
            ready_queue.pop_front();
        }
    }

}

int create_process(int length, int arrival, int burst) {
    int begin = allocate_memory(length);
    if (begin == -1)return -1;

    ready_queue.emplace_back(PCB{
            ID_INDEX++,
            0,
            READY,
            begin,
            length,
            arrival,
            burst
    });
    //dispatch();
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
    //dispatch();
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
    //dispatch();
    monitor();
}

/**
 *
 * @param length : needed length of a process.
 * @return -1 :false, a integer :staring location.
 */
int allocate_memory(int length) {
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