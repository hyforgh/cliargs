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

#ifndef __HY_CLIARGS_HPP__
#define __HY_CLIARGS_HPP__

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
#include <set>
#include <unordered_set>
#include <tuple>
#include <sstream>
#include <memory>
#include <iostream>
#include <iomanip>
#include <functional>
#include <regex>

namespace cliargs {

#define CLIARGS_VERSION_MAJOR 1
#define CLIARGS_VERSION_MINOR 0
#define CLIARGS_VERSION_PATCH 0

class ArgParser {
public:
    virtual ~ArgParser() {}
    virtual void domain_begin(std::string type_name , std::string member_prefix = "."
        , std::string member_suffix = "") = 0;
    virtual void domain_end() = 0;
    template <typename Tm>
    bool assign(Tm &value, const std::string &name, Tm default_value = Tm());
    virtual void check(bool is_true, std::string msg) = 0;
    virtual void set_optional() = 0;
    virtual void *get_context() const = 0;
protected:
    virtual char *alloc(const std::string &name, const char *type) = 0;
    virtual std::string concat_name(const std::string &name) const = 0;
    virtual void append_errors(std::list<std::string> &err_list) = 0;
    virtual void discontinue() = 0;
    virtual bool is_optional() const = 0;
}; // ArgParser

template <typename T>
void __parse_by_parser(T &val, ArgParser &parser, const std::string &name = "") {
    parser.assign(val, name);
}

template <typename T>
const char *__parse_by_format(T &var, char *psz, const std::string &var_name
    , std::list<std::string> &err_list, void *context, char *parent);

template <typename T>
struct type_traits {
    static const std::string &name();
};
template <> struct type_traits<char *> {
    static const std::string &name() {
        static std::string s_name = "char *";
        return s_name;
    }
};
template <> struct type_traits<const char *> {
    static const std::string &name() {
        static std::string s_name = "const char *";
        return s_name;
    }
};
template <> struct type_traits<std::string> {
    static const std::string &name() {
        static std::string s_name = "string";
        return s_name;
    }
};
template <> struct type_traits<bool> {
    static const std::string &name() {
        static std::string s_name = "bool";
        return s_name;
    }
};
template <> struct type_traits<float> {
    static const std::string &name() {
        static std::string s_name = "float";
        return s_name;
    }
};
template <> struct type_traits<double> {
    static const std::string &name() {
        static std::string s_name = "double";
        return s_name;
    }
};
template <> struct type_traits<long long> {
    static const std::string &name() {
        static std::string s_name = "long long";
        return s_name;
    }
};
template <> struct type_traits<unsigned long long> {
    static const std::string &name() {
        static std::string s_name = "unsigned long long";
        return s_name;
    }
};
template <> struct type_traits<long> {
    static const std::string &name() {
        static std::string s_name = "long";
        return s_name;
    }
};
template <> struct type_traits<unsigned long> {
    static const std::string &name() {
        static std::string s_name = "unsigned long";
        return s_name;
    }
};
template <> struct type_traits<int> {
    static const std::string &name() {
        static std::string s_name = "int";
        return s_name;
    }
};
template <> struct type_traits<unsigned int> {
    static const std::string &name() {
        static std::string s_name = "unsigned int";
        return s_name;
    }
};
template <> struct type_traits<short> {
    static const std::string &name() {
        static std::string s_name = "short";
        return s_name;
    }
};
template <> struct type_traits<unsigned short> {
    static const std::string &name() {
        static std::string s_name = "unsigned short";
        return s_name;
    }
};
template <> struct type_traits<signed char> {
    static const std::string &name() {
        static std::string s_name = "signed char";
        return s_name;
    }
};
template <> struct type_traits<unsigned char> {
    static const std::string &name() {
        static std::string s_name = "unsigned char";
        return s_name;
    }
};
template <> struct type_traits<char> {
    static const std::string &name() {
        static std::string s_name = "char";
        return s_name;
    }
};
template <typename T, std::size_t N>
struct type_traits<std::array<T, N>> {
    static const std::string &name();
};
template <typename T>
struct type_traits<std::vector<T>> {
    static const std::string &name();
};
template <typename Tk, typename Ta>
struct type_traits<std::pair<Tk, Ta>> {
    static const std::string &name();
};
template <typename Tk, typename Ta>
struct type_traits<std::map<Tk, Ta>> {
    static const std::string &name();
};
template <typename Tk, typename Ta>
struct type_traits<std::unordered_map<Tk, Ta>> {
    static const std::string &name();
};
template <typename T>
struct type_traits<std::set<T>> {
    static const std::string &name();
};
template <typename T>
struct type_traits<std::unordered_set<T>> {
    static const std::string &name();
};
template <std::size_t N, typename... Targs>
struct ____cli_tuple {
    static const std::string &name() {
        static const std::string s_name = ____cli_tuple<N - 1, Targs...>::name() + ", "
            + type_traits<typename std::tuple_element<N - 1, std::tuple<Targs...>>::type>::name();
        return s_name;
    }
};
template <typename... Targs>
struct ____cli_tuple<1, Targs...> {
    static const std::string &name() {
        return type_traits<typename std::tuple_element<0, std::tuple<Targs...>>::type>::name();
    }
};
template <typename... Targs>
struct type_traits<std::tuple<Targs...>> {
    static const std::string &name();
};
template <typename T, std::size_t N>
const std::string &type_traits<std::array<T, N>>::name() {
    static const std::string s_name = std::string("array<")
        + type_traits<T>::name() + ", " + std::to_string(N) + ">";
    return s_name;
}
template <typename T>
const std::string &type_traits<std::vector<T>>::name() {
    static const std::string s_name = std::string("vector<")
        + type_traits<T>::name() + ">";
    return s_name;
}
template <typename Tk, typename Ta>
const std::string &type_traits<std::map<Tk, Ta>>::name() {
    static const std::string s_name = std::string("map<")
        + type_traits<Tk>::name() + ", " + type_traits<Ta>::name() + ">";
    return s_name;
}
template <typename Tk, typename Ta>
const std::string &type_traits<std::unordered_map<Tk, Ta>>::name() {
    static const std::string s_name = std::string("unordered_map<")
        + type_traits<Tk>::name() + ", " + type_traits<Ta>::name() + ">";
    return s_name;
}
template <typename T>
const std::string &type_traits<std::set<T>>::name() {
    static const std::string s_name = std::string("set<")
        + type_traits<T>::name() + ">";
    return s_name;
}
template <typename T>
const std::string &type_traits<std::unordered_set<T>>::name() {
    static const std::string s_name = std::string("unordered_set<")
        + type_traits<T>::name() + ">";
    return s_name;
}
template <typename... Targs>
const std::string &type_traits<std::tuple<Targs...>>::name() {
    static const std::string s_name = std::string("tuple<")
        + ____cli_tuple<sizeof ...(Targs), Targs...>::name() + ">";
    return s_name;
}

template <typename T>
const char *__parse_atom(T &, char *psz) {
    return reinterpret_cast<const char *>(-1l);
}
const char *__parse_atom(char *&v, char *psz) {
    if (!psz) {
        return type_traits<char *>::name().c_str();
    }
    v = psz;
    return nullptr;
}
const char *__parse_atom(const char *&v, char *psz) {
    if (!psz) {
        return type_traits<const char *>::name().c_str();
    }
    v = psz;
    return nullptr;
}
const char *__parse_atom(std::string &v, char *psz) {
    if (!psz) {
        return type_traits<std::string>::name().c_str();
    }
    v = psz;
    return nullptr;
}
const char *__parse_atom(bool &v, char *psz) {
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
const char *__parse_atom(float &v, char *psz) {
    static const char *type_name = type_traits<float>::name().c_str();
    if (!psz || !psz[0]) {
        return type_name;
    }
    char *pend = nullptr;
    v = std::strtof(psz, &pend);
    return *pend == '\0' ? nullptr : type_name;
}
const char *__parse_atom(double &v, char *psz) {
    static const char *type_name = type_traits<double>::name().c_str();
    if (!psz || !psz[0]) {
        return type_name;
    }
    char *pend = nullptr;
    v = std::strtod(psz, &pend);
    return *pend == '\0' ? nullptr : type_name;
}
template <typename Tv, typename Tconvert_method>
const char *____parse_integer(Tv &v, char *psz, Tconvert_method convert_method) {
    static const char *type_name = type_traits<Tv>::name().c_str();
    if (!psz || !psz[0]) {
        return type_name;
    }
    char *pend = nullptr;
    if (strncmp(psz, "0x", 2) == 0 || strncmp(psz, "0X", 2) == 0) {
        v = convert_method(psz, &pend, 16);
    } else {
        v = convert_method(psz, &pend, 10);
    }
    if (*pend == '\0') {
        static auto v_min = std::numeric_limits<Tv>::min();
        static auto v_max = std::numeric_limits<Tv>::max();
        if (v < v_min || v > v_max) {
            static std::string s_type_name = [&]() {
                std::stringstream ss;
                ss << type_name << "(" << v_min << "~" << v_max << ")";
                return ss.str();
            }();
            return s_type_name.c_str();
        }
        return nullptr;
    }
    return type_name;
}
const char *__parse_atom(long long &v, char *psz) {
    return ____parse_integer(v, psz, std::strtoll);
}
const char *__parse_atom(unsigned long long &v, char *psz) {
    return ____parse_integer(v, psz, std::strtoull);
}
const char *__parse_atom(long &v, char *psz) {
    return ____parse_integer(v, psz, std::strtol);
}
const char *__parse_atom(unsigned long &v, char *psz) {
    return ____parse_integer(v, psz, std::strtoul);
}
const char *__parse_atom(int &v, char *psz) {
    return ____parse_integer(v, psz, std::strtol);
}
const char *__parse_atom(unsigned int &v, char *psz) {
    return ____parse_integer(v, psz, std::strtoul);
}
const char *__parse_atom(short &v, char *psz) {
    return ____parse_integer(v, psz, std::strtol);
}
const char *__parse_atom(unsigned short &v, char *psz) {
    return ____parse_integer(v, psz, std::strtoul);
}
const char *__parse_atom(signed char &v, char *psz) {
    return ____parse_integer(v, psz, std::strtol);
}
const char *__parse_atom(unsigned char &v, char *psz) {
    return ____parse_integer(v, psz, std::strtoul);
}
const char *__parse_atom(char &v, char *psz) {
    return ____parse_integer(v, psz, std::strtol);
}

template <typename T>
const char *__parse_by_format(T &var, char *psz, const std::string &var_name
        , std::list<std::string> &err_list, void *context, char *parent) {
    auto err_type_name = __parse_atom(var, psz);
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

class __ArgParser : public ArgParser {
public:
    __ArgParser(char *argv[], int argc, std::list<std::string> &err_list , void *context)
            : _argc(argc), _argv(argv), _err_list(err_list), _context(context)
            , _argi(0), _is_optional(false), _at_least(0) {
    }
public:
    void domain_begin(std::string type_name
            , std::string member_name_prefix = "."
            , std::string member_name_suffix = "") override {
        _name_stack.push_back({std::move(type_name), std::move(member_name_prefix)
            , std::move(member_name_suffix)});
    }
    void domain_end() override {
        _name_stack.pop_back();
    }
    void check(bool is_true, std::string msg) override {
        if (!is_true) {
            _err_list.emplace_back(std::move(msg));
        }
    }
    void set_optional() override {
        if (!_is_optional) {
            _at_least = _argi;
            _is_optional = true;
        }
    }
    void *get_context() const override {
        return _context;
    }
protected:
    char *alloc(const std::string &name, const char *type) override {
        _item_traits.emplace_back(std::make_pair(type, name));
        if (_argi < _argc) {
            return _argv[_argi++];
        }
        if (!_is_optional) {
            std::stringstream ss;
            ss << "a(n) '" << type << "' value is required";
            if (!name.empty()) {
                ss << " as '" << concat_name(name) << "'";
            }
            _err_list.emplace_back(ss.str());
        }
        ++_argi;
        return nullptr;
    }
    std::string concat_name(const std::string &name) const override {
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
    void append_errors(std::list<std::string> &err_list) override {
        _err_list.splice(_err_list.end(), err_list);
    }
    void discontinue() override {
        _argc = _argi - 1;
    }
    bool is_optional() const override {
        return _is_optional;
    }
public:
    int submit(const std::string &name) {
        if (_argc < _at_least) {
            std::stringstream ss;
            if (!name.empty()) {
                ss << "'" << name << "'";
            }
            ss << "expects ";
            if (_at_least < _argi) {
                ss << _at_least << "~" << _argi;
            } else {
                ss << _at_least;
            }
            ss << " value(s) but got " << _argc;
            _err_list.emplace_back(ss.str());
        }
        return _argi < _argc ? _argi : _argc;
    }
    int at_least() const {
        return _at_least;
    }
    int at_most() const {
        return _argi;
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
private:
    int _argc;
    char **_argv;
    std::list<std::string> &_err_list;
    void *_context;
    int _argi;
    bool _is_optional;
    int _at_least;
    std::list<std::array<std::string, 3>> _name_stack;
    std::list<std::pair<std::string, std::string>> _item_traits;
}; // __ArgParser

template <typename T>
const std::string &type_traits<T>::name() {
    static std::string s_name = []() -> std::string {
        std::list<std::string> err_tmp;
        __ArgParser parser(nullptr, 0, err_tmp, nullptr);
        T arg_value;
        __parse_by_parser(arg_value, parser, "");
        return parser.type_name();
    }();
    return s_name;
}

class __ArgDataI {
public:
    virtual ~__ArgDataI() {}
    virtual bool valid() const = 0;
    virtual const void *context() const = 0;
    virtual unsigned appear_count() const = 0;
    virtual int appear(char *argv[], int argc, std::list<std::string> &err_list) = 0;
    virtual std::string finish() = 0;
}; // __ArgDataI

class __ArgAttrI : public std::enable_shared_from_this<__ArgAttrI> {
public:
    virtual ~__ArgAttrI() {}
private:
    friend class Parser;
    virtual std::shared_ptr<__ArgDataI> create_data(unsigned max_count = 0) const = 0;
    virtual bool is_positional() const = 0;
    virtual bool has_default_value() const = 0;
    virtual bool has_implicit_value() const = 0;
    virtual bool has_constraint() const = 0;
    virtual std::string get_constraint_desc() const = 0;
    virtual std::string get_data_desc() const = 0;
}; // __ArgAttrI

template <typename>
struct __is_stl_array : public std::false_type {};
template <typename T, std::size_t N>
struct __is_stl_array<std::array<T, N>> : public std::true_type {};

template <typename>
struct __is_stl_vector : public std::false_type {};
template <typename T>
struct __is_stl_vector<std::vector<T>> : public std::true_type {};

template <typename>
struct __is_stl_tuple : public std::false_type {};
template <typename... Targs>
struct __is_stl_tuple<std::tuple<Targs...>> : public std::true_type {};

template <typename>
struct __is_stl_map : public std::false_type {};
template <typename Tk, typename Ta>
struct __is_stl_map<std::map<Tk, Ta>> : public std::true_type {};
template <typename Tk, typename Ta>
struct __is_stl_map<std::unordered_map<Tk, Ta>> : public std::true_type {};

template <typename>
struct __is_stl_set : public std::false_type {};
template <typename T>
struct __is_stl_set<std::set<T>> : public std::true_type {};
template <typename T>
struct __is_stl_set<std::unordered_set<T>> : public std::true_type {};

template <typename T>
struct __is_stl_container {
private:
    template <typename C> static char test(typename C::iterator *);
    template <typename C> static long test(...);
public:
    static const bool value = sizeof(test<T>(0)) == sizeof(char);
};

template <typename T, bool IS_CONTAINER> 
struct ____get_stl_value_type;
template <typename T>
struct ____get_stl_value_type<T, true> {
    typedef typename T::value_type type;
};
template <typename T>
struct ____get_stl_value_type<T, false> {
    typedef T type;
};
template <typename T>
struct __get_stl_value_type
    : public  ____get_stl_value_type<T, __is_stl_container<T>::value> {
};
template <>
struct __get_stl_value_type<std::string> {
    typedef std::string type;
};

template <typename>
struct __is_cli_container : public std::false_type {};
template <typename T>
struct __is_cli_container<std::vector<T>> : public std::true_type {};
template <typename Tk, typename Ta>
struct __is_cli_container<std::map<Tk, Ta>> : public std::true_type {};
template <typename Tk, typename Ta>
struct __is_cli_container<std::unordered_map<Tk, Ta>> : public std::true_type {};

template <typename T>
struct __get_cli_value_type
    : public  ____get_stl_value_type<T, __is_cli_container<T>::value> {
};
template <>
struct __get_cli_value_type<std::string> {
    typedef std::string type;
};
template <typename Tk, typename Ta>
struct __get_cli_value_type<std::map<Tk, Ta>> {
    typedef Ta type;
};
template <typename Tk, typename Ta>
struct __get_cli_value_type<std::unordered_map<Tk, Ta>> {
    typedef Ta type;
};

template <typename T>
std::string to_string(const T &data
        , const std::string &delimiter = ",", const std::string &gap = " ");
template <typename... Targs>
std::string to_string(const std::tuple<Targs...> &data
        , const std::string &delimiter = ",", const std::string &gap = " ");

template <typename T, typename Ta>
struct __to_string {
    static std::string from(const T &data
        , const std::string &delimiter, const std::string &gap);
};
template <typename T>
struct __to_string<T, T> {
    static std::string from(const T &value
        , const std::string &delimiter, const std::string &gap);
};
template <typename Tk, typename Ta>
struct __to_string<std::pair<Tk, Ta>, std::pair<Tk, Ta>> {
    static std::string from(const std::pair<Tk, Ta> &value
        , const std::string &delimiter, const std::string &gap);
};
template <typename Tk, typename Ta>
struct __to_string<std::map<Tk, Ta>, std::pair<const Tk, Ta>> {
    static std::string from(const std::map<Tk, Ta> &value
        , const std::string &delimiter, const std::string &gap);
};
template <typename Tk, typename Ta>
struct __to_string<std::unordered_map<Tk, Ta>, std::pair<const Tk, Ta>> {
    static std::string from(const std::unordered_map<Tk, Ta> &value
        , const std::string &delimiter, const std::string &gap);
};
template <std::size_t N, typename... Targs>
struct ____tuple_to_string;
template <std::size_t N, typename... Targs>
struct ____tuple_to_string<N, std::tuple<Targs...>> {
    static std::string from(const std::tuple<Targs...> &data
        , const std::string &delimiter, const std::string &gap);
};
template <typename... Targs>
struct ____tuple_to_string<1, std::tuple<Targs...>> {
    static std::string from(const std::tuple<Targs...> &data
        , const std::string &delimiter, const std::string &gap);
};
template <typename T>
std::string to_string(const T &data
        , const std::string &delimiter, const std::string &gap) {
    return __to_string<T, typename __get_stl_value_type<T>::type>::from(
        data, delimiter, gap);
}
template <typename... Targs>
std::string to_string(const std::tuple<Targs...> &data
        , const std::string &delimiter, const std::string &gap) {
    return std::string("(")
        + ____tuple_to_string<std::tuple_size<std::tuple<Targs ...>>::value
            , std::tuple<Targs ...>>::from(data, delimiter, gap)
        + ")";
}
template <typename T, typename Ta>
std::string __to_string<T, Ta>::from(const T &data
        , const std::string &delimiter, const std::string &gap) {
    std::stringstream ss;
    if (__is_stl_set<T>::value) {
        ss << "{";
    } else {
        ss << "[";
    }
    bool is_first = true;
    for (const auto &it : data) {
        if (is_first) {
            is_first = false;
        } else {
            ss << delimiter << gap;
        }
        ss << to_string(it, delimiter, gap);
    }
    if (__is_stl_set<T>::value) {
        ss << "}";
    } else {
        ss << "]";
    }
    return ss.str();
}
template <typename T>
std::string __to_string<T, T>::from(const T &value
        , const std::string &delimiter, const std::string &gap) {
    std::stringstream ss;
    if (std::is_base_of<const std::string, T>::value
            || std::is_convertible<T, const char *>::value) {
        ss << "\"";
    }
    ss << value;
    if (std::is_base_of<const std::string, T>::value
            || std::is_convertible<T, const char *>::value) {
        ss << "\"";
    }
    return ss.str();
}
template <typename Tk, typename Ta>
std::string __to_string<std::pair<Tk, Ta>, std::pair<Tk, Ta>>::from(
        const std::pair<Tk, Ta> &value
        , const std::string &delimiter, const std::string &gap) {
    std::stringstream ss;
    ss << "("
       << to_string(value.first, delimiter, gap)
       << delimiter << gap
       << to_string(value.second, delimiter, gap)
       << ")";
    return ss.str();
}
template <typename T>
std::string ____map_to_string(const T &value
        , const std::string &delimiter, const std::string &gap) {
    std::stringstream ss;
    ss << "{";
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
    ss << "}";
    return ss.str();
}
template <typename Tk, typename Ta>
std::string __to_string<std::map<Tk, Ta>, std::pair<const Tk, Ta>>::from(
        const std::map<Tk, Ta> &value
        , const std::string &delimiter, const std::string &gap) {
    return ____map_to_string(value, delimiter, gap);
}
template <typename Tk, typename Ta>
std::string __to_string<std::unordered_map<Tk, Ta>, std::pair<const Tk, Ta>>::from(
        const std::unordered_map<Tk, Ta> &value
        , const std::string &delimiter, const std::string &gap) {
    return ____map_to_string(value, delimiter, gap);
}
template <std::size_t N, typename... Targs>
std::string ____tuple_to_string<N, std::tuple<Targs...>>::from(
        const std::tuple<Targs...> &data
        , const std::string &delimiter, const std::string &gap) {
    return ____tuple_to_string<N - 1, std::tuple<Targs...>>::from(
            data, delimiter, gap)
        + delimiter + gap
        + to_string(std::get<N - 1>(data), delimiter, gap);
}
template <typename... Targs>
std::string ____tuple_to_string<1, std::tuple<Targs...>>::from(
        const std::tuple<Targs...> &data
        , const std::string &delimiter, const std::string &gap) {
    return to_string(std::get<0>(data), delimiter, gap);
}

bool __is_arg_name(const char *p) {
    return p && p[0] == '-' && p[1];
}
bool __is_arg_name_short(const char *p) {
    return p && p[0] == '-' && p[1] != '-' && p[1];
}
bool __is_arg_name_long(const char *p) {
    return p && p[0] == '-' && p[1] == '-' && p[2];
}

template <typename Tm>
bool ArgParser::assign(Tm &value, const std::string &name, Tm default_value) {
    static const char *type_name = [this, &name]() -> const char * {
        Tm arg_value;
        std::list<std::string> err_tmp;
        const char *tn = __parse_by_format(arg_value, nullptr, name, err_tmp, get_context(), nullptr);
        if (reinterpret_cast<long>(tn) == -1l) {
            return type_traits<Tm>::name().c_str();
        }
        return tn ? tn : "";
    }();
    auto psz = alloc(name, type_name);
    if (!psz) {
        value = std::move(default_value);
        return false;
    }
    std::list<std::string> err_tmp;
    __parse_by_format(value, psz, concat_name(name), err_tmp, get_context(), psz);
    if (err_tmp.size()) {
        if (__is_arg_name(psz)) {
            discontinue();
            value = std::move(default_value);
        } else {
            append_errors(err_tmp);
        }
        return false;
    }
    return true;
}

template <typename T> struct __is_cli_scalar : public std::false_type {};
template <> struct __is_cli_scalar<char> : public std::true_type {};
template <> struct __is_cli_scalar<char *> : public std::true_type {};
template <> struct __is_cli_scalar<const char *> : public std::true_type {};
template <> struct __is_cli_scalar<std::string> : public std::true_type {};
template <> struct __is_cli_scalar<bool> : public std::true_type {};
template <> struct __is_cli_scalar<signed char> : public std::true_type {};
template <> struct __is_cli_scalar<short> : public std::true_type {};
template <> struct __is_cli_scalar<int> : public std::true_type {};
template <> struct __is_cli_scalar<long> : public std::true_type {};
template <> struct __is_cli_scalar<long long> : public std::true_type {};
template <> struct __is_cli_scalar<unsigned char> : public std::true_type {};
template <> struct __is_cli_scalar<unsigned short> : public std::true_type {};
template <> struct __is_cli_scalar<unsigned int> : public std::true_type {};
template <> struct __is_cli_scalar<unsigned long> : public std::true_type {};
template <> struct __is_cli_scalar<unsigned long long> : public std::true_type {};
template <> struct __is_cli_scalar<float> : public std::true_type {};
template <> struct __is_cli_scalar<double> : public std::true_type {};

template <typename T, typename Ta>
struct __get_max_capacity {
    enum { value = INT32_MAX };
};
template <typename T>
struct __get_max_capacity<T, T> {
    enum { value = 1 };
};
template <typename... Targs>
struct __get_max_capacity<std::tuple<Targs...>, std::tuple<Targs...>> {
    enum { value = sizeof ...(Targs) };
};

template <typename T, typename Tg, typename Ta>
struct ____get_implicit_value_type {
    typedef Tg type;
};
template <typename T, typename Tg, typename... Targs>
struct ____get_implicit_value_type<T, Tg, std::tuple<Targs...>> {
    typedef std::tuple<Targs...> type;
};
template <typename Ta>
struct ____get_implicit_value_type<std::vector<Ta>, Ta, Ta> {
    typedef std::vector<Ta> type;
};
template <typename... Targs>
struct ____get_implicit_value_type<std::vector<std::tuple<Targs...>>, std::tuple<Targs...>, std::tuple<Targs...>> {
    typedef std::tuple<Targs...> type;
};
template <typename T>
struct __get_implicit_value_type : public ____get_implicit_value_type<T
    , typename __get_cli_value_type<T>::type
    , typename __get_cli_value_type<typename __get_cli_value_type<T>::type>::type> {
};

template <typename T>
class __ArgDataT;
template <typename T, typename Tg, typename Ta>
class ____ArgDataT;

template <typename T>
class __ArgAttrT : public __ArgAttrI {
protected:
    typedef typename __get_cli_value_type<T>::type Tg;
    typedef typename __get_cli_value_type<Tg>::type Ta;
    typedef typename __get_stl_value_type<T>::type Tg_stl;
public:
    typedef typename ____get_implicit_value_type<T, Tg, Ta>::type T_implicit_value;

public:
    explicit __ArgAttrT()
            : _is_positional(false)
            , _has_default_value(false)
            , _has_implicit_value(false)
            , _dim_0_at_least(0)
            , _dim_0_at_most(__get_max_capacity<T, Tg>::value)
            , _dim_1_at_least(1)
            , _dim_1_at_most(__get_max_capacity<Tg, Ta>::value)
            , _context(nullptr)
    {
        static_assert(std::is_same<Ta, typename __get_cli_value_type<Ta>::type>::value
            , "Too many nested levels of containers");
        static_assert(!(__is_stl_vector<T>::value && __is_stl_vector<Tg>::value && __is_stl_tuple<Ta>::value)
            , "std::vector<std::vector<std::tuple<...>>> is not allowed");
        static_assert(!__is_stl_map<Tg>::value
            , "std::vector<std::map> or std::map<std::map> is not allowed");
    }

protected:
    friend class __ArgDataT<T>;
    friend class ____ArgDataT<T, Tg_stl, Ta>;
    std::shared_ptr<__ArgDataI> create_data(unsigned max_count = 0) const override {
        return std::make_shared<____ArgDataT<T, Tg_stl, Ta>>(
            *this, max_count);
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
        return _match_choices || _match_ranges || !_match_examine_desc.empty();
    }
    std::string get_constraint_desc() const override {
        // (choices || ranges) && examine
        std::string desc;
        if (_match_choices) {
            desc += "in set:" + _match_choices_desc;
        }
        if (_match_ranges) {
            if (!desc.empty()) {
                desc += " or ";
            }
            desc += _match_ranges_desc;
        }
        if (!_match_examine_desc.empty()) {
            if (_match_choices && _match_ranges) {
                desc = "(" + desc + ")";
            }
            if (!desc.empty()) {
                desc += " and ";
            }
            desc += "'" + _match_examine_desc + "'";
        }
        return desc;
    }
    virtual std::string get_data_desc() const override {
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
        if (__is_cli_container<T>::value) {
            print_range(_dim_0_at_least, _dim_0_at_most);
            if (__is_cli_container<Tg>::value || __is_stl_tuple<Ta>::value) {
                print_range(_dim_1_at_least, _dim_1_at_most);
            }
        } else {
            if (__is_stl_tuple<Ta>::value) {
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
            ss << "; default:" << to_string(default_value());
        }
        if (has_implicit_value()) {
            ss << "; implicit:" << to_string(implicit_value());
        }
        return ss.str();
    }
    void *get_context() const {
        return _context;
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
    std::string examine(Ta &value, void *data) const {
        // (choices || ranges) && examine
        bool is_in_range = false;
        if (_match_choices && _match_choices(value)) {
            is_in_range = true;
        }
        if (!is_in_range && _match_ranges && _match_ranges(value)) {
            is_in_range = true;
        }
        auto err_detail = "should meet constraint: " + get_constraint_desc();
        if ((_match_choices || _match_ranges) && !is_in_range) {
            return err_detail;
        }
        if (!_match_examine || _match_examine(value, _context, data)) {
            return "";
        }
        return err_detail;
    }
    const T &default_value() const {
        return _default_value;
    }
    const T_implicit_value &implicit_value() const {
        return _implicit_value;
    }

protected:
    void set_positional() {
        _is_positional = true;
    }
    void set_default_value(T &&value) {
        _default_value = std::move(value);
        _has_default_value = true;
    }
    void set_implicit_value(T_implicit_value value) {
        _implicit_value = std::move(value);
        _has_implicit_value = true;
    }
    void set_context(void *context) {
        _context = context;
    }
    void set_appear_count(int at_least, int at_most = -1) {
        _dim_0_at_least = (at_least >= 0 ? at_least : _dim_0_at_least);
        _dim_0_at_most = (at_most >= 0 ? at_most : _dim_0_at_most);
    }
    void set_appear_width(int at_least, int at_most = -1) {
        _dim_1_at_least = (at_least >= 0 ? at_least : _dim_1_at_least);
        _dim_1_at_most = (at_most >= 0 ? at_most : _dim_1_at_most);
    }
    void set_match_choices(std::function<bool (const Ta &)> func, std::string desc) {
        _match_choices = std::move(func);
        _match_choices_desc = std::move(desc);
    }
    void set_match_ranges(std::function<bool (const Ta &)> func, std::string desc) {
        _match_ranges = std::move(func);
        _match_ranges_desc = std::move(desc);
    }
    void set_match_examine(std::function<bool (Ta &, void *context, void *data)> func, std::string desc) {
        _match_examine = std::move(func);
        _match_examine_desc = std::move(desc);
    }
protected:
    bool _is_positional;
    bool _has_default_value;
    bool _has_implicit_value;
    unsigned _dim_0_at_least;
    unsigned _dim_0_at_most;
    unsigned _dim_1_at_least;
    unsigned _dim_1_at_most;
    void *_context;
    T _default_value;
    T_implicit_value _implicit_value;
    std::function<bool (const Ta &)> _match_choices;
    std::string _match_choices_desc;
    std::function<bool (const Ta &)> _match_ranges;
    std::string _match_ranges_desc;
    std::function<bool (Ta &, void *context, void *data)> _match_examine;
    std::string _match_examine_desc;
}; // __ArgAttrT

template <typename T>
class __ArgDataT : public __ArgDataI {
    typedef typename __get_cli_value_type<T>::type Tg;
public:
    __ArgDataT(const __ArgAttrT<T> &arg_attr)
        : _arg_attr(arg_attr), _appear_count(0), _data_count(0) {
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
public:
    const T &data() const {
        return _data;
    }
protected:
    T &data() {
        return _data;
    }
protected:
    const __ArgAttrT<T> &_arg_attr;
    unsigned _appear_count;
    unsigned _data_count;
    T _data;
}; // __ArgDataT

template <typename T, typename Tg, typename Ta>
class ____ArgDataT : public __ArgDataT<T> {
public:
    ____ArgDataT(const __ArgAttrT<T> &arg_attr, unsigned max_count)
            : __ArgDataT<T>(arg_attr) {
    }
};
template <typename Tg, typename Ta>
class ____ArgDataT<std::vector<Tg>, Tg, Ta> : public __ArgDataT<std::vector<Tg>> {
    typedef std::vector<Tg> T;
public:
    ____ArgDataT(const __ArgAttrT<T> &arg_attr, unsigned max_count)
                : __ArgDataT<T>(arg_attr) {
        __ArgDataT<T>::_data.reserve(max_count);
    }
};

template <typename T>
class ArgAttr;

enum __AtomType {
    __AT_VALUE = 0,
    __AT_BOOL,
    __AT_STRING,
    __AT_TUPLE,
    __AT_STRUCT,
};
template <typename Ta, bool IS_TD_SCALAR>
struct ____get_cli_atom_type;
template <typename Ta>
struct ____get_cli_atom_type<Ta, true> {
    static const __AtomType value = __AT_VALUE;
};
template <>
struct ____get_cli_atom_type<bool, true> {
    static const __AtomType value = __AT_BOOL;
};
template <>
struct ____get_cli_atom_type<char *, true> {
    static const __AtomType value = __AT_STRING;
};
template <>
struct ____get_cli_atom_type<const char *, true> {
    static const __AtomType value = __AT_STRING;
};
template <>
struct ____get_cli_atom_type<std::string, true> {
    static const __AtomType value = __AT_STRING;
};
template <typename... Targs>
struct ____get_cli_atom_type<std::tuple<Targs...>, false> {
    static const __AtomType value = __AT_TUPLE;
};
template <typename Ta>
struct ____get_cli_atom_type<Ta, false> {
    static const __AtomType value = __AT_STRUCT;
};
template <typename T>
struct __get_cli_atom_type : public ____get_cli_atom_type<T, __is_cli_scalar<T>::value> {
};

#define __ArgAttr_required()                                                            \
    std::shared_ptr<ArgAttr<T>> required() {                                            \
        __ArgAttrT<T>::set_appear_count(1, -1);                                         \
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this()); \
    }
#define __ArgAttr_positional()                                                          \
    std::shared_ptr<ArgAttr<T>> positional() {                                          \
        __ArgAttrT<T>::set_positional();                                                \
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this()); \
    }
#define __ArgAttr_default_value()                                                       \
    std::shared_ptr<ArgAttr<T>> default_value(T &&value) {                              \
        __ArgAttrT<T>::set_default_value(std::move(value));                             \
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this()); \
    }
#define __ArgAttr_implicit_value()                                                      \
    std::shared_ptr<ArgAttr<T>> implicit_value(                                         \
            typename __ArgAttrT<T>::T_implicit_value &&value) {                         \
        __ArgAttrT<T>::set_implicit_value(std::move(value));                            \
        if (__is_cli_scalar<typename __ArgAttrT<T>::Tg>::value                          \
                || __get_cli_atom_type<typename __ArgAttrT<T>::Tg>::value == __AT_STRUCT) { \
            __ArgAttrT<T>::set_appear_width(0);                                         \
        }                                                                               \
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this()); \
    }
#define __ArgAttr_condition()                                                           \
    std::shared_ptr<ArgAttr<T>> examine(                                                \
            std::function<bool (Ta &)> func, std::string desc = "") {                   \
        __ArgAttrT<T>::set_match_examine(                                               \
            [&func](Ta &v, void *, void *) { return func(v); }                          \
            , std::move(desc));                                                         \
        return std::static_pointer_cast<ArgAttr<T>>(                                    \
            __ArgAttrT<T>::shared_from_this());                                         \
    }                                                                                   \
    std::shared_ptr<ArgAttr<T>> examine(                                                \
            std::function<bool (Ta &, void *context)> func, std::string desc = "") {    \
        __ArgAttrT<T>::set_match_examine(                                               \
            [&func](Ta &v, void *context, void *) { return func(v, context); }          \
            , std::move(desc));                                                         \
        return std::static_pointer_cast<ArgAttr<T>>(                                    \
            __ArgAttrT<T>::shared_from_this());                                         \
    }                                                                                   \
    std::shared_ptr<ArgAttr<T>> examine(                                                \
            std::function<bool (Ta &, void *context, void *data)> func, std::string desc = "") { \
        __ArgAttrT<T>::set_match_examine(std::move(func), std::move(desc));             \
        return std::static_pointer_cast<ArgAttr<T>>(                                    \
            __ArgAttrT<T>::shared_from_this());                                         \
    }
#define __ArgAttr_line_width()                                                           \
    std::shared_ptr<ArgAttr<T>> line_width(int fixed_count) {                            \
        __ArgAttrT<T>::set_appear_count(fixed_count, fixed_count);                      \
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this()); \
    }                                                                                   \
    std::shared_ptr<ArgAttr<T>> line_width(int at_least, int at_most) {                  \
        __ArgAttrT<T>::set_appear_count(at_least, at_most);                             \
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this()); \
    }
#define __ArgAttr_context()                                                             \
    std::shared_ptr<ArgAttr<T>> context(void *ctx) {                                    \
        __ArgAttrT<T>::set_context(ctx);                                                \
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this()); \
    }

template <typename T, __AtomType ADTC>
class __ArgAttrAT;
template <typename T>
class __ArgAttrAT<T, __AT_VALUE> : public __ArgAttrT<T> {
    typedef typename __ArgAttrT<T>::Ta Ta;
public:
    __ArgAttrAT() : __ArgAttrT<T>() {}
    __ArgAttr_required()
    __ArgAttr_positional()
    __ArgAttr_default_value()
    __ArgAttr_condition()
    __ArgAttr_context()
    std::shared_ptr<ArgAttr<T>> choices(std::unordered_set<Ta> value_set) {
        _choices = std::move(value_set);
        __ArgAttrT<T>::set_match_choices(
            std::bind(&__ArgAttrAT<T, __AT_VALUE>::is_in_choices, this, std::placeholders::_1)
            , to_string(_choices));
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> ranges(std::vector<std::pair<Ta, Ta>> range_pairs, std::string desc = "") {
        _value_ranges = std::move(range_pairs);
        if (desc.empty()) {
            desc = "within ranges:" + to_string(_value_ranges);
        }
        __ArgAttrT<T>::set_match_ranges(
            std::bind(&__ArgAttrAT<T, __AT_VALUE>::is_in_ranges, this, std::placeholders::_1)
            , std::move(desc));
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
private:
    bool is_in_choices(const Ta &value) const {
        return _choices.find(value) != _choices.end();
    }
    bool is_in_ranges(const Ta &value) const {
        for (auto &it : _value_ranges) {
            if (value >= it.first && value <= it.second) {
                return true;
            }
        }
        return false;
    }
private:
    std::unordered_set<Ta> _choices;
    std::vector<std::pair<Ta, Ta>> _value_ranges;
};
template <typename T>
class __ArgAttrAT<T, __AT_BOOL> : public __ArgAttrT<T> {
public:
    __ArgAttrAT() : __ArgAttrT<T>() {}
    __ArgAttr_required()
    __ArgAttr_positional()
    __ArgAttr_default_value()
};
template <typename T>
class __ArgAttrAT<T, __AT_STRING> : public __ArgAttrT<T> {
    typedef typename __ArgAttrT<T>::Ta Ta;
public:
    __ArgAttrAT() : __ArgAttrT<T>() {}
    __ArgAttr_required()
    __ArgAttr_positional()
    __ArgAttr_default_value()
    __ArgAttr_condition()
    __ArgAttr_context()
    std::shared_ptr<ArgAttr<T>> choices(std::unordered_set<std::string> values) {
        _choices = std::move(values);
        __ArgAttrT<T>::set_match_choices(
            std::bind(&__ArgAttrAT<T, __AT_STRING>::is_in_choices, this, std::placeholders::_1)
            , to_string(_choices));
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> regex(std::string regex_string, std::string desc = "") {
        _regex_string = std::move(regex_string);
        _regex_object = std::regex(_regex_string);
        if (desc.empty()) {
            desc = "match regex:" + to_string(_regex_string);
        }
        __ArgAttrT<T>::set_match_ranges(
            std::bind(&__ArgAttrAT<T, __AT_STRING>::is_match_regex, this, std::placeholders::_1)
            , std::move(desc));
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
private:
    bool is_in_choices(const Ta &value) const {
        return _choices.find(value) != _choices.end();
    }
    bool is_match_regex(const Ta &value) const {
        return std::regex_match(value, _regex_object);
    }
private:
    std::unordered_set<std::string> _choices;
    std::string _regex_string;
    std::regex _regex_object;
};
template <typename T>
class __ArgAttrAT<T, __AT_TUPLE> : public __ArgAttrT<T> {
    typedef typename __ArgAttrT<T>::Ta Ta;
public:
    __ArgAttrAT() : __ArgAttrT<T>() {}
    __ArgAttr_required()
    __ArgAttr_positional()
    __ArgAttr_default_value()
    __ArgAttr_condition()
    __ArgAttr_context()
};
template <typename T>
class __ArgAttrAT<T, __AT_STRUCT> : public __ArgAttrT<T> {
    typedef typename __ArgAttrT<T>::Ta Ta;
public:
    __ArgAttrAT() : __ArgAttrT<T>() {}
    __ArgAttr_required()
    __ArgAttr_positional()
    __ArgAttr_default_value()
    __ArgAttr_condition()
    __ArgAttr_context()
};

template <typename T, typename Tg, typename Ta>
class __ArgAttrGT : public __ArgAttrAT<T, __get_cli_atom_type<Ta>::value> {
public:
    __ArgAttrGT() : __ArgAttrAT<T, __get_cli_atom_type<Ta>::value>() {}
};
template <typename T, typename... Targs>
class __ArgAttrGT<T, std::tuple<Targs...>, std::tuple<Targs...>>
        : public __ArgAttrAT<T, __AT_TUPLE> {
public:
    __ArgAttrGT() : __ArgAttrAT<T, __AT_TUPLE>() {
    }
    std::shared_ptr<ArgAttr<T>> line_width(int at_least) {
        __ArgAttrT<T>::set_appear_width(at_least);
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
};
template <typename T, typename Ta>
class __ArgAttrGT<T, std::vector<Ta>, Ta>
        : public __ArgAttrAT<T, __get_cli_atom_type<Ta>::value> {
public:
    __ArgAttrGT() : __ArgAttrAT<T, __get_cli_atom_type<Ta>::value>() {
    }
    std::shared_ptr<ArgAttr<T>> line_width(int fixed_count) {
        __ArgAttrT<T>::set_appear_width(fixed_count, fixed_count);
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> line_width(int at_least, int at_most) {
        __ArgAttrT<T>::set_appear_width(at_least, at_most);
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
};
template <typename T, typename Tg, typename Ta>
class __ArgAttrLT : public __ArgAttrGT<T, Tg, Ta> {
public:
    __ArgAttrLT() : __ArgAttrGT<T, Tg, Ta>() {}
    __ArgAttr_implicit_value()
};
template <>
class __ArgAttrLT<bool, bool, bool> : public __ArgAttrT<bool> {
public:
    __ArgAttrLT() : __ArgAttrT<bool>() {
        __ArgAttrT<bool>::set_appear_width(0, 1);
        __ArgAttrT<bool>::_has_implicit_value = true;
        __ArgAttrT<bool>::_implicit_value = true;
    }
    std::shared_ptr<ArgAttr<bool>> implicit_value(bool value) {
        __ArgAttrT<bool>::set_implicit_value(value);
        __ArgAttrT<bool>::set_default_value(!value);
        return std::static_pointer_cast<ArgAttr<bool>>(__ArgAttrT<bool>::shared_from_this());
    }
private:
    std::string get_data_desc() const override {
        return __ArgAttrT<bool>::_implicit_value ? "" : __ArgAttrT<bool>::get_data_desc();
    }
};
template <typename Tg, typename Ta>
class __ArgAttrLT<std::vector<Tg>, Tg, Ta> : public __ArgAttrGT<std::vector<Tg>, Tg, Ta> {
    typedef std::vector<Tg> T;
public:
    __ArgAttrLT() : __ArgAttrGT<T, Tg, Ta>() {}
    __ArgAttr_implicit_value()
    std::shared_ptr<ArgAttr<T>> data_count(int fixed_count) {
        __ArgAttrT<T>::set_appear_count(fixed_count, fixed_count);
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> data_count(int at_least, int at_most) {
        __ArgAttrT<T>::set_appear_count(at_least, at_most);
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
};
template <typename Ta>
class __ArgAttrLT<std::vector<Ta>, Ta, Ta> : public __ArgAttrGT<std::vector<Ta>, Ta, Ta> {
    typedef std::vector<Ta> T;
public:
    __ArgAttrLT() : __ArgAttrGT<T, Ta, Ta>() {
        __ArgAttrT<T>::_dim_1_at_most = __ArgAttrT<T>::_dim_0_at_most;
    }
    __ArgAttr_implicit_value()
    std::shared_ptr<ArgAttr<T>> data_count(int fixed_count) {
        __ArgAttrT<T>::set_appear_count(fixed_count, fixed_count);
        __ArgAttrT<T>::set_appear_width(1, fixed_count);
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
    std::shared_ptr<ArgAttr<T>> data_count(int at_least, int at_most) {
        __ArgAttrT<T>::set_appear_count(at_least, at_most);
        __ArgAttrT<T>::set_appear_width(1, at_most);
        return std::static_pointer_cast<ArgAttr<T>>(__ArgAttrT<T>::shared_from_this());
    }
};

template <typename T>
class ArgAttr : public __ArgAttrLT<T
        , typename __get_cli_value_type<T>::type
        , typename __get_cli_value_type<typename __get_cli_value_type<T>::type>::type> {
}; // ArgAttr

#undef __ArgAttr_required
#undef __ArgAttr_positional
#undef __ArgAttr_default_value
#undef __ArgAttr_implicit_value
#undef __ArgAttr_condition
#undef __ArgAttr_line_width
#undef __ArgAttr_context

template <typename T>
std::shared_ptr<ArgAttr<T>> value() {
    return std::make_shared<ArgAttr<T>>();
}

class ArgDesc {
public:
    ArgDesc(char flag, const std::string &name, const std::string &desc
                , std::shared_ptr<__ArgAttrI> attr)
            : _sname("-"), _lname(name), _desc(desc), _attr(attr) {
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
    const __ArgAttrI *attr() const {
        return _attr.get();
    }

private:
    std::string _sname;
    std::string _lname;
    std::string _desc;
    std::shared_ptr<__ArgAttrI> _attr;
}; // ArgDesc

#ifndef CLIARGS_NO_EXCEPTION
class bad_cast : public std::bad_cast {
public:
    bad_cast(const std::string &what) : std::bad_cast(), _what(what) {
    }
    const char *what() const noexcept override {
        return _what.c_str();
    }
private:
    const std::string _what;
};
#endif // CLIARGS_NO_EXCEPTION

class ArgData {
public:
    ArgData() {}
    ArgData(std::shared_ptr<__ArgDataI> data, const std::string &arg_name)
            : _arg_data(data), _arg_name(arg_name) {}

    bool valid() const {
        return _arg_data && _arg_data->valid();
    }
    const void *context() const {
        return _arg_data ? _arg_data->context() : nullptr;
    }
    template <typename T> const T &as() const {
        auto arg_data = dynamic_cast<const __ArgDataT<T> *>(_arg_data.get());
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
    std::shared_ptr<__ArgDataI> _arg_data;
    std::string _arg_name;
}; // ArgData

class Result {
public:
    const ArgData &operator [](const std::string &arg_name) {
        auto it = _arg_data_map.find(arg_name);
        if (it == _arg_data_map.end()) {
            std::stringstream ss;
            ss  << "fault: ['" << arg_name << "']: unknown arg-name";
#ifdef CLIARGS_NO_EXCEPTION
            std::cerr << ss.str() << std::endl;
            std::exit(EXIT_FAILURE);
#else
            throw std::invalid_argument(ss.str().c_str());
#endif // CLIARGS_NO_EXCEPTION
        }
        return it->second;
    }
private:
    friend class Parser;
    void add_data(const std::string &name, ArgData data) {
        _arg_data_map.insert(std::make_pair(name, std::move(data)));
    }
private:
    std::unordered_map<std::string, ArgData> _arg_data_map;
}; // Result

class Parser {
private:
    class ArgAdder;

public:
    explicit Parser(std::string app_name = "<THIS>", std::string app_desc = "")
        : _app_name(std::move(app_name)), _app_desc(std::move(app_desc))
        , _allow_unknown(false), _help_width(-1), _concise_help(false) {
    }
    Parser &allow_unknow() {
        _allow_unknown = true;
        return *this;
    }
    Parser &set_width(int width) {
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
    bool error() const {
        return !_err_list.empty();
    }
    const std::list<std::string> &error_details() const {
        return _err_list;
    }
    void print_help(const std::string &indent = ""
            , std::ostream &os = std::cout) const;
    Result parse(int argc, char *argv[]);

private:
    template <typename T>
    void add_arg(char flag, std::string name, std::string desc
        , std::shared_ptr<ArgAttr<T>> attr);

    class ArgAdder {
    public:
        ArgAdder &operator () (char flag, const std::string &name
                , const std::string &desc) {
            _parser.add_arg(flag, name, desc, value<bool>()->implicit_value(true));
            return *this;
        }
        ArgAdder &operator () (const std::string &name
                , const std::string &desc) {
            _parser.add_arg(0, name, desc, value<bool>()->implicit_value(true));
            return *this;
        }
        template<typename T>
        ArgAdder &operator () (char flag, const std::string &name
                , const std::string &desc, std::shared_ptr<ArgAttr<T>> attr) {
            _parser.add_arg(flag, name, desc, attr);
            return *this;
        }
        template<typename T>
        ArgAdder &operator () (const std::string &name
                , const std::string &desc, std::shared_ptr<ArgAttr<T>> attr) {
            _parser.add_arg(0, name, desc, attr);
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
    int _help_width;
    bool _concise_help;
    std::list<ArgDesc> _arg_desc_list;
    std::unordered_map<std::string, ArgDesc *> _arg_desc_dict;
    std::list<std::string> _err_list;
}; // Parser

template <typename T>
void Parser::add_arg(char flag, std::string name
        , std::string desc, std::shared_ptr<ArgAttr<T>> attr) {
    auto err_header = std::string("define[") + to_string(_arg_desc_list.size()) + "]:";
    if (name.empty()) {
        _err_list.emplace_back(err_header + "long name is required");
        return;
    }
    if (name == "-" || name == "--") {
        _err_list.emplace_back(err_header + "invalid long name '" + name + "'");
        return;
    }
    if (flag == '-') {
        _err_list.emplace_back(err_header + "invalid short name '" + flag + "'");
        return;
    }
    auto sname = std::string("-") + flag;
    if (flag && _arg_desc_dict.find(sname) != _arg_desc_dict.end()) {
        _err_list.emplace_back(err_header + "short name '" + flag + "' is in used");
        return;
    }
    if (_arg_desc_dict.find(name) != _arg_desc_dict.end()) {
        _err_list.emplace_back(err_header + "long name '" + name + "' is in used");
        return;
    }
    if (name[0] != '-') {
        name = "--" + name;
    } else if (name[1] != '-') {
        name = "-" + name;
    }
    _arg_desc_list.emplace_back(flag, name, desc, attr);
    ArgDesc &arg_desc = *_arg_desc_list.rbegin();
    if (flag) {
        _arg_desc_dict[sname] = &arg_desc;
    }
    _arg_desc_dict[name] = &arg_desc;
}

void Parser::print_help(const std::string &indent, std::ostream &os) const {
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
                os << "[POSITIONAL ARGUMENTS:";
            }
            os << " " << it.name().substr(2);
        }
    }
    if (!is_first_positional) {
        os << "]";
    }
    lname_width += 1;
    os << " ...\n";
    static const std::string indent_inner = "    ";
    static const std::string name_delimiter = ", ";
    static const unsigned flag_width = std::string("constraint: ").length();
    unsigned name_width = sname_width + lname_width + name_delimiter.length();
    int help_width = _help_width - name_width - flag_width \
        - indent_inner.length() - indent.length();
    auto is_space = [&](char c) { return c == ' ' || c == '\t'; };
    auto print_desc = [&](const std::string &desc) {
        if (help_width > 0) {
            size_t b = 0;
            while (b < desc.length()) {
                size_t e = desc.find('\n', b);
                if (e > b + help_width) {
                    e = b + help_width;
                }
                while (e > b && desc[e] && !is_space(desc[e]) && desc[e] != '\n') {
                    --e;
                }
                if (e == b) {
                    e = b + help_width;
                }
                if (b) {
                    os << "\n" << indent << indent_inner
                        << std::setw(name_width) << "";
                    if (!_concise_help) {
                        os << std::setw(flag_width) << "";
                    }
                }
                os << desc.substr(b, e - b);
                b = e + 1;
            }
        } else {
            os << indent << desc;
        }
    };
    for (auto &it : _arg_desc_list) {
        os << indent << indent_inner
            << std::right << std::setw(sname_width) << it.flag();
        if (it.flag().empty()) {
            os << std::setw(name_delimiter.length()) << "";
        } else {
            os << name_delimiter;
        }
        os << std::left << std::setw(lname_width) << it.name();
        auto attr = it.attr();
        print_desc(it.desc());
        if (_concise_help) {
            os << "\n";
            continue;
        }
        if (attr->is_positional()) {
            os << " (positional)";
        }
        os << "\n";
        auto data_desc = attr->get_data_desc();
        if (!data_desc.empty()) {
            os << indent  << indent_inner
                << std::setw(name_width) << ""
                    << std::setw(flag_width) << std::right << "value: ";
            print_desc(data_desc);
            os << "\n";
        }
        if (attr->has_constraint()) {
            os << indent << indent_inner
                << std::setw(name_width) << ""
                << std::setw(flag_width) << std::right << "constraint: ";
            print_desc(attr->get_constraint_desc());
            os << "\n";
        }
    }
    if (!_concise_help && !_err_list.empty()) {
        os << indent << "Error:\n";
        for (auto &it : _err_list) {
            os << indent << indent_inner << it << "\n";
        }
    }
    os.flags(flags);
}

template <typename T>
struct __DataParser {
    static int parse(T &value, char *argv[], int argc
            , std::list<std::string> &err_list
            , void *context, void *data, unsigned at_least, unsigned at_most
            , std::function<std::string(T &, void *)> examine
            , std::function<const typename __get_implicit_value_type<T>::type &()> get_implicit_value
            , const std::string &name = ""
            ) {
        std::list<std::string> err_tmp;
        __ArgParser parser(argv, argc, err_tmp, context);
        if (at_least < 1) {
            parser.set_optional();
        }
        __parse_by_parser(value, parser, name);
        auto i = parser.submit(name);
        if (i || at_least > 0) {
            err_list.splice(err_list.end(), err_tmp);
        } else if (get_implicit_value) {
            value = get_implicit_value();
        } else {
            err_list.splice(err_list.end(), err_tmp);
            return i;
        }
        if (examine) {
            auto err_detail = examine(value, data);
            if (!err_detail.empty()) {
                std::stringstream ss;
                ss << "unexpected value '";
                if (i > 1) {
                    ss << to_string(std::vector<char *>(argv, argv + i));
                } else {
                    ss << *argv;
                }
                ss << "', " << err_detail;
                err_list.emplace_back(ss.str());
            }
        }
        return i;
    }
};
template <typename... Targs>
struct __DataParser<std::tuple<Targs...>> {
    static int parse(std::tuple<Targs...> &value, char *argv[], int argc
            , std::list<std::string> &err_list
            , void *context, void *data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name = "tuple"
            );
};
template <typename Tg>
struct __DataParser<std::vector<Tg>> {
    typedef std::vector<Tg> T;
    typedef typename __get_cli_value_type<Tg>::type Ta;
    static int parse(T &value, char *argv[], int argc
            , std::list<std::string> &err_list
            , void *context, void *data, unsigned at_least, unsigned at_most
            , std::function<std::string(Ta &, void *)> examine
            , std::function<const typename __get_implicit_value_type<T>::type &()> get_implicit_value
            , const std::string &name = "vector"
            );
};
template <typename Tmap, typename Tk, typename Tg>
struct ____MapParser {
    static int parse(Tmap &value, char *argv[], int argc
            , std::list<std::string> &err_list
            , void *context, void *data, unsigned at_least, unsigned at_most
            , std::function<std::string(typename __get_cli_value_type<Tg>::type &, void *)> examine
            , std::function<const typename __get_implicit_value_type<Tg>::type &()> implicit_value
            , const std::string &name = "map"
            );
};
template <typename Tk, typename Ta>
struct __DataParser<std::map<Tk, Ta>> : public ____MapParser<std::map<Tk, Ta>, Tk, Ta> {};
template <typename Tk, typename Ta>
struct __DataParser<std::unordered_map<Tk, Ta>> : public ____MapParser<std::unordered_map<Tk, Ta>, Tk, Ta> {};
template <std::size_t N, typename... Targs>
struct ____TupleParser {
    static int parse(std::tuple<Targs...> &value, char *argv[], int argc
            , std::list<std::string> &err_list
            , void *context, void *data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name
            ) {
        int i = 0;
        if (N) {
            i = ____TupleParser<N - 1, Targs...>::parse(value, argv, argc, err_list
                , context, data, at_least, at_most, nullptr, get_implicit_value, name);
        }
        std::list<std::string> err_tmp;
        auto j = __DataParser<typename std::tuple_element<N, std::tuple<Targs...>>::type>::parse(
            std::get<N>(value), argv + N, (argc > (int)N ? argc - N : 0), err_tmp
            , context, data, (at_least > N ? 1 : 0), 1, nullptr, nullptr
            , name + "<" + to_string(N) + ">");
        if (j || at_least > N) {
            err_list.splice(err_list.end(), err_tmp);
        } else if (get_implicit_value) {
            std::get<N>(value) = std::get<N>(get_implicit_value());
        }
        i += j;
        return i;
    }
};
template <typename... Targs>
struct ____TupleParser<0, Targs...> {
    static int parse(std::tuple<Targs...> &value, char *argv[], int argc
            , std::list<std::string> &err_list
            , void *context, void *data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name
            ) {
        std::list<std::string> err_tmp;
        auto i = __DataParser<typename std::tuple_element<0, std::tuple<Targs...>>::type>::parse(
            std::get<0>(value), argv, argc, err_tmp
            , context, data, (at_least > 0 ? 1 : 0), 1, nullptr, nullptr, name + "<0>"
            );
        if (i || at_least > 0) {
            err_list.splice(err_list.end(), err_tmp);
        } else if (get_implicit_value) {
            std::get<0>(value) = std::get<0>(get_implicit_value());
        }
        return i;
    }
};
template <typename... Targs>
int __DataParser<std::tuple<Targs...>>::parse(std::tuple<Targs...> &value, char *argv[], int argc
        , std::list<std::string> &err_list
        , void *context, void *data, unsigned at_least, unsigned at_most
        , std::function<std::string(std::tuple<Targs...> &, void *)> examine
        , std::function<const std::tuple<Targs...> &()> get_implicit_value
        , const std::string &name
        ) {
    std::list<std::string> err_tmp;
    auto i = ____TupleParser<sizeof ...(Targs) - 1, Targs...>::parse(
        value, argv, argc, err_tmp
        , context, data, at_least, at_most, examine, get_implicit_value
        , (name.empty() ? std::string("tuple") : name)
        );
    if (err_tmp.empty()) {
        if (examine) {
            auto err_detail = examine(value, data);
            if (!err_detail.empty()) {
                err_list.emplace_back(std::move(err_detail));
            }
        }
    } else {
        err_list.splice(err_list.end(), err_tmp);
    }
    return i;
}
template <typename Tg, typename Ta>
struct ____VectorParser {
    static int parse(std::vector<Tg> &value, char *argv[], int argc
            , std::list<std::string> &err_list
            , void *context, void *data, unsigned at_least, unsigned at_most
            , std::function<std::string(Ta &, void *)> examine
            , std::function<const typename __get_implicit_value_type<std::vector<Tg>>::type &()> get_implicit_value
            , const std::string &name
            ) {
        int i = 0;
        Tg arg_value;
        i = __DataParser<Tg>::parse(arg_value, argv + i, argc - i, err_list
            , context, data, at_least, at_most, examine, get_implicit_value, name);
        value.emplace_back(std::move(arg_value));
        return i;
    }
};
template <typename Ta>
struct ____VectorParser<Ta, Ta> {
    static int parse(std::vector<Ta> &value, char *argv[], int argc
            , std::list<std::string> &err_list
            , void *context, void *data, unsigned at_least, unsigned at_most
            , std::function<std::string(Ta &, void *)> examine
            , std::function<const typename __get_implicit_value_type<std::vector<Ta>>::type &()> get_implicit_value
            , const std::string &name
            ) {
        int item_at_most = 0;
        {
            std::list<std::string> err_tmp;
            __ArgParser parser(nullptr, 0, err_tmp, nullptr);
            Ta arg_value;
            __parse_by_parser(arg_value, parser, "");
            item_at_most = parser.at_most();
        }
        int i = 0;
        unsigned n = 0;
        while (n < at_most && value.size() < at_most) {
            Ta arg_value;
            std::list<std::string> err_tmp;
            auto j = __DataParser<Ta>::parse(arg_value, argv + i, argc - i, err_tmp
                , context, data, (n < at_least ? 1 : 0), 1, examine, nullptr
                , (name + "[" + to_string(value.size()) + "]"));
            if (j == 0) {
                break;
            }
            value.emplace_back(std::move(arg_value));
            i += j;
            ++n;
            err_list.splice(err_list.end(), err_tmp);
            if (item_at_most > 1) {
                break;
            }
        }
        if (n < at_least) {
            std::stringstream ss;
            ss << "expects " << at_least;
            if (at_least < at_most) {
                ss << " ~ " << at_most;
            }
            ss << " value(s)" << ", but got " << n;
            err_list.emplace_back(ss.str());
        } else if (get_implicit_value) {
            auto &implicit_value = get_implicit_value();
            for (; n < at_most && n < implicit_value.size(); ++n) {
                value.emplace_back(implicit_value[n]);
            }
        }
        return i;
    }
};
template <typename... Targs>
struct ____VectorParser<std::tuple<Targs...>, std::tuple<Targs...>> {
    static int parse(std::vector<std::tuple<Targs...>> &value, char *argv[], int argc
            , std::list<std::string> &err_list
            , void *context, void *data, unsigned at_least, unsigned at_most
            , std::function<std::string(std::tuple<Targs...> &, void *)> examine
            , std::function<const std::tuple<Targs...> &()> get_implicit_value
            , const std::string &name
            ) {
        int i = 0;
        std::tuple<Targs...> arg_value;
        i = __DataParser<std::tuple<Targs...>>::parse(arg_value, argv + i, argc - i, err_list
            , context, data, at_least, at_most, examine, get_implicit_value
            , name + "[" + to_string(value.size()) + "]");
        value.emplace_back(std::move(arg_value));
        return i;
    }
};
template <typename Tg>
int __DataParser<std::vector<Tg>>::parse(std::vector<Tg> &value, char *argv[], int argc
        , std::list<std::string> &err_list
        , void *context, void *data, unsigned at_least, unsigned at_most
        , std::function<std::string(Ta &, void *)> examine
        , std::function<const typename __get_implicit_value_type<T>::type &()> get_implicit_value
        , const std::string &name
        ) {
    return ____VectorParser<Tg, Ta>::parse(value, argv, argc, err_list, context, data
        , at_least, at_most, examine, get_implicit_value, name);
}
template <typename Tmap, typename Tk, typename Tg, typename Ta>
struct ____MapInserter;
template <typename Tmap, typename Tk, typename Ta>
struct ____MapInserter<Tmap, Tk, Ta, Ta> {
    static void insert(Tmap &value, const std::string &name
            , Tk &map_key, Ta &map_value, std::list<std::string> &err_list) {
        if (value.find(map_key) != value.end()) {
            err_list.emplace_back("repeated " + name + ".key '" + to_string(map_key) + "'");
        } else {
            value.insert(std::make_pair(std::move(map_key), std::move(map_value)));
        }
    }
};
template <typename Tmap, typename Tk, typename Ta>
struct ____MapInserter<Tmap, Tk, std::vector<Ta>, Ta> {
    static void insert(Tmap &value, const std::string &name
            , Tk &map_key, std::vector<Ta> &map_value, std::list<std::string> &err_list) {
        auto &item = value[std::move(map_key)];
        item.insert(item.end(), map_value.begin(), map_value.end());
    }
};
template <typename Tmap, typename Tk, typename Tg>
int ____MapParser<Tmap, Tk, Tg>::parse(Tmap &value, char *argv[], int argc
        , std::list<std::string> &err_list
        , void *context, void *data, unsigned at_least, unsigned at_most
        , std::function<std::string(typename __get_cli_value_type<Tg>::type &, void *)> examine
        , std::function<const typename __get_implicit_value_type<Tg>::type &()> implicit_value
        , const std::string &name
        ) {
    if (argc < 1) {
        std::stringstream ss;
        ss << "a(n) '" << type_traits<Tk>::name() << "' value is required as '" + name + ".key'";
        err_list.emplace_back(ss.str());
        return 0;
    }
    int i = 0;
    Tk map_key;
    std::list<std::string> err_key;
    __parse_by_format(map_key, argv[i], name + ".key", err_key, context, argv[i]);
    if (!err_key.empty()) {
        err_list.splice(err_list.end(), err_key);
    }
    ++i;
    Tg map_value;
    std::list<std::string> err_value;
    i += __DataParser<Tg>::parse(map_value, argv + i, argc - i, err_value
        , context, data, at_least, at_most, examine, implicit_value
        , name + "[" + to_string(map_key) + "]");
    if (!err_value.empty()) {
        err_list.splice(err_list.end(), err_value);
        return i;
    }
    if (err_key.empty()) {
        ____MapInserter<Tmap, Tk, Tg, typename __get_cli_value_type<Tg>::type>::insert(
            value, name, map_key, map_value, err_list);
    }
    return i;
}

template <typename T>
int __ArgDataT<T>::appear(char *argv[], int argc, std::list<std::string> &err_list) {
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
    auto err_header = "[" + to_string(_appear_count) + "th appearance]: ";
    if (dim_0_at_most && _appear_count > dim_0_at_most) {
        std::stringstream ss;
        ss << "too many appearances";
        T v_tmp;
        auto n = __DataParser<T>::parse(v_tmp, argv, argc, err_tmp
            , _arg_attr.get_context(), &v_tmp, dim_1_at_least, dim_1_at_most
            , nullptr, nullptr);
        while (i < argc && i < n) {
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
    std::function<const typename __get_implicit_value_type<T>::type &()> func_implicit_value;
    if (_arg_attr.has_implicit_value()) {
        func_implicit_value = std::bind(&__ArgAttrT<T>::implicit_value, _arg_attr);
    }
    i = __DataParser<T>::parse(_data, argv, argc, err_tmp
        , _arg_attr.get_context(), &_data, dim_1_at_least, dim_1_at_most
        , std::bind(&__ArgAttrT<T>::examine, _arg_attr, std::placeholders::_1, std::placeholders::_2)
        , func_implicit_value
        );
    for (auto &it : err_tmp) {
        err_list.emplace_back(err_header + it);
    }
    ++_data_count;
    return i;
}

template <typename T>
std::string __ArgDataT<T>::finish() {
    if (!_data_count && _arg_attr.has_default_value()) {
        _data = _arg_attr.default_value();
        _appear_count = 1;
        return "";
    }
    auto dim_0_at_least = _arg_attr.dim_0_at_least();
    auto dim_0_at_most = _arg_attr.dim_0_at_most();
    std::stringstream ss;
    if (__is_cli_container<Tg>::value) {
        if (_appear_count < dim_0_at_least) {
            ss << ": expects " << dim_0_at_least;
            if (dim_0_at_least < dim_0_at_most) {
                ss << " ~ " << dim_0_at_most;
            }
            ss << " appearance(s)" << ", but got " << _appear_count;
            return ss.str();
        }
    } else {
        if (_data_count < dim_0_at_least) {
            ss << ": expects " << dim_0_at_least;
            if (dim_0_at_least < dim_0_at_most) {
                ss << " ~ " << dim_0_at_most;
            }
            ss << " value(s)" << ", but got " << _data_count;
            return ss.str();
        }
    }
    return "";
}

Result Parser::parse(int argc, char *argv[]) {
    if (error()) {
        print_help();
        return Result();
    }
    std::unordered_map<std::string, std::shared_ptr<__ArgDataI>> result_impl;
    std::vector<ArgDesc *> pos_arg_vec;
    pos_arg_vec.reserve(_arg_desc_list.size());
    int i = 0;
    for (auto &it : _arg_desc_list) {
        if (it.attr()->is_positional()) {
            pos_arg_vec.emplace_back(&it);
        }
        auto arg_data = it.attr()->create_data(argc - 1);
        result_impl.insert(std::make_pair(it.name(), arg_data));
        ++i;
    }
    size_t pos_arg_idx = 0;
    ArgDesc arg_desc_unknown(0, "", "", value<std::vector<const char *>>());
    auto arg_data_unknown = arg_desc_unknown.attr()->create_data(argc - 1);
    std::string arg_name;
    std::shared_ptr<__ArgDataI> arg_data;
    i = 1;
    while (i < argc) {
        auto p = argv[i];
        char *binded_data[1] = {nullptr};
        ArgDesc *desc = nullptr;
        if (__is_arg_name_short(p)) {
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
                        _err_list.emplace_back(ss.str());
                    }
                    arg_data = arg_data_unknown;
                    continue;
                }
                desc = it_desc->second;
                arg_data = result_impl[desc->name()];
                if (!p[j] || p[j] == '=') {
                    break;
                }
                std::list<std::string> detail_list;
                arg_data->appear(nullptr, 0, detail_list);
                for (auto &it : detail_list) {
                    _err_list.emplace_back(ss.str() + it);
                }
            }
            ++i;
        } else if (__is_arg_name_long(p)) {
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
                    _err_list.emplace_back(ss.str());
                }
                arg_data = arg_data_unknown;
            } else {
                desc = it_desc->second;
                arg_data = result_impl[desc->name()];
            }
            ++i;
        } else if (pos_arg_idx < pos_arg_vec.size() && pos_arg_vec[pos_arg_idx]) {
            desc = pos_arg_vec[pos_arg_idx];
            arg_name = desc->name();
            arg_data = result_impl[arg_name];
        } else {
            std::stringstream ss;
            ss << "usage: arg['" << arg_name << "']";
            if (arg_data) {
                ss << "[" << arg_data->appear_count() << "th appearance]";
            }
            ss << ": too many value '" << p << "'";
            _err_list.emplace_back(ss.str());
            ++i;
            continue;
        }

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
            _err_list.emplace_back(err_header + it);
        }
    }

    Result result;
    for (auto &it : result_impl) {
        auto err_detail = it.second->finish();
        if (err_detail.length()) {
            auto &arg_desc = _arg_desc_dict[it.first];
            std::stringstream ss;
            ss << "usage: arg['";
            auto &flag = arg_desc->flag();
            if (!flag.empty()) {
                ss << flag << ", ";
            }
            ss << it.first << "']" << err_detail;
            _err_list.emplace_back(ss.str());
        }
        auto arg_name = it.first.substr(2);
        result.add_data(arg_name, std::move(ArgData(it.second, arg_name)));
    }
    return result;
}

} // cliargs

#endif // __HY_CLIARGS_HPP__
