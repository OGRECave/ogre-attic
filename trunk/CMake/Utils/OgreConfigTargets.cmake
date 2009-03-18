# Configure settings and install targets

if (WIN32)
  set(OGRE_RELEASE_PATH "/Release")
  set(OGRE_DEBUG_PATH "/Debug")
  set(OGRE_PLUGIN_PATH "/opt")
elseif (UNIX)
  set(OGRE_RELEASE_PATH "")
  set(OGRE_DEBUG_PATH "")
  set(OGRE_PLUGIN_PATH "/OGRE")
endif ()

# create vcproj.user file for Visual Studio to set debug working directory
function(ogre_create_vcproj_userfile TARGETNAME)
  if (MSVC)
    configure_file(
	  ${OGRE_TEMPLATES_DIR}/VisualStudioUserFile.vcproj.user.in
	  ${CMAKE_CURRENT_BINARY_DIR}/${TARGETNAME}.vcproj.user
	  @ONLY
	)
  endif ()
endfunction(ogre_create_vcproj_userfile)

# setup common target settings
function(ogre_config_common TARGETNAME)
  set_target_properties(${TARGETNAME} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${OGRE_BINARY_DIR}/bin
  )
  ogre_create_vcproj_userfile(${TARGETNAME})
endfunction(ogre_config_common)


# setup library build
function(ogre_config_lib LIBNAME)
  ogre_config_common(${LIBNAME})
  if (OGRE_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${LIBNAME} PROPERTIES OUTPUT_NAME ${LIBNAME}Static)
  else (OGRE_STATIC)
    if (CMAKE_COMPILER_IS_GNUCXX)
      # add GCC visibility flags to shared library build
      set_target_properties(${LIBNAME} PROPERTIES COMPILE_FLAGS "${OGRE_GCC_VISIBILITY_FLAGS}")
	endif (CMAKE_COMPILER_IS_GNUCXX)
  endif (OGRE_STATIC)
  install(TARGETS ${LIBNAME}
    RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" 
      CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
    LIBRARY DESTINATION "lib" CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
    ARCHIVE DESTINATION "lib" CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
    FRAMEWORK DESTINATION "bin${OGRE_RELEASE_PATH}" 
      CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
  )
  install(TARGETS ${LIBNAME}
    RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG
    LIBRARY DESTINATION "lib" CONFIGURATIONS DEBUG
    ARCHIVE DESTINATION "lib" CONFIGURATIONS DEBUG
    FRAMEWORK DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG
  )
endfunction(ogre_config_lib)

# setup plugin build
function(ogre_config_plugin PLUGINNAME)
  ogre_config_common(${PLUGINNAME})
  if (OGRE_STATIC)
    # add static prefix, if compiling static version
    set_target_properties(${PLUGINNAME} PROPERTIES OUTPUT_NAME ${PLUGINNAME}Static)
  else (OGRE_STATIC)
    if (CMAKE_COMPILER_IS_GNUCXX)
      # add GCC visibility flags to shared library build
      set_target_properties(${PLUGINNAME} PROPERTIES COMPILE_FLAGS "${OGRE_GCC_VISIBILITY_FLAGS}")
      # disable "lib" prefix on Unix
      set_target_properties(${PLUGINNAME} PROPERTIES PREFIX "")
	endif (CMAKE_COMPILER_IS_GNUCXX)
  endif ()
  install(TARGETS ${PLUGINNAME}
    RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" 
      CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
    LIBRARY DESTINATION "lib${OGRE_PLUGIN_PATH}" 
      CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
    ARCHIVE DESTINATION "lib${OGRE_PLUGIN_PATH}" 
      CONFIGURATIONS Release MinSizeRel RelWithDebInfo None
  )
  install(TARGETS ${PLUGINNAME}
    RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS DEBUG
    LIBRARY DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS DEBUG
    ARCHIVE DESTINATION "lib${OGRE_PLUGIN_PATH}" CONFIGURATIONS DEBUG
  )
endfunction(ogre_config_plugin)

# setup Ogre demo build
function(ogre_config_sample SAMPLENAME)
  ogre_config_common(${SAMPLENAME})
  if (OGRE_INSTALL_SAMPLES)
    install(TARGETS ${SAMPLENAME}
      RUNTIME DESTINATION "bin${OGRE_RELEASE_PATH}" 
        CONFIGURATIONS Release MinSizeRel RelWithDebInfo None OPTIONAL
    )
    install(TARGETS ${SAMPLENAME}
      RUNTIME DESTINATION "bin${OGRE_DEBUG_PATH}" CONFIGURATIONS Debug OPTIONAL
    )
  endif ()	
endfunction(ogre_config_sample)

