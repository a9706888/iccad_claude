#include "Timer.hpp"

#include "../global/global.hpp"

Timer::TimerComponent::TimerComponent() {
    startTime = stopTime = std::chrono::high_resolution_clock::now();
    isStopped = false;
}

void Timer::TimerComponent::start() {
    startTime = std::chrono::high_resolution_clock::now();
    isStopped = false;
}

void Timer::TimerComponent::stop() {
    stopTime = std::chrono::high_resolution_clock::now();
    isStopped = true;
}

std::chrono::milliseconds Timer::TimerComponent::getDuration() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime);
}

Timer::Timer()
    : timeLimit(std::chrono::seconds(60 * 60 - 10)), limitTimer(), maxTagLength(10) {}

Timer::Timer(int timeLimitInSeconds)
    : timeLimit(std::chrono::seconds(timeLimitInSeconds)), limitTimer(), maxTagLength(10) {}

bool Timer::overTime() {
    limitTimer.stop();
    return limitTimer.getDuration() >= timeLimit;
}

void Timer::startTimer(const std::string &tag) {
    tagToTimer[tag].start();
    if (maxTagLength < tag.size()) {
        maxTagLength = tag.size();
    }
}

void Timer::stopTimer(const std::string &tag) {
    if (!tagToTimer.count(tag)) {
        std::cerr << "[Error] Cannot stop timer. Timer \"" << tag << "\" never started.\n";
        return;
    }
    tagToTimer[tag].stop();
    endOrder.push_back(tag);
}

void Timer::printTime(const std::string &tag) {
    if (!tagToTimer.count(tag)) {
        std::cerr << "[Error] Cannot print time. Timer \"" << tag << "\" never started.\n";
        return;
    } else if (!tagToTimer[tag].isStopped) {
        std::cerr << "[Error] Cannot print time. Timer \"" << tag << "\" does not stop after starting.\n";
        return;
    }

    std::chrono::duration<double> second = tagToTimer[tag].getDuration();
    std::cout << std::setprecision(5)
              << std::setw((int)(maxTagLength + 2))
              << std::left << tag + ":" << second.count() << " s\n";
}

void Timer::printAllTimers() {
    std::cout << "\n========== Runtime statistics ===========\n";
    for (const auto &tag : endOrder) {
        printTime(tag);
    }
}

void Timer::stopAndPrintTime(const std::string &tag) {
    stopTimer(tag);
    printTime(tag);
}