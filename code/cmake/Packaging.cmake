SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ROOT}/cmake/packaging)

#############################################################################################
# Parameters
#############################################################################################
SET(PACKAGE_ARCHITECTURE "i386")
IF(${ARCHITECTURE} MATCHES "64bit")
	SET(PACKAGE_ARCHITECTURE "amd64")
ENDIF()

INCLUDE(PackagingFiles)
#############################################################################################



#############################################################################################
# Basic package information
#############################################################################################
IF(${DISTRO} STREQUAL "Debian")
	SET(CPACK_GENERATOR "DEB")
ELSEIF(${DISTRO} STREQUAL "Redhat")
	SET(CPACK_GENERATOR "RPM")
ENDIF()
SET(CPACK_PACKAGING_INSTALL_PREFIX ${INSTALL_LOCATION})
SET(CPACK_PACKAGE_EXECUTABLES launcher;visore)
SET(CPACK_PACKAGE_DESCRIPTION_FILE ${README_FILE})
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${SHORT_DESCRIPTION})
SET(CPACK_PACKAGE_FILE_NAME "${PACKAGE}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}-${PACKAGE_ARCHITECTURE}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${NAME} ${VERSION_MAJOR}.${VERSION_MINOR}")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
SET(CPACK_PACKAGE_NAME ${NAME})
SET(CPACK_PACKAGE_VENDOR ${NAME})
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_PACKAGE_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${PACKAGE}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
SET(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${PACKAGE_ARCHITECTURE}")
SET(CPACK_RESOURCE_FILE_LICENSE ${COPYRIGHT_FILE})
SET(CPACK_RESOURCE_FILE_README ${README_FILE})
#############################################################################################



#############################################################################################
# Specific package information
#############################################################################################
IF(${DISTRO} STREQUAL "Debian")
	SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "${MAINTAINER} <${EMAIL}>")
	SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${PACKAGE_ARCHITECTURE})
	SET(CPACK_DEBIAN_PACKAGE_SECTION sound)
	SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION_SUMMARY ${SHORT_DESCRIPTION})
	SET(CPACK_DEBIAN_PACKAGE_DESCRIPTION "${SHORT_DESCRIPTION}
		${LONG_DESCRIPTION}
	Homepage: ${HOMEPAGE}")
	SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-core (>= 4.7), libqt4-gui (>= 4.7)")
	SET(CPACK_DEBIAN_PACKAGE_RECOMMENDS "lame (>= 3.98), flac (>= 1.2)")
ELSEIF(${DISTRO} STREQUAL "Redhat")
	SET(CPACK_RPM_PACKAGE_PROVIDES "libbass.so")
	SET(CPACK_RPM_PACKAGE_SUMMARY ${SHORT_DESCRIPTION})
	SET(CPACK_RPM_PACKAGE_ARCHITECTURE ${PACKAGE_ARCHITECTURE})
	SET(CPACK_RPM_PACKAGE_LICENSE ${LICENSE})
	SET(CPACK_RPM_PACKAGE_GROUP "Applications/Multimedia")
	SET(CPACK_RPM_PACKAGE_DESCRIPTION "${SHORT_DESCRIPTION}
	${LONG_DESCRIPTION}")
	#set(CPACK_RPM_PACKAGE_REQUIRES "python >= 2.5.0, cmake >= 2.8")
ENDIF()
#############################################################################################



INCLUDE(CPack)
