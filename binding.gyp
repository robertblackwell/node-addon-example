{
	"conditions": [
		['OS=="mac"', {
			"targets": [{
				"target_name": "addon",
				"sources": [ 
					"src/addon.cpp", "src/addon.h",
					"src/arguments.cpp", "src/arguments.h",
					"src/caller_context.cpp", "src/caller_context.h",
					"src/event.cpp", "src/event.h",
					"src/napi_ext.cpp",  "src/napi_ext.h",
					"src/threadsafe_function.cpp", "src/threadsafe_function.h",
					"src/worker_thread.cpp", "src/worker_thread.h"
				],
				"include_dirs":["<!@(node -p \"require('node-addon-api').include\")"],
				'cflags!':['-fno-exceptions'],
				'cflags_cc!': ['-fno-exceptions']
				"xcode_settings": {
					'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
					"OTHER_LDFLAGS": [
						"-Wl,-bind_at_load",
						"-framework CoreFoundation -framework CoreServices"
					]
				}
			}, {
				"target_name": "action_after_build",
				"type": "none",
				"dependencies": ["addon"],
				"copies": [{
					"files": ["<(PRODUCT_DIR)/addon.node"],
					"destination": "./"
				}]
			}]
		}]
	]
}
