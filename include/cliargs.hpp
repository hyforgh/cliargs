/**
 * github: https://github.com/hyforgh/cliargs
 */

/*
MIT License

Copyright (c) 2025-2034 Hongyun Liu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// #define CLIARGS_NO_RTTI      // Define this BEFORE including cliargs.hpp if RTTI is unavailable.
// #define CLIARGS_NO_EXCEPTION // Define this BEFORE including cliargs.hpp if exception handling is disabled.
// #define CLIARGS_NO_WARNING   // Define this BEFORE including cliargs.hpp to suppress all cliargs compiler warnings.
// #define CLIARGS_NO_REGEX     // Define this BEFORE including cliargs.hpp to disable ArgAttr::regex(...).
    // Fallback: You can use ArgAttr::examine(...) alongside a third-party regex.
    // (Useful as a workaround for known compiler bugs in older GCC versions).

#ifndef H_CLIARGS_HPP
#define H_CLIARGS_HPP

#include <limits>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <tuple>
#include <cstring>
#include <memory>
#include <functional>
#include <sstream>
#include <iomanip>
#include <iostream>
#ifndef CLIARGS_NO_REGEX
#include <regex>
#endif // CLIARGS_NO_REGEX

namespace cliargs {

constexpr unsigned CLIARGS_VERSION_MAJOR = 3;
constexpr unsigned CLIARGS_VERSION_MINOR = 0;
constexpr unsigned CLIARGS_VERSION_PATCH = 0;

#ifndef CLIARGS_NO_EXCEPTION
class bad_cast : public std::bad_cast {
public:
    explicit bad_cast(std::string what) : std::bad_cast(), _what(std::move(what)) {
    }
    const char *what() const noexcept override {
        return _what.c_str();
    }
private:
    const std::string _what;
};
#endif // CLIARGS_NO_EXCEPTION

enum class SmartMode {
    Gnu = 0,
    Eof,
    Name,
};

class ArgParser {
public:
    ArgParser(char *argv[], int argc, SmartMode smart_mode, void *context, std::string parent_name="");
    virtual ~ArgParser() {}
    virtual void domain_begin(std::string type_name, std::string member_prefix = "."
        , std::string member_suffix = "");
    void domain_end();
    template <typename T>
    bool assign(T &value, const std::string &name, T default_value = T());
    void check(bool is_true, std::string msg);
    void set_optional();
    void *get_context() const;
private:
    std::string concat_name(const std::string &name) const;
protected:
    int _argc;
    char **_argv;
    SmartMode _smart_mode;
    void *_context;
    std::string _parent_name;
    int _argi;
    int _vali;
    bool _is_optional;
    int _at_least;
    int _at_most;
    bool _is_terminated;
    bool _is_serializing;
    std::string _item_name_prefix;
    std::string _item_name_suffix;
    std::string _main_type_name;
    std::list<std::string> _err_list;
    struct ItemTraits {
        std::string type_name;
        std::string item_name;
        std::string item_value;
    };
    std::list<ItemTraits> _item_traits;
}; // ArgParser

template <typename T>
void cliargs_parse_custom(T &var, ArgParser &parser);

template <typename T>
std::string to_string(const T &data
        , const std::string &delimiter = ",", const std::string &gap = " "
        , const char *prefix = nullptr, const char *suffix = nullptr);

template <typename T>
struct type_traits {
    static const std::string &name();
    typedef std::true_type is_cli_custom;
};

#define DEFINE_TYPE_TRAITS_SCALAR(type_, name_, ...)                          \
template <> struct type_traits<type_> {                                       \
    static const std::string &name() {                                        \
        static std::string s_name = name_ __VA_ARGS__;                        \
        return s_name;                                                        \
    }                                                                         \
    typedef std::true_type is_cli_scalar;                                     \
    typedef std::true_type is_cli_primary;                                    \
};
#define DEFINE_TYPE_TRAITS_NUMERIC(type, name)                                \
    DEFINE_TYPE_TRAITS_SCALAR(type, name, + std::to_string(sizeof(type) * 8))
DEFINE_TYPE_TRAITS_SCALAR(bool, "bool")
DEFINE_TYPE_TRAITS_SCALAR(char, "char")
DEFINE_TYPE_TRAITS_NUMERIC(float, "float")
DEFINE_TYPE_TRAITS_NUMERIC(double, "float")
DEFINE_TYPE_TRAITS_NUMERIC(long long, "int")
DEFINE_TYPE_TRAITS_NUMERIC(unsigned long long, "uint")
DEFINE_TYPE_TRAITS_NUMERIC(long, "int")
DEFINE_TYPE_TRAITS_NUMERIC(unsigned long, "uint")
DEFINE_TYPE_TRAITS_NUMERIC(int, "int")
DEFINE_TYPE_TRAITS_NUMERIC(unsigned int, "uint")
DEFINE_TYPE_TRAITS_NUMERIC(short, "int")
DEFINE_TYPE_TRAITS_NUMERIC(unsigned short, "uint")
DEFINE_TYPE_TRAITS_NUMERIC(signed char, "int")
DEFINE_TYPE_TRAITS_NUMERIC(unsigned char, "uint")
#undef DEFINE_TYPE_TRAITS_NUMERIC
#undef DEFINE_TYPE_TRAITS_SCALAR

#define DEFINE_TYPE_TRAITS_STRING(type_, name_) \
template <> struct type_traits<type_> {         \
    static const std::string &name() {          \
        static std::string s_name = name_;      \
        return s_name;                          \
    }                                           \
    typedef std::true_type is_cli_string;       \
    typedef std::true_type is_cli_primary;      \
};
DEFINE_TYPE_TRAITS_STRING(char *, "char *")
DEFINE_TYPE_TRAITS_STRING(const char *, "char *")
DEFINE_TYPE_TRAITS_STRING(std::string, "string")
#undef DEFINE_TYPE_TRAITS_STRING

template <typename T>
struct type_traits<std::vector<T>> {
    static const std::string &name() {
        static const std::string s_name = std::string("vector<")
            + type_traits<T>::name() + ">";
        return s_name;
    }
};
template <typename Tkey, typename Tval>
struct type_traits<std::pair<Tkey, Tval>> {
    static const std::string &name() {
        static const std::string s_name = std::string("pair<")
            + type_traits<Tkey>::name() + ", "
            + type_traits<Tval>::name() + ">";
        return s_name;
    }
};
template <typename Tkey, typename Tval>
struct type_traits<std::map<Tkey, Tval>> {
    static const std::string &name() {
        static const std::string s_name = std::string("map<")
            + type_traits<Tkey>::name() + ", "
            + type_traits<Tval>::name() + ">";
        return s_name;
    }
};
template <typename Tkey, typename Tval>
struct type_traits<std::unordered_map<Tkey, Tval>> {
    static const std::string &name() {
        static const std::string s_name = std::string("unordered_map<")
            + type_traits<Tkey>::name() + ", "
            + type_traits<Tval>::name() + ">";
        return s_name;
    }
};

class Parser;
template <typename T>
class ArgAttr;

namespace detail {

template <typename T>
const char *parse_primary(T &, char *psz) {
    return "";
}
inline const char *parse_primary(char *&v, char *psz) {
    if (!psz) {
        return type_traits<char *>::name().c_str();
    }
    v = psz;
    return nullptr;
}
inline const char *parse_primary(const char *&v, char *psz) {
    if (!psz) {
        return type_traits<const char *>::name().c_str();
    }
    v = psz;
    return nullptr;
}
inline const char *parse_primary(std::string &v, char *psz) {
    if (!psz) {
        return type_traits<std::string>::name().c_str();
    }
    v = psz;
    return nullptr;
}
inline const char *parse_primary(bool &v, char *psz) {
    static const char *type_name = "bool{True,true,1,Yes,yes,Y,y,False,false,0,No,no,N,n}";
    if (!psz || !psz[0]) {
        return type_name;
    }
    if (strcmp(psz, "True") == 0 || strcmp(psz, "true") == 0 || strcmp(psz, "1") == 0
            || strcmp(psz, "Yes") == 0 || strcmp(psz, "yes") == 0
            || strcmp(psz, "Y") == 0  || strcmp(psz, "y") == 0
            ) {
        v = true;
    } else if (strcmp(psz, "False") == 0 || strcmp(psz, "false") == 0 || strcmp(psz, "0") == 0
            || strcmp(psz, "No") == 0 || strcmp(psz, "no") == 0
            || strcmp(psz, "N") == 0 || strcmp(psz, "n") == 0) {
        v = false;
    } else {
        return type_name;
    }
    return nullptr;
}
inline const char *parse_primary(float &v, char *psz) {
    static const char *type_name = type_traits<float>::name().c_str();
    if (!psz || !psz[0]) {
        return type_name;
    }
    char *pend = nullptr;
    v = std::strtof(psz, &pend);
    return *pend == '\0' ? nullptr : type_name;
}
inline const char *parse_primary(double &v, char *psz) {
    static const char *type_name = type_traits<double>::name().c_str();
    if (!psz || !psz[0]) {
        return type_name;
    }
    char *pend = nullptr;
    v = std::strtod(psz, &pend);
    return *pend == '\0' ? nullptr : type_name;
}
template <typename Tval, typename Tbig>
const char *parse_integer(Tval &v, char *psz, Tbig (*str2int)(const char *, char **, int)) {
    static const char *type_name = type_traits<Tval>::name().c_str();
    if (!psz || !psz[0]) {
        return type_name;
    }
    Tbig v_big = 0;
    char *pend = nullptr;
    if (strncmp(psz, "0x", 2) == 0 || strncmp(psz, "0X", 2) == 0) {
        v_big = str2int(psz, &pend, 16);
    } else if (strncmp(psz, "0b", 2) == 0 || strncmp(psz, "0B", 2) == 0) {
        v_big = str2int(psz + 2, &pend, 2);
    } else {
        v_big = str2int(psz, &pend, 10);
    }
    if (*pend == '\0') {
        static auto v_min = static_cast<Tbig>(std::numeric_limits<Tval>::min());
        static auto v_max = static_cast<Tbig>(std::numeric_limits<Tval>::max());
        if (v_big < v_min || v_big > v_max) {
            static std::string s_type_name = [&]() {
                std::stringstream ss;
                ss << type_name << "(" << v_min << " ~ " << v_max << ")";
                return ss.str();
            }();
            return s_type_name.c_str();
        }
        v = v_big;
        return nullptr;
    }
    return type_name;
}
inline const char *parse_primary(long long &v, char *psz) {
    return parse_integer(v, psz, std::strtoll);
}
inline const char *parse_primary(unsigned long long &v, char *psz) {
    return parse_integer(v, psz, std::strtoull);
}
inline const char *parse_primary(long &v, char *psz) {
    return parse_integer(v, psz, std::strtol);
}
inline const char *parse_primary(unsigned long &v, char *psz) {
    return parse_integer(v, psz, std::strtoul);
}
inline const char *parse_primary(int &v, char *psz) {
    return parse_integer(v, psz, std::strtol);
}
inline const char *parse_primary(unsigned int &v, char *psz) {
    return parse_integer(v, psz, std::strtoul);
}
inline const char *parse_primary(short &v, char *psz) {
    return parse_integer(v, psz, std::strtol);
}
inline const char *parse_primary(unsigned short &v, char *psz) {
    return parse_integer(v, psz, std::strtoul);
}
inline const char *parse_primary(signed char &v, char *psz) {
    return parse_integer(v, psz, std::strtol);
}
inline const char *parse_primary(unsigned char &v, char *psz) {
    return parse_integer(v, psz, std::strtoul);
}
inline const char *parse_primary(char &v, char *psz) {
    return parse_integer(v, psz, std::strtol);
}

inline bool is_numeric(const char *p) {
    double v;
    return parse_primary(v, const_cast<char *>(p)) == nullptr;
}

template <typename... Ts>
struct make_void_t { typedef void type; };
template <typename... Ts> using void_t = typename make_void_t<Ts...>::type;

template <typename>
struct is_stl_vector : std::false_type {};
template <typename T>
struct is_stl_vector<std::vector<T>> : std::true_type {};

template <typename>
struct is_stl_tuple : std::false_type {};
template <typename... Targs>
struct is_stl_tuple<std::tuple<Targs...>> : std::true_type {};

template <typename>
struct is_stl_map : std::false_type {};
template <typename Tkey, typename Tval>
struct is_stl_map<std::map<Tkey, Tval>> : std::true_type {};
template <typename Tkey, typename Tval>
struct is_stl_map<std::unordered_map<Tkey, Tval>> : std::true_type {};

template <typename T, typename = void>
struct is_stl_container : std::false_type {};
template <typename T>
struct is_stl_container<T, void_t<typename T::value_type, typename T::iterator>> : std::true_type {};

template <typename>
struct is_cli_container : std::false_type {};
template <typename T>
struct is_cli_container<std::vector<T>> : std::true_type {};
template <typename Tkey, typename Tval>
struct is_cli_container<std::map<Tkey, Tval>> : std::true_type {};
template <typename Tkey, typename Tval>
struct is_cli_container<std::unordered_map<Tkey, Tval>> : std::true_type {};

template <typename T, typename = void>
struct is_cli_scalar : std::false_type {};
template <typename T>
struct is_cli_scalar<T, void_t<typename type_traits<T>::is_cli_scalar>> : std::true_type {};

template <typename T, typename = void>
struct is_cli_string : std::false_type {};
template <typename T>
struct is_cli_string<T, void_t<typename type_traits<T>::is_cli_string>> : std::true_type {};

template <typename T, typename = void>
struct is_cli_custom : std::false_type {};
template <typename T>
struct is_cli_custom<T, void_t<typename type_traits<T>::is_cli_custom>> : std::true_type {};

template <typename T, typename = void>
struct is_cli_primary : std::false_type {};
template <typename T>
struct is_cli_primary<T, void_t<typename type_traits<T>::is_cli_primary>> : std::true_type {};

template <typename T, bool IS_CONTAINER> 
struct get_container_value_type;
template <typename T>
struct get_container_value_type<T, true> { typedef typename T::value_type type; };
template <typename T>
struct get_container_value_type<T, false> { typedef T type; };

template <typename T>
struct get_stl_value_type : get_container_value_type<T, is_stl_container<T>::value> {};
template <>
struct get_stl_value_type<std::string> { typedef std::string type; };

template <typename T>
struct get_cli_value_type : get_container_value_type<T, is_cli_container<T>::value> {};
template <>
struct get_cli_value_type<std::string> { typedef std::string type; };
template <typename Tkey, typename Tval>
struct get_cli_value_type<std::map<Tkey, Tval>> {
    typedef typename get_cli_value_type<Tval>::type type;
};
template <typename Tkey, typename Tval>
struct get_cli_value_type<std::unordered_map<Tkey, Tval>> {
    typedef typename get_cli_value_type<Tval>::type type;
};

template <typename T>
struct get_cli_level_type {
    typedef T top_type;
    typedef typename get_cli_value_type<top_type>::type mid_type;
    typedef typename get_cli_value_type<mid_type>::type val_type;
};
template <typename Tkey, typename Ttop>
struct get_cli_level_type<std::map<Tkey, Ttop>> : get_cli_level_type<Ttop> {};
template <typename Tkey, typename Ttop>
struct get_cli_level_type<std::unordered_map<Tkey, Ttop>> : get_cli_level_type<Ttop> {};

template <typename T, typename Enable = void>
struct to_string_t {
    static std::string from(const T &data
            , const std::string &delimiter, const std::string &gap
            , const char *prefix, const char *suffix
            ) {
        return to_string(data, delimiter, gap, prefix, suffix);
    }
};
template <typename Tval>
struct to_string_t<Tval, typename std::enable_if<is_cli_primary<Tval>::value>::type> {
    static std::string from(const Tval &value
            , const std::string &delimiter, const std::string &gap
            , const char *prefix, const char *suffix
            ) {
        std::stringstream ss;
        if (prefix) {
            ss << prefix;
        }
        if (is_cli_string<Tval>::value) {
            ss << "\"";
        }
        ss << value;
        if (is_cli_string<Tval>::value) {
            ss << "\"";
        }
        if (suffix) {
            ss << suffix;
        }
        return ss.str();
    }
};
template <typename Tval>
struct to_string_t<std::vector<Tval>> {
    static std::string from(const std::vector<Tval> &data
            , const std::string &delimiter, const std::string &gap
            , const char *prefix, const char *suffix
            ) {
        std::stringstream ss;
        ss << (prefix ? prefix : "[");
        bool is_first = true;
        for (const auto &it : data) {
            if (is_first) {
                is_first = false;
            } else {
                ss << delimiter << gap;
            }
            ss << to_string(it, delimiter, gap);
        }
        ss << (suffix ? suffix : "]");
        return ss.str();
    }
};
template <typename Tkey, typename Tval>
struct to_string_t<std::pair<Tkey, Tval>> {
    static std::string from(const std::pair<Tkey, Tval> &value
            , const std::string &delimiter, const std::string &gap
            , const char *prefix, const char *suffix
            ) {
        std::stringstream ss;
        ss << (prefix ? prefix : "(")
           << to_string(value.first, delimiter, gap)
           << delimiter << gap
           << to_string(value.second, delimiter, gap)
           << (suffix ? suffix : ")");
        return ss.str();
    }
};
template <typename T>
std::string map_to_string(const T &value
        , const std::string &delimiter, const std::string &gap
        , const char *prefix, const char *suffix
        ) {
    std::stringstream ss;
    ss << (prefix ? prefix : "{");
    bool is_first = true;
    for (auto &it : value) {
        if (is_first) {
            is_first = false;
        } else {
            ss << delimiter << gap;
        }
        ss << to_string(it.first, delimiter, gap)
           << ":" << gap
           << to_string(it.second, delimiter, gap);
    }
    ss << (suffix ? suffix : "}");
    return ss.str();
}
template <typename Tkey, typename Tval>
struct to_string_t<std::map<Tkey, Tval>> {
    static std::string from(const std::map<Tkey, Tval> &value
            , const std::string &delimiter, const std::string &gap
            , const char *prefix, const char *suffix
            ) {
        return map_to_string(value, delimiter, gap
            , prefix, suffix
            );
    }
};
template <typename Tkey, typename Tval>
struct to_string_t<std::unordered_map<Tkey, Tval>> {
    static std::string from(const std::unordered_map<Tkey, Tval> &value
            , const std::string &delimiter, const std::string &gap
            , const char *prefix, const char *suffix
            ) {
        return map_to_string(value, delimiter, gap
            , prefix, suffix
            );
    }
};
template <std::size_t N, typename... Targs>
struct tuple_to_string_t;
template <std::size_t N, typename... Targs>
struct tuple_to_string_t<N, std::tuple<Targs...>> {
    static std::string from(const std::tuple<Targs...> &data
            , const std::string &delimiter, const std::string &gap
            ) {
        return tuple_to_string_t<N - 1, std::tuple<Targs...>>::from(data, delimiter, gap)
            + delimiter + gap
            + to_string(std::get<N - 1>(data), delimiter, gap);
    }
};
template <typename... Targs>
struct tuple_to_string_t<1, std::tuple<Targs...>> {
    static std::string from(const std::tuple<Targs...> &data
        , const std::string &delimiter, const std::string &gap
        ) {
        return to_string(std::get<0>(data), delimiter, gap);
    }
};
template <typename Tval>
struct to_string_t<Tval, typename std::enable_if<is_stl_tuple<Tval>::value>::type> {
    static std::string from(const Tval &data
            , const std::string &delimiter, const std::string &gap
            , const char *prefix, const char *suffix
            ) {
        return (prefix ? std::string(prefix) : std::string("("))
            + tuple_to_string_t<std::tuple_size<Tval>::value, Tval>::from(data, delimiter, gap)
            + (suffix ? std::string(suffix) : std::string(")"));
    }
};

template <std::size_t N, typename... Targs>
struct tuple_traits {
    static const std::string &name() {
        static const std::string s_name = tuple_traits<N - 1, Targs...>::name() + ", "
            + type_traits<typename std::tuple_element<N - 1, std::tuple<Targs...>>::type>::name();
        return s_name;
    }
};
template <typename... Targs>
struct tuple_traits<1, Targs...> {
    static const std::string &name() {
        return type_traits<typename std::tuple_element<0, std::tuple<Targs...>>::type>::name();
    }
};

template <typename Ttop, typename Tval>
struct get_max_capacity {
    enum { value = std::numeric_limits<int>::max() };
};
template <typename Ttop>
struct get_max_capacity<Ttop, Ttop> {
    enum { value = 1 };
};
template <typename... Targs>
struct get_max_capacity<std::tuple<Targs...>, std::tuple<Targs...>> {
    enum { value = sizeof ...(Targs) };
};

template <typename T, typename Enable = void>
struct get_implicit_value_type {
    typedef typename get_cli_level_type<T>::val_type type;
};
template <typename Tval>
struct get_implicit_value_type<std::vector<Tval>
        , typename std::enable_if<is_cli_primary<Tval>::value>::type> {
    typedef std::vector<Tval> type;
};
template <typename Tval>
struct get_implicit_value_type<std::vector<std::vector<Tval>>
        , typename std::enable_if<is_cli_primary<Tval>::value>::type> {
    typedef std::vector<Tval> type;
};
template <typename Tkey, typename Ttop>
struct get_implicit_value_type<std::map<Tkey, Ttop>> {
    typedef typename get_implicit_value_type<Ttop>::type type;
};
template <typename Tkey, typename Ttop>
struct get_implicit_value_type<std::unordered_map<Tkey, Ttop>> {
    typedef typename get_implicit_value_type<Ttop>::type type;
};

template <typename T, typename = std::false_type>
struct get_choices_value_type {
    typedef typename get_cli_level_type<T>::val_type type;
};
template <typename T>
struct get_choices_value_type<T, is_cli_string<typename get_cli_level_type<T>::val_type>> {
    typedef std::string type;
};

struct ParseRet {
    int argi;
    int vali;
    bool is_terminated;
    std::list<std::string> errors;
};

class ArgParserImpl : public ArgParser {
public:
    ArgParserImpl(char *argv[], int argc, SmartMode smart_mode, void *context, std::string parent_name = "")
            : ArgParser(argv, argc, smart_mode, context, std::move(parent_name)) {
    }
    std::list<std::string> &errors() {
        return _err_list;
    }
    ParseRet submit() {
        return ParseRet {_argi, _vali, _is_terminated, _err_list};
    }
    int at_least() const {
        return _at_least;
    }
    int at_most() const {
        return _at_most;
    }
    template <typename T>
    std::string to_string(bool with_item_type = true, const T *value = nullptr) {
        std::stringstream ss;
        if (value) {
            _is_serializing = true;
            cliargs_parse_custom(const_cast<T &>(*value), *this);
            _is_serializing = false;
        } else {
            ss << _main_type_name;
        }
        ss << "{";
        int i = 0;
        for (auto &it : _item_traits) {
            if (with_item_type && !value) {
                if (i == _at_least && _is_optional) {
                    ss << " [";
                }
            }
            if (i) {
                ss << ", ";
            }
            ss << "." << it.item_name;
            if (with_item_type) {
                ss << ":" << it.type_name;
            }
            if (value) {
                ss << "=" << it.item_value;
            }
            ++i;
        }
        if (_at_least < _at_most && _is_optional) {
            ss << "]";
        }
        ss << "}";
        return ss.str();
    }
}; // ArgParserImpl

template <typename Tval>
struct to_string_t<Tval, typename std::enable_if<is_cli_custom<Tval>::value>::type> {
    static std::string from(const Tval &value
            , const std::string &delimiter, const std::string &gap
            , const char *prefix, const char *suffix
            ) {
        ArgParserImpl parser(nullptr, 0, SmartMode::Gnu, nullptr);
        return parser.to_string(false, &value);
    }
};

class ArgDataI {
public:
    virtual ~ArgDataI() {}
    virtual bool valid() const = 0;
    virtual const void *context() const = 0;
    virtual unsigned appear_count() const = 0;
    virtual int appear(char *argv[], int argc, std::list<std::string> &err_list) = 0;
    virtual std::string finish() = 0;
    virtual SmartMode set_smart_mode(SmartMode mode) = 0;
    virtual bool eat_anything() const = 0;
    virtual const std::string &type_name() const = 0;
}; // ArgDataI

class ArgAttrI : public std::enable_shared_from_this<ArgAttrI> {
public:
    virtual ~ArgAttrI() {}
private:
    friend class cliargs::Parser;
    virtual std::shared_ptr<ArgDataI> create_data(unsigned max_count = 0) const = 0;
    virtual bool is_positional() const = 0;
    virtual bool has_default_value() const = 0;
    virtual bool has_implicit_value() const = 0;
    virtual bool has_constraint() const = 0;
    virtual std::string get_constraint_desc() const = 0;
    virtual std::string get_data_type_desc() const = 0;
    virtual std::string get_default_value_desc() const = 0;
    virtual std::string get_implicit_value_desc() const = 0;
    virtual bool is_hidden() const = 0;
    virtual bool is_concise_help() const = 0;
    virtual SmartMode smart_mode() const = 0;
    virtual const std::string &name() const = 0;
}; // ArgAttrI

template <typename T>
class ArgDataT;

template <typename T>
class ArgAttrT : public ArgAttrI {
protected:
    typedef typename get_cli_level_type<T>::top_type Ttop;
    typedef typename get_cli_level_type<T>::mid_type Tmid;
    typedef typename get_cli_level_type<T>::val_type Tval;
public:
    typedef typename get_implicit_value_type<Ttop>::type implicit_value_t;
    typedef typename get_choices_value_type<Ttop>::type choices_value_t;

public:
    explicit ArgAttrT(std::string name = "")
            : _is_positional(false)
            , _has_default_value(false)
            , _has_implicit_value(false)
            , _dim_0_at_least(0)
            , _dim_0_at_most(get_max_capacity<Ttop, Tmid>::value)
            , _dim_1_at_least(1)
            , _dim_1_at_most(get_max_capacity<Tmid, Tval>::value)
            , _context(nullptr)
            , _is_hidden(false)
            , _concise_help(false)
            , _smart_mode(SmartMode::Gnu)
            , _name(std::move(name)
            )
    {
        static_assert(!is_cli_container<Tval>::value && !(is_cli_container<Tmid>::value && is_stl_tuple<Tval>::value)
            , "Too many nested levels of containers");
        static_assert(!is_stl_map<Tmid>::value
            , "std::vector<std::map> or std::map<std::map> is not allowed");
        if (std::is_same<Ttop, bool>::value) {
            set_dim_1_limit(0, 1);
        } else if (is_stl_vector<Ttop>::value && std::is_same<Tmid, Tval>::value) {
            _dim_1_at_most = _dim_0_at_most;
        }
        if (_name.empty()) {
            _name = type_traits<T>::name();
        }
    }

    std::shared_ptr<ArgAttr<T>> required() {
        set_dim_0_limit(1, -1);
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }

    std::shared_ptr<ArgAttr<T>> positional() {
        _is_positional = true;
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }

    std::shared_ptr<ArgAttr<T>> default_value(T value) {
        _default_value = std::move(value);
        _has_default_value = true;
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }

    template <typename T_ = T>
    typename std::enable_if<!std::is_same<T_, bool>::value, std::shared_ptr<ArgAttr<T>>>::type
    implicit_value(implicit_value_t value) {
        _implicit_value = std::move(value);
        _has_implicit_value = true;
        if (is_cli_primary<Tmid>::value || is_cli_custom<Tmid>::value) {
            set_dim_1_limit(0);
        }
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }
    template <typename T_ = T>
    typename std::enable_if<std::is_same<T_, bool>::value, std::shared_ptr<ArgAttr<T>>>::type
    implicit_value(bool value) {
        _implicit_value = value;
        _has_implicit_value = true;
        _default_value = !value;
        _has_default_value = true;
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }

    template <typename Tval_ = Tval>
    typename std::enable_if<is_cli_primary<Tval_>::value, std::shared_ptr<ArgAttr<T>>>::type
    choices(std::vector<choices_value_t> value_set) {
        _choices_values = std::move(value_set);
        _match_choices_func = [this](const Tval &value) {
            for (auto &it : _choices_values) {
                if (it == value) {
                    return true;
                }
            }
            return false;
        };
        _match_choices_desc = std::move(to_string(_choices_values, ",", " ", "{", "}"));
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }

    std::shared_ptr<ArgAttr<T>> examine(std::function<bool (Tval &)> func, std::string desc = "") {
        _match_examine_func = [func](Tval &v, void *, void *) { return func(v); };
        _match_examine_desc = std::move(desc);
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> examine(
            std::function<bool (Tval &, void *context)> func, std::string desc = "") {
        _match_examine_func = [func](Tval &v, void *context, void *) { return func(v, context); };
        _match_examine_desc = std::move(desc);
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> examine(
            std::function<bool (Tval &, void *context, void *arg_data)> func
            , std::string desc = "") {
        _match_examine_func = std::move(func);
        _match_examine_desc = std::move(desc);
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> context(void *ctx) {
         _context = ctx;
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> hide() {
        _is_hidden = true;
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> concise_help() {
        _concise_help = true;
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }

    template <typename Tval_ = Tval>
    typename std::enable_if<(is_cli_string<Tval_>::value|| is_stl_tuple<Tval_>::value || is_cli_custom<Tval_>::value)
        , std::shared_ptr<ArgAttr<T>>>::type
    stop_at_eof() {
        _smart_mode = SmartMode::Eof;
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }

    template <typename Ttop_ = Ttop>
    typename std::enable_if<detail::is_stl_vector<Ttop_>::value, std::shared_ptr<ArgAttr<T>>>::type
    data_count(int at_least, int at_most = -1) {
        set_dim_0_limit(at_least, at_most);
        if (std::is_same<Tmid, Tval>::value) {
            set_dim_1_limit(at_least ? 1 : 0, at_most);
        }
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }
    template <typename Tval_ = Tval>
    typename std::enable_if<detail::is_stl_tuple<Tval_>::value, std::shared_ptr<ArgAttr<T>>>::type
    line_width(int at_least) {
        set_dim_1_limit(at_least);
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }
    template <typename Ttop_ = Ttop, typename Tmid_ = Tmid, typename Tval_ = Tval>
    typename std::enable_if<(detail::is_stl_vector<Tmid_>::value || detail::is_stl_vector<Ttop_>::value)
            && !detail::is_stl_tuple<Tval_>::value
        , std::shared_ptr<ArgAttr<T>>>::type
    line_width(int at_least, int at_most = -1) {
        set_dim_1_limit(at_least, at_most);
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }

protected:
    friend class ArgDataT<T>;
    std::shared_ptr<ArgDataI> create_data(unsigned max_count = 0) const override {
        return std::make_shared<ArgDataT<T>>(*this);
    }
    bool is_positional() const override {
        return _is_positional;
    }
    bool has_default_value() const override {
        return _has_default_value;
    }
    bool has_implicit_value() const override {
        return _has_implicit_value;
    }
    bool has_constraint() const override {
        return _match_choices_func || _match_ranges_func || !_match_examine_desc.empty();
    }
    std::string get_constraint_desc() const override {
        // (choices || ranges) && examine
        std::string desc;
        if (_match_choices_func) {
            desc += "in-set" + _match_choices_desc;
        }
        if (_match_ranges_func) {
            if (!desc.empty()) {
                desc += " or ";
            }
            desc += _match_ranges_desc;
        }
        if (!_match_examine_desc.empty()) {
            if (_match_choices_func && _match_ranges_func) {
                desc = "(" + desc + ")";
            }
            if (!desc.empty()) {
                desc += " and ";
            }
            desc += "'" + _match_examine_desc + "'";
        }
        return desc;
    }
    std::string get_data_type_desc() const override {
        if (hide_data_desc()) {
            return "";
        }
        std::stringstream ss;
        ss << type_traits<T>::name();
        auto print_range = [&ss](unsigned at_least, unsigned at_most) {
            ss << "[" << at_least << "~";
            if (at_most == std::numeric_limits<int>::max()) {
                ss << "N";
            } else {
                ss << at_most;
            }
            ss << "]";
        };
        if (is_cli_container<T>::value) {
            print_range(_dim_0_at_least, _dim_0_at_most);
            if (is_cli_container<Tmid>::value || is_stl_tuple<Tval>::value) {
                print_range(_dim_1_at_least, _dim_1_at_most);
            }
        } else {
            if (is_stl_tuple<Tval>::value) {
                print_range(_dim_0_at_least, _dim_0_at_most);
            } else {
                if (_dim_0_at_least > 0) {
                    ss << "[required]";
                } else {
                    ss << "[optional]";
                }
            }
        }
        return ss.str();
    }
    std::string get_default_value_desc() const override {
        if (hide_data_desc() || !has_default_value()) {
            return "";
        }
        return to_string(get_default_value());
    }
    std::string get_implicit_value_desc() const override {
        if (hide_data_desc() || !has_implicit_value()) {
            return "";
        }
        return to_string(get_implicit_value());
    }
    void *get_context() const {
        return _context;
    }
    bool is_hidden() const override {
        return _is_hidden;
    }
    bool is_concise_help() const override {
        return _concise_help;
    }
    SmartMode smart_mode() const override {
        return _smart_mode;
    }
    const std::string &name() const override {
        return _name;
    }

private:
    unsigned dim_0_at_least() const {
        return _dim_0_at_least;
    }
    unsigned dim_0_at_most() const {
        return _dim_0_at_most;
    }
    unsigned dim_1_at_least() const {
        return _dim_1_at_least;
    }
    unsigned dim_1_at_most() const {
        return _dim_1_at_most;
    }
    std::string examine(Tval &value, void *arg_data) const {
        // (choices || ranges) && examine
        bool is_in_range = false;
        if (_match_choices_func && _match_choices_func(value)) {
            is_in_range = true;
        }
        if (!is_in_range && _match_ranges_func && _match_ranges_func(value)) {
            is_in_range = true;
        }
        auto err_detail = "should meet constraint: " + get_constraint_desc();
        if ((_match_choices_func || _match_ranges_func) && !is_in_range) {
            return err_detail;
        }
        if (!_match_examine_func || _match_examine_func(value, _context, arg_data)) {
            return "";
        }
        return err_detail;
    }
    const T &get_default_value() const {
        return _default_value;
    }
    const implicit_value_t &get_implicit_value() const {
        return _implicit_value;
    }
    template <typename T_ = T>
    typename std::enable_if<std::is_same<T_, bool>::value, bool>::type hide_data_desc() const {
        return _implicit_value;
    }
    template <typename T_ = T>
    typename std::enable_if<!std::is_same<T_, bool>::value, bool>::type hide_data_desc() const {
        return false;
    }
    void set_dim_0_limit(int at_least, int at_most = -1) {
        _dim_0_at_least = (at_least >= 0 ? at_least : _dim_0_at_least);
        _dim_0_at_most = (at_most >= 0 ? at_most : _dim_0_at_most);
    }
    void set_dim_1_limit(int at_least, int at_most = -1) {
        _dim_1_at_least = (at_least >= 0 ? at_least : _dim_1_at_least);
        _dim_1_at_most = (at_most >= 0 ? at_most : _dim_1_at_most);
    }

protected:
    void set_match_ranges_func(std::function<bool (const Tval &)> func, std::string desc) {
        _match_ranges_func = std::move(func);
        _match_ranges_desc = std::move(desc);
    }

private:
    bool _is_positional;
    bool _has_default_value;
    bool _has_implicit_value;
    unsigned _dim_0_at_least;
    unsigned _dim_0_at_most;
    unsigned _dim_1_at_least;
    unsigned _dim_1_at_most;
    void *_context;
    bool _is_hidden;
    bool _concise_help;
    T _default_value;
    SmartMode _smart_mode;
    implicit_value_t _implicit_value;
    std::function<bool (const Tval &)> _match_choices_func;
    std::vector<choices_value_t> _choices_values;
    std::string _match_choices_desc;
    std::function<bool (const Tval &)> _match_ranges_func;
    std::string _match_ranges_desc;
    std::function<bool (Tval &, void *context, void *arg_data)> _match_examine_func;
    std::string _match_examine_desc;
    std::string _name;
}; // ArgAttrT

template <typename T>
class ArgDataT : public ArgDataI {
    typedef typename get_cli_level_type<T>::top_type Ttop;
    typedef typename get_cli_level_type<T>::mid_type Tmid;
    typedef typename get_cli_level_type<T>::val_type Tval;
public:
    ArgDataT(const ArgAttrT<T> &arg_attr)
        : _arg_attr(arg_attr), _appear_count(0), _data_count(0)
        , _smart_mode(SmartMode::Gnu) {
    }
    bool valid() const override {
        return _appear_count > 0;
    }
    const void *context() const override {
        return _arg_attr.get_context();
    }
    unsigned appear_count() const override {
        return _appear_count;
    }
    int appear(char *argv[], int argc, std::list<std::string> &err_list) override;
    std::string finish() override;
    SmartMode set_smart_mode(SmartMode mode) override {
        auto tmp = _smart_mode;
        _smart_mode = mode;
        return tmp;
    }
    bool eat_anything()  const override {
        return std::is_convertible<Tval, std::string>::value && _smart_mode != SmartMode::Name;
    }
    const std::string &type_name() const override {
        return type_traits<T>::name();
    }
    const T &data() const {
        return _data;
    }
protected:
    T &data() {
        return _data;
    }
private:
    const ArgAttrT<T> &_arg_attr;
    unsigned _appear_count;
    unsigned _data_count;
    T _data;
    SmartMode _smart_mode;
}; // ArgDataT

template <typename T, typename Enable = void>
class ArgAttrTval : public ArgAttrT<T> {
public:
    template<typename... Ts>
    ArgAttrTval(Ts... args) : ArgAttrT<T>(args...) {}
};
template <typename T>
class ArgAttrTval<T, typename std::enable_if<is_cli_scalar<typename get_cli_level_type<T>::val_type>::value>::type>
        : public ArgAttrT<T> {
    typedef typename ArgAttrT<T>::Tval Tval;
public:
    template<typename... Ts>
    ArgAttrTval(Ts... args) : ArgAttrT<T>(args...) {}
    std::shared_ptr<ArgAttr<T>> range(Tval min_value, Tval max_value) {
        _value_ranges.emplace_back(min_value, max_value);
        return ranges(std::move(_value_ranges));
    }
    std::shared_ptr<ArgAttr<T>> ranges(std::vector<std::pair<Tval, Tval>> range_pairs, std::string desc = "") {
        _value_ranges.insert(_value_ranges.end(), range_pairs.begin(), range_pairs.end());
        if (desc.empty()) {
            desc = "within-ranges" + to_string(_value_ranges);
        }
        auto is_in_ranges = [this](const Tval &value) {
            for (auto &it : _value_ranges) {
                if (value >= it.first && value <= it.second) {
                    return true;
                }
            }
            return false;
        };
        ArgAttrT<T>::set_match_ranges_func(is_in_ranges, std::move(desc));
        return std::static_pointer_cast<ArgAttr<T>>(ArgAttrT<T>::shared_from_this());
    }
private:
    std::vector<std::pair<Tval, Tval>> _value_ranges;
};
template <typename T>
class ArgAttrTval<T, typename std::enable_if<is_cli_string<typename get_cli_level_type<T>::val_type>::value>::type>
        : public ArgAttrT<T> {
public:
    template<typename... Ts>
    ArgAttrTval(Ts... args) : ArgAttrT<T>(args...) {}
#ifndef CLIARGS_NO_REGEX
    std::shared_ptr<ArgAttr<T>> regex(std::string regex_string, std::string desc = "") {
        _regex_string = std::move(regex_string);
        _regex_object = std::regex(_regex_string);
        if (desc.empty()) {
            desc = "match-regex(\"" + _regex_string + "\")";
        }
        ArgAttrT<T>::set_match_ranges_func(
            [this](const typename ArgAttrT<T>::Tval &value) { return std::regex_match(value, _regex_object); }
            , std::move(desc));
        return std::static_pointer_cast<ArgAttr<T>>(ArgAttrT<T>::shared_from_this());
    }
private:
    std::string _regex_string;
    std::regex _regex_object;
#endif // CLIARGS_NO_REGEX
};

template <typename Tval>
struct get_capacity {
    static int at_most(SmartMode smart_mode) {
        ArgParserImpl parser(nullptr, 0, smart_mode, nullptr);
        Tval arg_value;
        cliargs_parse_custom(arg_value, parser);
        return parser.at_most();
    }
};

template <typename Tval>
struct DataParser {
    static ParseRet parse(Tval &value, char *argv[], int argc, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(Tval &, void *)> examine
            , std::function<const typename get_implicit_value_type<Tval>::type &()> get_implicit_value
            , const std::string &name
            ) {
        ArgParserImpl parser(argv, argc, smart_mode, context, name);
        if (at_least < 1) {
            parser.set_optional();
        }
        cliargs_parse_custom(value, parser);
        auto ret = parser.submit();
        if (ret.vali || at_least > 0) {
            // do nothing
        } else if (get_implicit_value) {
            value = get_implicit_value();
            ret.vali = 1;
        } else {
            return ret;
        }
        if ((ret.vali || get_implicit_value) && examine) {
            auto err_detail = examine(value, arg_data);
            if (!err_detail.empty()) {
                auto item_at_most = get_capacity<Tval>::at_most(smart_mode);
                std::stringstream ss;
                ss << "invalid value";
                if (item_at_most > 1) {
                    ss << " group '" << to_string(std::vector<char *>(argv, argv + ret.vali))
                       << " (as type " << type_traits<Tval>::name() << ")";
                } else {
                    ss << " '" << *argv << "'";
                }
                ss << ", " << err_detail;
                ret.errors.emplace_back(ss.str());
            }
        }
        return ret;
    }
};
template <typename Tmid>
struct DataParser<std::vector<Tmid>> {
    typedef std::vector<Tmid> Ttop;
    typedef typename get_cli_value_type<Tmid>::type Tval;
    static ParseRet parse(Ttop &value, char *argv[], int argc, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(Tval &, void *)> examine
            , std::function<const typename get_implicit_value_type<Ttop>::type &()> get_implicit_value
            , const std::string &name
            );
};
template <typename Tmap, typename Tkey, typename Ttop>
struct MapParser {
    static ParseRet parse(Tmap &value, char *argv[], int argc, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(typename get_cli_level_type<Ttop>::val_type &, void *)> examine
            , std::function<const typename get_implicit_value_type<Ttop>::type &()> implicit_value
            , const std::string &name
            );
};
template <typename Tkey, typename Ttop>
struct DataParser<std::map<Tkey, Ttop>> : MapParser<std::map<Tkey, Ttop>, Tkey, Ttop> {};
template <typename Tkey, typename Ttop>
struct DataParser<std::unordered_map<Tkey, Ttop>> : MapParser<std::unordered_map<Tkey, Ttop>, Tkey, Ttop> {};
template <std::size_t N, typename... Targs>
struct TupleParser {
    static ParseRet parse(std::tuple<Targs...> &value, char *argv[], int argc, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name
            ) {
        ParseRet result {0, 0, false};
        if (N) {
            result = TupleParser<N - 1, Targs...>::parse(value, argv, argc, smart_mode
                , context, arg_data, at_least, at_most, nullptr, get_implicit_value, name);
        }
        if (!result.is_terminated) {
            auto ret = DataParser<typename std::tuple_element<N, std::tuple<Targs...>>::type>::parse(
                std::get<N>(value), argv + result.argi
                , ((result.argi >= (int)N && argc > (int)N) ? argc - result.argi : 0), smart_mode
                , context, arg_data, (at_least > N ? 1 : 0), 1, nullptr, nullptr
                , name + "<" + to_string(N) + ">");
            if (ret.argi || at_least > N) {
                result.errors.splice(result.errors.end(), ret.errors);
            }
            result.argi += ret.argi;
            result.vali += ret.vali;
            result.is_terminated = ret.is_terminated;
        }
        if (result.is_terminated) {
            if (get_implicit_value) {
                std::get<N>(value) = std::get<N>(get_implicit_value());
            }
        }
        return result;
    }
};
template <typename... Targs>
struct TupleParser<0, Targs...> {
    static ParseRet parse(std::tuple<Targs...> &value, char *argv[], int argc, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name
            ) {
        auto ret = DataParser<typename std::tuple_element<0, std::tuple<Targs...>>::type>::parse(
            std::get<0>(value), argv, argc, smart_mode
            , context, arg_data, (at_least > 0 ? 1 : 0), 1, nullptr, nullptr, name + "<0>"
            );
        if (ret.is_terminated && get_implicit_value) {
            std::get<0>(value) = std::get<0>(get_implicit_value());
        }
        return ret;
    }
};
template <typename... Targs>
struct DataParser<std::tuple<Targs...>> {
    static ParseRet parse(std::tuple<Targs...> &value, char *argv[], int argc, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name
            ) {
        auto ret = TupleParser<sizeof ...(Targs) - 1, Targs...>::parse(
            value, argv, argc, smart_mode
            , context, arg_data, at_least, at_most, examine, get_implicit_value, name);
        if (ret.errors.empty()) {
            if (examine) {
                auto err_detail = examine(value, arg_data);
                if (!err_detail.empty()) {
                    ret.errors.emplace_back(std::move(err_detail));
                }
            }
        }
        if (ret.vali) {
            ret.vali = 1;
        }
        return ret;
    }
};
template <typename Tmid, bool IS_PRIMARY>
struct VectorParser;
template <typename Tmid>
struct VectorParser<Tmid, false> {
    typedef std::vector<Tmid> Ttop;
    typedef typename get_cli_level_type<Tmid>::val_type Tval;
    static ParseRet parse(std::vector<Tmid> &value, char *argv[], int argc, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(Tval &, void *)> examine
            , std::function<const typename get_implicit_value_type<Ttop>::type &()> get_implicit_value
            , const std::string &name
            ) {
        value.emplace_back(Tmid());
        auto ret = DataParser<Tmid>::parse(*value.rbegin(), argv, argc, smart_mode
            , context, arg_data, at_least, at_most, examine, get_implicit_value
            , (name + "[" + to_string(value.size() - 1) + "]"));
        ret.vali = value.size();
        return ret;
    }
};
template <typename Tval>
struct VectorParser<Tval, true> {
    typedef std::vector<Tval> Ttop;
    static ParseRet parse(std::vector<Tval> &value, char *argv[], int argc, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(Tval &, void *)> examine
            , std::function<const typename get_implicit_value_type<Ttop>::type &()> get_implicit_value
            , const std::string &name
            ) {
        auto item_at_most = get_capacity<Tval>::at_most(smart_mode);
        unsigned n = 0;
        ParseRet result {0, 0, false};
        while (n < at_most && value.size() < at_most) {
            Tval arg_value;
            auto ret = DataParser<Tval>::parse(arg_value, argv + result.argi, argc - result.argi, smart_mode
                , context, arg_data, (n < at_least ? 1 : 0), 1, examine, nullptr
                , (name + "[" + to_string(value.size()) + "]"));
            result.argi += ret.argi;
            result.is_terminated = ret.is_terminated;
            result.errors.splice(result.errors.end(), ret.errors);
            if (!ret.vali) {
                break;
            }
            value.emplace_back(std::move(arg_value));
            ++n;
            if (item_at_most > 1 || ret.is_terminated) {
                break;
            }
        }
        result.vali = value.size();
        if (value.size() >= at_most) {
            return result;
        }
        if (n >= at_least && get_implicit_value) {
            auto &implicit_value = get_implicit_value();
            for (; n < at_most && n < implicit_value.size(); ++n) {
                value.emplace_back(implicit_value[n]);
            }
            result.vali = value.size();
        }
        return result;
    }
};
template <typename Tmid>
ParseRet DataParser<std::vector<Tmid>>::parse(std::vector<Tmid> &value, char *argv[], int argc, SmartMode smart_mode
        , void *context, void *arg_data, unsigned at_least, unsigned at_most
        , std::function<std::string(Tval &, void *)> examine
        , std::function<const typename get_implicit_value_type<Ttop>::type &()> get_implicit_value
        , const std::string &name
        ) {
    return VectorParser<Tmid, is_cli_primary<Tmid>::value>::parse(value, argv, argc, smart_mode
        , context, arg_data, at_least, at_most, examine, get_implicit_value, name);
}
template <typename Tmap, typename Tkey, typename Ttop>
ParseRet MapParser<Tmap, Tkey, Ttop>::parse(Tmap &value, char *argv[], int argc, SmartMode smart_mode
        , void *context, void *arg_data, unsigned at_least, unsigned at_most
        , std::function<std::string(typename get_cli_level_type<Ttop>::val_type &, void *)> examine
        , std::function<const typename get_implicit_value_type<Ttop>::type &()> implicit_value
        , const std::string &name
        ) {
    if (argc < 1) {
        std::stringstream ss;
        ss << "a(n) '" << type_traits<Tkey>::name() << "' value is required as '" + name + ".key'";
        return ParseRet{0, 0, true, {ss.str()}};
    }
    Tkey map_key;
    std::list<std::string> err_key;
    auto result = DataParser<Tkey>::parse(map_key, argv, argc, smart_mode
        , context, arg_data, 1, 1, nullptr, nullptr, name + ".key");
    auto it = value.insert(std::make_pair(map_key, Ttop()));
    auto ret = DataParser<Ttop>::parse(it.first->second, argv + result.argi
        , result.is_terminated ? 0 : (argc - result.argi), smart_mode
        , context, arg_data, at_least, at_most, examine, implicit_value
        , name + "[" + to_string(map_key) + "]");
    result.argi += ret.argi;
    result.is_terminated = ret.is_terminated;
    if (!ret.errors.empty()) {
        result.errors.splice(result.errors.end(), ret.errors);
    }
    result.vali = 1;
    return result;
}

template <typename T>
int ArgDataT<T>::appear(char *argv[], int argc, std::list<std::string> &err_list) {
    if (argc == -1) {
        argc = 1;
    } else if (argc > 0) {
        if (std::is_same<T, bool>::value) {
            argc = 0;
        }
    }
    ++_appear_count;
    auto dim_0_at_most = _arg_attr.dim_0_at_most();
    auto dim_1_at_least = _arg_attr.dim_1_at_least();
    auto dim_1_at_most = _arg_attr.dim_1_at_most();
    int i = 0;
    auto err_header = " " + to_string(_appear_count) + "th: ";
    if (dim_0_at_most > 1 && _appear_count > dim_0_at_most) {
        std::stringstream ss;
        ss << "too many appearances";
        Ttop v_tmp;
        auto ret = DataParser<Ttop>::parse(v_tmp, argv, argc, _smart_mode
            , _arg_attr.get_context(), &v_tmp, dim_1_at_least, dim_1_at_most
            , nullptr, nullptr, _arg_attr.name());
        while (i < argc && i < ret.argi) {
            if (i == 0) {
                ss << " ['" << argv[i] << "'";
            } else {
                ss << " '" << argv[i] << "'";
            }
            ++i;
        }
        if (i) {
            ss << "]";
        }
        err_list.emplace_back(err_header + ss.str());
        return i;
    }
    std::function<const typename get_implicit_value_type<T>::type &()> func_implicit_value;
    if (_arg_attr.has_implicit_value()) {
        func_implicit_value = std::bind(&ArgAttrT<T>::get_implicit_value, _arg_attr);
    }
    auto ret = DataParser<T>::parse(_data, argv, argc, _smart_mode
        , _arg_attr.get_context(), &_data, dim_1_at_least, dim_1_at_most
        , [this](Tval &value, void *arg_data) { return this->_arg_attr.examine(value, arg_data); }
        , func_implicit_value, _arg_attr.name()
        );
    for (auto &it : ret.errors) {
        err_list.emplace_back(err_header + it);
    }
    _data_count = ret.vali;
    return ret.argi;
}

template <typename T>
std::string ArgDataT<T>::finish() {
    if (!_data_count && _arg_attr.has_default_value()) {
        _data = _arg_attr.get_default_value();
        _appear_count = 1;
        return "";
    }
    auto dim_0_at_least = _arg_attr.dim_0_at_least();
    auto dim_0_at_most = _arg_attr.dim_0_at_most();
    std::stringstream ss;
    if (is_cli_container<Tmid>::value) {
        if (_appear_count < dim_0_at_least) {
            ss << ": expects " << dim_0_at_least;
            if (dim_0_at_least < dim_0_at_most) {
                ss << " ~ " << dim_0_at_most;
            }
            ss << " appearance(s), but got " << _appear_count;
            return ss.str();
        }
    } else {
        if (_data_count < dim_0_at_least) {
            ss << ": expects " << dim_0_at_least;
            if (dim_0_at_least < dim_0_at_most) {
                ss << " ~ " << dim_0_at_most;
            }
            ss << " value(s), but got " << _data_count;
            return ss.str();
        }
    }
    return "";
}

} // detail

template <typename T>
void cliargs_parse_custom(T &var, ArgParser &parser) {
    static_assert(!detail::is_cli_custom<T>::value,
        "Please specialize the 'cliargs_parse_custom' template for your custom type.");
    parser.assign(var, "");
}

template <typename T>
std::string to_string(const T &data
        , const std::string &delimiter, const std::string &gap
        , const char *prefix, const char *suffix) {
    return detail::to_string_t<T>::from(data, delimiter, gap, prefix, suffix);
}
template <typename T>
std::string to_string(T *array, size_t count
        , const std::string &delimiter = ",", const std::string &gap = " "
        , const char *prefix = nullptr, const char *suffix = nullptr) {
    std::stringstream ss;
    ss << (prefix ? prefix : "[");
    bool is_first = true;
    for (size_t i = 0; i < count; ++i) {
        if (is_first) {
            is_first = false;
        } else {
            ss << delimiter << gap;
        }
        ss << to_string(array[i], delimiter, gap);
    }
    ss << (suffix ? suffix : "]");
    return ss.str();
}

template <typename T>
const std::string &type_traits<T>::name() {
    static std::string s_name = []() -> std::string {
        detail::ArgParserImpl parser(nullptr, 0, SmartMode::Gnu, nullptr);
        T arg_value;
        cliargs_parse_custom(arg_value, parser);
        return parser.to_string<T>();
    }();
    return s_name;
}
template <typename... Targs>
struct type_traits<std::tuple<Targs...>> {
    static const std::string &name() {
        static const std::string s_name = std::string("tuple<")
            + detail::tuple_traits<sizeof ...(Targs), Targs...>::name() + ">";
        return s_name;
    }
};

ArgParser::ArgParser(char *argv[], int argc, SmartMode smart_mode, void *context, std::string parent_name)
        : _argc(argc), _argv(argv), _smart_mode(smart_mode), _context(context)
        , _parent_name(std::move(parent_name))
        , _argi(0), _vali(0), _is_optional(false)
        , _at_least(0), _at_most(0)
        , _is_terminated(false), _is_serializing(false)
        , _item_name_prefix("."), _item_name_suffix("")
        {
}
void ArgParser::domain_begin(std::string type_name
        , std::string member_name_prefix
        , std::string member_name_suffix
        ) {
    _main_type_name = std::move(type_name);
    _item_name_prefix = std::move(member_name_prefix);
    _item_name_suffix = std::move(member_name_suffix);
}
void ArgParser::domain_end() {
}
template <typename T>
bool ArgParser::assign(T &value, const std::string &name, T default_value) {
    static const auto &type_name = type_traits<T>::name();
    _item_traits.emplace_back(ItemTraits{type_name, name});
    if (_is_serializing) {
        _item_traits.rbegin()->item_value = to_string(value);
        return true;
    }
    ++_at_most;
    char *psz = nullptr;
    if (_argi < _argc && _at_most <= _argc) {
        psz = _argv[_argi];
        while (psz == nullptr && _argi < _argc) {
            psz = _argv[_argi];
            ++_argi;
        }
    }
    if (!_is_terminated && psz != nullptr && _at_most <= _argc
            && (psz[0] != '-'                                                         /*normal string*/
                || detail::is_numeric(psz + 1)                                        /*negative numberic*/
                || (_smart_mode == SmartMode::Gnu && detail::is_cli_string<T>::value) /*force assigning*/
                || (_smart_mode == SmartMode::Eof && (psz[1] != '-' || psz[2]))       /*stop at end: '--'*/
            )) {
    } else {
        _is_terminated = true;
        if (_is_optional) {
            value = std::move(default_value);
        } else {
            std::stringstream ss;
            ss << "a(n) '" << type_name << "' value is required" << " as '" << concat_name(name) << "'";
            _err_list.emplace_back(ss.str());
        }
        return false;
    }
    if (_smart_mode != SmartMode::Gnu && psz[0] == '\\') {
        ++psz;
    }
    if (detail::is_cli_primary<T>::value) {
        ++_argi;
        ++_vali;
        if (auto tn = detail::parse_primary(value, psz)) {
            std::stringstream ss;
            ss << "format error: '" << psz << "', expect a(n) '" << tn << "' value as " << concat_name(name);
            _err_list.emplace_back(ss.str());
            return false;
        }
    } else {
        detail::ParseRet ret {0, 0, false};
        if (detail::is_cli_custom<T>::value) {
            detail::ArgParserImpl sub_parser(_argv + _argi, _argc - _argi, _smart_mode, _context, concat_name(name));
            if (_is_optional) {
                sub_parser.set_optional();
            }
            cliargs_parse_custom(value, sub_parser);
            ret = sub_parser.submit();
        } else {
            ret = detail::DataParser<T>::parse(value, _argv + _argi, _argc - _argi, _smart_mode
                , _context, &value, 0
                , detail::get_max_capacity<T, typename detail::get_cli_level_type<T>::val_type>::value
                , nullptr, nullptr, concat_name(name));
        }
        _argi += ret.argi;
        _vali += ret.vali;
        if (!ret.errors.empty()) {
            _err_list.splice(_err_list.end(), ret.errors);
            return false;
        }
    }
    return true;
}
void ArgParser::check(bool is_true, std::string msg) {
    if (!is_true) {
        _err_list.emplace_back(std::move(msg));
    }
}
void ArgParser::set_optional() {
    if (!_is_optional) {
        _at_least = _at_most;
        _is_optional = true;
    }
}
void *ArgParser::get_context() const {
    return _context;
}
std::string ArgParser::concat_name(const std::string &name) const {
    if (_parent_name.empty() && _main_type_name.empty()) {
        return name;
    }
    std::stringstream ss;
    if (!_parent_name.empty()) {
        ss << _parent_name;
    } else {
        ss << _main_type_name;
    }
    if (!name.empty()) {
        ss << _item_name_prefix  << name << _item_name_suffix;
    }
    return ss.str();
}

