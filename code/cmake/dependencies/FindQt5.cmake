FUNCTION(INSTALL_QT_COMPONENT COMPONENT)

	FOREACH(LIBRARY ${Qt5${COMPONENT}_LIBRARIES})
		GET_TARGET_PROPERTY(LIBRARY_LOCATION ${LIBRARY} IMPORTED_LOCATION_RELEASE)
		IF(${LIBRARY_LOCATION} STREQUAL LIBRARY_LOCATION-NOTFOUND)
			GET_TARGET_PROPERTY(LIBRARY_LOCATION ${LIBRARY} IMPORTED_LOCATION_DEBUG)
		ENDIF()

		INSTALL(FILES ${LIBRARY_LOCATION} DESTINATION external/qt)

		STRING(REPLACE .so.5.0.0 .so.5.0 LIBRARY_LOCATION_LINK ${LIBRARY_LOCATION})
		INSTALL(FILES ${LIBRARY_LOCATION_LINK} DESTINATION external/qt)

		STRING(REPLACE .so.5.0.0 .so.5 LIBRARY_LOCATION_LINK ${LIBRARY_LOCATION})
		INSTALL(FILES ${LIBRARY_LOCATION_LINK} DESTINATION external/qt)

		STRING(REPLACE .so.5.0.0 .so LIBRARY_LOCATION_LINK ${LIBRARY_LOCATION})
		INSTALL(FILES ${LIBRARY_LOCATION_LINK} DESTINATION external/qt)
	ENDFOREACH()

ENDFUNCTION()

IF(Qt5_FIND_COMPONENTS)
  
	FOREACH(COMPONENT ${Qt5_FIND_COMPONENTS})
		SET(Qt5${COMPONENT}_DIR ${QT5BASE}/lib/cmake/Qt5${COMPONENT})
		FIND_PACKAGE(Qt5${COMPONENT})
		INCLUDE_DIRECTORIES(${Qt5${COMPONENT}_INCLUDE_DIRS})
		INSTALL_QT_COMPONENT(${COMPONENT})
	ENDFOREACH()

ENDIF(Qt5_FIND_COMPONENTS)

#For QGlobal linking errors
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")