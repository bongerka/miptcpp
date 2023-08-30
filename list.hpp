#include <iostream>

template <size_t N>
class StackStorage {
public:
    StackStorage(): _memory_pointer(_memory) {};
    StackStorage(const StackStorage&) = delete;
    ~StackStorage() = default;
    uint8_t* get_pointer(size_t count_objects, size_t align_size);
private:
    uint8_t _memory[N];
    uint8_t* _memory_pointer;
};

template <size_t N>
uint8_t* StackStorage<N>::get_pointer(size_t count_objects, size_t align_size) {
    uint8_t* ptr = _memory_pointer;
    uintptr_t int_ptr = reinterpret_cast<uintptr_t>(_memory_pointer);
    if (int_ptr % align_size != 0) {
        ptr += align_size - (int_ptr % align_size);
        _memory_pointer += align_size - (int_ptr % align_size);
    }
    _memory_pointer += count_objects;
    return ptr;
}


template <typename T, size_t N>
class StackAllocator {
public:
    using value_type = T;

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

    StackAllocator() = default;
    explicit StackAllocator(StackStorage<N>& storage): _memory(&storage) {};
    ~StackAllocator() = default;

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& other)
            : _memory(other.get_memory()){};

    template <typename U>
    StackAllocator<T, N>& operator=(const StackAllocator<U, N>& other);
    T* allocate(size_t count_objects) noexcept;
    void deallocate(T*, size_t) noexcept {};
    StackStorage<N>* get_memory() const;

private:
    StackStorage<N>* _memory;
};


template <typename T, size_t N>
template <typename U>
StackAllocator<T, N>& StackAllocator<T, N>::
operator=(const StackAllocator<U, N>& other) {
    _memory = other.get_memory();
    return *this;
}

template <typename T, size_t N>
T* StackAllocator<T, N>::allocate(size_t count_objects) noexcept {
    return reinterpret_cast<T*>(
            _memory->get_pointer(count_objects * sizeof(T), alignof(T)));
}

template <typename T, size_t N>
StackStorage<N>* StackAllocator<T, N>::get_memory() const {
    return _memory;
}

template <typename T, size_t N, typename U, size_t M>
bool operator==(const StackAllocator<T, N>& lhs
        , const StackAllocator<U, M>& rhs) {
    return lhs.get_memory() == rhs.get_memory();
}

template <typename T, size_t N, typename U, size_t M>
bool operator!=(const StackAllocator<T, N>& lhs
        , const StackAllocator<U, M>& rhs) {
    return lhs.get_memory() != rhs.get_memory();
}


template <bool B, typename T, typename U>
struct Conditional;

template <typename T, typename U>
struct Conditional<false, T, U> {
    using type = U;
};

template <typename T, typename U>
struct Conditional<true, T, U> {
    using type = T;
};

template <bool B, typename T, typename U>
using ConditionalValue = typename Conditional<B, T, U>::type;


template <typename T, typename Alloc = std::allocator<T>>
class List {
private:
    template <bool IsConst>
    struct common_iterator;

    struct BaseNode {
        BaseNode* prev;
        BaseNode* next;
    };

    struct Node: BaseNode {
        Node() = default;
        explicit Node(const T& val): value(val) {}
        T value;
    };

public:
    using value_type = T;
    using allocator_type = typename Alloc::template rebind<Node>::other;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = const value_type&;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    List();
    explicit List(size_t count);
    List(size_t count, const T& value);
    explicit List(const Alloc& allocator);
		List(size_t count, const Alloc& allocator);
    List(size_t count, const T& value, const Alloc& allocator);
    List(const List<T, Alloc>& other);
    ~List();
    List<T, Alloc>& operator=(const List<T, Alloc>& other);

    allocator_type get_allocator() const {
        return _alloc;
    }

    bool empty() const {
		return !(_list_size);
    }

    size_t size() const {
        return _list_size;
    }

    void push_back(const T& value) {
        insert(end(), value);
    }

    void push_front(const T& value) {
        insert(begin(), value);
    }

    void pop_back() {
        erase(--end());
    }

    void pop_front() {
        erase(begin());
    }

    void insert(iterator it, const T& value);
    void insert(const_iterator it, const T& value);
    void insert(iterator it);
    void erase(iterator it);
    void erase(const_iterator it);

    iterator end() {
        return iterator(_fake_node);
    }