template <typename T>
class ArgAttr : public detail::ArgAttrTval<T> {
public:
    template<typename... Ts>
    ArgAttr(Ts... args) : detail::ArgAttrTval<T>(args...) {}
};

template <typename T, typename... Ts>
std::shared_ptr<ArgAttr<T>> value(Ts... args) {
    return std::make_shared<ArgAttr<T>>(args...);
}

class ArgDesc {
public:
    ArgDesc(char flag, std::string name, std::string desc
                , std::shared_ptr<detail::ArgAttrI> attr, std::string alias="")
            : _sname("-"), _lname(std::move(name)), _desc(std::move(desc))
            , _alias(std::move(alias)), _attr(attr) {
        if (flag) {
            _sname += flag;
        } else {
            _sname.clear();
        }
    }

    const std::string &flag() const {
        return _sname;
    }
    const std::string &name() const {
        return _lname;
    }
    const std::string &desc() const {
        return _desc;
    }
    const std::string &alias() const {
        return _alias;
    }
    const detail::ArgAttrI *attr() const {
        return _attr.get();
    }

private:
    std::string _sname;
    std::string _lname;
    std::string _desc;
    std::string _alias;
    std::shared_ptr<detail::ArgAttrI> _attr;
}; // ArgDesc

class ArgData {
public:
    ArgData() {}
    ArgData(std::shared_ptr<detail::ArgDataI> data, const std::string &arg_name)
            : _arg_data(data), _arg_name(arg_name) {}
    bool valid() const {
        return _arg_data && _arg_data->valid();
    }
    const void *context() const {
        return _arg_data ? _arg_data->context() : nullptr;
    }
    template <typename T> const T &as() const {
#ifdef CLIARGS_NO_RTTI
#ifndef CLIARGS_NO_WARNING
        #warning "Ensure that every custom data type returns a unique type_name via cliargs::type_traits<T>::name()." \
            "define 'CLIARGS_NO_WARNING' BEFORE including cliargs.hpp to suppress all cliargs compiler warnings."
#endif // CLIARGS_NO_WARNING
        const detail::ArgDataT<T> *arg_data = nullptr;
        if (_arg_data->type_name() == type_traits<T>::name()) {
            arg_data = static_cast<const detail::ArgDataT<T> *>(_arg_data.get());
        }
#else
        auto arg_data = dynamic_cast<const detail::ArgDataT<T> *>(_arg_data.get());
#endif // CLIARGS_NO_RTTI
        if (!arg_data) {
            std::stringstream ss;
            ss << "fault: arg['" << _arg_name << "']: bad_cast as '"
               << type_traits<T>::name() << "'";
#ifdef CLIARGS_NO_EXCEPTION
            std::cerr << ss.str() << std::endl;
            std::exit(EXIT_FAILURE);
#else
            throw bad_cast(ss.str());
#endif // CLIARGS_NO_EXCEPTION
        }
        return arg_data->data();
    }
private:
    std::shared_ptr<detail::ArgDataI> _arg_data;
    std::string _arg_name;
}; // ArgData

