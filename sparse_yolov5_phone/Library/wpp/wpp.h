
#define WPP_CONTROL_GUIDS	\
	WPP_DEFINE_CONTROL_GUID(ADU_Libraries, (0,0,0,0,0),	\
	WPP_DEFINE_BIT(Application) \
	WPP_DEFINE_BIT(Process) \
	WPP_DEFINE_BIT(File) \
	WPP_DEFINE_BIT(Registry) \
	WPP_DEFINE_BIT(Service) \
	WPP_DEFINE_BIT(COM) \
	WPP_DEFINE_BIT(Device) \
	WPP_DEFINE_BIT(Others) \
	)

// WPP LEVEL
#define TRACE_LEVEL_NONE        0   // Tracing is not on
#define TRACE_LEVEL_CRITICAL    1   // Abnormal exit or termination
#define TRACE_LEVEL_FATAL       1   // Deprecated name for Abnormal exit or termination
#define TRACE_LEVEL_ERROR       2   // Severe errors that need logging
#define TRACE_LEVEL_WARNING     3   // Warnings such as allocation failure
#define TRACE_LEVEL_INFORMATION 4   // Includes non-error cases(e.g.,Entry-Exit)
#define TRACE_LEVEL_VERBOSE     5   // Detailed traces from intermediate steps
#define TRACE_LEVEL_DETAILED    6
#define TRACE_LEVEL_RESERVED7   7
#define TRACE_LEVEL_RESERVED8   8
#define TRACE_LEVEL_RESERVED9   9

#define WPP_LEVEL_FLAGS_ENABLED(level, flags) (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= level)
#define WPP_LEVEL_FLAGS_LOGGER(level, flags) WPP_LEVEL_LOGGER(flags)

// MACRO: _TRACE_D,V,I,W,E,C
//
// begin_wpp config
// USEPREFIX (_TRACE_D, "%!STDPREFIX![DETAIL] :");
// USEPREFIX (_TRACE_V, "%!STDPREFIX![VERB] :");
// USEPREFIX (_TRACE_I, "%!STDPREFIX![INFO] :");
// USEPREFIX (_TRACE_W, "%!STDPREFIX![WARN] :");
// USEPREFIX (_TRACE_E, "%!STDPREFIX![ERROR] :");
// USEPREFIX (_TRACE_C, "%!STDPREFIX![FATAL] :");
// FUNC _TRACE_D{LEVEL=TRACE_LEVEL_DETAILED, TRACE=0}(FLAGS, MSG, ...);
// FUNC _TRACE_V{LEVEL=TRACE_LEVEL_VERBOSE, TRACE=0}(FLAGS, MSG, ...);
// FUNC _TRACE_I{LEVEL=TRACE_LEVEL_INFORMATION, TRACE=0}(FLAGS, MSG, ...);
// FUNC _TRACE_W{LEVEL=TRACE_LEVEL_WARNING, TRACE=0}(FLAGS, MSG, ...);
// FUNC _TRACE_E{LEVEL=TRACE_LEVEL_ERROR, TRACE=0}(FLAGS, MSG, ...);
// FUNC _TRACE_C{LEVEL=TRACE_LEVEL_CRITICAL, TRACE=0}(FLAGS, MSG, ...);
// end_wpp
#define WPP_LEVEL_TRACE_FLAGS_ENABLED(LEVEL, name, FLAGS) WPP_LEVEL_FLAGS_ENABLED(LEVEL, FLAGS)
#define WPP_LEVEL_TRACE_FLAGS_LOGGER(LEVEL, name, FLAGS) WPP_LEVEL_FLAGS_LOGGER(LEVEL, FLAGS)

// MACRO: _TRACE_EX
//
// begin_wpp config
// FUNC _TRACE_EX{FLAGS=Application, LEVEL=TRACE_LEVEL_ERROR}(TRACE_EX);
// USESUFFIX (_TRACE_EX, "[ERROR] :%s", (const char*)TRACE_EX->ToShortString());
// end_wpp
#define WPP_FLAGS_LEVEL_TRACE_EX_ENABLED(FLAGS, LEVEL, px) WPP_LEVEL_FLAGS_ENABLED(LEVEL, FLAGS)
#define WPP_FLAGS_LEVEL_TRACE_EX_LOGGER(FLAGS, LEVEL, px) WPP_LEVEL_FLAGS_LOGGER(LEVEL, FLAGS)
////#define WPP_FLAGS_LEVEL_TRACE_EX_PRE if (WPP_LEVEL_FLAGS_ENABLED(LEVEL, FLAGS)) { array<string> astr = string((wchar_t*)px->ToString()).Split(string(L"\r\n").GetChars(), System::EStringSplitOptions::RemoveEmptyEntries); for (int n = 0; n < astr.GetLength(); n++) {
////#define WPP_FLAGS_LEVEL_TRACE_EX_POST(FLAGS, LEVEL, px) ; } }

// MACRO: _TRACE_EX_UNK
//
// begin_wpp config
// FUNC _TRACE_EX_UNK{FLAGS=Application, LEVEL=TRACE_LEVEL_ERROR, EX_UNK=0}(...);
// USESUFFIX (_TRACE_EX_UNK, "[ERROR] :Unknown Exception");
// end_wpp
#define WPP_FLAGS_LEVEL_EX_UNK_ENABLED(FLAGS, LEVEL, name) WPP_LEVEL_FLAGS_ENABLED(LEVEL, FLAGS)
#define WPP_FLAGS_LEVEL_EX_UNK_LOGGER(FLAGS, LEVEL, name) WPP_LEVEL_FLAGS_LOGGER(LEVEL, FLAGS)


