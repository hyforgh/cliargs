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

#ifndef H_CLIARGS_HPP
#define H_CLIARGS_HPP

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits>
#include <string>
#include <vector>
#include <array>
#include <list>
#include <map>
#include <unordered_map>
#include <tuple>
#include <sstream>
#include <memory>
#include <iostream>
#include <iomanip>
#include <functional>
#include <regex>

namespace cliargs {

#define CLIARGS_VERSION_MAJOR 3
#define CLIARGS_VERSION_MINOR 0
#define CLIARGS_VERSION_PATCH 0

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
    ArgParser(char *argv[], int argc, std::list<std::string> &err_list
        , SmartMode smart_mode, void *context);
    virtual ~ArgParser() {}
    virtual void domain_begin(std::string type_name , std::string member_prefix = "."
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
    std::list<std::string> &_err_list;
    SmartMode _smart_mode;
    void *_context;
    int _argi;
    int _vali;
    bool _is_optional;
    int _at_least;
    int _at_most;
    bool _is_terminated;
    std::list<std::array<std::string, 3>> _name_stack;
    std::list<std::pair<std::string, std::string>> _item_traits;
}; // ArgParser

template <typename T>
void cliargs_parse_by_parser(T &val, ArgParser &parser, const std::string &name = "") {
    parser.assign(val, name);
}

template <typename T>
const char *cliargs_parse_by_format(T &var, char *psz, const std::string &var_name
    , std::list<std::string> &err_list, void *context, char *parent);

template <typename T>
std::string to_string(const T &data
        , const std::string &delimiter = ",", const std::string &gap = " "
        , const char *prefix = nullptr, const char *suffix = nullptr);
template <typename... Targs>
std::string to_string(const std::tuple<Targs...> &data
        , const std::string &delimiter = ",", const std::string &gap = " "
        , const char *prefix = nullptr, const char *suffix = nullptr);

template <typename T>
struct type_traits {
    static const std::string &name();
};

#define DEFINE_TYPE_TRAITS_SCALAR(type_, name_, ...)                          \
template <> struct type_traits<type_> {                                       \
    static const std::string &name() {                                        \
        static std::string s_name = name_ __VA_ARGS__;                        \
        return s_name;                                                        \
    }                                                                         \
    typedef std::true_type is_cli_scalar;                                     \
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

#define DEFINE_TYPE_TRAITS_STRING(type_)      \
template <> struct type_traits<type_> {       \
    static const std::string &name() {        \
        static std::string s_name = "string"; \
        return s_name;                        \
    }                                         \
    typedef std::true_type is_cli_string;     \
};
DEFINE_TYPE_TRAITS_STRING(char *)
DEFINE_TYPE_TRAITS_STRING(const char *)
DEFINE_TYPE_TRAITS_STRING(std::string)
#undef DEFINE_TYPE_TRAITS_STRING

template <typename T, std::size_t N>
struct type_traits<std::array<T, N>> {
    static const std::string &name() {
        static const std::string s_name = std::string("array<")
            + type_traits<T>::name() + ", " + std::to_string(N) + ">";
        return s_name;
    }
};
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
    static const std::string &name();
};
template <typename Tkey, typename Tval>
struct type_traits<std::map<Tkey, Tval>> {
    static const std::string &name() {
        static const std::string s_name = std::string("map<")
            + type_traits<Tkey>::name() + ", " + type_traits<Tval>::name() + ">";
        return s_name;
    }
};
template <typename Tkey, typename Tval>
struct type_traits<std::unordered_map<Tkey, Tval>> {
    static const std::string &name() {
        static const std::string s_name = std::string("unordered_map<")
            + type_traits<Tkey>::name() + ", " + type_traits<Tval>::name() + ">";
        return s_name;
    }
};

class Parser;
template <typename T>
class ArgAttr;

namespace detail {

template <typename T>
const char *parse_primary(T &, char *psz) {
    return "unknown";
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
    static const char *type_name = "bool{True,true,1,False,false,0}";
    if (!psz || !psz[0]) {
        return type_name;
    }
    if ((*psz == 'T' || *psz == 't') && (strncmp(psz + 1, "rue\0", 4) == 0)) {
        v = true;
    } else if (strncmp(psz, "1\0", 2) == 0) {
        v = true;
    } else if ((*psz == 'F' || *psz == 'f') && (strncmp(psz + 1, "alse\0", 5) == 0)) {
        v = false;
    } else if (strncmp(psz, "0\0", 2) == 0) {
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

inline bool is_digital(const char *p) {
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

template <typename T>
struct is_cli_primary {
    static constexpr bool value = is_cli_scalar<T>::value || is_cli_string<T>::value;
};

template <typename T>
struct is_cli_custom {
    static constexpr bool value = !is_cli_primary<T>::value \
        && !is_cli_container<T>::value && !is_stl_tuple<T>::value;
};

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

template <typename T, typename Tval>
struct to_string_t {
    static std::string from(const T &data
            , const std::string &delimiter, const std::string &gap
            , const char *prefix = nullptr, const char *suffix = nullptr) {
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
template <typename T>
struct to_string_t<T, T> {
    static std::string from(const T &value
            , const std::string &delimiter, const std::string &gap
            , const char *prefix = nullptr, const char *suffix = nullptr) {
        std::stringstream ss;
        if (prefix) {
            ss << prefix;
        }
        if (std::is_base_of<const std::string, T>::value
                || std::is_convertible<T, const char *>::value) {
            ss << "\"";
        }
        ss << value;
        if (std::is_base_of<const std::string, T>::value
                || std::is_convertible<T, const char *>::value) {
            ss << "\"";
        }
        if (suffix) {
            ss << suffix;
        }
        return ss.str();
    }
};
template <typename Tkey, typename Tval>
struct to_string_t<std::pair<Tkey, Tval>, std::pair<Tkey, Tval>> {
    static std::string from(const std::pair<Tkey, Tval> &value
            , const std::string &delimiter, const std::string &gap
            , const char *prefix = nullptr, const char *suffix = nullptr) {
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
        , const char *prefix, const char *suffix) {
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
struct to_string_t<std::map<Tkey, Tval>, std::pair<const Tkey, Tval>> {
    static std::string from(const std::map<Tkey, Tval> &value
            , const std::string &delimiter, const std::string &gap
            , const char *prefix = nullptr, const char *suffix = nullptr) {
        return map_to_string(value, delimiter, gap, prefix, suffix);
    }
};
template <typename Tkey, typename Tval>
struct to_string_t<std::unordered_map<Tkey, Tval>, std::pair<const Tkey, Tval>> {
    static std::string from(const std::unordered_map<Tkey, Tval> &value
            , const std::string &delimiter, const std::string &gap
            , const char *prefix = nullptr, const char *suffix = nullptr) {
        return map_to_string(value, delimiter, gap, prefix, suffix);
    }
};
template <std::size_t N, typename... Targs>
struct tuple_to_string_t;
template <std::size_t N, typename... Targs>
struct tuple_to_string_t<N, std::tuple<Targs...>> {
    static std::string from(const std::tuple<Targs...> &data
            , const std::string &delimiter, const std::string &gap) {
        return tuple_to_string_t<N - 1, std::tuple<Targs...>>::from(
                data, delimiter, gap)
            + delimiter + gap
            + to_string(std::get<N - 1>(data), delimiter, gap);
    }
};
template <typename... Targs>
struct tuple_to_string_t<1, std::tuple<Targs...>> {
    static std::string from(const std::tuple<Targs...> &data
        , const std::string &delimiter, const std::string &gap) {
        return to_string(std::get<0>(data), delimiter, gap);
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

template <typename T>
struct get_implicit_value_type {
    typedef typename get_cli_level_type<T>::mid_type type;
};
template <typename Tval>
struct get_implicit_value_type<std::vector<Tval>> {
    typedef std::vector<Tval> type;
};
template <typename Tval>
struct get_implicit_value_type<std::vector<std::vector<Tval>>> {
    typedef std::vector<Tval> type;
};
template <typename... Ts>
struct get_implicit_value_type<std::vector<std::tuple<Ts...>>> {
    typedef std::tuple<Ts...> type;
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

struct ParseResult {
    int argi;
    int vali;
    bool is_terminated;
};

class ArgParserImpl : public ArgParser {
public:
    ArgParserImpl(char *argv[], int argc, std::list<std::string> &err_list
                , SmartMode smart_mode, void *context)
            : ArgParser(argv, argc, err_list, smart_mode, context) {
    }
    ParseResult submit(const std::string &name) {
        return ParseResult {_argi, _vali, _is_terminated};
    }
    int at_least() const {
        return _at_least;
    }
    int at_most() const {
        return _at_most;
    }
    std::string type_name() const {
        std::stringstream ss;
        ss << "{";
        int i = 0;
        for (auto &it : _item_traits) {
            if (i >= _at_least && _is_optional) {
                ss << "[";
            }
            if (i) {
                ss << ", ";
            }
            ss << it.first;
            ++i;
        }
        for (i = 0; i < (int)_item_traits.size(); ++i) {
            if (i >= _at_least && _is_optional) {
                ss << "]";
            }
        }
        ss << "}";
        return ss.str();
    }
}; // ArgParserImpl

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
    virtual std::string get_data_desc() const = 0;
    virtual bool is_hidden() const = 0;
    virtual bool is_concise_help() const = 0;
    virtual SmartMode smart_mode() const = 0;
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
    explicit ArgAttrT()
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
        _match_examine_func = [&func](Tval &v, void *, void *) { return func(v); };
        _match_examine_desc = std::move(desc);
        return std::static_pointer_cast<ArgAttr<T>>(shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> examine(
            std::function<bool (Tval &, void *context)> func, std::string desc = "") {
        _match_examine_func = [&func](Tval &v, void *context, void *) { return func(v, context); };
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
    typename std::enable_if<(detail::is_stl_vector<Tmid_>::value || detail::is_stl_vector<Ttop_>::value) \
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
    std::string get_data_desc() const override {
        if (hide_data_desc()) {
            return "";
        }
        std::stringstream ss;
        ss << "'" << type_traits<T>::name() << "'";
        auto print_range = [&ss](unsigned at_least, unsigned at_most) {
            ss << "[" << at_least << "~";
            if (at_most == INT32_MAX) {
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
        if (has_default_value()) {
            ss << "; default:" << to_string(get_default_value());
        }
        if (has_implicit_value()) {
            ss << "; implicit:" << to_string(get_implicit_value());
        }
        return ss.str();
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
class ArgAttrTval : public ArgAttrT<T> {};
template <typename T>
class ArgAttrTval<T, typename std::enable_if<is_cli_scalar<typename get_cli_level_type<T>::val_type>::value>::type>
        : public ArgAttrT<T> {
    typedef typename ArgAttrT<T>::Tval Tval;
public:
    ArgAttrTval() : ArgAttrT<T>() {}
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
    ArgAttrTval() : ArgAttrT<T>() {}
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
};

template <typename Tval>
struct get_capacity {
    static int at_most(SmartMode smart_mode) {
        static auto s_at_most = [](SmartMode smart_mode) -> int {
            std::list<std::string> err_tmp;
            ArgParserImpl parser(nullptr, 0, err_tmp, smart_mode, nullptr);
            Tval arg_value;
            cliargs_parse_by_parser(arg_value, parser, "");
            return parser.at_most();
        }(smart_mode);
        return s_at_most;
    }
};

template <typename Ttop>
struct DataParser {
    static ParseResult parse(Ttop &value, char *argv[], int argc
            , std::list<std::string> &err_list, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(Ttop &, void *)> examine
            , std::function<const typename get_implicit_value_type<Ttop>::type &()> get_implicit_value
            , const std::string &name = ""
            ) {
        std::list<std::string> err_tmp;
        ArgParserImpl parser(argv, argc, err_tmp, smart_mode, context);
        if (at_least < 1) {
            parser.set_optional();
        }
        cliargs_parse_by_parser(value, parser, name);
        auto ret = parser.submit(name);
        if (ret.vali || at_least > 0) {
            err_list.splice(err_list.end(), err_tmp);
        } else if (get_implicit_value) {
            value = get_implicit_value();
            ret.vali = 1;
        } else {
            err_list.splice(err_list.end(), err_tmp);
            return ret;
        }
        if ((ret.vali || get_implicit_value) && examine) {
            auto err_detail = examine(value, arg_data);
            if (!err_detail.empty()) {
                static auto item_at_most = get_capacity<Ttop>::at_most(smart_mode);
                std::stringstream ss;
                ss << "invalid value";
                if (item_at_most > 1) {
                    ss << " group '" << to_string(std::vector<char *>(argv, argv + ret.vali))
                       << " (as type " << type_traits<Ttop>::name() << ")";
                } else {
                    ss << " '" << *argv << "'";
                }
                ss << ", " << err_detail;
                err_list.emplace_back(ss.str());
            }
        }
        return ret;
    }
};
template <typename... Targs>
struct DataParser<std::tuple<Targs...>> {
    static ParseResult parse(std::tuple<Targs...> &value, char *argv[], int argc
            , std::list<std::string> &err_list, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name = "tuple"
            );
};
template <typename Tmid>
struct DataParser<std::vector<Tmid>> {
    typedef std::vector<Tmid> Ttop;
    typedef typename get_cli_value_type<Tmid>::type Tval;
    static ParseResult parse(Ttop &value, char *argv[], int argc
            , std::list<std::string> &err_list, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(Tval &, void *)> examine
            , std::function<const typename get_implicit_value_type<Ttop>::type &()> get_implicit_value
            , const std::string &name = "vector"
            );
};
template <typename Tmap, typename Tkey, typename Ttop>
struct MapParser {
    static ParseResult parse(Tmap &value, char *argv[], int argc
            , std::list<std::string> &err_list, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(typename get_cli_level_type<Ttop>::val_type &, void *)> examine
            , std::function<const typename get_implicit_value_type<Ttop>::type &()> implicit_value
            , const std::string &name = "map"
            );
};
template <typename Tkey, typename Ttop>
struct DataParser<std::map<Tkey, Ttop>> : MapParser<std::map<Tkey, Ttop>, Tkey, Ttop> {};
template <typename Tkey, typename Ttop>
struct DataParser<std::unordered_map<Tkey, Ttop>> : MapParser<std::unordered_map<Tkey, Ttop>, Tkey, Ttop> {};
template <std::size_t N, typename... Targs>
struct TupleParser {
    static ParseResult parse(std::tuple<Targs...> &value, char *argv[], int argc
            , std::list<std::string> &err_list, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name
            ) {
        ParseResult result;
        if (N) {
            result = TupleParser<N - 1, Targs...>::parse(value, argv, argc, err_list, smart_mode
                , context, arg_data, at_least, at_most, nullptr, get_implicit_value, name);
            if (result.is_terminated) {
                return result;
            }
        }
        std::list<std::string> err_tmp;
        auto ret = DataParser<typename std::tuple_element<N, std::tuple<Targs...>>::type>::parse(
            std::get<N>(value), argv + result.argi
            , ((result.argi >= (int)N && argc > (int)N) ? argc - result.argi : 0)
            , err_tmp, smart_mode
            , context, arg_data, (at_least > N ? 1 : 0), 1, nullptr, nullptr
            , name + "<" + to_string(N) + ">");
        if (ret.argi || at_least > N) {
            err_list.splice(err_list.end(), err_tmp);
        } else if (get_implicit_value) {
            std::get<N>(value) = std::get<N>(get_implicit_value());
        }
        result.argi += ret.argi;
        result.vali += ret.vali;
        result.is_terminated = ret.is_terminated;
        return result;
    }
};
template <typename... Targs>
struct TupleParser<0, Targs...> {
    static ParseResult parse(std::tuple<Targs...> &value, char *argv[], int argc
            , std::list<std::string> &err_list, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name
            ) {
        std::list<std::string> err_tmp;
        auto ret = DataParser<typename std::tuple_element<0, std::tuple<Targs...>>::type>::parse(
            std::get<0>(value), argv, argc, err_tmp, smart_mode
            , context, arg_data, (at_least > 0 ? 1 : 0), 1, nullptr, nullptr, name + "<0>"
            );
        if (ret.vali || at_least > 0) {
            err_list.splice(err_list.end(), err_tmp);
        } else if (get_implicit_value) {
            std::get<0>(value) = std::get<0>(get_implicit_value());
        }
        return ret;
    }
};
template <typename... Targs>
ParseResult DataParser<std::tuple<Targs...>>::parse(std::tuple<Targs...> &value, char *argv[], int argc
        , std::list<std::string> &err_list, SmartMode smart_mode
        , void *context, void *arg_data, unsigned at_least, unsigned at_most
        , std::function<std::string(std::tuple<Targs...> &, void *)> examine
        , std::function<const std::tuple<Targs...> &()> get_implicit_value
        , const std::string &name
        ) {
    std::list<std::string> err_tmp;
    auto ret = TupleParser<sizeof ...(Targs) - 1, Targs...>::parse(
        value, argv, argc, err_tmp, smart_mode
        , context, arg_data, at_least, at_most, examine, get_implicit_value
        , (name.empty() ? std::string("tuple") : name)
        );
    if (err_tmp.empty()) {
        if (examine) {
            auto err_detail = examine(value, arg_data);
            if (!err_detail.empty()) {
                err_list.emplace_back(std::move(err_detail));
            }
        }
    } else {
        err_list.splice(err_list.end(), err_tmp);
    }
    return ParseResult{ret.argi, ret.vali ? 1 : 0, ret.is_terminated};
}
template <typename Tmid, typename Tval>
struct VectorParser {
    static ParseResult parse(std::vector<Tmid> &value, char *argv[], int argc
            , std::list<std::string> &err_list, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(Tval &, void *)> examine
            , std::function<const typename get_implicit_value_type<std::vector<Tmid>>::type &()> get_implicit_value
            , const std::string &name
            ) {
        Tmid arg_value;
        auto ret = DataParser<Tmid>::parse(arg_value, argv, argc, err_list, smart_mode
            , context, arg_data, at_least, at_most, examine, get_implicit_value, name);
        value.emplace_back(std::move(arg_value));
        return ParseResult{ret.argi, (int)value.size(), ret.is_terminated};
    }
};
template <typename Tval>
struct VectorParser<Tval, Tval> {
    static ParseResult parse(std::vector<Tval> &value, char *argv[], int argc
            , std::list<std::string> &err_list, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(Tval &, void *)> examine
            , std::function<const typename get_implicit_value_type<std::vector<Tval>>::type &()> get_implicit_value
            , const std::string &name
            ) {
        static auto item_at_most = get_capacity<Tval>::at_most(smart_mode);
        int i = 0;
        unsigned n = 0;
        bool is_terminated = false;
        while (n < at_most && value.size() < at_most) {
            Tval arg_value;
            std::list<std::string> err_tmp;
            auto ret = DataParser<Tval>::parse(arg_value, argv + i, argc - i, err_tmp, smart_mode
                , context, arg_data, (n < at_least ? 1 : 0), 1, examine, nullptr
                , (name + "[" + to_string(value.size()) + "]"));
            i += ret.argi;
            is_terminated = ret.is_terminated;
            err_list.splice(err_list.end(), err_tmp);
            if (!ret.vali) {
                break;
            }
            value.emplace_back(std::move(arg_value));
            ++n;
            if (item_at_most > 1 || ret.is_terminated) {
                break;
            }
        }
        if (value.size() >= at_most) {
            return ParseResult{i, (int)value.size(), is_terminated};
        }
        if (n >= at_least && get_implicit_value) {
            auto &implicit_value = get_implicit_value();
            for (; n < at_most && n < implicit_value.size(); ++n) {
                value.emplace_back(implicit_value[n]);
            }
        }
        return ParseResult{i, (int)value.size(), is_terminated};
    }
};
template <typename... Targs>
struct VectorParser<std::tuple<Targs...>, std::tuple<Targs...>> {
    static ParseResult parse(std::vector<std::tuple<Targs...>> &value, char *argv[], int argc
            , std::list<std::string> &err_list, SmartMode smart_mode
            , void *context, void *arg_data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name
            ) {
        std::tuple<Targs...> arg_value;
        auto ret = DataParser<std::tuple<Targs...>>::parse(arg_value, argv, argc, err_list, smart_mode
            , context, arg_data, at_least, at_most, examine, get_implicit_value
            , name + "[" + to_string(value.size()) + "]");
        value.emplace_back(std::move(arg_value));
        return ParseResult{ret.argi, ret.vali ? 1 : 0, ret.is_terminated};
    }
};
template <typename Tmid>
ParseResult DataParser<std::vector<Tmid>>::parse(std::vector<Tmid> &value, char *argv[], int argc
        , std::list<std::string> &err_list, SmartMode smart_mode
        , void *context, void *arg_data, unsigned at_least, unsigned at_most
        , std::function<std::string(Tval &, void *)> examine
        , std::function<const typename get_implicit_value_type<Ttop>::type &()> get_implicit_value
        , const std::string &name
        ) {
    return VectorParser<Tmid, Tval>::parse(value, argv, argc, err_list, smart_mode, context, arg_data
        , at_least, at_most, examine, get_implicit_value, name);
}
template <typename Tmap, typename Tkey, typename Tmid, typename Tval>
struct MapInserter;
template <typename Tmap, typename Tkey, typename Tval>
struct MapInserter<Tmap, Tkey, Tval, Tval> {
    static void insert(Tmap &value, const std::string &name
            , Tkey &map_key, Tval &map_value, std::list<std::string> &err_list) {
        if (value.find(map_key) != value.end()) {
            err_list.emplace_back("repeated " + name + ".key '" + to_string(map_key) + "'");
        } else {
            value.insert(std::make_pair(std::move(map_key), std::move(map_value)));
        }
    }
};
template <typename Tmap, typename Tkey, typename Tval>
struct MapInserter<Tmap, Tkey, std::vector<Tval>, Tval> {
    static void insert(Tmap &value, const std::string &name
            , Tkey &map_key, std::vector<Tval> &map_value, std::list<std::string> &err_list) {
        auto &item = value[std::move(map_key)];
        item.insert(item.end(), map_value.begin(), map_value.end());
    }
};
template <typename Tmap, typename Tkey, typename Ttop>
ParseResult MapParser<Tmap, Tkey, Ttop>::parse(Tmap &value, char *argv[], int argc
        , std::list<std::string> &err_list, SmartMode smart_mode
        , void *context, void *arg_data, unsigned at_least, unsigned at_most
        , std::function<std::string(typename get_cli_level_type<Ttop>::val_type &, void *)> examine
        , std::function<const typename get_implicit_value_type<Ttop>::type &()> implicit_value
        , const std::string &name
        ) {
    if (argc < 1) {
        std::stringstream ss;
        ss << "a(n) '" << type_traits<Tkey>::name() << "' value is required as '" + name + ".key'";
        err_list.emplace_back(ss.str());
        return ParseResult{0, 0, true};
    }
    int i = 0;
    Tkey map_key;
    std::list<std::string> err_key;
    auto ret = DataParser<Tkey>::parse(map_key, argv + i, argc - i, err_key, smart_mode
        , context, arg_data, 1, 1, nullptr, nullptr, name + ".key");
    i += ret.argi;
    if (!err_key.empty()) {
        err_list.splice(err_list.end(), err_key);
    }
    Ttop map_value;
    std::list<std::string> err_value;
    ret = DataParser<Ttop>::parse(map_value, argv + i, ret.is_terminated ? 0 : (argc - i)
        , err_value, smart_mode
        , context, arg_data, at_least, at_most, examine, implicit_value
        , name + "[" + to_string(map_key) + "]");
    i += ret.argi;
    if (!err_value.empty()) {
        err_list.splice(err_list.end(), err_value);
        return ParseResult{i, 0, ret.is_terminated};
    }
    if (err_key.empty()) {
        MapInserter<Tmap, Tkey, Ttop, typename get_cli_value_type<Ttop>::type>::insert(
            value, name, map_key, map_value, err_list);
    }
    return ParseResult{i, 1, ret.is_terminated};
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
    std::list<std::string> err_tmp;
    auto err_header = " " + to_string(_appear_count) + "th: ";
    if (dim_0_at_most > 1 && _appear_count > dim_0_at_most) {
        std::stringstream ss;
        ss << "too many appearances";
        Ttop v_tmp;
        auto ret = DataParser<Ttop>::parse(v_tmp, argv, argc, err_tmp, _smart_mode
            , _arg_attr.get_context(), &v_tmp, dim_1_at_least, dim_1_at_most
            , nullptr, nullptr);
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
    auto ret = DataParser<T>::parse(_data, argv, argc, err_tmp, _smart_mode
        , _arg_attr.get_context(), &_data, dim_1_at_least, dim_1_at_most
        , [this](Tval &value, void *arg_data) { return this->_arg_attr.examine(value, arg_data); }
        , func_implicit_value
        );
    for (auto &it : err_tmp) {
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
const char *cliargs_parse_by_format(T &var, char *psz, const std::string &var_name
        , std::list<std::string> &err_list, void *context, char *parent) {
    auto err_type_name = detail::parse_primary(var, psz);
    if (err_type_name) {
        std::stringstream ss;
        ss << "format error: '" << (parent ? parent : psz)
           << "', expect a(n) '" << err_type_name << "' value";
        if (var_name.length()) {
            ss << " for '" << var_name << "'";
        }
        err_list.emplace_back(ss.str());
    }
    return err_type_name;
}

template <typename T>
std::string to_string(const T &data
        , const std::string &delimiter, const std::string &gap
        , const char *prefix, const char *suffix) {
    return detail::to_string_t<T, typename detail::get_stl_value_type<T>::type>::from(
        data, delimiter, gap, prefix, suffix);
}
template <typename... Targs>
std::string to_string(const std::tuple<Targs...> &data
        , const std::string &delimiter, const std::string &gap
        , const char *prefix, const char *suffix) {
    return (prefix ? std::string(prefix) : std::string("("))
        + detail::tuple_to_string_t<std::tuple_size<std::tuple<Targs ...>>::value
            , std::tuple<Targs ...>>::from(data, delimiter, gap)
        + (suffix ? std::string(suffix) : std::string(")"));
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
        std::list<std::string> err_tmp;
        detail::ArgParserImpl parser(nullptr, 0, err_tmp, SmartMode::Gnu, nullptr);
        T arg_value;
        cliargs_parse_by_parser(arg_value, parser, "");
        return parser.type_name();
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

ArgParser::ArgParser(char *argv[], int argc, std::list<std::string> &err_list
            , SmartMode smart_mode, void *context)
        : _argc(argc), _argv(argv), _err_list(err_list)
        , _smart_mode(smart_mode), _context(context)
        , _argi(0), _vali(0), _is_optional(false), _at_least(0), _at_most(0)
        , _is_terminated(false) {
}
void ArgParser::domain_begin(std::string type_name
        , std::string member_name_prefix
        , std::string member_name_suffix) {
    _name_stack.push_back({std::move(type_name)
        , std::move(member_name_prefix)
        , std::move(member_name_suffix)});
}
void ArgParser::domain_end() {
    _name_stack.pop_back();
}
template <typename T>
bool ArgParser::assign(T &value, const std::string &name, T default_value) {
    static const char *type_name = [this, &name]() -> const char * {
        T arg_value;
        std::list<std::string> err_tmp;
        const char *tn = cliargs_parse_by_format(arg_value, nullptr, name, err_tmp, get_context(), nullptr);
        if (reinterpret_cast<long>(tn) == -1l) {
            return type_traits<T>::name().c_str();
        }
        return tn ? tn : "";
    }();
    _item_traits.emplace_back(std::make_pair(type_name, name));
    ++_at_most;
    char *psz = nullptr;
    if (_argi < _argc && _at_most <= _argc) {
        psz = _argv[_argi];
        while (psz == nullptr && _argi < _argc) {
            ++_argi;
            psz = _argv[_argi];
        }
    }
    if (!_is_terminated && psz != nullptr && _at_most <= _argc                                              \
            && (psz[0] != '-'                                                         /*normal string*/     \
                || detail::is_digital(psz + 1)                                        /*negative numberic*/ \
                || (_smart_mode == SmartMode::Gnu && detail::is_cli_string<T>::value) /*force assigning*/   \
                || (_smart_mode == SmartMode::Eof && (psz[1] != '-' || psz[2]))       /*stop at end: '--'*/ \
            )) {
        ++_argi;
    } else {
        _is_terminated = true;
        if (_is_optional) {
            value = std::move(default_value);
        } else {
            std::stringstream ss;
            ss << "a(n) '" << type_name << "' value is required";
            if (!name.empty()) {
                ss << " as '" << concat_name(name) << "'";
            }
            _err_list.emplace_back(ss.str());
        }
        return false;
    }
    ++_vali;
    // maybe psz is a degital
    std::list<std::string> err_tmp;
    if (_smart_mode != SmartMode::Gnu && psz[0] == '\\') {
        ++psz;
    }
    cliargs_parse_by_format(value, psz, concat_name(name), err_tmp, get_context(), psz);
    if (err_tmp.size()) {
        _err_list.splice(_err_list.end(), err_tmp);
        return false;
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
    if (_name_stack.empty()) {
        return name;
    }
    std::stringstream ss;
    auto it = _name_stack.begin();
    if (it != _name_stack.end()) {
        ss << (*it)[0];
        while (it !=  _name_stack.end()) {
            auto it_next = it;
            ++it_next;
            if (it_next != _name_stack.end()) {
                ss << (*it)[1] << (*it_next)[0] << (*it)[2];
                ++it;
            } else {
                break;
            }
        }
    }
    if (!name.empty()) {
        ss << (*it)[1]  << name << (*it)[2];
    }
    return ss.str();
}

template <typename T>
class ArgAttr : public detail::ArgAttrTval<T> {};

template <typename T>
std::shared_ptr<ArgAttr<T>> value() {
    return std::make_shared<ArgAttr<T>>();
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
        auto arg_data = dynamic_cast<const detail::ArgDataT<T> *>(_arg_data.get());
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
    auto err_header = std::string("define[") + to_string(_arg_desc_list.size()) + "]:";
    if (name.empty()) {
        _err_list.emplace_back(err_header + "long name is required");
        return;
    }
    if (name == "-" || name == "--" || name.find(' ') != name.npos) {
        _err_list.emplace_back(err_header + "invalid long name '" + name + "'");
        return;
    }
    if (name[0] != '-') {
        name = "--" + name;
    } else if (name[1] != '-') {
        name = "-" + name;
    }
    if (_arg_desc_dict.find(name) != _arg_desc_dict.end()) {
        _err_list.emplace_back(err_header + "invalid long name '" + name + "', conflict");
        return;
    }
    if (flag == '-') {
        _err_list.emplace_back(err_header + "invalid short name '" + flag + "'");
        return;
    }
    auto sname = std::string("-") + flag;
    if (flag && _arg_desc_dict.find(sname) != _arg_desc_dict.end()) {
        _err_list.emplace_back(err_header + "short name '" + flag + "', conflict");
        return;
    }
    if (alias == "-" || alias == "--") {
        _err_list.emplace_back(err_header + "invalid alias '" + alias + "'");
        return;
    }
    if (!alias.empty()) {
        if (alias[0] != '-') {
            alias = "--" + alias;
        } else if (alias[1] != '-') {
            alias = "-" + alias;
        }
        if (alias == name) {
            alias.clear();
        } else if (_arg_desc_dict.find(alias) != _arg_desc_dict.end()) {
            _err_list.emplace_back(err_header + "invalid alias '" + alias + "', conflict");
            return;
        }
    }
    _arg_desc_list.emplace_back(flag, name, desc, attr, alias);
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
            auto data_desc = attr->get_data_desc();
            if (!data_desc.empty()) {
                os << indent  << _help_indent
                    << std::setw(name_width) << ""
                        << std::setw(flag_width) << std::right << "value: ";
                print_desc(data_desc, is_concise_help);
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
        print_help(nullptr);
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
        } else if (p[0] == '-' && !detail::is_digital(p + 1)) {
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
            if (after_eof && desc->attr()->smart_mode() != SmartMode::Eof) {
                --i;
                result.set_tail(argc - i, argv + i);
                break;
            }
        } else {
            if (after_eof) {
                result.set_tail(argc - i, argv + i);
                break;
            }
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
        }
        after_eof = false;

        if (desc && desc->attr()->is_positional()) {
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
