//
// Created by tomot on 2021/5/13.
//

#ifndef OS_1_PCB_H
#define OS_1_PCB_H

#include <ostream>

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

struct PCB {
    // description
    int pid;
    int uid;
    // control and managemnt
    PCB_STATUS status;
    // resources
    int memory_segment_pointer;
    int length;

    // about CPU..
    int arrival_time; //到达时间
    int burst_time; //运行时间
    //int finished_time; //结束运行时间
    int runned_time = 0; //已运行时间

    /*PCB(int pid, int uid, PCB_STATUS status, int memorySegmentPointer, int length) : pid(pid), uid(uid), status(status),
                                                                                     memory_segment_pointer(
                                                                                             memorySegmentPointer),
                                                                                     length(length) {}

    PCB(int pid, int uid, PCB_STATUS status, int memorySegmentPointer, int length, int arrivalTime, int burstTime)
            : pid(pid), uid(uid), status(status), memory_segment_pointer(memorySegmentPointer), length(length),
              arrival_time(arrivalTime), burst_time(burstTime) {}*/

    bool operator<(const PCB &rhs) const {
        return pid < rhs.pid;
    }

    friend std::ostream &operator<<(std::ostream &os, const PCB &pcb) {
        os << "pid: " << pcb.pid << " uid: " << pcb.uid << " status: ";
        if (pcb.status == READY)os << "READY";
        else if (pcb.status == RUN)os << "RUN";
        else if (pcb.status == BLOCK)os << "BLOCK";
        os << " memory_segment_pointer: "
           << pcb.memory_segment_pointer << " length: " << pcb.length;
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

#endif //OS_1_PCB_H
