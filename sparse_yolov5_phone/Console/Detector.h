#pragma once

class Detector : public QThread
{
	Q_OBJECT
public:
	Detector(const string& strModelPath, const string& strCaptureDir, bool bShowCam);
	virtual ~Detector(void);

	void run();
	void stop();

signals:
	void ShowBlockScreen(int nClassId, bool fShow);
	void ShowCam(cv::Mat img);

private:
	EventWaitHandle* m_pStopEvent;

	string m_strModelPath;
	string m_strCaptureDir;

	bool m_bShowCam;
};
