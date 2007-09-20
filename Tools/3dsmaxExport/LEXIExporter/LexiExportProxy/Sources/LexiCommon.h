

// The exporter name
#define NDS_EXPORTER_TITLE			"LEXIExporter"

// Remember to change every release
#define NDS_EXPORTER_VERSION		"1.0.8"

// Change only at config protocol break
#define NDS_EXPORTER_CONFIG_VERSION	10002

// Should the plugin unload itself when the utility panel is closed
#define NDS_EXPORTER_AUTOUNLOAD 1


#ifdef _DEBUG
#define NDS_EXPORTER_PLUGIN			"LEXIExporter_d.dll"
#else
#define NDS_EXPORTER_PLUGIN			"LEXIExporter.dll"
#endif