class Result {
    friend class Parser;
public:
    struct Tail {
        int argc;
        char **argv;
    public:
        Tail() : argc(0), argv(nullptr) {}
    };
public:
    explicit Result(const Parser *parser = nullptr);
    const ArgData &operator [](const std::string &arg_name) {
        auto it = _arg_data_map.find(arg_name);
        if (it == _arg_data_map.end()) {
            std::stringstream ss;
            ss  << "fault: ['" << arg_name << "']: unknown arg-name";
#ifdef CLIARGS_NO_EXCEPTION
            std::cerr << ss.str() << std::endl;
            std::exit(EXIT_FAILURE);
#else
            throw std::invalid_argument(ss.str());
#endif // CLIARGS_NO_EXCEPTION
        }
        return it->second;
    }
    const Tail &tail() const {
        return _arg_tail;
    }
    bool error() const;
    const std::list<std::string> &error_details() const {
        return _err_list;
    }
    void print_help(const std::string &indent = ""
            , std::ostream &os = std::cout) const;
    const Parser *parser() const {
        return _parser;
    }
private:
    void add_error(std::string err) {
        _err_list.emplace_back(std::move(err));
    }
    void add_data(const std::string &name, ArgData data) {
        _arg_data_map.insert(std::make_pair(name, std::move(data)));
    }
    void set_tail(int argc, char *argv[]) {
        _arg_tail.argc = argc;
        _arg_tail.argv = argv;
    }
private:
    const Parser *_parser;
    std::list<std::string> _err_list;
    Tail _arg_tail;
    std::unordered_map<std::string, ArgData> _arg_data_map;
}; // Result

