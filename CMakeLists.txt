cmake_minimum_required(VERSION 2.8.12)

project(qapt-qt6)

set(PROJECT_VERSION_MAJOR 3)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 6)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" )
set(PROJECT_SOVERSION ${PROJECT_VERSION_MAJOR})

if (NOT DEFINED INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/include/${PROJECT_NAME})
endif()

set(QAPT_INCLUDE_DIRS ${INCLUDE_INSTALL_DIR})

if (NOT DEFINED CMAKECONFIG_INSTALL_PREFIX)
    set(CMAKECONFIG_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/cmake")
endif()

find_package(ECM 0.0.14 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH
    ${ECM_MODULE_PATH}
    ${ECM_KDE_MODULE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

include(ECMGenerateHeaders)
include(ECMPackageConfigHelpers)
include(ECMSetupVersion)
include(FeatureSummary)

find_package(Xapian REQUIRED)
find_package(AptPkg REQUIRED)

# Find the required Libaries
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${XAPIAN_INCLUDE_DIR}
    ${APTPKG_INCLUDE_DIR})

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX QAPT
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/qaptversion.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/QApt-qt6ConfigVersion.cmake"
                  SOVERSION ${PROJECT_SOVERSION})

set(QAPT_WORKER_VERSION ${PROJECT_SOVERSION})
set(QAPT_WORKER_RDN org.kubuntu.qapt6worker)
set(QAPT_WORKER_RDN_VERSIONED ${QAPT_WORKER_RDN}${QAPT_WORKER_VERSION})

# For forwarding into C++ convert them into properly excaped strings.
set(QAPT_WORKER_VERSION_STRING \"${QAPT_WORKER_VERSION}\")
set(QAPT_WORKER_RDN_VERSIONED_STRING \"${QAPT_WORKER_RDN_VERSIONED}\")

add_definitions(-DQAPT_WORKER_VERSION_STRING=${QAPT_WORKER_VERSION_STRING})
add_definitions(-DQAPT_WORKER_RDN_VERSIONED_STRING=${QAPT_WORKER_RDN_VERSIONED_STRING})

# Also forward version for utils.
add_definitions(-DCMAKE_PROJECT_VERSION=\"${PROJECT_VERSION}\")

# find_package(DebconfKDE 1.0)
# find_package(GStreamer 1.0)
find_package(GLIB2 2.0)

# Find Qt version
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
message("   >>> Found Qt version: ${QT_VERSION_MAJOR}")
set(QT_DESIRED_VERSION ${QT_VERSION_MAJOR})

# if (KF5CoreAddons_FOUND
#         AND KF5I18n_FOUND
#         AND KF5KIO_FOUND
#         AND KF5Runner_FOUND
#         AND KF5TextWidgets_FOUND
#         AND KF5WidgetsAddons_FOUND
#         AND KF5WindowSystem_FOUND
#         AND KF5IconThemes_FOUND
#         AND DebconfKDE_FOUND)
#     set(WITH_UTILS true)
# endif()

# if (WITH_UTILS AND GSTREAMER_FOUND AND GLIB2_FOUND)
#     set(WITH_GSTREAMER true)
#     add_definitions(${GSTREAMER_DEFINITIONS})
# endif()

# add_feature_info(qapt-utils WITH_UTILS "Runtime utilities using KDE frameworks")
# add_feature_info(qapt-gst-helper WITH_GSTREAMER "GStreamer codec helper util")

# message(WARNING "gettext and tr in the same source is insanely tricky, maybe we should give some ki18n to qapt...")
if (QT_DESIRED_VERSION MATCHES 5)
include(ECMPoQmTools)
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
    ecm_install_po_files_as_qm(po)
endif()
endif()


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${CMAKECONFIG_INSTALL_PREFIX}/QApt-qt6")

ecm_configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/QAptConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/QApt-qt6Config.cmake"
    INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR})

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/QApt-qt6Config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/QApt-qt6ConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel)
install(EXPORT QApt-qt6Targets
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE QApt-qt6Targets.cmake
        NAMESPACE QApt:: )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qaptversion.h
        DESTINATION ${INCLUDE_INSTALL_DIR}/qapt
        COMPONENT Devel)

# add_subdirectory(autotests)
add_subdirectory(src)

# if(WITH_UTILS)
#     add_subdirectory(utils)

#     #Do not remove or modify these.  The release script substitutes in for these
#     #comments with appropriate doc and translation directories.
#     #PO_SUBDIR
#     #DOC_SUBDIR
# endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/lib${PROJECT_NAME}.pc.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.pc
    @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    COMPONENT Devel)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