    iterator begin() {
        return iterator(_fake_node->next);
    }

    const_iterator end() const {
        return const_iterator(_fake_node);
    }

    const_iterator begin() const {
        return const_iterator(_fake_node->next);
    }

    const_iterator cend() const {
        return const_iterator(_fake_node);
    }

    const_iterator cbegin() const {
        return const_iterator(_fake_node->next);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator(begin());
    }

private:
    template <bool IsConst>
    struct common_iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = ConditionalValue<IsConst, const T, T>;
        using pointer = ConditionalValue<IsConst, const T*, T*>;
        using reference = ConditionalValue<IsConst, const T&, T&>;
        using iterator_category = std::bidirectional_iterator_tag;

        explicit common_iterator(BaseNode* node)
            : _node(node) {};

		common_iterator(const common_iterator<false>& other)
            : _node(other.get_node()) {}

        common_iterator& operator=(const common_iterator<false>& other) {
            _node = other.get_node();
            return *this;
        }

        ConditionalValue<IsConst, const T&, T&> operator*() {
            return static_cast<Node*>(_node)->value;
        }

        ConditionalValue<IsConst, const T*, T*> operator->() {
            return &(static_cast<Node*>(_node)->value);
        }

        common_iterator<IsConst> operator++(int) & {
            common_iterator<IsConst> it(_node);
            _node = _node->next;
            return it;
        }

        common_iterator<IsConst>& operator++() {
            _node = _node->next;
            return *this;
        }

        common_iterator<IsConst> operator--(int) & {
            common_iterator<IsConst> it(_node);
            _node = _node->prev;
            return it;
        }

        common_iterator<IsConst>& operator--() {
            _node = _node->prev;
            return *this;
        }

        common_iterator<IsConst>& operator+=(size_t n) {
            for (size_t i = 0; i < n; ++i) {
                _node = _node->next;
            }
            return *this;
        }

        common_iterator<IsConst>& operator-=(size_t n) {
            for (size_t i = 0; i < n; ++i) {
                _node = _node->prev;
            }
            return *this;
        }

        common_iterator<IsConst> operator+(size_t n) {
            common_iterator<IsConst> it(_node);
            it += n;
            return it;
        }

        common_iterator<IsConst> operator-(size_t n) {
            common_iterator<IsConst> it(_node);
            it -= n;
            return it;
        }

        bool operator==(const common_iterator<IsConst>& rhs) {
            return _node == rhs._node;
        }

        bool operator!=(const common_iterator<IsConst>& rhs) {
            return _node != rhs._node;
        }

        BaseNode* get_node() const {
            return _node;
        }

    private:
        BaseNode* _node;
    };

    using AllocTraits = std::allocator_traits<allocator_type>;
    allocator_type  _alloc;
    BaseNode* _fake_node;
    size_t _list_size;
};


template <typename T, typename Alloc>
List<T, Alloc>::List()
        : _fake_node(AllocTraits::allocate(_alloc, 1))
        ,	_list_size(0) {
    AllocTraits::construct(_alloc, _fake_node);
    _fake_node->prev = _fake_node;
    _fake_node->next = _fake_node;
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t count): List() {
    try {
        for (size_t i = 0; i < count; ++i) {
            insert(end());
        }
    } catch (...) {
        while (!empty()) {
            erase(begin());
        }
        throw;
    }
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t count, const T& value): List() {
    try {
        for (size_t i = 0; i < count; ++i) {
            insert(end(), value);
        }
    } catch (...) {
        while (!empty()) {
            erase(begin());
        }
        AllocTraits::destroy(_alloc, _fake_node);
        AllocTraits::deallocate(_alloc, static_cast<Node*>(_fake_node), 1);
        throw;
    }
}

template <typename T, typename Alloc>
List<T, Alloc>::List(const Alloc& allocator)
        : _alloc(AllocTraits::select_on_container_copy_construction(allocator))
        , _fake_node(AllocTraits::allocate(_alloc, 1))
        ,	_list_size(0) {
    AllocTraits::construct(_alloc, _fake_node);
    _fake_node->prev = _fake_node;
    _fake_node->next = _fake_node;
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t count, const Alloc& allocator)
        : List(allocator) {
    try {
        for (size_t i = 0; i < count; ++i) {
            insert(end());
        }
    } catch (...) {
        while (!empty()) {
            erase(begin());
        }
        AllocTraits::destroy(_alloc, _fake_node);
        AllocTraits::deallocate(_alloc, static_cast<Node*>(_fake_node), 1);
        throw;
    }
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t count, const T& value, const Alloc& allocator)
        : List(allocator) {
    try {
        for (size_t i = 0; i < count; ++i) {
            insert(end(), value);
        }
    } catch (...) {
        while (!empty()) {
            erase(begin());
        }
        AllocTraits::destroy(_alloc, _fake_node);
        AllocTraits::deallocate(_alloc, static_cast<Node*>(_fake_node), 1);
        throw;
    }
}