class Parser {
private:
    class ArgAdder;

public:
    explicit Parser(std::string app_name = "<THIS>", std::string app_desc = "")
        : _app_name(std::move(app_name)), _app_desc(std::move(app_desc))
        , _allow_unknown(false), _smart_mode(SmartMode::Name)
        , _help_width(0), _concise_help(false)
        , _help_indent("    ")
        {
    }
    Parser &allow_unknown() {
        _allow_unknown = true;
        return *this;
    }
    Parser &gnu_mode() {
        _smart_mode = SmartMode::Gnu;
        return *this;
    }
    Parser &set_width(unsigned width) {
        _help_width = width;
        return *this;
    }
    Parser &concise_help() {
        _concise_help = true;
        return *this;
    }
    ArgAdder add_args() {
        _err_list.clear();
        return ArgAdder(*this);
    };
    bool error(const Result *result) const {
        if (result) {
            return result->error();
        }
        return !_err_list.empty();
    }
    const std::list<std::string> &error_details(const Result *result) const {
        if (result) {
            return result->error_details();
        }
        return _err_list;
    }
    void print_help(const Result *result
            , const std::string &indent = ""
            , std::ostream &os = std::cout) const;
    Result parse(int argc, char *argv[], unsigned start_index = 1);

private:
    template <typename T>
    void add_arg(char flag, std::string name, std::string desc
        , std::shared_ptr<ArgAttr<T>> attr
        , std::string alias="");

