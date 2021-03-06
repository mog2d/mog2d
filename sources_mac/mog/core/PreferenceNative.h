#ifndef PreferenceNative_h
#define PreferenceNative_h

#include <string>

namespace mog {
    class PreferenceNative {
    public:
        static void setInt(const char *key, int intVal);
        static int getInt(const char *key, int defaultVal = 0);

        static void setDouble(const char *key, double doubleVal);
        static double getDouble(const char *key, double defaultVal = 0);

        static void setString(const char *key, std::string strVal);
        static std::string getString(const char *key, std::string defaultVal = nullptr);
        
        static void setBool(const char *key, bool boolVal);
        static bool getBool(const char *key, bool defaultVal = false);
    };
}

#endif /* PreferenceNative_h */
