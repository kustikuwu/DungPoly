#pragma once
#include <cstdint>
#include <optional>
#include <type_traits>
#include <sockpp/tcp_socket.h>
#include <cstddef>

namespace Monopoly::Network::Messages {
    using sockpp::stream_socket;

    class InvalidMessageError : std::runtime_error {
        using std::runtime_error::runtime_error;
    };

    enum class MessageType : uint32_t {
        //other:
        INVALID_MESSAGE,
        //s2c:
        BOARD,
        REQUEST,
        MESSAGE,
        //c2s:
        REPLY,
        INITIALIZE
    };

    struct MessageHeader {
        explicit MessageHeader(MessageType type, std::string_view s);
        MessageHeader() = default;
        MessageType messageType = MessageType::INVALID_MESSAGE;
        uint32_t messageSize = 0;
        uint64_t dataHash = 0;
    };


    namespace Internal {

        inline ssize_t read(stream_socket& sock, std::byte* buf, size_t n, bool timeout) {
            if(!timeout) return sock.read_n(buf, n);
            ssize_t x = sock.read(buf, n);
            if(x < 0) return x;
            ssize_t y = read(sock, buf + x, n - x, false);
            return (y < 0) ? y : x + y;
        }

        template<typename T>
        std::enable_if_t<std::is_integral_v<T>, size_t> send(stream_socket& sock, T val) {
            constexpr size_t SIZE = sizeof(T);
            std::byte buf[SIZE];
            for(size_t i = 0; i < SIZE; i++) {
                buf[i] = std::byte(val % 0xFF);
                val >>= 8u;
            }
            return sock.write(buf, SIZE);
        }

        template<typename T>
        std::enable_if_t<std::is_integral_v<T>, ssize_t> receive(stream_socket& sock, T& val, bool timeout) {
            constexpr size_t SIZE = sizeof(T);
            std::byte buf[SIZE];
            auto x = read(sock, buf, SIZE, timeout);
            if(x < 0) return x;
            for(size_t i = SIZE; i > 0; i--) {
                val |= std::to_integer<T>(buf[i - 1]);
                val <<= 8u;
            }
            return x;
        }

        template<typename T>
        std::enable_if_t<std::is_enum_v<T>, size_t> send(stream_socket& sock, const T& val) {
            return send(sock, std::underlying_type_t<T>(val));
        }

        template<typename T>
        std::enable_if_t<std::is_enum_v<T>, ssize_t> receive(stream_socket& sock, T& val, bool timeout) {
            std::underlying_type_t<T> tmp;
            auto x = receive(sock, tmp, timeout);
            if(x < 0) return x;
            val = T{tmp};
            return x;
        }

        inline size_t send(stream_socket& sock, const MessageHeader& header) {
            size_t s = 0;
            s += send(sock, header.messageSize);
            s += send(sock, header.messageType);
            s += send(sock, header.dataHash);
            return s;
        }

        inline ssize_t receive(stream_socket& sock, MessageHeader& header, bool timeout) {
            ssize_t s = 0;
            auto x = receive(sock, header.messageSize, timeout);
            if(x < 0) return x;
            s += x;
            x = receive(sock, header.messageType, false);
            if(x < 0) return x;
            s += x;
            x = receive(sock, header.dataHash, false);
            if(x < 0) return x;
            s += x;
            return s;
        }
    }
    void send(stream_socket& sock, MessageType type, const std::string& msg);
    std::optional<std::pair<MessageType, std::string>> receive(stream_socket& sock, bool timeout);

}