template <typename T, typename Alloc>
List<T, Alloc>::List(const List<T, Alloc>& other)
        : List(other._alloc) {
    try {
        for (BaseNode* node = other._fake_node->next; node != other._fake_node; node = node->next) {
            insert(end(), static_cast<Node*>(node)->value);
        }
    } catch (...) {
        while (!empty()) {
            erase(begin());
        }
        throw;
    }
}

template <typename T, typename Alloc>
List<T, Alloc>& List<T, Alloc>::operator=(const List<T, Alloc>& other) {
    if (&other != this) {
        if (AllocTraits::propagate_on_container_copy_assignment::value) {
            _alloc = other._alloc;
        }
        size_t copy_size = _list_size;
        try {
            for (BaseNode* node = other._fake_node->next; node != other._fake_node; node = node->next) {
                insert(end(), static_cast<Node*>(node)->value);
            }
        } catch (...) {
            while(_list_size > copy_size) {
                erase(--end());
            }
            throw;
        }
        for(size_t i = 0; i < copy_size; ++i) {
            erase(begin());
        }
    }
    return *this;
}

template <typename T, typename Alloc>
void List<T, Alloc>::insert(iterator it, const T& value) {
    try {
        Node* new_node = AllocTraits::allocate(_alloc, 1);
        AllocTraits::construct(_alloc, new_node, value);
        new_node->prev = it.get_node()->prev;
        it.get_node()->prev->next = new_node;
        it.get_node()->prev = new_node;
        new_node->next = it.get_node();
        ++_list_size;
    } catch (...) {
        throw;
    }
}

template <typename T, typename Alloc>
void List<T, Alloc>::insert(const_iterator it, const T& value) {
    try {
        Node* new_node = AllocTraits::allocate(_alloc, 1);
        AllocTraits::construct(_alloc, new_node, value);
        new_node->prev = it.get_node()->prev;
        it.get_node()->prev->next = new_node;
        it.get_node()->prev = new_node;
        new_node->next = it.get_node();
        ++_list_size;
    } catch (...) {
        throw;
    }
}

template <typename T, typename Alloc>
void List<T, Alloc>::insert(iterator it) {
    try {
        Node* new_node = AllocTraits::allocate(_alloc, 1);
        AllocTraits::construct(_alloc, new_node);
        new_node->prev = it.get_node()->prev;
        it.get_node()->prev->next = new_node;
        it.get_node()->prev = new_node;
        new_node->next = it.get_node();
        ++_list_size;
    } catch (...) {
        throw;
    }
}

template <typename T, typename Alloc>
void List<T, Alloc>::erase(iterator it) {
    try {
        BaseNode* old_node = it.get_node();
        old_node->prev->next = old_node->next;
        old_node->next->prev = old_node->prev;
        AllocTraits::destroy(_alloc, static_cast<Node*>(old_node));
        AllocTraits::deallocate(_alloc, static_cast<Node*>(old_node), 1);
        --_list_size;
    } catch (...) {
        throw;
    }
}

template <typename T, typename Alloc>
void List<T, Alloc>::erase(const_iterator it) {
    try {
        BaseNode* old_node = it.get_node();
        old_node->prev->next = old_node->next;
        old_node->next->prev = old_node->prev;
        AllocTraits::destroy(_alloc, static_cast<Node*>(old_node));
        AllocTraits::deallocate(_alloc, static_cast<Node*>(old_node), 1);
        --_list_size;
    } catch (...) {
        throw;
    }
}

template <typename T, typename Alloc>
List<T, Alloc>::~List() {
    while(!empty()) {
        erase(begin());
    }
    AllocTraits::destroy(_alloc, _fake_node);
    AllocTraits::deallocate(_alloc, static_cast<Node*>(_fake_node), 1);
}

