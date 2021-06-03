//
// Created by haipinHu on 2021/5/28.
//

#include "os.h"

std::list<PCB> ready_queue;
std::list<PCB> run_queue;
std::list<PCB> blocked_queue;
std::vector<bitset<WORD_SIZE>> bitmap;
int u_bound = 0, l_bound = 0;
int page_frame_num = 0;
int request[10]{};
int ID_INDEX = 1;

void monitor() {
    std::cout << "<process>" << std::endl;
    auto whole_processes{ready_queue};
    auto temp_run_queue{run_queue};
    auto temp_block_queue{blocked_queue};
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

void monitor(double missing_rate) {

    if (missing_rate != -1) {
        monitor();
        std::cout << "missing page rate: " << missing_rate << std::endl;
    } else {
        std::cout << "out of range" << std::endl;
    }
}

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

std::deque<PCB::page_table_item> allocate_memory(int length) {
    int page_num = length % PAGE_FORM_SIZE == 0 ? length / PAGE_FORM_SIZE : length / PAGE_FORM_SIZE + 1;
    int live_page = page_num > MIN_PAGE ? MIN_PAGE : page_num;

    std::deque<PCB::page_table_item> page_table;
    auto temp_bitmap{bitmap};
    auto temp{live_page};
    for (int i = 0; i < page_frame_num, live_page > 0; ++i) {
        for (int j = 0; j < WORD_SIZE, live_page > 0; ++j) {
            if (temp_bitmap[i][j] != 1) {
                temp_bitmap[i].set(j);
                page_table.push_back(PCB::page_table_item{i * WORD_SIZE + j, ON, 1});
                --live_page;
            }
        }
    }
    if (live_page == 0) {
        bitmap = temp_bitmap;
        for (int i = 0; i < page_num - temp; ++i) {
            page_table.push_back(PCB::page_table_item{-1, OFF, 0});
        }
        return page_table;
    } else {
        throw std::string("allocation err");
    }
}

int free_memory(int pid) {
    // According to pid to locate the staring place of it.
    std::deque<PCB::page_table_item> page_table;

    for (const auto &item : run_queue) {
        if (item.pid == pid) {
            page_table = item.page_table;
            break;
        }
    }
    // There is not the process.
    if (page_table.empty())return -1;
    while (!page_table.empty()) {
        if (page_table.front().status == ON) {
            bitmap[page_table.front().mem_block / WORD_SIZE].reset(page_table.front().mem_block % WORD_SIZE);
        }
        page_table.pop_back();
    }
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

int block_process(int pid) {
    free_memory(pid);
    std::list<PCB>::iterator target;
    for (target = run_queue.begin(); target != run_queue.end(); ++target) {
        if (target->pid == pid)break;
    }
    target->status = BLOCK;
    blocked_queue.emplace_back(*target);
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
        for (target = blocked_queue.begin(); target != blocked_queue.end(); ++target) {
            if (target->pid == pid) {
                target_pid = pid;
                blocked_queue.erase(target);
                break;
            }
        }

    monitor();
}

double access(int pid, int addr, std::function<void(std::deque<PCB::page_table_item> &, int, int)> alg) {

    // Get the iterator of the process PCB based on the process id.
    std::list<PCB>::iterator target;
    for (target = run_queue.begin(); target != run_queue.end(); ++target) {
        if (target->pid == pid) {
            break;
        }
    }
    // Perform id legitimacy verification and address out-of-bounds checks.
    if (target->pid != pid || addr >= target->length)return -1;

    int live_num = 0; // Number of pages in memory for the process.
    int visit = 0; // Total number of page views, used to calculate the missing page rate.
    auto &page_table{target->page_table};
    for (const auto &item : page_table) {
        if (item.status == ON)++live_num;
        visit += item.ac_fds;
    }

    int page_number = addr / PAGE_FORM_SIZE;
    //int page_offset = addr % WORD_SIZE;

    if (page_table[page_number].status == ON) {
        // Case 1: The page corresponding to the logical address is in memory.
        page_table[page_number].ac_fds++;
        return double(request[pid] + live_num) / (visit + 1);
    } else {
        for (int i = 0; i < page_frame_num; ++i) {
            for (int j = 0; j < WORD_SIZE > 0; ++j) {
                if (bitmap[i][j] == 0) {
                    if (live_num < MAX_PAGE) {
                        // Case 2: The page corresponding to the logical address is not in memory
                        // and the number of pages has not yet reached its page limit.
                        bitmap[i].set(j);
                        page_table[page_number] = PCB::page_table_item{i * WORD_SIZE + j, ON, 1};
                        request[pid]++;
                        return double(request[pid] + live_num) / (visit + 1);
                    } else {
                        // Case 3: The page corresponding to the logical address is not in memory
                        // and the maximum number of pages has been reached.
                        alg(page_table, i, j); // Callback, executing a FIFO or LRU policy depending on the parameters.
                        request[pid]++;
                        return double(request[pid] + live_num) / (visit + 1);
                    }
                }
            }
        }
        return -1;
    }
}

void FIFO(std::deque<PCB::page_table_item> &page_table, int i, int j) {
    page_table.pop_front();
    std::deque<PCB::page_table_item>::iterator target;
    for (target = page_table.begin(); target != page_table.end(); ++target) {
        if (target->status == OFF) {
            break;
        }
    }
    page_table.insert(target, {i * WORD_SIZE + j, ON, 1});
}

void LRU(std::deque<PCB::page_table_item> &page_table, int i, int j) {
    int least_ac_fds = INT_MAX;
    int least_idx = 0;
    for (int k = 0; k < page_table.size() && page_table[k].ac_fds > 0; ++k) {
        if (page_table[k].ac_fds < least_ac_fds) {
            least_ac_fds = page_table[k].ac_fds;
            least_idx = k;
        }
    }
    page_table[least_idx].mem_block = i * WORD_SIZE + j;
    page_table[least_idx].ac_fds++;
}

void add_random(int num) {
    srand(time(0));
    for (int i = 0; i < num; ++i) {
        int x = rand() % (WORD_SIZE * bitmap.size());
        bitmap[x / WORD_SIZE].set(x % WORD_SIZE);
    }
    monitor();
}