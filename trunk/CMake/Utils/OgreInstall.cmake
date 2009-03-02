if (WIN32)
  set(OGRE_RELEASE_PATH "/Release")
  set(OGRE_DEBUG_PATH "/Debug")
  set(OGRE_PLUGIN_PATH "/opt")
elseif (UNIX)
  set(OGRE_RELEASE_PATH "")
  set(OGRE_DEBUG_PATH "")
  set(OGRE_PLUGIN_PATH "/OGRE")
elseif (APPLE)
  # TODO
endif ()

function(ogre_install_lib LIBNAME)
  install(TARGETS ${LIBNAME}
    RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo
	LIBRARY DESTINATION "lib" CONFIGURATIONS Release MinSizeRel RelWithDebInfo
	ARCHIVE DESTINATION "lib" CONFIGURATIONS Release MinSizeRel RelWithDebInfo
  )
  install(TARGETS ${LIBNAME}
    RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG
	LIBRARY DESTINATION "lib" CONFIGURATIONS DEBUG
	ARCHIVE DESTINATION "lib" CONFIGURATIONS DEBUG
  )
endfunction(ogre_install_lib)

function(ogre_install_plugin PLUGINNAME)
  install(TARGETS ${PLUGINNAME}
    RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo
	LIBRARY DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo
	ARCHIVE DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo
  )
  install(TARGETS ${PLUGINNAME}
    RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG
	LIBRARY DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS DEBUG
	ARCHIVE DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS DEBUG
  )
endfunction(ogre_install_plugin)

function(ogre_install_sample SAMPLENAME)
  if (OGRE_INSTALL_SAMPLES)
    install(TARGETS ${SAMPLENAME}
      RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" CONFIGURATIONS Release MinSizeRel RelWithDebInfo OPTIONAL
    )
    install(TARGETS ${SAMPLENAME}
      RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG OPTIONAL
    )
  endif ()	
endfunction(ogre_install_sample)
