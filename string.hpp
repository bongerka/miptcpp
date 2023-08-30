#include <iostream>
#include <cstring>


class String {
public:
    String();
    String(const char*);
    String(size_t, char);
    String(const String&);
    ~String();

    size_t length() const;
    bool empty() const;
    void push_back(char);
    void pop_back(); 
    void clear();

    size_t find(const String&) const;
    size_t rfind(const String&) const;
    String substr(size_t, size_t) const;
    char& front();
    char front() const;
    char& back();
    char back() const;    

    bool operator==(const String&) const;
    String& operator=(const String&);
    char& operator[](size_t);
    char operator[](size_t) const;
    String& operator+=(const String&);
    String& operator+=(char);

private:
    void swap(String&);
    void grow_buffer();
    size_t stringSize = 0;
    size_t bufferSize = 0;
    char* string = nullptr;
};



String::String(): bufferSize(1), string(new char[bufferSize]) {}

String::String(const char* cstyle_string): stringSize(strlen(cstyle_string)), 
                                           bufferSize(stringSize), 
                                           string(new char[bufferSize]) {
    memcpy(string, cstyle_string, stringSize);
}

String::String(size_t n, char symbol): stringSize(n), bufferSize(n),
string(new char[n]) {
    memset(string, symbol, n);
}

String::String(const String& old_string): stringSize(old_string.stringSize),
                                           bufferSize(stringSize),
                                           string(new char[bufferSize]) {
    memcpy(string, old_string.string, stringSize);
}

String::~String() {
    delete[] string;
}



size_t String::length() const{
    return stringSize;
}

bool String::empty() const {
    return stringSize == 0;
}

void String::push_back(char c) { 
    while (bufferSize <= stringSize) {
        grow_buffer(); 
    }
    string[stringSize++] = c;
}

void String::pop_back() {
    --stringSize;
}

void String::clear() {
    stringSize = 0;
}



size_t String::find(const String& substring) const {
    for (size_t i = 0; i < stringSize; ++i) {
        size_t index = i;
        for (size_t j = 0; j < substring.stringSize; ++j) {
            if (string[index] != substring[j]) break;
            if (j == substring.stringSize - 1) {
                return i;
            }
            ++index;
        }
    }
    return stringSize;
}

size_t String::rfind(const String& substring) const {
    for (size_t i = 0; i < stringSize; ++i) {
        size_t index = stringSize - i - 1;
        for (size_t j = 0; j < substring.stringSize; ++j) {
            if (string[index] != substring[substring.stringSize - j - 1]) break;
            if (j == substring.stringSize - 1) {
                return stringSize - i - substring.stringSize;
            }
            --index;
        }
    }
    return stringSize;
}   

String String::substr(size_t start, size_t count) const {
    String new_string;
    for (size_t i = start; i < start + count; ++i) {
        new_string += string[i];
    }
    return new_string;
}

char& String::front() {
    return string[0];
}

char String::front() const {
    return string[0];
}

char& String::back() {
    return string[stringSize - 1];
}

char String::back() const {
    return string[stringSize - 1];
}



bool String::operator==(const String& equal_string) const {
    if (stringSize == equal_string.stringSize) {
        for (size_t i = 0; i < stringSize; ++i) {
            if (string[i] != equal_string[i]) break;
            if (i == stringSize - 1) return true;
        }
    }
    return false;
}

String& String::operator=(const String& new_string) {
    String copy_string = new_string;
    swap(copy_string);
    return *this;
}

char& String::operator[](size_t index) {
    return string[index];
}

char String::operator[](size_t index) const {
    return string[index];
}

String& String::operator+=(const String& new_string) {
    for (size_t i = 0; i < new_string.stringSize; ++i) {
        push_back(new_string[i]);
    }
    return *this;
}

String& String::operator+=(char c) {
    push_back(c);
    return *this;
}



void String::swap (String& copy_string) {
    std::swap(copy_string.stringSize, stringSize);
    std::swap(copy_string.string, string);
    std::swap(copy_string.bufferSize, bufferSize);
}

void String::grow_buffer() {
    bufferSize *= 2;
    char* new_string = new char[bufferSize];
    memcpy(new_string, string, stringSize);
    delete[] string;
    string = new_string;
}



std::istream& operator>>(std::istream& in, String& string){
    string.clear();
    while (isspace(in.peek())) in.get();
    for (char c; !isspace(in.peek()) && in >> c;){
        string += c;
    }
    return in;
}

std::ostream& operator<<(std::ostream& out, const String& string) {
    for (size_t i = 0; i < string.length(); ++i) {
        out << string[i];
    }
    return out;
}

String operator+(String string1, const String& string2) {
    string1 += string2;
    return string1;
}

String operator+(String string1, char c) {
    string1 += c;
    return string1;
}

String operator+(char c, const String& string1) {
    String new_string;
    new_string += c;
    new_string += string1;
    return new_string;
}

