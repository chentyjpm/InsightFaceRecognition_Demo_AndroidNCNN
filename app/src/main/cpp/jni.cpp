// Tencent is pleased to support the open source community by making ncnn available.
//
// Copyright (C) 2017 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the BSD 3-Clause License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// https://opensource.org/licenses/BSD-3-Clause
//
// Unless required by applicable law or agreed to in writing, software distributed
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
// CONDITIONS OF ANY KIND, either express or implied. See the License for the
// specific language governing permissions and limitations under the License.

#include <stdio.h>
#include <time.h>
#include <vector>
#include <jni.h>

#include <android/log.h>

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "jni", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO , "jni", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN , "libssd", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "libssd", __VA_ARGS__))

#include "arcface/interface.h"


#ifdef __cplusplus
extern "C" {
#endif

#define FACE_DETECT_SIZEH   448
int sizeh, sizev;


int faceinfo2float(float *out, FaceInfo *in)
{
    if(sizeh == 0 || sizev == 0)
        return 20;
    *out = (float)in->x[0]/sizeh;out++;
    *out = (float)in->y[0]/sizev;out++;
    *out = (float)in->x[1]/sizeh;out++;
    *out = (float)in->y[1]/sizev;out++;
    *out = in->landmark[0];out++;
    *out = in->landmark[1];out++;
    *out = in->landmark[2];out++;
    *out = in->landmark[3];out++;
    *out = in->landmark[4];out++;
    *out = in->landmark[5];out++;
    *out = in->landmark[6];out++;
    *out = in->landmark[7];out++;
    *out = in->landmark[8];out++;
    *out = in->landmark[9];out++;
    *out = in->score;out++;
    *out = in->area;out++;
    *out = in->regreCoord[0];out++;
    *out = in->regreCoord[1];out++;
    *out = in->regreCoord[2];out++;
    *out = in->regreCoord[3];out++;
    return 20;
}

int float2faceinfo(FaceInfo *out, float *in)
{
    if(sizeh == 0 || sizev == 0)
        return 20;
    out->x[0]=(*in)*sizeh;in++;
    out->y[0]=(*in)*sizev;in++;
    out->x[1]=(*in)*sizeh;in++;
    out->y[1]=(*in)*sizev;in++;
    out->landmark[0]=*in;in++;
    out->landmark[1]=*in;in++;
    out->landmark[2]=*in;in++;
    out->landmark[3]=*in;in++;
    out->landmark[4]=*in;in++;
    out->landmark[5]=*in;in++;
    out->landmark[6]=*in;in++;
    out->landmark[7]=*in;in++;
    out->landmark[8]=*in;in++;
    out->landmark[9]=*in;in++;
    out->score=*in;in++;
    out->area=*in;in++;
    out->regreCoord[0]=*in;in++;
    out->regreCoord[1]=*in;in++;
    out->regreCoord[2]=*in;in++;
    out->regreCoord[3]=*in;in++;
    return 20;
}

extern "C" JNIEXPORT jfloatArray JNICALL
Java_com_chenty_testncnn_CameraNcnnFragment_detectface(JNIEnv *env, jobject thiz, jbyteArray frame, jint src_width,
                                                        jint src_height) {
    char *yuv_frame = (char*)env->GetPrimitiveArrayCritical(frame, NULL);

    int size = env->GetArrayLength(frame);
    int objectcnt = 0;
    int i;


    sizeh = FACE_DETECT_SIZEH;
    sizev = FACE_DETECT_SIZEH*src_height/src_width;

    //shift argb to rgba
    char *yuv = (char *)malloc(size);
    memcpy(yuv, yuv_frame, size);

    env->ReleasePrimitiveArrayCritical(frame, yuv_frame, JNI_ABORT);

    ncnn::Mat in = ncnn::Mat::from_pixels_resize((const unsigned char *)yuv, ncnn::Mat::PIXEL_GRAY2BGR, src_width, src_height, sizeh, sizev);
    LOGD("detect face  in %dX%d", src_width, src_height);
    vector<FaceInfo> faceinfo = face_detect(in);

    free(yuv);
    int gap = 20;
    int cnt = faceinfo.size();
    if(cnt)
    {
        float detect_out[240];
        float *out = detect_out;

        if(cnt > 240/gap)
            cnt = 240/gap;

        for (int i = 0 ; i < cnt ; i ++)
        {
            FaceInfo face = faceinfo[i];
            LOGD("get face %d %d %d %d", face.x[0], face.y[0], face.x[1], face.y[1]);
            int res = faceinfo2float(out, &face);
            out=out+res;
        }

        FaceInfo face = faceinfo[0];
        vector<float> feature = face_exactfeature(in, face);

        LOGD("get feature %d ", feature.size());
        float feature_f[256];
        jfloatArray detect = env->NewFloatArray(cnt*gap + 128);
        LOGD("vect2float %d", feature.size());
        out = feature_f;
        for(int i = 0 ; i < feature.size(); i++)
        {
            *out = feature[i];
            out++;
        }
        //vect2float(feature_f, feature);
        //memcpy(feature_f, &feature[0], feature.size()*sizeof(float));
        LOGD("set feature %d ", feature.size());
        env->SetFloatArrayRegion(detect,0,128, feature_f);
        LOGD("set face %d ", cnt*gap);
        env->SetFloatArrayRegion(detect, 128, cnt*gap, detect_out);

        return detect;
    } else{

        return nullptr;
    }
}



extern "C" JNIEXPORT jfloat JNICALL
Java_com_chenty_testncnn_CameraNcnnFragment_compareface(JNIEnv *env, jobject thiz, jfloatArray face0, jfloatArray face1) {

    float face0cache[128], face1cache[128];

    env->GetFloatArrayRegion(face0,0,128,face0cache);
    env->GetFloatArrayRegion(face1,0,128,face1cache);

    vector<float> face0feature(face0cache, face0cache+sizeof(face0cache)/sizeof(float));
    vector<float> face1feature(face1cache, face1cache+sizeof(face1cache)/sizeof(float));

    jfloat result = face_calcSimilar(face0feature, face1feature);
    LOGD("calcSimilar result %f\n", result);

    return result;


}


#ifdef __cplusplus  
}  
#endif