    class ArgAdder {
    public:
        ArgAdder &operator () (char flag, std::string name
                , std::string desc, std::string alias="") {
            _parser.add_arg(flag, std::move(name), std::move(desc)
                , value<bool>()->implicit_value(true)
                , std::move(alias));
            return *this;
        }
        ArgAdder &operator () (std::string name
                , std::string desc, std::string alias="") {
            _parser.add_arg(0, std::move(name), std::move(desc)
                , value<bool>()->implicit_value(true)
                , std::move(alias));
            return *this;
        }
        template<typename T>
        ArgAdder &operator () (char flag, std::string name
                , std::string desc, std::shared_ptr<ArgAttr<T>> attr
                , std::string alias="") {
            _parser.add_arg(flag, std::move(name), std::move(desc)
                , std::move(attr), std::move(alias));
            return *this;
        }
        template<typename T>
        ArgAdder &operator () (std::string name
                , std::string desc, std::shared_ptr<ArgAttr<T>> attr
                , std::string alias="") {
            _parser.add_arg(0, std::move(name), std::move(desc)
                , std::move(attr), std::move(alias));
            return *this;
        }

        explicit ArgAdder(Parser &parser) : _parser(parser) {}
    private:
        Parser &_parser;
    }; // ArgAdder

private:
    std::string _app_name;
    std::string _app_desc;
    bool _allow_unknown;
    SmartMode _smart_mode;
    int _help_width;
    bool _concise_help;
    std::list<ArgDesc> _arg_desc_list;
    std::unordered_map<std::string, ArgDesc *> _arg_desc_dict;
    std::list<std::string> _err_list;
    std::string _help_indent;
}; // Parser

