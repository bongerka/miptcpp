#include <iostream>
#include <vector>
#include <type_traits>


template <typename T>
class Deque {
   private:
    template <bool IsConst>
    struct common_iterator;

   public:
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    Deque();
    explicit Deque(int count);
    Deque(int count, const T& value);
    Deque(const Deque& deque);
    ~Deque();
    Deque& operator=(const Deque& deque);
    T& operator[](size_t index);
    const T& operator[](size_t index) const;
    T& at(size_t index);
    void push_back(const T& value);
    void push_front(const T& value);
    void pop_back();
    void pop_front();
    void insert(iterator it, const T& value);
    void erase(iterator it);

    size_t size() const { return _deque_size; }

    const_iterator cbegin() const {
        return const_iterator(_first_index, array.begin() + _first_bucket);
    }

    const_iterator cend() const {
        return const_iterator(_last_index + 1, array.begin() + _last_bucket);
    }

    iterator begin() {
        return iterator(_first_index, array.begin() + _first_bucket);
    }

    iterator end() {
        return iterator(_last_index + 1, array.begin() + _last_bucket);
    }

    const_iterator begin() const {
        return const_iterator(_first_index, array.begin() + _first_bucket);
    }

    const_iterator end() const {
        return const_iterator(_last_index + 1, array.begin() + _last_bucket);
    }

    reverse_iterator rbegin() { return reverse_iterator(end()); }

    reverse_iterator rend() { return reverse_iterator(begin()); }

    const_reverse_iterator rcbegin() const {
        return const_reverse_iterator(cend());
    }

    const_reverse_iterator rcend() const {
        return const_reverse_iterator(cbegin());
    }

   private:
    template <bool IsConst>
    struct common_iterator {
       public:
        common_iterator(
            size_t index,
            std::conditional_t<IsConst, typename std::vector<T*>::const_iterator,
                             typename std::vector<T*>::iterator>
                bucket);

        std::conditional_t<IsConst, const T&, T&> operator*() {
            return *(*(_bucket) + _index);
        }

        std::conditional_t<IsConst, const T*, T*> operator->() {
            return *_bucket + _index;
        }

        common_iterator<IsConst> operator++(int) &;
        common_iterator<IsConst>& operator++() &;
        common_iterator<IsConst> operator--(int) &;
        common_iterator<IsConst>& operator--() &;
        common_iterator<IsConst>& operator+=(size_t n);
        common_iterator<IsConst> operator+(size_t n);
        common_iterator<IsConst> operator-(size_t n);

        size_t operator-(const common_iterator<IsConst>& rhs) {
            return (_bucket - rhs._bucket) * BUCKET_LENGTH + _index -
                   rhs._index;
        }

        bool operator==(const common_iterator<IsConst>& rhs) {
            return _index == rhs._index && _bucket == rhs._bucket;
        }

        bool operator!=(const common_iterator<IsConst>& rhs) {
            return !(_index == rhs._index && _bucket == rhs._bucket);
        }

        bool operator>(const common_iterator<IsConst>& rhs);
        bool operator<(const common_iterator<IsConst>& rhs);

        bool operator<=(const common_iterator<IsConst>& rhs) {
            return (*this < rhs) || (*this == rhs);
        }

        bool operator>=(const common_iterator<IsConst>& rhs) {
            return (*this > rhs) || (*this == rhs);
        }

       private:
        size_t _index;
        std::conditional_t<IsConst, typename std::vector<T*>::const_iterator,
                         typename std::vector<T*>::iterator>
            _bucket;
    };

    static const int BUCKET_LENGTH = 16;
    size_t _deque_size = 0;
    size_t _capacity = BUCKET_LENGTH;
    std::vector<T*> array;

    size_t _first_bucket = 0;
    size_t _last_bucket = 0;
    size_t _first_index = 0;
    size_t _last_index = BUCKET_LENGTH - 1;

    void _grow_buffer();
};

template <typename T>
Deque<T>::Deque() {
    T* new_sub_array =
        reinterpret_cast<T*>(new uint8_t[BUCKET_LENGTH * sizeof(T)]);
    array.push_back(new_sub_array);
}

template <typename T>
Deque<T>::Deque(int count) : Deque() {
    for (size_t i = 0; i < static_cast<size_t>(count); ++i) {
        push_back(T());
    }
}

template <typename T>
Deque<T>::Deque(int count, const T& value) : Deque() {
    for (size_t i = 0; i < static_cast<size_t>(count); ++i) {
        push_back(value);
    }
}

template <typename T>
Deque<T>::Deque(const Deque& deque) {
    *this = deque;
}

