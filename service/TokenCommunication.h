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
#define ACK_MSG 1
#define NON_ACK_MSG 2

class TokenCommunication {
private:
    int id;
    int numberOfProcesses;
    bool hasToken;
    MPI_Datatype datatype;
    int lastTokenId = 0;
    std::random_device rd;

    void sendMessage(Message message, int processId, int tag);
    [[nodiscard]] int nextProcess() const;
    [[nodiscard]] int previousProcess() const;
    [[nodiscard]] int nextId(int messageId) const;
    Message* receiveMessage();
    Message* receiveMessageTimeout(int processId);
    void processToken(Message* message);
    void processRecoveryToken(Message* message);
    bool shouldAcceptToken();
    void handleNonAckMessage(Message* message);
public:
    TokenCommunication(int id, int numberOfProcesses, bool hasToken);
    TokenCommunication(int id, int numberOfProcesses) : TokenCommunication(id, numberOfProcesses, false) {}

    void waitForToken();
    void sendToken();
};


#endif //TOKENMISPLACEDALGORITHM_TOKENCOMMUNICATION_H
