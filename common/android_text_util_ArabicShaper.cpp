/*
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define LOG_TAG "AndroidUnicode"

#include "android_text_util_ArabicShaper.h"
#include <android_runtime/AndroidRuntime.h>
#include "unicode/ubidi.h"
#include "unicode/uchar.h"
#include "unicode/ushape.h"
#include <android/log.h>

extern "C" {

static JNINativeMethod gArabicShaperMethods[] = {
		/* name, signature, funcPtr */
		{"reorderReshapeBidiText", "([C[CII)I",
			(void*)Java_android_text_util_ArabicShaper_reorderReshapeBidiText},
		{"reshapeArabicText", "([C[CII)I",
			(void*)Java_android_text_util_ArabicShaper_reshapeArabicText}
};

static int registerNativeMethods(JNIEnv* env, const char* className,
		JNINativeMethod* gMethods, int numMethods)
{
	jclass clazz;

	clazz = env->FindClass(className);
	if (clazz == NULL)
		return JNI_FALSE;

	if (env->RegisterNatives(clazz, gMethods, numMethods) < 0)
		return JNI_FALSE;

	return JNI_TRUE;
}

static int registerNatives(JNIEnv* env)
{
	if (!registerNativeMethods(env, "android/text/util/ArabicShaper",
			gArabicShaperMethods, sizeof(gArabicShaperMethods) / sizeof(gArabicShaperMethods[0])))
		return JNI_FALSE;

	return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jint result = -1;

	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
		goto bail;

	//assert(env != NULL);

	if (!registerNatives(env))
		goto bail;

	/* success -- return valid version number */
	result = JNI_VERSION_1_4;

	bail:
	return result;
}


/*
 * Class:     android_text_util_ArabicShaper
 * Method:    reorderReshapeBidiText
 * Signature: ([C[CII)I
 */
JNIEXPORT jint JNICALL Java_android_text_util_ArabicShaper_reorderReshapeBidiText
  (JNIEnv* env, jclass c, jcharArray srcArray, jcharArray destArray, jint offset, jint n) {
    bool hasErrors = false;
    jint outputSize = 0;
    UChar *intermediate = new UChar[n];
    UChar *output = new UChar[n];
    UErrorCode status = U_ZERO_ERROR;

    UBiDi *para = ubidi_openSized(n, 0, &status);

    ubidi_setReorderingMode(para, UBIDI_REORDER_INVERSE_LIKE_DIRECT);

    jchar* src = env->GetCharArrayElements(srcArray, NULL);

    if (src != NULL && para != NULL && U_SUCCESS(status)) {

        ubidi_setPara(para, src+offset, n, UBIDI_DEFAULT_RTL, NULL, &status);

        if (U_SUCCESS(status)) {

            ubidi_writeReordered(para, intermediate, n, UBIDI_DO_MIRRORING | UBIDI_REMOVE_BIDI_CONTROLS, &status);

            if (U_SUCCESS(status)) {

                outputSize = u_shapeArabic(intermediate, n, output, n, U_SHAPE_TEXT_DIRECTION_VISUAL_LTR | U_SHAPE_LETTERS_SHAPE | U_SHAPE_LENGTH_FIXED_SPACES_AT_END, &status);

                if (U_SUCCESS(status))
                    env->SetCharArrayRegion(destArray, 0, outputSize, output);
                else
                    hasErrors = true;
            } else
                hasErrors = true;
        } else
            hasErrors = true;
    } else
        hasErrors = true;

    delete [] intermediate;
    delete [] output;

    if (para != NULL)
        ubidi_close(para);

    env->ReleaseCharArrayElements(srcArray, src, JNI_ABORT);

    if (hasErrors) {
        jclass Exception = env->FindClass("java/lang/RuntimeException");
        env->ThrowNew(Exception,NULL);
    }
        //jniThrowException(env, "java/lang/RuntimeException", NULL);

    return outputSize;


 }

/*
 * Class:     android_text_util_ArabicShaper
 * Method:    reshapeArabicText
 * Signature: ([C[CII)I
 */
JNIEXPORT jint JNICALL Java_android_text_util_ArabicShaper_reshapeArabicText
  (JNIEnv *env, jclass c, jcharArray srcArray, jcharArray destArray, jint offset, jint n) {
    bool hasErrors = false;
    jint outputSize = 0;
    UChar *intermediate = new UChar[n];
    UChar *intermediate2 = new UChar[n];
    UChar *output = new UChar[n];
    UErrorCode status = U_ZERO_ERROR;

    jchar* src = env->GetCharArrayElements(srcArray, NULL);

    if (src != NULL) {

        ubidi_writeReverse (src+offset, n, intermediate, n, UBIDI_DO_MIRRORING | UBIDI_REMOVE_BIDI_CONTROLS, &status);

        if (U_SUCCESS(status)) {
            outputSize = u_shapeArabic(intermediate, n, intermediate2, n, U_SHAPE_TEXT_DIRECTION_VISUAL_LTR | U_SHAPE_LETTERS_SHAPE | U_SHAPE_LENGTH_FIXED_SPACES_AT_END, &status);

            if (U_SUCCESS(status)) {

                ubidi_writeReverse (intermediate2, n, output, n, UBIDI_REMOVE_BIDI_CONTROLS, &status);

                env->SetCharArrayRegion(destArray, 0, outputSize, output);
            } else
                hasErrors = true;
        } else
            hasErrors = true;
    } else
        hasErrors = true;

    delete [] intermediate;
    delete [] intermediate2;
    delete [] output;

    env->ReleaseCharArrayElements(srcArray, src, JNI_ABORT);

    if (hasErrors) {
            jclass Exception = env->FindClass("java/lang/RuntimeException");
            env->ThrowNew(Exception,NULL);
    }


    return outputSize;
}

}
