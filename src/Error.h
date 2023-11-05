#pragma once
#include <exception>
#include <string>

class SemanticError : public std::exception {
public:
    SemanticError(const std::string& message) : message(message) {}

    const char* what() const noexcept override {
        return message.c_str();
    }

private:
    std::string message;
};
class SymbolError : public std::exception {
public:
    SymbolError(const std::string& message) : message(message) {}

    const char* what() const noexcept override {
        return message.c_str();
    }

private:
    std::string message;
};