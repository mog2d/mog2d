#include <mutex>
#include <android/asset_manager_jni.h>
#include "mog/core/Engine.h"
#include "app/App.h"
#include "mog/Constants.h"
#include "mog/core/NativeClass.h"
#include "mog/os/AndroidHelper.h"

using namespace mog;

JavaVM *AndroidHelper::vm = nullptr;

class MogRenderer {
public:
    static MogRenderer *initialize() {
        MogRenderer::instance = new MogRenderer();
        return MogRenderer::instance;
    }
    static MogRenderer *getInstance() {
        return MogRenderer::instance;
    }

    void onCreate(JNIEnv* env, jobject obj, jobject jActivity, jobject jAssetManager, float scaleFactor) {
        this->engine = Engine::initInstance();

        JavaVM *vm;
        env->GetJavaVM(&vm);
        AndroidHelper::vm = vm;
        this->engine->setNativeObject(MOG_ACTIVITY, NativeObject::create(jActivity));
        this->engine->setNativeObject(MOG_AASET_MANAGER, NativeObject::create(jAssetManager));
        this->scaleFactor = scaleFactor;

        this->app = make_shared<App>();
        this->engine->setApp(app);
    }

    void onDestroy(JNIEnv* env, jobject obj) {
    }

    void onPause(JNIEnv* env, jobject obj) {
        Engine::getInstance()->stopEngine();
    }

    void onResume(JNIEnv* env, jobject obj) {
        Engine::getInstance()->startEngine();
    }

    void onStart(JNIEnv* env, jobject obj) {
    }

    void onStop(JNIEnv* env, jobject obj) {
    }

    void onDrawFrame(JNIEnv* env, jobject obj) {
        std::lock_guard<std::mutex> lock(mtx);
        this->engine->onDrawFrame(this->touches);
        this->touches.clear();
    }

    void onSurfaceCreated(JNIEnv* env, jobject obj) {
    }

    void onSurfaceChanged(JNIEnv* env, jobject obj, jint w, jint h) {
        this->engine->setDisplaySize(Size(w, h), this->scaleFactor);
        this->engine->setScreenSizeBasedOnHeight(BASE_SCREEN_HEIGHT);
        if (!this->initialized) {
            this->engine->initEngine();
            this->initialized = true;
        }
        this->engine->startEngine();
    }

    void onTouchEvent(JNIEnv* env, jobject obj, jint pointerId, jint touchAction, jfloat x, jfloat y) {
        NativeTouchAction action = (NativeTouchAction)touchAction;

        std::lock_guard<std::mutex> lock(mtx);
        switch (action) {
            case NativeTouchAction::Down:
                this->touches[pointerId] = TouchInput(TouchAction::TouchDown, pointerId, x, y);
                break;

            case NativeTouchAction::Up:
                if (this->touches.count(pointerId) > 0 &&
                        (this->touches[pointerId].action == TouchAction::TouchDown || this->touches[pointerId].action == TouchAction::TouchDownUp)) {
                    this->touches[pointerId] = TouchInput(TouchAction::TouchDownUp, pointerId, x, y);
                } else {
                    this->touches[pointerId] = TouchInput(TouchAction::TouchUp, pointerId, x, y);
                }
                break;

            case NativeTouchAction::Move:
                if (this->touches.count(pointerId) == 0 || this->touches[pointerId].action == TouchAction::TouchMove) {
                    this->touches[pointerId] = TouchInput(TouchAction::TouchMove, pointerId, x, y);
                }
                break;
        }
    }

private:
    enum NativeTouchAction {
        Down = 1,
        Up = 2,
        Move = 3,
    };

    static MogRenderer *instance;
    Engine *engine;
    shared_ptr<App> app;
    float scaleFactor;
    map<unsigned int, TouchInput> touches;
    std::mutex mtx;
    bool initialized = false;
};
MogRenderer *MogRenderer::instance;


