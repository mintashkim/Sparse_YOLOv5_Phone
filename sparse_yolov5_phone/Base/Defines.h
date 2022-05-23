#pragma once

#define SUPPORTED_WIN_VERSION_MIN							1005	// EOSName::Win10rs5
#define SUPPORTED_WIN_VERSION_MAX							1011	// EOSName::Win10_21h2

#define COMPANY_NAME										L"Ados"

#define APPLICATION_ID										L"abg"
#define APPLICATION_NAME									L"BossGo"
#define APPLICATION_LOWER_NAME								L"bossgo"

#define ABG_BINARY_CONSOLE									L"abgConsole.exe"
#define ABG_BINARY_WATCHERSERVICE							L"abgWatcherSvc.exe"

#define ABG_SERVICENAME_WATCHERSERVICE						L"abgWatcherSvc"

#define ABG_SERVICEDISPLAY_WATCHERSERVICE					L"ADOS BossGo Watcher Service"

#define ABG_MUTEX_CONSOLE									L"ADOS.BossGo.CNSL.MTX"
#define ABG_EVENT_CONSOLE_EXIT								L"ADOS.BossGo.CNSL.XT"

#define ABG_DETECT_INPUT_WIDTH								320
#define ABG_DETECT_INPUT_HEIGHT								320
#define ABG_DETECT_CAP_FRAME_WIDTH							320
#define ABG_DETECT_CAP_FRAME_HEIGHT							180
#define ABG_DETECT_IMAGE_WIDTH								640
#define ABG_DETECT_IMAGE_HEIGHT								360
#define ABG_DETECT_CAP_FPS									5
#define ABG_DETECT_MIN_AREA									2000
#define ABG_DETECT_CONF_THRESHOLD							0.5f
#define ABG_DETECT_IOU_THRESHOLD							0.5f
#define ABG_DETECT_MOBILE_CONF_THRESHOLD					0.6f
#define ABG_DETECT_MOBILE_CONF_VIEW							2
#define ABG_DETECT_MOBILE_ZSCORE_THRESHOLD					0.5f
#define ABG_DETECT_MOBILE_ZSCORE_WINDOW						20
#define ABG_DETECT_PERSON_CONF_THRESHOLD					0.6f
#define ABG_DETECT_PERSON_CONF_VIEW							15
#define ABG_DETECT_PERSON_ZSCORE_THRESHOLD					0.5f
#define ABG_DETECT_PERSON_ZSCORE_WINDOW						20
