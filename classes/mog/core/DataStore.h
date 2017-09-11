#ifndef DataStore_h
#define DataStore_h

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include "mog/core/FileUtils.h"
#include "mog/core/Data.h"
#include "mog/libs/sha256.h"

static inline string _getStoreDirectory() {
    return mog::FileUtils::getDocumentsDirectory() + "/data_store/";
}

static inline string _getStoreFilePath(string key) {
    return _getStoreDirectory() + sha256(key);
}


using namespace std;

namespace mog {
    
    class DataStore {
    public:
        template <class T, typename enable_if<is_base_of<Data, T>::value>::type*& = enabler>
        static T getData(string key) {
            auto defaultValue = T();
            defaultValue.type = DataType::Null;
            return getData<T>(key, defaultValue);
        }

        template <class T, typename enable_if<is_base_of<Data, T>::value>::type*& = enabler>
        static T getData(string key, const T &defaultValue) {
            if (!DataStore::hasKey(key)) {
                return defaultValue;
            }
            
            auto data = DataStore::caches[key];
            if (data) {
                return *static_pointer_cast<T>(data).get();
            } else {
                auto d = deserialize<T>(key);
                DataStore::caches[key] = shared_ptr<T>(new T(d));
                return d;
            }
        }

        template <class T, typename enable_if<is_base_of<Data, T>::value>::type*& = enabler>
        static void setData(string key, const T &value, bool immediatelySave = false) {
            DataStore::caches[key] = shared_ptr<T>(new T(value));
            if (immediatelySave) {
                DataStore::save(key);
            } else {
                DataStore::unsaved[key] = true;
            }
        }
        
        static bool hasKey(string key);
        static void remove(string key);
        static void removeAll();
        static void save();
        static void save(string key);
        static void enableEncryption(string key);
        
    private:
        static unordered_map<string, shared_ptr<Data>> caches;
        static unordered_map<string, bool> unsaved;
        static string cryptKey;
        
        template <class T, typename enable_if<is_base_of<Data, T>::value>::type*& = enabler>
        static void serialize(string key, T &data) {
            struct stat st;
            if (stat(_getStoreDirectory().c_str(), &st) == -1) {
                mkdir(_getStoreDirectory().c_str(), S_IRWXU|S_IRWXG);
            }
            
            string file = _getStoreFilePath(key);
            ofstream fout;
            fout.open(file, ios::out|ios::binary);
            data.write(fout);
            fout.close();
        }
        
        template <class T, typename enable_if<is_base_of<Data, T>::value>::type*& = enabler>
        static T deserialize(string key) {
            T data;
            if (!hasKey(key)) {
                return data;
            }
            
            string file = _getStoreFilePath(key);
            ifstream fin;
            fin.open(file, ios::in|ios::binary);
            data.read(fin);
            fin.close();

            return data;
        }
    };
}

#endif /* DataStore_h */