Result::Result(const Parser *parser)
        : _parser(parser) {
    if (_parser) {
        _err_list = _parser->error_details(nullptr);
    }
}

bool Result::error() const {
    if (_parser && _parser->error(nullptr)) {
        return true;
    }
    return !_err_list.empty();
}

void Result::print_help(const std::string &indent, std::ostream &os) const {
    if (_parser) {
        _parser->print_help(this, indent, os);
    } else {
        for (auto &it : _err_list) {
            os << indent << it << "\n";
        }
    }
}

template <typename T>
void Parser::add_arg(char flag, std::string name
        , std::string desc, std::shared_ptr<ArgAttr<T>> attr, std::string alias) {
    auto def_index = to_string(_arg_desc_list.size());
    auto err_header = std::string("define[") + def_index + "]: ";
    bool is_ok = true;
    if (name.empty()) {
        _err_list.emplace_back(err_header + "long name is required");
        name = "<empty>";
        is_ok = false;
    }
    if (name == "-" || name == "--" || name.find(' ') != name.npos) {
        _err_list.emplace_back(err_header + "invalid long name '" + name + "'");
        is_ok = false;
    }
    if (name[0] != '-') {
        name = "--" + name;
    } else if (name[1] != '-') {
        name = "-" + name;
    }
    if (_arg_desc_dict.find(name) != _arg_desc_dict.end()) {
        _err_list.emplace_back(err_header + "invalid long name '" + name + "', conflict");
        is_ok = false;
    }
    if (flag == '-') {
        _err_list.emplace_back(err_header + "invalid short name '" + flag + "'");
        is_ok = false;
    }
    auto sname = std::string("-") + flag;
    if (flag && _arg_desc_dict.find(sname) != _arg_desc_dict.end()) {
        _err_list.emplace_back(err_header + "short name '" + flag + "', conflict");
        is_ok = false;
    }
    if (alias == "-" || alias == "--") {
        _err_list.emplace_back(err_header + "invalid alias '" + alias + "'");
        is_ok = false;
    }
    std::string err_alias;
    if (!alias.empty()) {
        if (alias[0] != '-') {
            alias = "--" + alias;
        } else if (alias[1] != '-') {
            alias = "-" + alias;
        }
        err_alias = alias;
        if (alias == name) {
            alias.clear();
        } else if (_arg_desc_dict.find(alias) != _arg_desc_dict.end()) {
            _err_list.emplace_back(err_header + "invalid alias '" + alias + "', conflict");
            err_alias = "<error-" + def_index + ">" + alias;
            is_ok = false;
        }
    }
    std::string err_name = name;
    if (!is_ok) {
        err_name = "<error-" + def_index + ">" + name;
    }
    _arg_desc_list.emplace_back(flag, err_name, desc, attr, err_alias);
    ArgDesc &arg_desc = *_arg_desc_list.rbegin();
    if (flag) {
        _arg_desc_dict[sname] = &arg_desc;
    }
    _arg_desc_dict[name] = &arg_desc;
    if (!alias.empty()) {
        _arg_desc_dict[alias] = &arg_desc;
    }
}

