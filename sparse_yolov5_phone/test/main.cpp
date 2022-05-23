#pragma once

#include "stdafx.h"

#include <opencv2/opencv.hpp>

#include <Detect/all.h>

#include "../Base/Defines.h"

void TestDetect(lstring strModelPath, lstring strCaptureDir)
{
    OrtDetector detector(std::string(strModelPath), false,
        cv::Size(ABG_DETECT_INPUT_WIDTH, ABG_DETECT_INPUT_HEIGHT));

    Decider deciderMobile(0, ABG_DETECT_MIN_AREA, ABG_DETECT_MOBILE_CONF_THRESHOLD, ABG_DETECT_MOBILE_CONF_VIEW,
        ABG_DETECT_MOBILE_ZSCORE_THRESHOLD, ABG_DETECT_MOBILE_ZSCORE_WINDOW, true);
    //Decider deciderPerson(1, ABG_DETECT_MIN_AREA, ABG_DETECT_PERSON_CONF_THRESHOLD, ABG_DETECT_PERSON_CONF_VIEW,
    //    ABG_DETECT_PERSON_ZSCORE_THRESHOLD, ABG_DETECT_PERSON_ZSCORE_WINDOW, false);

    cv::setNumThreads(1);
    cv::VideoCapture cap = cv::VideoCapture(0);
    
    cap.set(cv::CAP_PROP_FRAME_WIDTH, ABG_DETECT_CAP_FRAME_WIDTH);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, ABG_DETECT_CAP_FRAME_HEIGHT);
    cap.set(cv::CAP_PROP_FPS, ABG_DETECT_CAP_FPS);

    bool bMobileExisted = false;
    bool bPersonExisted = true;

    LEventWaitHandle ewhDelay(false, EEventResetMode::ManualReset);
    int nDelay = 1000 / ABG_DETECT_CAP_FPS;

    while (cap.isOpened())
    {
        clock_t start = clock();

        cv::Mat frame;
        cap.read(frame);
        if (frame.empty())
        {
            std::cout << "Read frame failed!" << std::endl;
            break;
        }

        cv::Mat img;
        cv::resize(frame, img, cv::Size(ABG_DETECT_IMAGE_WIDTH, ABG_DETECT_IMAGE_HEIGHT));
        std::vector<Detection> detections =
            detector.detect(img, ABG_DETECT_CONF_THRESHOLD, ABG_DETECT_IOU_THRESHOLD);

        LDateTime dtNow = LDateTime::GetNow();
        std::string strNowTime(dtNow.ToShortTimeString());
        bool bCapture = false;

        Decision decisionMobile = deciderMobile.decide(detections);
        if (bMobileExisted != decisionMobile.bExists)
        {
            std::cout << "[" << strNowTime << "] "
                << "Mobile " << (decisionMobile.bExists ? "O" : "X")
                << ", Conf=" << decisionMobile.fConf
                << ", Z=" << decisionMobile.fZScore
                << ", A=" << decisionMobile.nArea << std::endl;

            bCapture = decisionMobile.bExists;
            bMobileExisted = decisionMobile.bExists;
        }

        //Decision decisionPerson = deciderPerson.decide(detections);
        //if (bPersonExisted != decisionPerson.bExists)
        //{
        //    std::cout << "[" << strNowTime << "] "
        //        << "Person " << (decisionPerson.bExists ? "O" : "X")
        //        << ", Conf=" << decisionPerson.fConf
        //        << ", Z=" << decisionPerson.fZScore << std::endl;

        //    bCapture = decisionPerson.bExists;
        //    bPersonExisted = decisionPerson.bExists;
        //}

        visualizeDetection(img, detections, 0);

        if (bCapture)
        {
            lstring strCapPath = LString::Format(L"%ws\\cap_%llx.png",
                (const wchar_t*)strCaptureDir, dtNow.GetTicks());
            cv::imwrite(std::string(strCapPath), img);
        }

        clock_t elapsed = clock() - start;
        if (elapsed < nDelay)
            ewhDelay.WaitOne(nDelay - elapsed);

        float fFPS = (1000.0f / (clock() - start));
        lstring strMsg = LString::Format(L"FPS: %.1f", fFPS);
        cv::putText(img, std::string(strMsg), cv::Point(50, 50),
            cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

        cv::imshow("", img);
        if (cv::waitKey(1) == 27) break;
    }
}

int main(int argc, char* argv[]) 
{
    ::NALInitialize(false);

    try
    {
        lstring strModelPath = LApplication::GetStartupPath() + L"\\abg.onnx";

        lstring strCaptureDir = LApplication::GetStartupPath() + L"\\cap";
        if (INVALID_FILE_ATTRIBUTES == ::GetFileAttributesW(strCaptureDir))
            ::CreateDirectoryW(strCaptureDir, NULL);

        TestDetect(strModelPath, strCaptureDir);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    catch (LException* px)
    {
        LConsole::WriteLine(px->ToString());
        delete px;
    }
    catch (...)
    {
        LConsole::WriteLine(L"unknown");
    }

    ::NALFinalize();
    return 0;
}
