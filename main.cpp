//
// Created by haipinHu on 2021/5/28.
//
#include "os.h"

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
    page_frame_num = (u_bound - l_bound) / PAGE_FORM_SIZE;
    int left_bit = page_frame_num % WORD_SIZE;
    int word_num = left_bit == 0 ? page_frame_num / WORD_SIZE : page_frame_num / WORD_SIZE + 1;
    bitmap.resize(word_num);
    if (left_bit != 0) {
        for (int i = left_bit; i < WORD_SIZE; ++i) {
            bitmap[word_num - 1].set(i);
        }
    }
    std::cout << "-------------INSTRUCTIONS------------" << std::endl;
    std::cout << "ONE: OperationCode + PID/SPACE" << std::endl;
    std::cout << "  1-->Create a new process" << std::endl;
    std::cout << "  2-->Suspend a process" << std::endl;
    std::cout << "  3-->Block a process" << std::endl;
    std::cout << "  4-->End a process" << std::endl;
    std::cout << "  5-->[FIFO]Access a address" << std::endl;
    std::cout << "  6-->[LRU]Access a address" << std::endl;
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
            } else if (oc == 5) {
                double FIFO_missing_rate = access(stoi(instruction.substr(2, 1)), stoi(instruction.substr(4)), FIFO);
                monitor(FIFO_missing_rate);
            } else if (oc == 6) {
                double LRU_missing_rate = access(stoi(instruction.substr(2, 1)), stoi(instruction.substr(4)), LRU);
                monitor(LRU_missing_rate);
            }
            goto LOOP;
        } catch (std::exception e) {
            std::cout << "***Illegal instruction***" << std::endl;
            goto LOOP;
        }
        return 0;
    }
}