void Parser::print_help(const Result *result
        , const std::string &indent, std::ostream &os) const {
    if (_app_desc.length()) {
        os << indent << _app_desc << "\n";
    }
    auto flags = os.flags();
    unsigned sname_width = 2, lname_width = 0;
    os << indent << "Usage: " << _app_name << " ";
    bool is_first_positional = true;
    for (auto &it : _arg_desc_list) {
        if (lname_width < it.name().length()) {
            lname_width = it.name().length();
        }
        if (it.attr()->is_positional()) {
            if (is_first_positional) {
                is_first_positional = false;
                os << "[";
            } else {
                os << " ";
            }
            os << "<" << it.name().substr(2) << ">";
        }
    }
    if (!is_first_positional) {
        os << "]";
    }
    lname_width += 1;
    os << " ...\n";
    static const std::string name_delimiter = ", ";
    static const int flag_width = std::string("constraint: ").length();
    int name_width = sname_width + lname_width + name_delimiter.length();
    int help_width = _help_width - name_width - flag_width - _help_indent.length() - indent.length();
    auto is_space = [&](char c) { return c == ' ' || c == '\t'; };
    auto print_desc = [&](const std::string &desc, bool concise_help) {
        size_t b = 0;
        while (b < desc.length()) {
            size_t e = desc.find('\n', b);
            if (e == desc.npos) {
                e = desc.length();
            }
            if (help_width > 0 && e > b + help_width) {
                e = b + help_width;
            }
            while (e > b && desc[e] && !is_space(desc[e]) && desc[e] != '\n') {
                --e;
            }
            if (e == b) {
                e = help_width > 0 ? b + help_width : desc.length();
            }
            if (b) {
                os << "\n" << indent << _help_indent
                    << std::setw(name_width) << "";
                if (!concise_help) {
                    os << std::setw(flag_width) << "";
                }
            }
            os << desc.substr(b, e - b);
            b = e + 1;
        }
    };
    for (auto &it : _arg_desc_list) {
        auto attr = it.attr();
        if (attr->is_hidden()) {
            continue;
        }
        auto is_concise_help = attr->is_concise_help() || _concise_help;
        auto &flag = it.flag();
        auto &name = it.name();
        os << indent << _help_indent
            << std::right << std::setw(sname_width) << flag;
        if (flag.empty()) {
            os << std::setw(name_delimiter.length()) << "";
        } else {
            os << name_delimiter;
        }
        os << std::left << std::setw(lname_width) << name;
        print_desc(it.desc(), is_concise_help);
        if (attr->is_positional()) {
            os << " (positional)";
        }
        os << "\n";
        if (!is_concise_help) {
            auto data_type = attr->get_data_type_desc();
            if (!data_type.empty()) {
                os << indent  << _help_indent
                   << std::setw(name_width) << ""
                   << std::setw(flag_width) << std::right << "data type: ";
                print_desc(data_type, is_concise_help);
                os << "\n";
            }
            auto default_value = attr->get_default_value_desc();
            if (!default_value.empty()) {
                os << indent  << _help_indent
                   << std::setw(name_width) << ""
                   << std::setw(flag_width) << std::right << "default: ";
                print_desc(default_value, is_concise_help);
                os << "\n";
            }
            auto implicit_value = attr->get_implicit_value_desc();
            if (!implicit_value.empty()) {
                os << indent  << _help_indent
                   << std::setw(name_width) << ""
                   << std::setw(flag_width) << std::right << "implicit: ";
                print_desc(implicit_value, is_concise_help);
                os << "\n";
            }
            if (attr->has_constraint()) {
                os << indent << _help_indent
                    << std::setw(name_width) << ""
                    << std::setw(flag_width) << std::right << "constraint: ";
                print_desc(attr->get_constraint_desc(), is_concise_help);
                os << "\n";
            }
        }
        auto &alias = it.alias();
        if (!alias.empty()) {
            os << indent << _help_indent
                << std::right << std::setw(sname_width) << "";
            os << std::setw(name_delimiter.length()) << "";
            os << std::left << std::setw(lname_width) << alias;
            std::stringstream ss;
            ss << "same as '";
            if (!flag.empty()) {
                ss << flag << name_delimiter;
            }
            ss << name << "'";
            print_desc(ss.str(), is_concise_help);
            os << "\n";
        }
    }
    if (!_concise_help && (!_err_list.empty() || (result && result->error()))) {
        os << indent << "Error:\n";
        if (result && result->error()) {
            for (auto &it : result->error_details()) {
                os << indent << _help_indent << it << "\n";
            }
        } else {
            for (auto &it : _err_list) {
                os << indent << _help_indent << it << "\n";
            }
        }
    }
    os.flags(flags);
}

