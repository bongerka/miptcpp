#include <iostream>
#include <vector>
#include <string>
#include <sstream>

class BigInteger {
public:
	BigInteger() = delete;
    BigInteger(int);
    BigInteger(std::string);
 
    std::string toString() const;

    BigInteger& operator=(const BigInteger&);
    BigInteger& operator+=(const BigInteger&);
    BigInteger& operator-=(const BigInteger&);
    BigInteger& operator*=(const BigInteger&);
    BigInteger& operator/=(const BigInteger&);
    BigInteger& operator%=(const BigInteger&);
    BigInteger& operator-();
    BigInteger& operator++();
    BigInteger& operator--();
    BigInteger operator++(int);
    BigInteger operator--(int);
    bool operator==(const BigInteger&) const;
    bool operator!=(const BigInteger&) const;
    bool operator<=(const BigInteger&) const;
    bool operator>=(const BigInteger&) const;
    bool operator>(const BigInteger&) const;
    bool operator<(const BigInteger&) const;

    explicit operator int();
    explicit operator bool();

private:
    void sum_similar_sign(const BigInteger&);
    void sum_diff_sign(const BigInteger&);
    void reverse();
    void normalize();
    int shift();
    bool is_bigger_abs(const BigInteger&);

    std::vector<long long> _bigInteger;
    bool _isPositive;
    static const int BASE = 1e9;
    static const int DIGITS_COUNT = 9;
};



BigInteger::BigInteger(int number = 0) {
    _isPositive = (number >= 0);
    if (!_isPositive) {
        number = -number;
    }
    if (number == 0) {
        _bigInteger.push_back(0);
    }
    while (number) {
        _bigInteger.push_back(number % BASE);
        number /= BASE;
    }
    normalize();
}

BigInteger::BigInteger(std::string string) {
    if (string[0] == '-') {
        _isPositive = false;
        std::string sub_string;

        for (int i = string.size() - 1; i > 0; i -= DIGITS_COUNT) {
            sub_string = "";
            for (int j = i - std::min(DIGITS_COUNT - 1, i - 1); j <= i; ++j) {
                sub_string += string[j];
            }
            _bigInteger.push_back(std::atoi(sub_string.c_str()));
        }

    } else {
        _isPositive = true;
        std::string sub_string;

        for (int i = string.size() - 1; i >= 0; i -= DIGITS_COUNT) {
            sub_string = "";
            for (int j = i - std::min(DIGITS_COUNT - 1, i); j <= i; ++j) {
                sub_string += string[j];
            }
            _bigInteger.push_back(std::atoi(sub_string.c_str()));
        }

    }
    normalize();
}



std::string BigInteger::toString() const {
    std::stringstream ss;
    if (!_isPositive)  {
        ss << "-";
    }
    for (size_t i = 0; i < _bigInteger.size(); ++i) {
        std::string str = std::to_string(_bigInteger[_bigInteger.size() - 1 - i]);
        int amount_zeros = 9 - str.size();

        for (int j = 0; j < amount_zeros && i > 0; ++j) {
            ss << 0;
        }
        ss << _bigInteger[_bigInteger.size() - i - 1];
    }
    std::string string;
    ss >> string;
    return string;
}



BigInteger& BigInteger::operator=(const BigInteger& rhs) {
    if (rhs == *this) {
        return *this;
    }

    BigInteger copy = rhs;
    std::swap(_isPositive, copy._isPositive);
    std::swap(_bigInteger, copy._bigInteger);
    return *this;
}

BigInteger& BigInteger::operator+=(const BigInteger& rhs) {
    if (rhs == *this) {
        return (*this *= 2);
    }

    if (_isPositive) {
        if (rhs._isPositive) {
            sum_similar_sign(rhs);
        } else {
            if (!is_bigger_abs(rhs)) {
                BigInteger copy = rhs;
                copy.sum_diff_sign(*this); 
                (*this) = copy;
            } else {
                sum_diff_sign(rhs);
            }
        }
    } else {
        if (rhs._isPositive) {
            if (!is_bigger_abs(rhs)) {
                BigInteger copy = rhs;
                copy.sum_diff_sign(*this); 
                (*this) = copy;
            } else {
                sum_diff_sign(rhs);
            }
        } else {
            sum_similar_sign(rhs);
        }
    }
    return *this;
}

BigInteger& BigInteger::operator-=(const BigInteger& rhs) {
    if (rhs == *this) {
        *this = 0;
        return *this;
    }

    BigInteger copy = rhs;
    (*this) += (-copy);
    if (_isPositive) {
        if (rhs._isPositive && _bigInteger.size() < rhs._bigInteger.size()) {
            _isPositive = !_isPositive;
        }
    } else {
        if (!rhs._isPositive && _bigInteger.size() < rhs._bigInteger.size()) {
            _isPositive = !_isPositive;
        }
    }
    return *this;
}

