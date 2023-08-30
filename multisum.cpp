#include <iostream>


size_t strlen(const char* str) {
    const char *copy = str;
    while (*copy != '\0') {
        ++copy;
    }
    return copy - str;
}

int pow(int number, int power) {
    int answer = 1;
    for (int i = 0; i < power; ++i) {
        answer *= number;
    }
    return answer;
}

int char_to_int(const char* str) {
    int number = 0;
    for (size_t i = 0; i < strlen(str); ++i) {
        number += (str[i] - '0') * pow(10, strlen(str) - i - 1);
    }
    return number;
}

long long get_sum(int** arrays, int* lens, int* indexes, int size, int current_count, long long mult) {
    long long sum = 0;
    for (int i = 0; i < lens[current_count]; ++i) {
        bool stop = false;
        for (int j = 0; j < current_count; ++j) {
            if (i == indexes[j]) {
                stop = true;
                break;
            }
        }
        if (stop) {
            continue;
        }

        if (current_count == size - 1) {
            sum += mult * arrays[current_count][i];
        } else {
            indexes[current_count] = i;
            sum += get_sum(arrays, lens, indexes, size, current_count + 1, mult * arrays[current_count][i]);
        }
    }
    return sum;
}

int main(int argc, char** argv) {
    int args_count = argc - 1;
    int** arrays = new int*[args_count];
    int* lens = new int[args_count];

    for (int i = 1; i < argc; ++i) {
        lens[i - 1] = char_to_int(argv[i]);
        arrays[i - 1] = new int[lens[i - 1]];
        for (int j = 0; j < lens[i - 1]; ++j) {
            std::cin >> arrays[i - 1][j];
        }
    }

    int* indexes = new int[args_count];
    long long answer = get_sum(arrays, lens, indexes, args_count, 0, 1);
    std::cout << answer;

    delete[] lens;
    delete[] indexes;
    for (int i = 0; i < args_count; ++i) {
        delete[] arrays[i];
    }
    delete[] arrays;
}


