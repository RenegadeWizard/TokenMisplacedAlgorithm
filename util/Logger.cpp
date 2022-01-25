//
// Created by krzysztof on 21.01.2022.
//

#include <thread>
#include "Logger.h"

std::mutex Logger::mutex;

void Logger::info(int id, const std::string& string) {
    log(id, "INFO", string);
}

void Logger::info(int id, const std::string& mainMessage, Message message) {
    std::string type;
    switch (message.type) {
        case TOKEN:
            type = "TOKEN";
            break;
        case ACK:
            type = "ACK";
            break;
        default:
            type = "NONE";
    }
    info(id, mainMessage + " | " + type + " | target: " + std::to_string(message.targetProcess) + " | message id: " + std::to_string(message.id));
}

void Logger::debug(int id, const std::string& string) {
    log(id, "DEBUG", string);
}

void Logger::debug(int id, const std::string& mainMessage, Message message) {
    std::string type;
    switch (message.type) {
        case TOKEN:
            type = "TOKEN";
            break;
        case ACK:
            type = "ACK";
            break;
        default:
            type = "NONE";
    }
    debug(id, mainMessage + " | " + type + " | target: " + std::to_string(message.targetProcess) + " | message id: " + std::to_string(message.id));
}

void Logger::log(int id, const std::string& level, const std::string& message) {
    mutex.lock();
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    std::cout << "[" + level + "] " << "[" << id << "] "
        << std::setfill('0') << std::setw(2) << ltm->tm_hour << ":"
        << std::setfill('0') << std::setw(2) << ltm->tm_min << ":"
        << std::setfill('0') << std::setw(2) << ltm->tm_sec << " | "
        << message << "\n" << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    mutex.unlock();
}