BigInteger& BigInteger::operator*=(const BigInteger& rhs) {
    bool copy_isPositive = _isPositive;
    BigInteger answer_rhs;
    answer_rhs._bigInteger.resize(_bigInteger.size() + rhs._bigInteger.size());
    for (size_t i = 0; i < _bigInteger.size(); ++i) {
        int safe_rank = 0;

    	for (size_t j = 0; j < rhs._bigInteger.size() || safe_rank; ++j) {
            long long rhs_to_add = (j == rhs._bigInteger.size() ? 0 : rhs._bigInteger[j]);
            long long prev = answer_rhs._bigInteger[i + j];
            answer_rhs._bigInteger[i + j] = (prev + _bigInteger[i] * rhs_to_add + safe_rank) % BASE;
		    safe_rank = (prev + _bigInteger[i] * rhs_to_add + safe_rank) / BASE;
        }

	}
    answer_rhs.normalize();
    *this = answer_rhs;
    _isPositive = !(copy_isPositive ^ rhs._isPositive); 
    normalize();
    return *this; 
}

BigInteger& BigInteger::operator/=(const BigInteger& rhs) {
    if (rhs == *this) {
        *this = 1;
        return *this;
    }

    BigInteger copy = *this; 
    BigInteger intermediate_answer;
    BigInteger bin_search;
    intermediate_answer._bigInteger.clear();
    _bigInteger.clear();

    while (copy._bigInteger.size()) {
        int new_rank = copy.shift();

        intermediate_answer.reverse();
        intermediate_answer._bigInteger.push_back(new_rank);
        intermediate_answer.reverse();
        intermediate_answer._isPositive = true;
        intermediate_answer.normalize();

        int right = BASE;
        int left = 0;
        int x = 0;

        while (right > left) {
            int mid = (left + right) / 2;
            bin_search = rhs;
            bin_search *= mid;
            if (!bin_search.is_bigger_abs(intermediate_answer)) {
                x = mid;
                left = mid + 1;
            } else if (bin_search.is_bigger_abs(intermediate_answer)) {
                right = mid;
            } else {
                x = mid;
                break;
            }
        }

        bin_search = rhs;
        bin_search *= x;
        bin_search._isPositive = true;
        intermediate_answer -= bin_search;
        _bigInteger.push_back(x);
    }

    _isPositive = !(_isPositive ^ rhs._isPositive);
    reverse();
    normalize();
    return *this;
}
    
BigInteger& BigInteger::operator%=(const BigInteger& rhs) {
    bool isPositive_copy = _isPositive;
	BigInteger copy = (*this);
    copy /= rhs;
    copy *= rhs;
    *this -= copy;
    _isPositive = !(isPositive_copy ^ rhs._isPositive);
    normalize();
    return *this; 
}



BigInteger& BigInteger::operator-() {
    _isPositive = !_isPositive;
    normalize();
    return *this;
}

BigInteger& BigInteger::operator++() {
    (*this) += 1;
    normalize();
    return *this;
}


BigInteger& BigInteger::operator--() {
    (*this) -= 1;
    normalize();
    return *this;
}

BigInteger BigInteger::operator++(int) {
    BigInteger copy = *this;
    (*this) += 1;
    normalize();
    return copy;
}


BigInteger BigInteger::operator--(int) {
    BigInteger copy = *this;
    (*this) -= 1;
    normalize();
    return copy;
}

