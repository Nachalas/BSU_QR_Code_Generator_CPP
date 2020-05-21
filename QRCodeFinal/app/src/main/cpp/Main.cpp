#include <iostream>
#include <string>
#include <fstream>
#include <jni.h>
#include "QRGenerator.h"

using namespace std;

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_qrcodefinal_MainActivity_Main(JNIEnv *env, jobject thiz, jstring input) {
    const char *str = env->GetStringUTFChars(input, NULL);
    string test = str;
    const QRCode qr = encodeString(test, ErrCorrLevel::LOW);
    string test2 = qr.toSvgString(4);
    const char* chr = test2.c_str();
    return env->NewStringUTF(chr);
}