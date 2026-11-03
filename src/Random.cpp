#include "Random.h"
#include <random>
#include <time.h>
RandomSource::RandomSource() {
    srand(time(nullptr));
}

int RandomSource::nextInt(int from, int to) {
    return (rand() % (to - from)) + from;
}


