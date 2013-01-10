/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __ANDROID_HAL_CAMERA2_TESTS_STREAM_FIXTURE__
#define __ANDROID_HAL_CAMERA2_TESTS_STREAM_FIXTURE__

#include <gtest/gtest.h>
#include <iostream>

#include <gui/CpuConsumer.h>
#include <gui/SurfaceTextureClient.h>

#include "CameraModuleFixture.h"
#include "TestExtensions.h"

namespace android {
namespace camera2 {
namespace tests {

struct CameraStreamParams {
    int mCameraId;
    int mFormat;
    int mHeapCount;
};

inline void PrintTo(const CameraStreamParams& p, ::std::ostream* os) {
    *os <<  "{ ";
    *os <<  "CameraID: "  << p.mCameraId << ", ";
    *os <<  "Format: "    << p.mCameraId << ", ";
    *os <<  "HeapCount: " << p.mCameraId;
    *os << " }";
}

class CameraStreamFixture
    : public CameraModuleFixture</*InfoQuirk*/true> {

public:
    CameraStreamFixture(CameraStreamParams p)
    : CameraModuleFixture(p.mCameraId) {
        TEST_EXTENSION_FORKING_CONSTRUCTOR;

        mParam = p;

        SetUp();
    }

    ~CameraStreamFixture() {
        TEST_EXTENSION_FORKING_DESTRUCTOR;

        TearDown();
    }

private:

    void SetUp() {
        TEST_EXTENSION_FORKING_SET_UP;

        CameraStreamParams p = mParam;
        sp<Camera2Device> device = mDevice;

        /* use an arbitrary w,h */
        {
            const int tag = ANDROID_SCALER_AVAILABLE_PROCESSED_SIZES;

            const android::camera2::CameraMetadata& staticInfo = device->info();
            camera_metadata_ro_entry entry = staticInfo.find(tag);
            ASSERT_NE(0u, entry.count)
                << "Missing tag android.scaler.availableProcessedSizes";

            ASSERT_LE(2u, entry.count);
            /* this seems like it would always be the smallest w,h
               but we actually make no contract that it's sorted asc */;
            mWidth = entry.data.i32[0];
            mHeight = entry.data.i32[1];
        }
    }
    void TearDown() {
        TEST_EXTENSION_FORKING_TEAR_DOWN;
    }

protected:

    void CreateStream() {
        sp<Camera2Device> device = mDevice;
        CameraStreamParams p = mParam;

        mCpuConsumer = new CpuConsumer(p.mHeapCount);
        mCpuConsumer->setName(String8("CameraStreamTest::mCpuConsumer"));

        mNativeWindow = new SurfaceTextureClient(
            mCpuConsumer->getProducerInterface());

        ASSERT_EQ(OK,
            device->createStream(mNativeWindow,
                mWidth, mHeight, p.mFormat, /*size (for jpegs)*/0,
                &mStreamId));

        ASSERT_NE(-1, mStreamId);
    }

    void DeleteStream() {
        ASSERT_EQ(OK, mDevice->deleteStream(mStreamId));
    }

    int mWidth;
    int mHeight;

    int mStreamId;
    android::sp<CpuConsumer>         mCpuConsumer;
    android::sp<ANativeWindow>       mNativeWindow;

private:
    CameraStreamParams mParam;
};

}
}
}

#endif
