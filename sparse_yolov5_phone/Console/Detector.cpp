#include "stdafx.h"
#include "ConsoleMain.h"
#include "Detector.h"

#include "wpp.h"
#include "Detector.tmh"

Detector::Detector(const string& strModelPath, const string& strCaptureDir, bool bShowCam) :
    m_strModelPath(strModelPath),
    m_strCaptureDir(strCaptureDir),
    m_bShowCam(bShowCam)
{
    m_pStopEvent = new EventWaitHandle(false, EEventResetMode::ManualReset);
}

Detector::~Detector(void)
{
    delete m_pStopEvent;
}

void Detector::run()
{
    OrtDetector detector(std::string(m_strModelPath), false,
        cv::Size(ABG_DETECT_INPUT_WIDTH, ABG_DETECT_INPUT_HEIGHT));

    Decider deciderMobile(0, ABG_DETECT_MIN_AREA, ABG_DETECT_MOBILE_CONF_THRESHOLD, ABG_DETECT_MOBILE_CONF_VIEW,
        ABG_DETECT_MOBILE_ZSCORE_THRESHOLD, ABG_DETECT_MOBILE_ZSCORE_WINDOW, true);
    //Decider deciderPerson(1, ABG_DETECT_MIN_AREA, ABG_DETECT_PERSON_CONF_THRESHOLD, ABG_DETECT_PERSON_CONF_VIEW,
    //    ABG_DETECT_PERSON_ZSCORE_THRESHOLD, ABG_DETECT_PERSON_ZSCORE_WINDOW, false);

    cv::setNumThreads(1);
    cv::VideoCapture cap = cv::VideoCapture(0);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 320);
    cap.set(cv::CAP_PROP_FPS, 5);

    bool bMobileExisted = false;
    bool bPersonExisted = true;

    int nWaitTime = 0;
    int nDelayTime = 1000 / ABG_DETECT_CAP_FPS;

    while (cap.isOpened() && !m_pStopEvent->WaitOne(nWaitTime))
    {
        clock_t start = clock();

        cv::Mat frame;
        cap.read(frame);
        if (frame.empty())
        {
            // !!TO-DO!!
            continue;
        }

        cv::Mat img;
        cv::resize(frame, img, cv::Size(ABG_DETECT_IMAGE_WIDTH, ABG_DETECT_IMAGE_HEIGHT));
        std::vector<Detection> detections =
            detector.detect(img, ABG_DETECT_CONF_THRESHOLD, ABG_DETECT_IOU_THRESHOLD);

        bool bCapture = false;

        Decision decisionMobile = deciderMobile.decide(detections);
        if (bMobileExisted != decisionMobile.bExists)
        {
            emit ShowBlockScreen(0, decisionMobile.bExists);
            bCapture = decisionMobile.bExists;
            bMobileExisted = decisionMobile.bExists;

            _TRACE_I(Application,
                "Detector: Mobile - Exists=%d, Conf=%.2f, Z=%.2f, A=%d",
                decisionMobile.bExists, decisionMobile.fConf,
                decisionMobile.fZScore, decisionMobile.nArea);
        }

        //Decision decisionPerson = deciderPerson.decide(detections);
        //if (bPersonExisted != decisionPerson.bExists)
        //{
        //    if (decisionPerson.bExists)
        //    {
        //        AppAPI::RaiseEventExisting(ABG_EVENT_CONSOLE_HIDE_PERSONABSENCE);
        //        bCapture = true;
        //    }
        //    else
        //    {
        //        AppAPI::RaiseEventExisting(ABG_EVENT_CONSOLE_SHOW_PERSONABSENCE);
        //    }

        //    _TRACE_I(Application, "Detector: Person - Exists=%d, Conf=%.2f, Z=%.2f",
        //        decisionPerson.bExists, decisionPerson.fConf, decisionPerson.fZScore);

        //    bCapture = decisionPerson.bExists;
        //    bPersonExisted = decisionPerson.bExists;
        //}

        visualizeDetection(img, detections, ABG_DETECT_MIN_AREA);

        if (bCapture)
        {
            string strCapPath = String::Format(L"%ws\\detect_%llx.png",
                (const wchar_t*)m_strCaptureDir, DateTime::GetNow().GetTicks());

            cv::imwrite(std::string(strCapPath), img);
        }

        if (m_bShowCam)
            emit ShowCam(img);

        clock_t elapsed = clock() - start;
        nWaitTime = nDelayTime - elapsed;

        if (nWaitTime < 0)
            nWaitTime = 0;
    }
}

void Detector::stop()
{
    m_pStopEvent->Set();
}
