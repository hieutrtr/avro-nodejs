/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef CPX_HH_1016804386__H_
#define CPX_HH_1016804386__H_


#include "boost/any.hpp"
#include "avro/Specific.hh"
#include "avro/Encoder.hh"
#include "avro/Decoder.hh"

namespace c {
struct cpx_json_Union__0__ {
private:
    size_t idx_;
    boost::any value_;
public:
    size_t idx() const { return idx_; }
    double get_double() const;
    void set_double(const double& v);
    bool is_null() const {
        return (idx_ == 1);
    }
    void set_null() {
        idx_ = 1;
        value_ = boost::any();
    }
    cpx_json_Union__0__();
};

struct cpx_json_Union__1__ {
private:
    size_t idx_;
    boost::any value_;
public:
    size_t idx() const { return idx_; }
    std::vector<std::string > get_array() const;
    void set_array(const std::vector<std::string >& v);
    bool is_null() const {
        return (idx_ == 1);
    }
    void set_null() {
        idx_ = 1;
        value_ = boost::any();
    }
    cpx_json_Union__1__();
};

struct cpx_json_Union__2__ {
private:
    size_t idx_;
    boost::any value_;
public:
    size_t idx() const { return idx_; }
    std::map<std::string, int32_t > get_map() const;
    void set_map(const std::map<std::string, int32_t >& v);
    cpx_json_Union__2__();
};

struct cpx {
    typedef cpx_json_Union__0__ re_t;
    typedef cpx_json_Union__1__ array_t;
    typedef cpx_json_Union__2__ map_t;
    re_t re;
    double im;
    std::string name;
    array_t array;
    map_t map;
};

inline
double cpx_json_Union__0__::get_double() const {
    if (idx_ != 0) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<double >(value_);
}

inline
void cpx_json_Union__0__::set_double(const double& v) {
    idx_ = 0;
    value_ = v;
}

inline
std::vector<std::string > cpx_json_Union__1__::get_array() const {
    if (idx_ != 0) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<std::vector<std::string > >(value_);
}

inline
void cpx_json_Union__1__::set_array(const std::vector<std::string >& v) {
    idx_ = 0;
    value_ = v;
}

inline
std::map<std::string, int32_t > cpx_json_Union__2__::get_map() const {
    if (idx_ != 0) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<std::map<std::string, int32_t > >(value_);
}

inline
void cpx_json_Union__2__::set_map(const std::map<std::string, int32_t >& v) {
    idx_ = 0;
    value_ = v;
}

inline cpx_json_Union__0__::cpx_json_Union__0__() : idx_(0), value_(double()) { }
inline cpx_json_Union__1__::cpx_json_Union__1__() : idx_(0), value_(std::vector<std::string >()) { }
inline cpx_json_Union__2__::cpx_json_Union__2__() : idx_(0), value_(std::map<std::string, int32_t >()) { }
}
namespace avro {
template<> struct codec_traits<c::cpx_json_Union__0__> {
    static void encode(Encoder& e, c::cpx_json_Union__0__ v) {
        e.encodeUnionIndex(v.idx());
        switch (v.idx()) {
        case 0:
            avro::encode(e, v.get_double());
            break;
        case 1:
            e.encodeNull();
            break;
        }
    }
    static void decode(Decoder& d, c::cpx_json_Union__0__& v) {
        size_t n = d.decodeUnionIndex();
        if (n >= 2) { throw avro::Exception("Union index too big"); }
        switch (n) {
        case 0:
            {
                double vv;
                avro::decode(d, vv);
                v.set_double(vv);
            }
            break;
        case 1:
            d.decodeNull();
            v.set_null();
            break;
        }
    }
};

template<> struct codec_traits<c::cpx_json_Union__1__> {
    static void encode(Encoder& e, c::cpx_json_Union__1__ v) {
        e.encodeUnionIndex(v.idx());
        switch (v.idx()) {
        case 0:
            avro::encode(e, v.get_array());
            break;
        case 1:
            e.encodeNull();
            break;
        }
    }
    static void decode(Decoder& d, c::cpx_json_Union__1__& v) {
        size_t n = d.decodeUnionIndex();
        if (n >= 2) { throw avro::Exception("Union index too big"); }
        switch (n) {
        case 0:
            {
                std::vector<std::string > vv;
                avro::decode(d, vv);
                v.set_array(vv);
            }
            break;
        case 1:
            d.decodeNull();
            v.set_null();
            break;
        }
    }
};

template<> struct codec_traits<c::cpx_json_Union__2__> {
    static void encode(Encoder& e, c::cpx_json_Union__2__ v) {
        e.encodeUnionIndex(v.idx());
        switch (v.idx()) {
        case 0:
            avro::encode(e, v.get_map());
            break;
        }
    }
    static void decode(Decoder& d, c::cpx_json_Union__2__& v) {
        size_t n = d.decodeUnionIndex();
        if (n >= 1) { throw avro::Exception("Union index too big"); }
        switch (n) {
        case 0:
            {
                std::map<std::string, int32_t > vv;
                avro::decode(d, vv);
                v.set_map(vv);
            }
            break;
        }
    }
};

template<> struct codec_traits<c::cpx> {
    static void encode(Encoder& e, const c::cpx& v) {
        avro::encode(e, v.re);
        avro::encode(e, v.im);
        avro::encode(e, v.name);
        avro::encode(e, v.array);
        avro::encode(e, v.map);
    }
    static void decode(Decoder& d, c::cpx& v) {
        avro::decode(d, v.re);
        avro::decode(d, v.im);
        avro::decode(d, v.name);
        avro::decode(d, v.array);
        avro::decode(d, v.map);
    }
};

}
#endif
