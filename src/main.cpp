#include "MonopolyFwd.h"
#include "Manager.h"
#include "Board.h"
//#include "include/network/internal/InternalSerialization.h"
#include <clocale>
#include <iostream>


int main() {
    setlocale(LC_ALL, "");
    Manager manager;
    manager.run();
    return 0;
}

