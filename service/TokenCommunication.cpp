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
        auto message = receiveMessage();
        switch (message->type) {
            case TOKEN:
                processToken(message);
                break;
            case REC_TOKEN:
                processRecoveryToken(message);
                break;
            case ACK:
                waitForToken();
                break;
        }
        delete message;
    }
}

void TokenCommunication::sendToken() {
    hasToken = false;
    auto message = new Message(nextId(lastTokenId), TOKEN, nextProcess());
    sendMessage(*message, nextProcess(), NON_ACK_MSG);
    auto ackMessage = receiveMessageTimeout(nextProcess());
    while (ackMessage != nullptr && ackMessage->id != message->id) {
        delete ackMessage;
        ackMessage = receiveMessageTimeout(nextProcess());
    }
    while (ackMessage == nullptr) {
        message->type = REC_TOKEN;
        message->targetProcess = nextProcess();
        sendMessage(*message, previousProcess(), NON_ACK_MSG);
        delete ackMessage;
        ackMessage = receiveMessageTimeout(previousProcess());
    }
    delete ackMessage;
    delete message;
}

void TokenCommunication::sendMessage(Message message, int processId, int tag) {
    Logger::debug(id, "Sending: ", message);
    MPI_Send(&message, 1, datatype, processId, tag, MPI_COMM_WORLD);
}

Message* TokenCommunication::receiveMessage() {
    MPI_Status status;
    auto message = new Message();
    MPI_Recv(message, 1, datatype, MPI_ANY_SOURCE, NON_ACK_MSG, MPI_COMM_WORLD, &status);
    Logger::debug(id, "Received: ", *message);
    return message;
}

Message* TokenCommunication::receiveMessageTimeout(int processId) {
    std::this_thread::sleep_for(std::chrono::milliseconds(TIME_OUT));
    Message* message = nullptr;
    int flag;
    MPI_Iprobe(processId, ACK_MSG, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
    MPI_Iprobe(processId, ACK_MSG, MPI_COMM_WORLD, &flag, MPI_STATUS_IGNORE);
    if (flag) {
        message = new Message();
        MPI_Recv(message, 1, datatype, processId, ACK_MSG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        Logger::debug(id, "Received (timeout): ", *message);
    } else {
        Logger::debug(id, "Received (timeout): NOTHING");
    }
    return message;
}

void TokenCommunication::processToken(Message* message) {
    if (!shouldAcceptToken()) {
        Logger::info(id, "Omitting token");
        return;
    }
    if (message->id > lastTokenId) {
        hasToken = true;
        lastTokenId = message->id;
    }
    message = new Message(message->id, ACK, previousProcess());
    sendMessage(*message, previousProcess(), ACK_MSG);
}

void TokenCommunication::processRecoveryToken(Message* message) {
    if (!shouldAcceptToken()) {
        Logger::info(id, "Omitting rec_token");
        return;
    }
    Message* ackMessage;
    if (message->targetProcess != id) {
        ackMessage = new Message(message->id, ACK, nextProcess());
        sendMessage(*ackMessage, nextProcess(), ACK_MSG);
        if (message->id <= lastTokenId) { // TODO: Optymalizacja
//            Logger::info(id, "This bitch is old");
            return;
        }
        do {
            sendMessage(*message, previousProcess(), NON_ACK_MSG);
            ackMessage = receiveMessageTimeout(previousProcess());
        } while (ackMessage == nullptr || ackMessage->id != message->id);
        return;
    }
    if (message->id > lastTokenId) {
        hasToken = true;
        lastTokenId = message->id;
    } else {
//        Logger::info(id, "This bitch is old v2");
    }
    message->type = ACK;
    message->targetProcess = nextProcess();
    sendMessage(*message, nextProcess(), ACK_MSG);
}

int TokenCommunication::nextProcess() const {
    return (id + 1) % numberOfProcesses;
}

int TokenCommunication::previousProcess() const {
    return id > 0 ? id - 1 : numberOfProcesses - 1;
}

int TokenCommunication::nextId(int messageId) const {
    return messageId + 1;
//    return (messageId + 1) % (numberOfProcesses + 1);
}

bool TokenCommunication::shouldAcceptToken() {
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(1, 5);
    if (dist(mt) == 1) {
        return false;
    }
    std::uniform_int_distribution<int> distTime(1, 200);
    int time = distTime(rd);
    std::this_thread::sleep_for(std::chrono::milliseconds(time));
    return true;
}
