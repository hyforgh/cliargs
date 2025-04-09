#!/usr/bin/bash
#
# MIT License
#
# Copyright (c) 2025-2034 Hongyun Liu
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

function __list_index() {
    local target=$1
    shift
    local list=($@)
    local i
    for i in ${!list[@]}; do
        if [ "${target}" == "${list[${i}]}" ]; then
            echo ${i}
            break
        fi
    done
}

function __is_in() {
    local target=$1
    shift
    if [[ "$@" =~ ${target} ]]; then
        echo 1
    fi
}

__ARG_REQUIREMENT_LIST__=(
    "_?"
    "_*"
    "_+"
)
__ARG_TYPE_LIST__=(
    "norm"
    "bool"
    "enum"
    "opts"
    "args"
)
__ARG_ATTR_BASE_DESC__=(
    "short_name"
    "arg_name"
    "var_name"
    "arg_type"
    "arg_requirement"
)
__ARG_ATTR_EXTEND_DESC__=(
    "arg_opt"
    "arg_help"
)
__ARG_ATTR_COUNT_BASE__=${#__ARG_ATTR_BASE_DESC__}
__ARG_ATTR_COUNT_EXTEND__=${#__ARG_ATTR_EXTEND_DESC__}

function cliargs_init() {
    local -n __arg_handle_init__=$1
    local -n __arg_error_init__=$2
    local exit_on_error=$(__is_in ${3} 1 y Y yes Yes)
    shift 3
    local arg_index
    local err_count=0
    __arg_handle_init__[0]=
    __arg_handle_init__[1]=
    __arg_handle_init__[2]=
    __arg_handle_init__[3]=
    __arg_handle_init__[4]=
    for arg_index in $(seq 0 $(($# - 1))); do
        local arg_desc="$1"
        shift
        local arg_attr=(${arg_desc//:/ })

        local name_info=(${arg_attr[0]/\[/ })
        local arg_name=(${name_info[0]/,/ })
        local short_name=${arg_name[0]}
        arg_name=${arg_name[1]:-${arg_name[0]}}
        local var_name=${name_info[1]/\]/}
        var_name=${var_name:-${arg_name}}
        local arg_type="${arg_attr[1]%,*}"
        local arg_req="${arg_attr[1]#*,}"
        if [ "${arg_req}" == "${arg_type}" ]; then
            arg_req="_?"
        elif [ "${arg_req}" == "*" ] || [ "${arg_req}" == "?" ] || [ "${arg_req}" == "+" ]; then
            arg_req="_${arg_req}"
        fi
        local arg_opt=""
        if [ ${#arg_attr[@]} -gt 2 ]; then
            arg_opt="${arg_desc#*:*:}"
        fi
        arg_opt=(${arg_opt%%:*})
        local arg_help="${arg_desc}"

        if [[ ! ${__ARG_REQUIREMENT_LIST__[@]} =~ ${arg_req} ]] && [[ ! ${arg_req} =~ ^[0-9]+$ ]]; then
            __arg_error_init__[$err_count]="def[${arg_index}]: unsupported arg_requirement '${arg_req}'"
            ((err_count++))
        fi

        if [ ${arg_type} == "bool" ]; then
            if [[ ! ${arg_name} =~ ^- ]]; then
                __arg_error_init__[$err_count]="def[${arg_index}]: positional argument with type '${arg_type}'"
                ((err_count++))
            fi
            if [ ${arg_req} == "_?" ]; then
                arg_req="0"
            fi
        elif [[ ! ${__ARG_TYPE_LIST__[@]} =~ ${arg_type} ]]; then
            __arg_error_init__[$err_count]="def[${arg_index}]: unsupported arg_type '${arg_type}'"
            ((err_count++))
        elif [ "${arg_req}" == 0 ]; then
            __arg_error_init__[$err_count]="def[${arg_index}]: invalid arg_requirement '0' for arg_type '${arg_type}'"
            ((err_count++))
        fi

        if [ ${arg_index} == 0 ]; then
            local short_list=${short_name}
            local name_list=${arg_name}
            local var_list=${var_name}
            local type_list=${arg_type}
            local require_list="${arg_req}"
        else
            local short_list="${short_list} ${short_name}"
            local name_list="${name_list} ${arg_name}"
            local var_list="${var_list} ${var_name}"
            local type_list="${type_list} ${arg_type}"
            local require_list="${require_list} ${arg_req}"
        fi
        local opt_index=$((${__ARG_ATTR_COUNT_BASE__} + ${arg_index} * ${__ARG_ATTR_COUNT_EXTEND__}))
        __arg_handle_init__[${opt_index}]="${arg_opt[@]}"
        __arg_handle_init__[$((${opt_index} + 1))]="${arg_help}"
    done
    if [ ${err_count} -gt 0 ] && [ "${exit_on_error}" ]; then
        echo "Define:"
        local err_index
        for err_index in ${!__arg_error_init__[@]}; do
            echo "    ${__arg_error_init__[${err_index}]}"
        done
        exit -1
    fi
    __arg_handle_init__[0]="${short_list}"
    __arg_handle_init__[1]="${name_list}"
    __arg_handle_init__[2]="${var_list}"
    __arg_handle_init__[3]="${type_list}"
    __arg_handle_init__[4]="${require_list}"
}

function cliargs_help() {
    local -n __arg_handle_help__=$1
    shift
    local -n __arg_error_help__=$1
    shift
    local program_name=${1:-<this-program>}
    shift
    local debug_message="$@"

    local name_list=(${__arg_handle_help__[1]})
    local var_list=(${__arg_handle_help__[2]})
    local require_list=(${__arg_handle_help__[4]})
    local positional_arg_help
    for arg_index in ${!name_list[@]}; do
        local arg_name=${name_list[${arg_index}]}
        local var_name=${var_list[${arg_index}]}
        if [[ ! ${arg_name} =~ ^- ]]; then
            local arg_req="${require_list[${arg_index}]}"
            if [ "${arg_req}" == "_?" ] || [ "${arg_req}" == "_*" ]; then
                positional_arg_help="${positional_arg_help}[${var_name}] "
            else
                positional_arg_help="${positional_arg_help}${var_name} "
            fi
        fi
    done
    echo "Usage: ${program_name} ${positional_arg_help}[OPTION]"
    echo "Options:"
    for arg_index in ${!name_list[@]}; do
        local arg_help="${__arg_handle_help__[$((${__ARG_ATTR_COUNT_BASE__} + ${arg_index} * ${__ARG_ATTR_COUNT_EXTEND__} + 1))]}"
        echo -e "${arg_help}" | sed 's/\[[[:alpha:]_]*\][[:blank:]]*:/ :/g'
    done

    if [ "${debug_message[@]}" ]; then
        echo
        echo "Debug:"
        echo "    ${debug_message[@]}"
    fi

    if [ "${__arg_error_help__}" ]; then
        local i
        echo
        echo "Error:"
        for i in ${!__arg_error_help__[@]}; do
            echo "    ${__arg_error_help__[$i]}"
        done
    fi
}

function cliargs_parse() {
    local -n __arg_handle_parse__=$1
    local -n __arg_errors__=$2
    local allow_unknown=$(__is_in $3 "1 y Y yes Yes")
    shift 3
    local name_list=(${__arg_handle_parse__[1]})
    local var_list=(${__arg_handle_parse__[2]})
    local type_list=(${__arg_handle_parse__[3]})
    local require_list=(${__arg_handle_parse__[4]})
    local arg_count=${#type_list[@]}
    local positional_list
    local positional_count=0
    local arg_index
    local val_count_list
    for arg_index in ${!name_list[@]}; do
        val_count_list[${arg_index}]=0
        if [[ ! ${name_list[${arg_index}]} =~ ^- ]]; then
            positional_list[$positional_count]=${arg_index}
            ((positional_count++))
        fi
    done
    local name_list_all=(${__arg_handle_parse__[0]} ${name_list[@]})
    local positional_index=0
    local arg_name
    local arg_type="bool"
    local arg_req
    local arg_opt
    local unknown_count=0
    local err_count=0
    while [ $# -gt 0 ]; do
        if [[ "$1" =~ ^- ]]; then
            arg_name="$1"
            shift
            arg_index=$(__list_index "${arg_name}" ${name_list_all[@]})
            if [ ! ${arg_index} ]; then
                arg_index=$((${arg_count} + ${unknown_count}))
                ((unknown_count++))
                if [ ! "${allow_unknown}" ]; then
                    __arg_errors__[${err_count}]="usage: arg[${arg_index}] '${arg_name}': not supported"
                    ((err_count++))
                fi
                arg_type="norm"
                arg_req="_*"
                var_list[${arg_index}]="${arg_name//-/}"
                val_count_list[${arg_index}]=0
            else
                arg_index=$((${arg_index} % ${arg_count}))
                arg_type=${type_list[$arg_index]}
                arg_req="${require_list[${arg_index}]}"
            fi
        elif [ ${positional_index} -lt ${positional_count} ]; then
            # after a named argument has eat enough value(s)
            arg_index=${positional_list[${positional_index}]}
            arg_name=${name_list[${arg_index}]}
            arg_type=${type_list[$arg_index]}
            arg_req="${require_list[${arg_index}]}"
            ((positional_index++))
        else
            __arg_errors__[${err_count}]="usage: arg[${arg_index}] '${arg_name}': too many value(\"$1\")"
            ((err_count++))
            shift
            continue
        fi

        local param_count=0
        if [ "${arg_type}" == "args" ]; then
            if [ "${arg_req}" == "_*" ] || [ "${arg_req}" == "_+" ]; then
                while [ $# -gt 0 ]; do
                    if [ "$1" == "--" ]; then
                        shift
                        break
                    fi
                    eval "${var_list[${arg_index}]}[${val_count_list[${arg_index}]}]='${1//\$/\\\$}'"
                    ((val_count_list[${arg_index}]++))
                    shift
                    ((param_count++))
                done
            else
                local N=0
                if [ "${arg_req}" != "_?" ]; then
                    N=$((${arg_req} - 1))
                fi
                while [ $# -gt 0 ]; do
                    if [ "$1" == "--" ]; then
                        shift
                        break
                    fi
                    if [[ $1 =~ ^- ]]; then
                        if [ ${N} == 0 ]; then
                            break
                        fi
                        ((N--))
                    fi
                    eval "${var_list[${arg_index}]}[${val_count_list[${arg_index}]}]='${1//\$/\\\$}'"
                    ((val_count_list[${arg_index}]++))
                    shift
                    ((param_count++))
                done
                if [ ${N} -gt 0 ]; then
                    eval "local arg_value=\${${var_list[${arg_index}]//\$/\\\$}}"
                    __arg_errors__[${err_count}]="error: arg[${arg_index}] '${arg_name}': too few values(${arg_value[@]}), expect ${arg_req}"
                    ((err_count++))
                fi
            fi
        else
            arg_opt=(${__arg_handle_parse__[$((${__ARG_ATTR_COUNT_BASE__} + ${arg_index} * ${__ARG_ATTR_COUNT_EXTEND__}))]})
            if [ "${arg_req}" == "_*" ] || [ "${arg_req}" == "_+" ]; then
                while [ $# -gt 0 ]; do
                    if [[ "$1" =~ ^- ]]; then
                        break
                    fi
                    if [[ "enum opts" =~ ${arg_type} ]] && [[ ! "${arg_opt[@]}" =~ $1 ]]; then
                        __arg_errors__[${err_count}]="error: arg[${arg_index}] '${arg_name}': unknown ${arg_type} value '\"$1\"', expect {${arg_opt[@]}}"
                        ((err_count++))
                    fi
                    eval "${var_list[${arg_index}]}[${val_count_list[${arg_index}]}]='${1//\$/\\\$}'"
                    ((val_count_list[${arg_index}]++))
                    shift
                    ((param_count++))
                done
            else
                local N=${val_count_list[${arg_index}]}
                local rn=1
                if [ "${arg_req}" != "_?" ]; then
                    rn=${arg_req}
                fi
                if [ ${N} -lt ${rn} ]; then
                    N=$((${rn} - ${N}))
                    while [ $# -gt 0 ] && [ ${N} -gt 0 ]; do
                        if [[ "$1" =~ ^- ]]; then
                            break
                        fi
                        if [[ "enum opts" =~ ${arg_type} ]] && [[ ! "${arg_opt[@]}" =~ "$1" ]]; then
                            __arg_errors__[${err_count}]="error: arg[${arg_index}] '${arg_name}': unknown ${arg_type} value '$1', expect {${arg_opt[@]}}"
                            ((err_count++))
                        fi
                        eval "${var_list[${arg_index}]}[${val_count_list[${arg_index}]}]='${1//\$/\\\$}'"
                        ((val_count_list[${arg_index}]++))
                        shift
                        ((N--))
                        ((param_count++))
                    done
                fi
            fi
        fi
        if [ ${param_count} == "0" ]; then
            if [ ${arg_type} == "bool" ]; then
                eval "${var_list[${arg_index}]}='\0'"
            else
                __arg_errors__[${err_count}]="usage: arg[${arg_index}] '${arg_name}': a value is required"
                ((err_count++))
            fi
        fi
    done

    for arg_index in ${!name_list[@]}; do
        arg_type=${type_list[${arg_index}]}
        if [ ${arg_type} == "bool" ]; then
            continue
        fi
        arg_opt=(${__arg_handle_parse__[$((${__ARG_ATTR_COUNT_BASE__} + ${arg_index} * ${__ARG_ATTR_COUNT_EXTEND__}))]})
        value_count=${val_count_list[${arg_index}]}
        arg_name="${name_list[${arg_index}]}"
        arg_req="${require_list[${arg_index}]}"
        if [ "${arg_type}" == "enum" ]; then
            if [ ${value_count} == 0 ]; then
                if [ "${arg_req}" == "_+" ] || [ "${arg_req}" == "_?" ]; then
                    eval "${var_list[${arg_index}]}[${val_count_list[${arg_index}]}]=${arg_opt[0]//\$/\\\$}"
                    ((val_count_list[${arg_index}]++))
                    value_count=1
                elif [[ "${arg_req}" =~ ^[1-9]+$ ]]; then
                    for i in $(seq 0 $((${arg_req} - 1))); do
                        eval "${var_list[${arg_index}]}[${val_count_list[${arg_index}]}]=${arg_opt[0]//\$/\\\$}"
                        ((val_count_list[${arg_index}]++))
                    done
                    value_count=${arg_req}
                fi
            fi
        elif [ ${value_count} == 0 ] && [ "${arg_type}" != "opts" ]; then
            value_count=${#arg_opt[@]}
            for i in ${!arg_opt[@]}; do
                eval "${var_list[${arg_index}]}[${val_count_list[${arg_index}]}]=${arg_opt[i]//\$/\\\$}"
                ((val_count_list[${arg_index}]++))
            done
        fi

        if [ "${arg_req}" == "_?" ] || [ "${arg_req}" == "_*" ]; then
            continue
        fi

        if [ "${arg_req}" == "_+" ]; then
            if [ ${value_count} -lt 1 ]; then
                __arg_errors__[${err_count}]="error: arg[${arg_index}] '${arg_name}': requires atleast 1 value"
                ((err_count++))
            fi
        elif [ ${value_count} -lt ${arg_req} ]; then
            eval "local arg_value=\${${var_list[${arg_index}]//\$/\\\$}[@]}"
            __arg_errors__[${err_count}]="error: arg[${arg_index}] '${arg_name}': requires ${arg_req} value(s), got ${value_count}:{${arg_value[@]}}"
            ((err_count++))
        fi
    done
    for arg_index in ${!name_list[@]}; do
        export ${var_list[${arg_index}]}
    done
}
