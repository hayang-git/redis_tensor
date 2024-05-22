#ifndef __REDIS_TENSOR_H__
#define __REDIS_TENSOR_H__

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <string.h>
#include <hiredis/hiredis.h>

template <typename T>
class RedisTensor {
public:
    RedisTensor(const std::string &host, int port, const std::string &password) 
        : mCtx(nullptr)
        , mIp(host)
        , mPort(port)
        , mPassword(password)
    {
        mTypeMapCpp2Py = {
            {"b", "bool"},    //bool
            {"a", "int8"},    //int8_t
            {"s", "int16"},   //int16_t
            {"i", "int32"},   //int32_t
            {"h", "uint8"},   //uint8_t
            {"t", "uint16"},  //uint16_t
            {"j", "uint32"},  //uint32_t
            {"f", "float32"}, //float
            {"d", "float64"}  //double
        };
        mCtx = redisConnect(mIp.c_str(), mPort);
        if (mCtx == NULL || mCtx->err) {
            if (mCtx) {
                printf("Error: %s\n", mCtx->errstr);
            } else {
                printf("Can't allocate redis context\n");
            }
            return;
        }

        if (!mPassword.empty()) {
            redisReply *reply = (redisReply *)redisCommand(mCtx, "AUTH %s", mPassword.c_str());
            if (reply == NULL) {
                printf("Error: %s\n", mCtx->errstr);
                redisFree(mCtx);
                mCtx = nullptr;
                return;
            }

            freeReplyObject(reply);
        }
    }

    ~RedisTensor() {
        if (mCtx) {
            redisFree(mCtx);
        }
    }
    
    bool set(const std::string &key, const std::vector<int32_t> &dims, const std::vector<T> &value) {
        if (mCtx == nullptr) {
            return false;
        }

        /*
        * keys:
        *     key_tensor %b
        *     key_shape  %s
        *     key_dtype  %s
        */
        std::string dims_str = dims2str(dims);
        std::string key_tensor = key + "_tensor";
        std::string key_shape  = key + "_shape";
        std::string key_dtype  = key + "_dtype";

        std::string typeName(typeid(T).name());

        redisReply *reply = (redisReply *)redisCommand(mCtx, 
            "MSET %s %b %s %s %s %s", 
            key_tensor.c_str(), value.data(), value.size() * sizeof(T),
            key_shape.c_str(), dims_str.c_str(),
            key_dtype.c_str(), mTypeMapCpp2Py[typeName].c_str());

        if (reply == NULL) {
            printf("Error: %s\n", mCtx->errstr);
            return false;
        }

        freeReplyObject(reply);
        return true;
    }

    bool set(const std::string &key, const std::vector<int32_t> &dims, T *value, int32_t size) {
        if (mCtx == nullptr) {
            return false;
        }

        /*
        * keys:
        *     key_tensor %b
        *     key_shape  %s
        *     key_dtype  %s
        */
        std::string dims_str = dims2str(dims);
        std::string key_tensor = key + "_tensor";
        std::string key_shape  = key + "_shape";
        std::string key_dtype  = key + "_dtype";

        std::string typeName(typeid(T).name());

        redisReply *reply = (redisReply *)redisCommand(mCtx, 
            "MSET %s %b %s %s %s %s", 
            key_tensor.c_str(), value, size * sizeof(T),
            key_shape.c_str(), dims_str.c_str(),
            key_dtype.c_str(), mTypeMapCpp2Py[typeName].c_str());

        if (reply == NULL) {
            printf("Error: %s\n", mCtx->errstr);
            return false;
        }

        freeReplyObject(reply);
        return true;
    }

    bool get(const std::string &key, std::vector<int32_t> &dims, std::vector<T> &value) {
        if (mCtx == nullptr) {
            return false;
        }

        std::string key_tensor = key + "_tensor";
        std::string key_shape  = key + "_shape";
        std::string key_dtype  = key + "_dtype";

        std::vector<const char *> argv = {
            "MGET",
            key_shape.c_str(),
            key_dtype.c_str(),
            key_tensor.c_str()
        };

        redisReply *reply = (redisReply *)redisCommandArgv(mCtx, argv.size(), argv.data(), NULL);
        if (nullptr == reply) {
            printf("Error: %s\n", mCtx->errstr);
            return false;
        }

        if (reply->type != REDIS_REPLY_ARRAY) {
            printf("Error: reply type is not REDIS_REPLY_ARRAY\n");
            freeReplyObject(reply);
            return false;
        }

        bool has_tensor = true;
        for (int i = 0; i < reply->elements; i++) {
            redisReply *sub_reply = reply->element[i];
            if (sub_reply->type == REDIS_REPLY_STRING) {
                if (i == 0) { // shape
                    str2dims(sub_reply->str, dims);
                } else if (i == 1) { // dtype
                    // do nothing

                } else if (i == 2) { // tensor
                    value.resize(sub_reply->len / sizeof(T));
                    memcpy(value.data(), sub_reply->str, sub_reply->len);
                }
            } else {
                has_tensor = false;
                break;
            }
        }
        freeReplyObject(reply);
        return has_tensor;
    }

private:
    std::string dims2str(const std::vector<int32_t> &dims) {
        std::string dims_str("(");
        for (int i = 0; i < dims.size(); i++) {
            dims_str += std::to_string(dims[i]);
            if (i < dims.size() - 1) {
                dims_str += ",";
            }
        }
        if (dims.size() == 1) {
            dims_str += ",)";
        } else {
            dims_str += ")";
        }
        return dims_str;
    }

    bool str2dims(const std::string &str, std::vector<int32_t> &dims) {
        dims.clear();
        int start = 1;
        int size = str.size() - 1;

        for (int i = 1; i < size; i++) {
            if (str[i] == ',') {
                dims.push_back(std::stoi(str.substr(start, i - start)));
                start = i + 1;
            }
        }
        if (start < size) {
            dims.push_back(std::stoi(str.substr(start)));
        }
        return true;
    }
private:
    redisContext *mCtx;
    std::string   mIp;
    int32_t       mPort;
    std::string   mPassword;
    std::map<std::string, std::string> mTypeMapCpp2Py;
};

#endif
