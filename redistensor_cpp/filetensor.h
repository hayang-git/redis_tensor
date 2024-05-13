#ifndef __FILE_TENSOR_H__
#define __FILE_TENSOR_H__

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <sys/stat.h>

template <typename T>
class FileTensor {
public:
    FileTensor(const std::string &rootpath) 
        : mRootPath("")
        , mRootExist(false)
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

        if (rootpath.back() != '/') {
            mRootPath = rootpath + "/";
        } else {
            mRootPath = rootpath;
        }

        if (access(mRootPath.c_str(), F_OK) == 0) {
            mRootExist = true;
        } else {
            if (mkdir(mRootPath.c_str(), 0777) == 0) {
                mRootExist = true;
            } else {
                printf("create root path failed(%s)\n", strerror(errno));
            }
        }
    }

    ~FileTensor() {
    }
    
    bool set(const std::string &key, const std::vector<int32_t> &dims, const std::vector<T> &value) {
        if (!mRootExist) {
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

        FILE *fp = fopen((mRootPath + key_tensor).c_str(), "wb");
        if (nullptr == fp) {
            printf("tensor file open failed(%s)\n", strerror(errno));
            return false;
        }

        int32_t wsize = fwrite(value.data(), sizeof(T), value.size(), fp);
        if (wsize != value.size()) {
            printf("tensor file write failed size error\n");
            fclose(fp);
            return false;
        }
        fclose(fp);

        fp = fopen((mRootPath + key_shape).c_str(), "wb");
        if (nullptr == fp) {
            printf("shape file open failed(%s)\n", strerror(errno));
            return false;
        }

        wsize = fwrite(dims_str.c_str(), sizeof(char), dims_str.size(), fp);
        if (wsize != dims_str.size()) {
            printf("shape file write failed size error\n");
            fclose(fp);
            return false;
        }

        fclose(fp);

        fp = fopen((mRootPath + key_dtype).c_str(), "wb");

        if (nullptr == fp) {
            printf("dtype file open failed(%s)\n", strerror(errno));
            return false;
        }

        wsize = fwrite(mTypeMapCpp2Py[typeName].c_str(), sizeof(char), mTypeMapCpp2Py[typeName].size(), fp);
        if (wsize != mTypeMapCpp2Py[typeName].size()) {
            printf("dtype file write failed size error\n");
            fclose(fp);
            return false;
        }

        fclose(fp);

        return true;
    }

    bool set(const std::string &key, const std::vector<int32_t> &dims, T *value, int32_t size) {
        if (!mRootExist) {
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

        FILE *fp = fopen((mRootPath + key_tensor).c_str(), "wb");
        if (nullptr == fp) {
            printf("tensor file open failed(%s)\n", strerror(errno));
            return false;
        }

        int32_t wsize = fwrite(value, sizeof(T), size, fp);
        if (wsize != size) {
            printf("tensor file write failed size error\n");
            fclose(fp);
            return false;
        }
        fclose(fp);

        fp = fopen((mRootPath + key_shape).c_str(), "wb");
        if (nullptr == fp) {
            printf("shape file open failed(%s)\n", strerror(errno));
            return false;
        }

        wsize = fwrite(dims_str.c_str(), sizeof(char), dims_str.size(), fp);
        if (wsize != dims_str.size()) {
            printf("shape file write failed size error\n");
            fclose(fp);
            return false;
        }

        fclose(fp);

        fp = fopen((mRootPath + key_dtype).c_str(), "wb");

        if (nullptr == fp) {
            printf("dtype file open failed(%s)\n", strerror(errno));
            return false;
        }

        wsize = fwrite(mTypeMapCpp2Py[typeName].c_str(), sizeof(char), mTypeMapCpp2Py[typeName].size(), fp);
        if (wsize != mTypeMapCpp2Py[typeName].size()) {
            printf("dtype file write failed size error\n");
            fclose(fp);
            return false;
        }

        fclose(fp);

        
        return true;
    }

    bool get(const std::string &key, std::vector<int32_t> &dims, std::vector<T> &value) {
        if (!mRootExist) {
            return false;
        }
        std::string key_tensor = key + "_tensor";
        std::string key_shape  = key + "_shape";
        std::string key_dtype  = key + "_dtype";

        FILE *fp = fopen((mRootPath + key_shape).c_str(), "rb");
        if (nullptr == fp) {
            printf("shape file open failed(%s)\n", strerror(errno));
            return false;
        }

        int32_t size = filesize(fp);
        std::string dims_str(size, '\0');
        int32_t rsize = fread(const_cast<char *>(dims_str.data()), sizeof(char), size, fp);
        if (rsize != size) {
            printf("shape file read failed size error\n");
            fclose(fp);
            return false;
        }
        fclose(fp);

        if (!str2dims(dims_str, dims)) {
            printf("shape file read failed format error\n");
            return false;
        }

        fp = fopen((mRootPath + key_tensor).c_str(), "rb");
        if (nullptr == fp) {
            printf("tensor file open failed(%s)\n", strerror(errno));
            return false;
        }

        size = filesize(fp);

        value.resize(size / sizeof(T));
        rsize = fread(value.data(), sizeof(T), value.size(), fp);
        if (rsize != value.size()) {
            printf("tensor file read failed size error\n");
            fclose(fp);
            return false;
        }

        fclose(fp);

        fp = fopen((mRootPath + key_dtype).c_str(), "rb");
        if (nullptr == fp) {
            printf("dtype file open failed(%s)\n", strerror(errno));
            return false;
        }

        size = filesize(fp);

        std::string dtype(size, '\0');
        rsize = fread(const_cast<char *>(dtype.data()), sizeof(char), size, fp);
        if (rsize != size) {
            printf("dtype file read failed size error\n");
            fclose(fp);
            return false;
        }
        fclose(fp);
        return true;        
    }

private:
    int32_t filesize(FILE *fp) {
        if (nullptr == fp) {
            return 0;
        }

        fseek(fp, 0, SEEK_END);
        int32_t size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        return size;
    }

    std::string dims2str(const std::vector<int32_t> &dims) {
        std::string dims_str("(");
        for (int i = 0; i < dims.size(); i++) {
            dims_str += std::to_string(dims[i]);
            if (i < dims.size() - 1) {
                dims_str += ",";
            }
        }
        if (dims.size() == 1) {
            dims_str + ",)";
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
    std::string mRootPath;
    bool mRootExist;
    std::map<std::string, std::string> mTypeMapCpp2Py;
};

#endif
