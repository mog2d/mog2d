#import "mog/core/FileUtilsNative.h"
#import "mog/Constants.h"
#import "mog/core/Engine.h"
#import <Foundation/Foundation.h>

using namespace mog;

string FileUtilsNative::readTextAsset(string filename) {
    NSString *filenameStr = [NSString stringWithFormat:@"assets_mac/%s", filename.c_str()];
    NSString *path = [[NSBundle mainBundle] pathForResource:filenameStr ofType:nil];
    if (path == nil) {
        filenameStr = [NSString stringWithFormat:@"assets/%s", filename.c_str()];
        path = [[NSBundle mainBundle] pathForResource:filenameStr ofType:nil];
        if (path == nil) {
            return "";
        }
    }
    
    NSError *error;
    NSString *content = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:&error];
    if (error) {
        LOGE([error.description UTF8String]);
        LOGE(filename.c_str());
        return "";
    }
    
    return string([content UTF8String]);
}

bool FileUtilsNative::readBytesAsset(string filename, unsigned char **data, int *len) {
    NSString *filenameStr = [NSString stringWithFormat:@"assets_mac/%s", filename.c_str()];
    NSString *path = [[NSBundle mainBundle] pathForResource:filenameStr ofType:nil];
    if (path == nil) {
        filenameStr = [NSString stringWithFormat:@"assets/%s", filename.c_str()];
        path = [[NSBundle mainBundle] pathForResource:filenameStr ofType:nil];
        if (path == nil) {
            *data = nullptr;
            *len = 0;
            return false;
        }
    }
    
    NSData *nsData = [NSData dataWithContentsOfFile:path];
    *len = (int)nsData.length;
    *data = (unsigned char *)malloc(sizeof(char) * *len);
    memcpy(*data, (unsigned char *)nsData.bytes, sizeof(char) * *len);
    return true;
}

string FileUtilsNative::getDocumentsDirectory() {
    NSString *path = [NSHomeDirectory() stringByAppendingString:@"/Documents"];
    return [path UTF8String];
}

string FileUtilsNative::getCachesDirectory() {
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
    return [[paths objectAtIndex:0] UTF8String];
}
