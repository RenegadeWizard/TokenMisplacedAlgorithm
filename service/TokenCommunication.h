//
// Created by krzysztof on 22.01.2022.
//

#ifndef TOKENMISPLACEDALGORITHM_TOKENCOMMUNICATION_H
#define TOKENMISPLACEDALGORITHM_TOKENCOMMUNICATION_H

#include <mpi.h>
#include <chrono>
#include <thread>
#include <random>
#include "../model/Message.h"
#include "../util/Logger.h"

#define MESSAGE_SIZE 3
#define TIME_OUT 100

class TokenCommunication {
private:
    int id;
    int numberOfProcesses;
    bool hasToken;
    MPI_Datatype datatype;
    int lastTokenId = 0;
    std::random_device rd;

    void sendMessage(Message message, int processId);
    [[nodiscard]] int nextProcess() const;
    [[nodiscard]] int previousProcess() const;
    [[nodiscard]] int nextId(int messageId) const;
    Message* receiveToken();
    Message* receiveAck(int processId);
    void processToken(Message* message);
    bool shouldAcceptToken();
    bool compareIds(int id);
public:
    TokenCommunication(int id, int numberOfProcesses, bool hasToken);
    TokenCommunication(int id, int numberOfProcesses) : TokenCommunication(id, numberOfProcesses, false) {}

    void waitForToken();
    void sendToken();
};


#endif //TOKENMISPLACEDALGORITHM_TOKENCOMMUNICATION_H