bool BigInteger::operator==(const BigInteger& rhs) const {
    if (_bigInteger.size() == rhs._bigInteger.size() && _isPositive == rhs._isPositive) {
        for (size_t i = 0; i < _bigInteger.size(); ++i) {
            if (_bigInteger[i] != rhs._bigInteger[i]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool BigInteger::operator!=(const BigInteger& rhs) const {
    return !((*this) == rhs);
}

bool BigInteger::operator>(const BigInteger& rhs) const {
    if (_isPositive ^ rhs._isPositive) {
        return _isPositive;
    } else {
        if (_bigInteger.size() > rhs._bigInteger.size()) {
            return true; 
        } else if (_bigInteger.size() < rhs._bigInteger.size()) {
            return false;
        } else {

            for (long long i = _bigInteger.size() - 1; i >= 0; --i) {
                if (_bigInteger[i] > rhs._bigInteger[i]) {
                    return _isPositive;
                } else if (_bigInteger[i] < rhs._bigInteger[i]) {
                    return !_isPositive;
                }
            }

            return false;
        }
    }
}

bool BigInteger::operator<(const BigInteger& rhs) const {
    return rhs > (*this);
}

bool BigInteger::operator>=(const BigInteger& rhs) const {
    return ((*this) > rhs || (*this) == rhs);
}

bool BigInteger::operator<=(const BigInteger& rhs) const {
    return ((*this) < rhs || (*this) == rhs);
}



void BigInteger::sum_similar_sign(const BigInteger& rhs) {
    int safe_rank = 0;
    for (size_t i = 0; i < std::max(_bigInteger.size(), rhs._bigInteger.size()) || safe_rank; ++i) {
        if (i == _bigInteger.size()) {
            _bigInteger.push_back(0);
        }	
        _bigInteger[i] += safe_rank + (i < rhs._bigInteger.size() ? rhs._bigInteger[i] : 0);

        if (_bigInteger[i] >= BASE) {
            _bigInteger[i] -= BASE;
            safe_rank = 1;
        } else {
            safe_rank = 0;
        }

    }
    normalize();
}

void BigInteger::sum_diff_sign(const BigInteger& rhs) {
    int safe_rank = 0;
    for (size_t i = 0; i < rhs._bigInteger.size() || safe_rank; ++i) {
        _bigInteger[i] -= safe_rank + (i < rhs._bigInteger.size() ? rhs._bigInteger[i] : 0);

        if (_bigInteger[i] < 0) {
            _bigInteger[i] += BASE;
            safe_rank = 1;
        } else {
            safe_rank = 0;
        }

    }
    normalize();
}

void BigInteger::reverse() {
    for (size_t i = 0; i < (_bigInteger.size() / 2); ++i) {
        std::swap(_bigInteger[i], _bigInteger[_bigInteger.size() - i - 1]);
    }
}

void BigInteger::normalize() {
    while (_bigInteger.size() > 1 && _bigInteger.back() == 0) {
	    _bigInteger.pop_back();
    }
    if (_bigInteger.size() == 1 && _bigInteger[0] == 0) _isPositive = true;
}

int BigInteger::shift() {
    int copy = _bigInteger.back();
    _bigInteger.pop_back();
    return copy;
}

bool BigInteger::is_bigger_abs(const BigInteger& number) {
    if (_bigInteger.size() > number._bigInteger.size()) {
        return true; 
    } 
    if (_bigInteger.size() < number._bigInteger.size()) {
        return false;
    } 
    for (long long i = _bigInteger.size() - 1; i >= 0; --i) {
        if (_bigInteger[i] > number._bigInteger[i]) {
            return true;
        } else if (_bigInteger[i] < number._bigInteger[i]) {
            return false;
        }
    }
    return false;  
}



BigInteger::operator int() {
    int answer_number = 0;
    for (long long i = _bigInteger.size() - 1; i >= 0; --i) {
        answer_number = answer_number * BASE + _bigInteger[i];
    }

    if (!_isPositive) {
        answer_number = -answer_number;
    }
    return answer_number;
}

BigInteger::operator bool() {
    if (_bigInteger.size() == 1 && _bigInteger[0] == 0) {
        return false;
    }
    return true;
}



BigInteger operator+(BigInteger lhs, const BigInteger& rhs) {
    lhs += rhs;
    return lhs;
}

BigInteger operator-(BigInteger lhs, const BigInteger& rhs) {
    lhs -= rhs;
    return lhs;
}

BigInteger operator*(BigInteger lhs, const BigInteger& rhs) {
    lhs *= rhs;
    return lhs;
}

BigInteger operator/(BigInteger lhs, const BigInteger& rhs) {
    lhs /= rhs;
    return lhs;
}

BigInteger operator%(BigInteger lhs, const BigInteger& rhs) {
    lhs %= rhs;
    return lhs;
}

std::istream& operator>>(std::istream& in, BigInteger& number) {
    std::string string;
    while (isspace(in.peek())) in.get();
    for (char c; !isspace(in.peek()) && in >> c;){
        string += c;
    }
    number = BigInteger(string);
    return in;
}

std::ostream& operator<<(std::ostream& out, const BigInteger& number) {
    out << number.toString();
    return out;
}



BigInteger gcd(const BigInteger&, const BigInteger&);


class Rational {
public:
    Rational(int);
    Rational(const BigInteger&);
    ~Rational() = default;
    
    std::string toString();
    std::string asDecimal(size_t);

    Rational& operator=(const Rational&);
    Rational& operator+=(const Rational&);
    Rational& operator-=(const Rational&);
    Rational& operator*=(const Rational&);
    Rational& operator/=(const Rational&);
    Rational& operator-();
    bool operator==(const Rational&) const;
    bool operator!=(const Rational&) const;
    bool operator<=(const Rational&) const;
    bool operator>=(const Rational&) const;
    bool operator>(const Rational&) const;
    bool operator<(const Rational&) const;

    explicit operator double();

private:
    void fraction_reduction();

    BigInteger _numerator;
    BigInteger _denominator;
    static const int BASE = 10;
};



Rational::Rational(int num = 0) {
    _numerator = num;
    _denominator = 1;
}

Rational::Rational(const BigInteger& number) {
    _numerator = number;
    _denominator = 1;
}



std::string Rational::toString() {
    fraction_reduction();
    std::string string;
    string += _numerator.toString();
    if (_denominator != 1) {
        string += "/";
        string += _denominator.toString();
    }
    return string;
}

std::string Rational::asDecimal(size_t precision = 0) { 
    fraction_reduction();
    BigInteger integer_part = _numerator / _denominator;
    BigInteger fractional_part = _numerator % _denominator;
    if (fractional_part < 0) {
        fractional_part = -fractional_part;
    }

    BigInteger degree_10 = 1;
    for (size_t i = 0; i < precision; ++i) {
        degree_10 *= BASE;
    }
    fractional_part *= degree_10;
    fractional_part /= _denominator;

    std::string frac_string = fractional_part.toString();
    std::string answer_string;
    if (_numerator < 0 && integer_part == 0) {
        answer_string += "-";
    }
    answer_string += integer_part.toString();

    if (precision) {
        answer_string += ".";
        for (size_t i = 0; i < precision - frac_string.size(); ++i) {
            answer_string += "0";
        }
        answer_string += frac_string; 
    }

    return answer_string;
}



Rational& Rational::operator=(const Rational& rhs) {
    if (rhs == *this) {
        return *this;
    }   

    Rational copy = rhs;
    std::swap(_numerator, copy._numerator);
    std::swap(_denominator, copy._denominator);
    return *this;
}

Rational& Rational::operator+=(const Rational& rhs) {
    if (rhs == *this) {
        return (*this *= 2);
    }

    _numerator = _numerator * rhs._denominator + _denominator * rhs._numerator;
    _denominator *= rhs._denominator;
    return *this;
}

Rational& Rational::operator-=(const Rational& rhs) {    
    if (rhs == *this) {
        *this = 0;
        return *this;
    }

    Rational copy = rhs;
    *this += (-copy);
    return *this;
}

Rational& Rational::operator*=(const Rational& rhs) {
    _numerator *= rhs._numerator;
    _denominator *= rhs._denominator;
    return *this; 
}

Rational& Rational::operator/=(const Rational& rhs) {
    if (rhs == *this) {
        *this = 1;
        return *this;
    }

    _numerator *= rhs._denominator;
    _denominator *= rhs._numerator;
    if (_denominator < 0) {
        _numerator = -_numerator;
        _denominator = -_denominator;
    }
    return *this; 
}

Rational& Rational::operator-() {
    _numerator = -_numerator;
    return *this;
}

bool Rational::operator==(const Rational& rhs) const {
    if (_numerator * rhs._denominator == _denominator * rhs._numerator) {
        return true;
    }
    return false;
}

bool Rational::operator!=(const Rational& rhs) const {
    return !((*this) == rhs);
}

bool Rational::operator>(const Rational& rhs) const {
    return (_numerator * rhs._denominator > _denominator * rhs._numerator);
}

bool Rational::operator<(const Rational& rhs) const {
    return rhs > (*this);
}

bool Rational::operator>=(const Rational& rhs) const {
    return ((*this) > rhs || (*this) == rhs);
}

bool Rational::operator<=(const Rational& rhs) const {
    return ((*this) < rhs || (*this) == rhs);
}



Rational::operator double() {
    return std::stod(asDecimal(200));
}



void Rational::fraction_reduction() {
    BigInteger divider = gcd(_numerator, _denominator);
    _numerator /= divider;
    _denominator /= divider;

    if (_denominator < 0) {
        _denominator = -_denominator;
        _numerator = -_numerator;
    }
}


BigInteger gcd(const BigInteger& lhs, const BigInteger& rhs) {
    if (rhs == 0) return lhs;
    return gcd(rhs, lhs % rhs);
}


Rational operator+(Rational lhs, const Rational& rhs) {
    lhs += rhs;
    return lhs;
}

Rational operator-(Rational lhs, const Rational& rhs) {
    lhs -= rhs;
    return lhs;
}

Rational operator*(Rational lhs, const Rational& rhs) {
    lhs *= rhs;
    return lhs;
}

Rational operator/(Rational lhs, const Rational& rhs) {
    lhs /= rhs;
    return lhs;
}

