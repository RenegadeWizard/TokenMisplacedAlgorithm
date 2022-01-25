//
// Created by krzysztof on 22.01.2022.
//


#include "TokenCommunication.h"


TokenCommunication::TokenCommunication(int id, int numberOfProcesses, bool hasToken) : id(id), numberOfProcesses(numberOfProcesses), hasToken(hasToken) {
    int blockLengths[MESSAGE_SIZE] = {1, 1, 1};
    MPI_Datatype types[MESSAGE_SIZE] = {MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[MESSAGE_SIZE];
    offsets[0] = offsetof(struct Message, id);
    offsets[1] = offsetof(struct Message, type);
    offsets[2] = offsetof(struct Message, targetProcess);

    if(MPI_Type_create_struct(MESSAGE_SIZE, blockLengths, offsets, types, &datatype) != MPI_SUCCESS){
        perror("Datatype could not be created");
    }
    MPI_Type_commit(&datatype);
}

void TokenCommunication::waitForToken() {
    while (!hasToken) {
        auto message = receiveToken();
        switch (message->type) {
            case TOKEN:
                processToken(message);
                break;
            case ACK:
                break;
        }
        delete message;
    }
    Logger::info(id, "Critical section (" + std::to_string(lastTokenId) + ")");
}

void TokenCommunication::sendToken() {
    hasToken = false;
    auto message = new Message(nextId(lastTokenId), TOKEN, nextProcess());
    sendMessage(*message, nextProcess());
    auto ackMessage = receiveAck(MPI_ANY_SOURCE);
    while (true) {
        if (ackMessage != nullptr && ackMessage->type == TOKEN) {
            processToken(ackMessage);
            if (ackMessage->id > message->id) {
                break;
            }
            ackMessage = receiveAck(MPI_ANY_SOURCE);
            continue;
        } else if (ackMessage != nullptr && ackMessage->id >= message->id) {
            break;
        }
        sendMessage(*message, nextProcess());
//        delete ackMessage;
        ackMessage = receiveAck(MPI_ANY_SOURCE);
    }
    delete ackMessage;
    delete message;
}

void TokenCommunication::sendMessage(Message message, int processId) {
    Logger::debug(id, "Sending(" + std::to_string(lastTokenId) + "): ", message);
    MPI_Send(&message, 1, datatype, processId, message.type, MPI_COMM_WORLD);
}

Message* TokenCommunication::receiveToken() {
    MPI_Status status;
    auto message = new Message();
    MPI_Recv(message, 1, datatype, MPI_ANY_SOURCE, TOKEN, MPI_COMM_WORLD, &status);
    Logger::debug(id, "Received(" + std::to_string(lastTokenId) + "): ", *message);
    return message;
}

Message* TokenCommunication::receiveAck(int processId) {
    Message* message = nullptr;
    int flag;
    std::chrono::time_point time = std::chrono::system_clock::now();
    while ((std::chrono::system_clock::now() - time).count() < TIME_OUT * 1000 * 1000) {
        MPI_Iprobe(processId, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
        if (flag) {
            message = new Message();
            MPI_Recv(message, 1, datatype, processId, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            Logger::debug(id, "Received ACK: ", *message);
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    if (message != nullptr) {
        Logger::debug(id, "Received ACK: NOTHING");
    }
    return message;
}

void TokenCommunication::processToken(Message* message) {
    if (!shouldAcceptToken()) {
        Logger::debug(id, "Omitting token");
    }
    if (compareIds(message->id)) {
        Logger::debug(id, "is being tokened");
        hasToken = true;
        lastTokenId = message->id;
    }
    message = new Message(message->id, ACK, previousProcess());
    sendMessage(*message, previousProcess());
}

int TokenCommunication::nextProcess() const {
    return (id + 1) % numberOfProcesses;
}

int TokenCommunication::previousProcess() const {
    return id > 0 ? id - 1 : numberOfProcesses - 1;
}

int TokenCommunication::nextId(int messageId) const {
    return messageId + 1;
//    return (messageId + 1) % (2 * numberOfProcesses + 1);
}

bool TokenCommunication::compareIds(int messageId) {
//    if (lastTokenId == numberOfProcesses * 2 && messageId == 0) {
//        return true;
//    }
    return messageId > lastTokenId;
}

bool TokenCommunication::shouldAcceptToken() {
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1, 20);
    if (dist(mt) == 1) {
        return false;
    }
    std::uniform_int_distribution<int> distTime(1, 55);
    int time = distTime(rd);
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
    return true;
}
