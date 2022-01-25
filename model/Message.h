//
// Created by krzysztof on 21.01.2022.
//

#ifndef TOKENMISPLACEDALGORITHM_MESSAGE_H
#define TOKENMISPLACEDALGORITHM_MESSAGE_H

#include "MessageType.h"

struct Message {
    int id;
    MessageType type;
    int targetProcess;

    Message() = default;
    Message(int id, MessageType type, int targetProcess) : id(id), type(type), targetProcess(targetProcess) {}
};

#endif //TOKENMISPLACEDALGORITHM_MESSAGE_H