template <typename T>
Deque<T>::~Deque() {
    for (size_t i = 0; i < _deque_size; ++i) {
        pop_back();
    }
    for (size_t i = 0; i < array.size(); ++i) {
        delete[] reinterpret_cast<uint8_t*>(array[i]);
    }
}

template <typename T>
Deque<T>& Deque<T>::operator=(const Deque& deque) {
    if (array == deque.array) return *this;

    size_t cur_size = _deque_size;
    for (size_t i = 0; i < cur_size; ++i) {
        pop_back();
    }
    for (size_t i = 0; i < array.size(); ++i) {
        delete[] reinterpret_cast<uint8_t*>(array[i]);
    }
    array.clear();

    _deque_size = deque._deque_size;
    array.resize(deque.array.size());
    _capacity = deque._capacity;
    _first_index = deque._first_index;
    _first_bucket = deque._first_bucket;
    _last_index = deque._last_index;
    _last_bucket = deque._last_bucket;

    for (size_t i = 0; i < array.size(); ++i) {
        T* new_bucket =
            reinterpret_cast<T*>(new uint8_t[BUCKET_LENGTH * sizeof(T)]);
        array[i] = new_bucket;
    }

    for (size_t i = _first_index; i < BUCKET_LENGTH; ++i) {
        try {
            new (array[_first_bucket] + i) T(deque.array[_first_bucket][i]);
        } catch (...) {
            for (size_t j = 0; j < i; ++j) {
                (array[_first_bucket + i])->~T();
            }
            throw;
        }
    }
    for (size_t add_element = 0; add_element <= _last_index; ++add_element) {
        try {
            new (array[_last_bucket]+add_element)
									T(deque.array[_last_bucket][add_element]);
        } catch (...) {
            for (size_t del_element = 0;
            		del_element < add_element; ++del_element) {
                (array[_last_bucket + del_element])->~T();
            }
            throw;
        }
    }
    if (_last_bucket) {
        for (size_t bucket = _first_bucket + 1; bucket <= _last_bucket - 1;
             ++bucket) {
            for (size_t add_element = 0;
            		add_element < BUCKET_LENGTH; ++add_element) {
                try {
                    new (array[bucket]+add_element)
                    	T(deque.array[bucket][add_element]);
                } catch (...) {
                    for (size_t del_bucket = _first_bucket + 1;
                    		del_bucket < bucket; ++del_bucket) {
                        for (size_t del_element = 0;
                        		del_element < BUCKET_LENGTH; ++del_element) {
                            (array[del_bucket]+del_element)->~T();
                        }
                    }
                    for (size_t del_element = 0;
                    		del_element < add_element; ++del_element) {
                        (array[bucket]+del_element)->~T();
                    }
                    for (size_t i = 0; i < array.size(); ++i) {
                        delete[] reinterpret_cast<uint8_t*>(array[i]);
                    }
                    throw;
                }
            }
        }
    }
    return *this;
}

template <typename T>
T& Deque<T>::operator[](size_t index) {
    size_t new_index = _first_index + index;
    size_t new_bucket = (_first_bucket + new_index / BUCKET_LENGTH);
    return array[new_bucket][new_index % BUCKET_LENGTH];
}

template <typename T>
const T& Deque<T>::operator[](size_t index) const {
    size_t new_index = _first_index + index;
    size_t new_bucket = (_first_bucket + new_index / BUCKET_LENGTH);
    return array[new_bucket][new_index % BUCKET_LENGTH];
}

template <typename T>
T& Deque<T>::at(size_t index) {
    size_t new_index = _first_index + index;
    if (index < 0 || new_index >= _deque_size) {
        throw std::out_of_range("out of range");
    }
    return (*this)[index];
}

template <typename T>
void Deque<T>::push_back(const T& value) {
    if (_last_index == BUCKET_LENGTH - 1) {
        if (_last_bucket == array.size() - 1) {
            _grow_buffer();
        }
        _last_index = 0;
        if (_deque_size) {
            ++_last_bucket;
        }
    } else {
        ++_last_index;
    }
    new (array[_last_bucket] + _last_index) T(value);
    ++_deque_size;
}

template <typename T>
void Deque<T>::push_front(const T& value) {
    if (_first_index == 0) {
        if (_first_bucket == 0) {
            _grow_buffer();
        }
        _first_index = BUCKET_LENGTH - 1;
        if (_deque_size) {
            --_first_bucket;
        }
    } else {
        --_first_index;
    }
    new (array[_first_bucket] + _first_index) T(value);
    ++_deque_size;
}

template <typename T>
void Deque<T>::pop_back() {
    (array[_last_bucket] + _last_index)->~T();
    if (_last_index == 0) {
        _last_index = BUCKET_LENGTH - 1;
        --_last_bucket;
    } else {
        --_last_index;
    }
    --_deque_size;
    if (_deque_size < 0) throw;
}

