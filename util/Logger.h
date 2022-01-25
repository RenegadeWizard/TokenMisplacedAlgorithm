//
// Created by krzysztof on 21.01.2022.
//

#ifndef TOKENMISPLACEDALGORITHM_LOGGER_H
#define TOKENMISPLACEDALGORITHM_LOGGER_H


#include <iostream>
#include <iomanip>
#include <string>
#include <mutex>
#include <ctime>
#include "../model/Message.h"

class Logger {
private:
    static std::mutex mutex;
    static void log(int id, const std::string&, const std::string&);
public:
    static void info(int id, const std::string&);
    static void info(int id, const std::string&, Message);
    static void debug(int id, const std::string&);
    static void debug(int id, const std::string&, Message);
};


#endif //TOKENMISPLACEDALGORITHM_LOGGER_H