Result Parser::parse(int argc, char *argv[], unsigned start_index) {
    if (!_err_list.empty()) {
        std::cerr << "Error: there are defination errors!" << std::endl;
        return Result(this);
    }
    std::unordered_map<std::string, std::shared_ptr<detail::ArgDataI>> result_data;
    std::vector<ArgDesc *> pos_arg_vec;
    pos_arg_vec.reserve(_arg_desc_list.size());
    int i = 0, reserve_size = (argc > (int)start_index) ? (argc - start_index) : 0;
    for (auto &it : _arg_desc_list) {
        if (it.attr()->is_positional()) {
            pos_arg_vec.emplace_back(&it);
        }
        auto arg_data = it.attr()->create_data(reserve_size);
        auto sm = it.attr()->smart_mode();
        if (sm == SmartMode::Gnu) {
            sm = _smart_mode;
        }
        arg_data->set_smart_mode(sm);
        result_data.insert(std::make_pair(it.name(), arg_data));
        ++i;
    }
    size_t pos_arg_idx = 0;
    ArgDesc arg_desc_unknown(0, "", "", value<std::vector<const char *>>()
        ->data_count(0, _smart_mode == SmartMode::Gnu ? -1 : 1));
    auto arg_data_unknown = arg_desc_unknown.attr()->create_data(reserve_size);
    std::string arg_name;
    Result result(this);
    bool after_eof = false;
    i = start_index;
    while (i < argc) {
        auto p = argv[i];
        if (!p) {
            ++i;
            continue;
        }
        ArgDesc *desc = nullptr;
        std::shared_ptr<detail::ArgDataI> arg_data;
        if (pos_arg_idx < pos_arg_vec.size() && pos_arg_vec[pos_arg_idx]) {
            desc = pos_arg_vec[pos_arg_idx];
            arg_name = desc->name();
            arg_data = result_data[arg_name];
        }

        char *binded_data[1] = {nullptr};
        if (arg_data && arg_data->eat_anything()) {
            // do nothing
        } else if (p[0] == '-' && !detail::is_numeric(p + 1)) {
            if (p[1] == '-') {
                if (p[2]) {
                    arg_name = p;
                    auto j = arg_name.find('=');
                    if (j != arg_name.npos) {
                        binded_data[0] = p + j + 1;
                        arg_name = arg_name.substr(0, j);
                    }
                    auto it_desc = _arg_desc_dict.find(arg_name);
                    if (it_desc == _arg_desc_dict.end()) {
                        if (!_allow_unknown) {
                            std::stringstream ss;
                            ss << "usage: arg['" << arg_name << "']: not supported";
                            result.add_error(std::move(ss.str()));
                        }
                        arg_data = arg_data_unknown;
                    } else {
                        desc = it_desc->second;
                        arg_data = result_data[desc->name()];
                    }
                    ++i;
                } else if (after_eof) {
                    result.set_tail(argc - i, argv + i);
                    break;
                } else {
                    ++i;
                    after_eof = true;
                    continue;
                }
            } else {
                int j = 1;
                while (p[j] && p[j] != '=') {
                    arg_name = std::string("-") + p[j];
                    ++j;
                    if (p[j] == '=') {
                        binded_data[0] = p + j + 1;
                    }
                    std::stringstream ss;
                    ss << "usage: arg['" << arg_name << "']";
                    auto it_desc = _arg_desc_dict.find(arg_name);
                    if (it_desc == _arg_desc_dict.end()) {
                        if (!_allow_unknown) {
                            ss << ": not supported";
                            result.add_error(std::move(ss.str()));
                        }
                        arg_data = arg_data_unknown;
                        continue;
                    }
                    desc = it_desc->second;
                    arg_data = result_data[desc->name()];
                    if (!p[j] || p[j] == '=') {
                        break;
                    }
                    std::list<std::string> detail_list;
                    arg_data->appear(nullptr, 0, detail_list);
                    for (auto &it : detail_list) {
                        result.add_error(std::move(ss.str() + it));
                    }
                }
                ++i;
            }
            if (after_eof && (!desc || desc->attr()->smart_mode() != SmartMode::Eof)) {
                --i;
                result.set_tail(argc - i, argv + i);
                break;
            }
        } else if (after_eof) {
            result.set_tail(argc - i, argv + i);
            break;
        }
        after_eof = false;

        if (!desc) {
            std::stringstream ss;
            ss << "usage: arg['" << arg_name << "']";
            if (arg_data) {
                ss << " " << arg_data->appear_count() << "th";
            }
            ss << ": too many value '" << p << "'";
            result.add_error(std::move(ss.str()));
            ++i;
            continue;
        }

        if (desc->attr()->is_positional()) {
            size_t k = 0;
            while (k < pos_arg_vec.size() && desc != pos_arg_vec[k]) ++k;
            if (k < pos_arg_vec.size()) {
                pos_arg_vec[k] = nullptr;
                pos_arg_idx = k;
                while (k < pos_arg_vec.size() && !pos_arg_vec[k]) ++k;
                if (k < pos_arg_vec.size()) {
                    pos_arg_idx = k;
                } else {
                    k = 0;
                    while (k < pos_arg_idx && !pos_arg_vec[k]) ++k;
                    if (k < pos_arg_idx) {
                        pos_arg_idx = k;
                    } else {
                        pos_arg_idx = pos_arg_vec.size();
                    }                
                }
            }
        }

        auto err_header = std::string("usage: arg['") + arg_name + "']";
        std::list<std::string> detail_list;
        if (binded_data[0]) {
            arg_data->appear(binded_data, -1, detail_list);
        } else {
            i += arg_data->appear(argv + i, argc - i, detail_list);
        }
        for (auto &it : detail_list) {
            result.add_error(std::move(err_header + it));
        }
    }

    for (auto &desc : _arg_desc_list) {
        auto arg_name = desc.name();
        auto &arg_data = result_data[arg_name];
        auto err_detail = arg_data->finish();
        if (err_detail.length()) {
            auto &arg_desc = _arg_desc_dict[arg_name];
            std::stringstream ss;
            ss << "usage: arg['";
            auto &flag = arg_desc->flag();
            if (!flag.empty()) {
                ss << flag << ", ";
            }
            ss << arg_name << "']" << err_detail;
            result.add_error(std::move(ss.str()));
        }
        arg_name = arg_name.substr(2);
        result.add_data(arg_name, std::move(ArgData(arg_data, arg_name)));
    }
    return result;
}

} // cliargs

#endif // H_CLIARGS_HPP
