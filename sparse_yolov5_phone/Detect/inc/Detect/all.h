#pragma once

#include <Detect/OrtDetector.h>
#include <Detect/Decider.h>
#include <Detect/Utils.h>

using namespace Detect;

#ifndef BG_DETECT_IMPL
#ifdef _DEBUG
#pragma comment (lib, "abgDetectd.lib")
#pragma comment (lib, "opencv_world455d.lib")
#pragma comment (lib, "onnxruntime.lib")
#else
#pragma comment (lib, "abgDetect.lib")
#pragma comment (lib, "opencv_world455.lib")
#pragma comment (lib, "onnxruntime.lib")
#endif
#endif

