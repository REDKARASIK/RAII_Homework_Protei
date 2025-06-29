#pragma once
#include <iostream>
#include <fstream>
#include <type_traits>
#include <utility>

struct DefaultFileDeleter {
    template<typename T>
    constexpr void operator()(T& file) const noexcept {
        if (file.is_open()) {
            file.close();
        }
    }
};

template<typename T>
class is_writable {
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U&>() << std::declval<const std::string&>(), std::true_type{}
    );
    template<typename>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
class is_readable {
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U&>() >> std::declval<std::string&>(), 
        std::true_type{}
    );

    template<typename>
    static std::false_type test(...);
public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
class is_openable {
    template<typename U>
    static auto test(int) -> decltype(
        std::declval<U&>().open(std::declval<const char*>()),
        std::declval<U&>().close(),
        static_cast<bool>(std::declval<U&>().is_open()),
        std::true_type{}
    );

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

template<typename T>
struct is_file{
    static constexpr bool value = is_openable<T>::value && (is_readable<T>::value || is_writable<T>::value);
};


template<typename Fstream, typename Deleter = DefaultFileDeleter>
class SmartFile final {
    static_assert(is_file<Fstream>::value, "Fstream must be a valid file stream type");
public:    
    template<typename S = Fstream>
    explicit SmartFile(S&& file) noexcept: 
    file_(std::move(file))
    {}

    SmartFile(const SmartFile& other) = delete;
    SmartFile& operator=(const SmartFile& other) = delete;

    SmartFile(SmartFile&& other) noexcept :
    file_(std::move(other.file_)),
    deleter_(std::move(other.deleter_))
    {}

    SmartFile& operator=(SmartFile&& other) noexcept {
        file_ = std::move(other.file_);
        deleter_ = std::move(other.deleter_);
        return *this;
    }

    ~SmartFile() noexcept {
        deleter_(file_);
    }

    template<typename S = Fstream, typename = typename std::enable_if<is_writable<S>::value>::type>
    void writeLine(const std::string& line) {
        file_ << line << '\n';
        if (!file_) {
            throw std::runtime_error("Write error to file");
        }
    }

    template<typename S = Fstream, typename = typename std::enable_if<is_readable<S>::value>::type>
    std::string readLine() {
        std::string line;
        if (!std::getline(file_, line)) {
            throw std::runtime_error("Read error from file");
        }
        return line;
    }

    void close() noexcept {
        if (file_.is_open()) {
            file_.close();
        }
    }

    bool is_open() const noexcept {
        return file_.is_open();
    }
private:
    Fstream file_;
    Deleter deleter_;
};
template<typename Fstream, typename Deleter = DefaultFileDeleter>
SmartFile<Fstream, Deleter> make_smart_file(const std::string& filename, std::ios::openmode mode = std::ios::in | std::ios::out | std::ios::app) {
    Fstream file(filename.c_str(), mode);
    if (!file.is_open()){ 
        throw std::runtime_error("Failed to open file: " + filename);
    }
    return SmartFile<Fstream, Deleter>(std::move(file));
}

template<typename Fstream, typename Deleter = DefaultFileDeleter>
SmartFile<Fstream, Deleter> make_smart_file(Fstream&& file) {
    return SmartFile<Fstream, Deleter>(std::move(file));
}