extern "C" {
JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_onSurfaceCreated(JNIEnv* env, jobject obj) {
    MogRenderer::getInstance()->onSurfaceCreated(env, obj);
}

JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_onSurfaceChanged(JNIEnv* env, jobject obj, jint w, jint h) {
    MogRenderer::getInstance()->onSurfaceChanged(env, obj, w, h);
}

JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_onDrawFrame(JNIEnv* env, jobject obj) {
    MogRenderer::getInstance()->onDrawFrame(env, obj);
}

JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_onTouchEvent(JNIEnv* env, jobject obj, jint pointerId, jint touchAction, jfloat x, jfloat y) {
    MogRenderer::getInstance()->onTouchEvent(env, obj, pointerId, touchAction, x, y);
}

JNIEXPORT jint JNICALL Java_org_mog2d_MogJniBridge_getDefaultFps(JNIEnv* env, jobject obj) {
    return DEFAULT_FPS;
}


JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_onCreate(JNIEnv* env, jobject obj, jobject jActivity, jobject jAssetManager, jfloat scaleFactor) {
    MogRenderer::initialize()->onCreate(env, obj, jActivity, jAssetManager, scaleFactor);
}

JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_onDestroy(JNIEnv* env, jobject obj) {
    MogRenderer::getInstance()->onDestroy(env, obj);
}

JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_onPause(JNIEnv* env, jobject obj) {
    MogRenderer::getInstance()->onPause(env, obj);
}

JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_onResume(JNIEnv* env, jobject obj) {
    MogRenderer::getInstance()->onResume(env, obj);
}

JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_onStart(JNIEnv* env, jobject obj) {
    MogRenderer::getInstance()->onStart(env, obj);
}

JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_onStop(JNIEnv* env, jobject obj) {
    MogRenderer::getInstance()->onStop(env, obj);
}

void __convertToNArg(JNIEnv* env, jobjectArray params, NArg **args, int *len) {
    env->PushLocalFrame(128);

    *len = env->GetArrayLength(params);
    *args = new NArg[*len];

    jclass integerClass = env->FindClass("java/lang/Integer");
    jmethodID intValueId = env->GetMethodID(integerClass, "intValue", "()I");
    jclass longClass = env->FindClass("java/lang/Long");
    jmethodID longValueId = env->GetMethodID(longClass, "longValue", "()J");
    jclass charClass = env->FindClass("java/lang/Character");
    jmethodID charValueId = env->GetMethodID(charClass, "charValue", "()C");
    jclass byteClass = env->FindClass("java/lang/Byte");
    jmethodID byteValueId = env->GetMethodID(byteClass, "byteValue", "()B");
    jclass floatClass = env->FindClass("java/lang/Float");
    jmethodID floatValueId = env->GetMethodID(floatClass, "floatValue", "()F");
    jclass doubleClass = env->FindClass("java/lang/Double");
    jmethodID doubleValueId = env->GetMethodID(doubleClass, "doubleValue", "()D");
    jclass booleanClass = env->FindClass("java/lang/Boolean");
    jmethodID booleanValueId = env->GetMethodID(booleanClass, "booleanValue", "()Z");

    for (int i = 0; i < *len; i++) {
        jobject jobj = env->GetObjectArrayElement(params, i);
        if (jobj == NULL) {
            (*args)[i] = NArg();
        } else if (env->IsInstanceOf(jobj, integerClass)) {
            jint v = env->CallIntMethod(jobj, intValueId);
            (*args)[i] = NArg((int)v);
        } else if (env->IsInstanceOf(jobj, longClass)) {
            jlong v = env->CallLongMethod(jobj, longValueId);
            (*args)[i] = NArg((long)v);
        } else if (env->IsInstanceOf(jobj, charClass)) {
            jchar v = env->CallCharMethod(jobj, charValueId);
            (*args)[i] = NArg((int)v);
        } else if (env->IsInstanceOf(jobj, byteClass)) {
            jbyte v = env->CallByteMethod(jobj, byteValueId);
            (*args)[i] = NArg((int)v);
        } else if (env->IsInstanceOf(jobj, floatClass)) {
            jfloat v = env->CallFloatMethod(jobj, floatValueId);
            (*args)[i] = NArg((float)v);
        } else if (env->IsInstanceOf(jobj, doubleClass)) {
            jdouble v = env->CallDoubleMethod(jobj, doubleValueId);
            (*args)[i] = NArg((double) v);
        } else if (env->IsInstanceOf(jobj, booleanClass)) {
            jboolean v = env->CallBooleanMethod(jobj, booleanValueId);
            (*args)[i] = NArg((bool)v);
        } else {
            jobject go = env->NewGlobalRef(jobj);
            (*args)[i] = NArg(go);
        }
    }
    env->PopLocalFrame(NULL);
}

JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_runCallback(JNIEnv* env, jobject obj, jlong functionPtr, jobjectArray params) {
    NFunction *nf = (NFunction *)functionPtr;
    int len;
    NArg *args;
    __convertToNArg(env, params, &args, &len);
    nf->invoke(args, len);
    delete[] args;
}

JNIEXPORT void JNICALL Java_org_mog2d_MogJniBridge_releaseNativeFunction(JNIEnv* env, jobject obj, jlong functionPtr) {
    delete (NFunction *)functionPtr;
}

}