template <typename T>
void Deque<T>::pop_front() {
    (array[_first_bucket] + _first_index)->~T();
    if ((++_first_index) == BUCKET_LENGTH) {
        _first_index = 0;
        ++_first_bucket;
    }
    --_deque_size;
    if (_deque_size < 0) throw;
}

template <typename T>
void Deque<T>::insert(iterator it, const T& value) {
    push_back((*this)[_deque_size - 1]);
    for (iterator i = end() - 2; i > it; --i) {
        *i = *(i - 1);
    }
    *it = value;
}

template <typename T>
void Deque<T>::erase(iterator it) {
    for (iterator i = it; i < end() - 1; ++i) {
        *i = *(i + 1);
    }
    pop_back();
}

template <typename T>
void Deque<T>::_grow_buffer() {
    std::vector<T*> new_array(array.size() * 2);
    size_t index = array.size() / 2;
    for (size_t i = 0; i < index; ++i) {
        new_array[i] =
            reinterpret_cast<T*>(new uint8_t[BUCKET_LENGTH * sizeof(T)]);
    }
    for (size_t i = index + array.size(); i < new_array.size(); ++i) {
        new_array[i] =
            reinterpret_cast<T*>(new uint8_t[BUCKET_LENGTH * sizeof(T)]);
    }
    for (size_t i = index; i < index + array.size(); ++i) {
        new_array[i] = array[i - index];
    }
    _first_bucket += array.size() / 2;
    _last_bucket += array.size() / 2;
    array = new_array;
    _capacity *= 2;
}

template <typename T>
template <bool IsConst>
Deque<T>::template common_iterator<IsConst>::common_iterator(
    size_t index,
    std::conditional_t<IsConst, typename std::vector<T*>::const_iterator,
                     typename std::vector<T*>::iterator>
        bucket)
    : _index(index), _bucket(bucket) {}

template <typename T>
template <bool IsConst>
typename Deque<T>::template common_iterator<IsConst>
    Deque<T>::common_iterator<IsConst>::operator++(int) & {
    common_iterator<IsConst> it(_index, _bucket);
    ++_index;
    _bucket += _index / BUCKET_LENGTH;
    _index %= BUCKET_LENGTH;
    return it;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template common_iterator<IsConst>&
    Deque<T>::common_iterator<IsConst>::operator++() & {
    ++_index;
    _bucket += _index / BUCKET_LENGTH;
    _index %= BUCKET_LENGTH;
    return *this;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template common_iterator<IsConst>
    Deque<T>::common_iterator<IsConst>::operator--(int) & {
    common_iterator<IsConst> it(_index, _bucket);
    if (_index == 0) {
        _index = BUCKET_LENGTH - 1;
        --_bucket;
    } else {
        --_index;
    }
    return it;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template common_iterator<IsConst>&
    Deque<T>::common_iterator<IsConst>::operator--() & {
    if (_index == 0) {
        _index = BUCKET_LENGTH - 1;
        --_bucket;
    } else {
        --_index;
    }
    return *this;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template common_iterator<IsConst>&
    Deque<T>::common_iterator<IsConst>::operator+=(size_t n) {
    _index += n;
    _bucket += _index / BUCKET_LENGTH;
    _index %= BUCKET_LENGTH;
    return *this;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template common_iterator<IsConst>
    Deque<T>::common_iterator<IsConst>::operator+(size_t n) {
    common_iterator<IsConst> it(_index, _bucket);
    it += n;
    return it;
}

template <typename T>
template <bool IsConst>
typename Deque<T>::template common_iterator<IsConst>
    Deque<T>::common_iterator<IsConst>::operator-(size_t n) {
    common_iterator<IsConst> it(_index, _bucket);
    long long diff = _index - n;
    if (diff >= 0) {
        it._index = diff;
        return it;
    }

    diff *= -1;
    it._bucket -= diff / BUCKET_LENGTH + 1;
    it._index = BUCKET_LENGTH - (diff % BUCKET_LENGTH);
    return it;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::common_iterator<IsConst>::operator>(
    const common_iterator<IsConst>& rhs) {
    if (_bucket > rhs._bucket) return true;
    if (_bucket == rhs._bucket && _index > rhs._index) return true;
    return false;
}

template <typename T>
template <bool IsConst>
bool Deque<T>::common_iterator<IsConst>::operator<(
    const common_iterator<IsConst>& rhs) {
    if (_bucket < rhs._bucket) return true;
    if (_bucket == rhs._bucket && _index < rhs._index) return true;
    return false;
}
