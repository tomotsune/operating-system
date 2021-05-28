//
// Created by tomot on 2021/5/13.
//

#ifndef OS_1_PCB_H
#define OS_1_PCB_H

#include <ostream>
#include <bitset>

using std::bitset;
/**
 * Process Control Block
 * Created by haipinHu on 5/15/2021
 */
enum PCB_STATUS {
    READY, RUN, BLOCK
};
enum MEMORY_STATUS {
    OCCUPIED, UNOCCUPIED
};
enum PAGE_STATUS {
    ON, OFF
};

struct PCB {
    struct page_table_item {

        int mem_block;
        PAGE_STATUS status;
        int ac_fds;
        //int e_addr;

    };
    // description
    int pid;
    int uid;
    // control and managemnt
    PCB_STATUS status;
    // resources
    std::deque<page_table_item> page_table;
    int length;

    // about CPU..
    bool operator<(const PCB &rhs) const {
        return pid < rhs.pid;
    }

    friend std::ostream &operator<<(std::ostream &os, const PCB &pcb) {
        os << "pid: " << pcb.pid << " uid: " << pcb.uid << " status: ";
        if (pcb.status == READY)os << "READY";
        else if (pcb.status == RUN)os << "RUN";
        else if (pcb.status == BLOCK)os << "BLOCK";
        os << " length: " << pcb.length << "page_table:";
        for (const auto &item : pcb.page_table) {
            os << item.mem_block << "(" << item.ac_fds << ")" << " ";
        }
        return os;
    }
};

struct partition {
    int begin;
    int length;
    MEMORY_STATUS status;

    partition(int begin, int length, MEMORY_STATUS status) : begin(begin), length(length),
                                                             status(status) {}

    friend std::ostream &operator<<(std::ostream &os, const partition &partition) {
        os << "begin: " << partition.begin << " length: " << partition.length << " status: ";
        if (partition.status == OCCUPIED)os << "OCCUPIED";
        else if (partition.status == UNOCCUPIED)os << "UNOCCUPIED";
        return os;
    }
};

struct bitmap {
    static const int pageFormSize = 4;
    std::vector<bitset<16>> data;
    int availablePageNum;
    int wordNum;

    bitmap() {}

    bitmap(int length) : availablePageNum(length / pageFormSize), wordNum(availablePageNum / 16),
                         data(std::vector<bitset<16>>(wordNum)) {}

    int operator[](const int index) const {
        return data[index / wordNum][index % wordNum];
    }

    void set(const int index) {
        data[index / wordNum].set(index % wordNum);
        availablePageNum -= 1;
    }

    void reset(const int index) {
        data[index / wordNum].reset(index % wordNum);
        availablePageNum += 1;
    }
};

#endif //OS_1_PCB_H
