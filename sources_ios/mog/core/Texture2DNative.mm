#import "mog/Constants.h"
#import "Texture2DNative.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <CoreText/CoreText.h>
#import "Texture2D.h"

using namespace mog;

unordered_map<string, string> Texture2DNative::registeredFontNames;

string Texture2DNative::registerCustomFont(const char *fontFilename) {
    string fontFilenameStr = fontFilename;
    if (registeredFontNames.count(fontFilenameStr) > 0) {
        return registeredFontNames[fontFilenameStr];
    }
    
    NSString *filenameStr = [NSString stringWithFormat:@"assets_ios/%s", fontFilename];
    NSString *path = [[NSBundle mainBundle] pathForResource:filenameStr ofType:nil];
    if (path == nil) {
        filenameStr = [NSString stringWithFormat:@"assets/%s", fontFilename];
        path = [[NSBundle mainBundle] pathForResource:filenameStr ofType:nil];
        if (path == nil) {
            return "";
        }
    }
    
    NSData *nsData = [NSData dataWithContentsOfFile:path];
    CFErrorRef error;
    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)nsData);
    CGFontRef font = CGFontCreateWithDataProvider(provider);
    NSString *fontName = (__bridge NSString *)CGFontCopyPostScriptName(font);
    if (!CTFontManagerRegisterGraphicsFont(font, &error)) {
        CFStringRef errorDescription = CFErrorCopyDescription(error);
        NSLog(@"Failed to load font: %@", errorDescription);
        CFRelease(errorDescription);
    }
    CFRelease(font);
    CFRelease(provider);
    
    string fontNameStr = fontName.UTF8String;
    registeredFontNames[fontFilenameStr] = fontNameStr;
    
    return fontNameStr;
}

void Texture2DNative::loadFontTexture(Texture2D *tex2d, const char *text, float fontSize, const char *fontFilename, float height) {
    UIFont* font = nil;
    
    NSString *textStr = [NSString stringWithUTF8String:text];
    if (fontFilename != nullptr && strlen(fontFilename) > 0) {
        string fontFace = registerCustomFont(fontFilename);
        if (fontFace.length() == 0) {
            fontFace = fontFilename;
        }
        NSString *fontFaceStr = [NSString stringWithUTF8String:fontFace.c_str()];
        if (fontSize == 0) {
            fontSize = (float)[UIFont systemFontSize];
        }
        if (fontFaceStr && fontFaceStr.length > 0) {
            font = [UIFont fontWithName:fontFaceStr size:fontSize];
        }
    }
    if (!font) {
        font = [UIFont systemFontOfSize:fontSize];
    }
    
    CGSize textSize = [textStr sizeWithAttributes:@{NSFontAttributeName:font}];
    int textWidth  = (int)(textSize.width + 0.5f);
    int textHeight = (int)(textSize.height + 0.5f);
    if (height > 0) {
        textHeight = (int)(height + 0.5f);
    }

    GLubyte *bitmap = (GLubyte *)rpcalloc(textWidth * textHeight * 4, sizeof(GLubyte));
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef context = CGBitmapContextCreate(bitmap, textWidth, textHeight,
                                                 8, textWidth * 4, colorSpace,
                                                 kCGImageAlphaPremultipliedLast);
    
    UIGraphicsPushContext(context);
    
    CGContextSetTextDrawingMode(context, kCGTextFill);
    
    [textStr drawAtPoint:CGPointZero withAttributes:@{NSFontAttributeName:font,
                                                      NSForegroundColorAttributeName:[UIColor whiteColor]}];
    
    UIGraphicsPopContext();
    
    CGColorSpaceRelease(colorSpace);
    CGContextRelease(context);
    
    // remove premultiplied alpha
    GLubyte rgb[3] = {255, 255, 255};
    for (int i = 0; i < textWidth * textHeight; i++) {
        memcpy(&bitmap[i * 4], rgb, sizeof(GLubyte) * 3);
    }
    
    tex2d->filename = "";
    tex2d->width = textWidth;
    tex2d->height = textHeight;
    tex2d->data = bitmap;
    tex2d->dataLength = textWidth * textHeight * 4;
    tex2d->textureType = TextureType::RGBA;
    tex2d->bitsPerPixel = 4;
    tex2d->isFlip = true;
}

string Texture2DNative::getLocalizedTextNative(const char *textKey, va_list args) {
    NSString *localizedStr = NSLocalizedString([NSString stringWithUTF8String:textKey], nil);
    if (!localizedStr) {
        return "";
    }
    
    const char *utf8Str = localizedStr.UTF8String;
    char *str = (char *)rpmalloc(sizeof(char) * strlen(utf8Str) + 4096);
    
    vsprintf(str, utf8Str, args);
    string ret = string(str);
    
    rpfree(str);
    
    return ret;
}