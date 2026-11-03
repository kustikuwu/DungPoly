#include "network/internal/MessageFormat.h"

namespace Monopoly::Network::Messages {

    MessageHeader::MessageHeader(MessageType type, std::string_view s) :
            messageType(type),
            messageSize(s.size()),
            dataHash(std::hash<std::string_view>()(s)) {}

    void send(stream_socket& sock, MessageType type, const std::string& msg) {
        MessageHeader header(type, msg);
        Internal::send(sock, header);
        sock.write_n(msg.data(), msg.size());
    }

    std::optional<std::pair<MessageType, std::string>> receive(stream_socket& sock, bool timeout) {
        MessageHeader header;
        auto x = Internal::receive(sock, header, timeout);
        if(x < 0) return {};
        if (header.messageType == MessageType::INVALID_MESSAGE)
            throw InvalidMessageError("Получен некорректный тип сообщения");
        std::string ret(header.messageSize, '\0');
        x = sock.read_n(ret.data(), header.messageSize);
        if(x < 0)
            throw InvalidMessageError("Не удалось прочитать содержимое сообщения");
        if (header.dataHash != std::hash<std::string>()(ret))
            throw InvalidMessageError("Контрольная сумма не совпадает");
        return {{header.messageType, ret}};
    }
}