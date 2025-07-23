#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class Timer {
    struct TimerComponent {
        std::chrono::high_resolution_clock::time_point startTime, stopTime;
        bool isStopped;

        TimerComponent();
        void start();
        void stop();
        std::chrono::milliseconds getDuration();
    };

    std::chrono::seconds timeLimit;
    TimerComponent limitTimer;
    size_t maxTagLength;
    std::unordered_map<std::string, TimerComponent> tagToTimer;
    std::vector<std::string> endOrder;

   public:
    Timer();
    Timer(int timeLimitInSeconds);

    bool overTime();
    void startTimer(const std::string &tag);
    void stopTimer(const std::string &tag);
    void printTime(const std::string &tag);
    void printAllTimers();
    void stopAndPrintTime(const std::string &tag);
};