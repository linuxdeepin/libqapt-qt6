cmake_minimum_required(VERSION 3.7)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(LIBNAME QApt-qt6)

set(qapt_SRCS
    backend.cpp
    cache.cpp
    package.cpp
    config.cpp
    history.cpp
    debfile.cpp
    dependencyinfo.cpp
    changelog.cpp
    transaction.cpp
    downloadprogress.cpp
    markingerrorinfo.cpp
    sourceentry.cpp
    sourceslist.cpp)

set(qt_required_components Core DBus Widgets)

if (QT_DESIRED_VERSION MATCHES 6)
    list(APPEND qt_required_components Core5Compat)
endif()

find_package(Qt${QT_DESIRED_VERSION} REQUIRED COMPONENTS ${qt_required_components})
find_package(PolkitQt${QT_DESIRED_VERSION}-1)

set(LINK_LIBS
    Qt${QT_DESIRED_VERSION}::Core
    Qt${QT_DESIRED_VERSION}::Widgets
)

add_subdirectory(worker)

set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/worker/${QAPT_WORKER_RDN_VERSIONED}.transaction.xml
    APPEND PROPERTY
        INCLUDE downloadprogress.h
)
set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/worker/${QAPT_WORKER_RDN_VERSIONED}.transaction.xml
    APPEND PROPERTY
        NO_NAMESPACE 1
)

if (QT_DESIRED_VERSION MATCHES 6)
    list(APPEND LINK_LIBS Qt${QT_DESIRED_VERSION}::Core5Compat)

    message("Qt6 terrible use of transaction xml file")
    qt_add_dbus_interface(qapt_SRCS
                       ${CMAKE_CURRENT_BINARY_DIR}/worker/${QAPT_WORKER_RDN_VERSIONED}.transaction.xml
                       transactioninterface_p)

    message("Qt6 terrible use of  worker xml file")
    qt_add_dbus_interface(qapt_SRCS
                            ${CMAKE_CURRENT_BINARY_DIR}/worker/${QAPT_WORKER_RDN_VERSIONED}.xml
                            workerinterface_p)

else()
    message("terrible use of transaction xml file")
    qt5_add_dbus_interface(qapt_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/worker/${QAPT_WORKER_RDN_VERSIONED}.transaction.xml
                        transactioninterface_p)

    message("terrible use of  worker xml file")
    qt5_add_dbus_interface(qapt_SRCS
                        ${CMAKE_CURRENT_BINARY_DIR}/worker/${QAPT_WORKER_RDN_VERSIONED}.xml
                        workerinterface_p)
endif()

set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/worker/${QAPT_WORKER_RDN_VERSIONED}.xml
    APPEND PROPERTY
        NO_NAMESPACE 1
)

configure_file(dbusinterfaces_p.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/dbusinterfaces_p.h)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(${LIBNAME} SHARED ${qapt_SRCS})
add_library(${LIBNAME}::Main ALIAS ${LIBNAME})

# Set header paths for target so the utils can find them.
target_include_directories(${LIBNAME} PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..;${CMAKE_CURRENT_BINARY_DIR}/..>")
target_include_directories(${LIBNAME} INTERFACE "$<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}/QApt>" )

set_target_properties(${LIBNAME}
    PROPERTIES
    VERSION ${QAPT_VERSION_STRING}
    SOVERSION ${QAPT_SOVERSION}
    EXPORT_NAME Main)

target_link_libraries(${LIBNAME}
    PUBLIC
        ${LINK_LIBS}
        ${APTPKG_LIBRARIES}
    PRIVATE
        Qt${QT_DESIRED_VERSION}::DBus
        ${XAPIAN_LIBRARIES})

install(TARGETS ${LIBNAME} EXPORT QApt-qt6Targets LIBRARY DESTINATION ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

ecm_generate_headers(QAPT_CAMEL_CASE_HEADERS
    HEADER_NAMES
        Backend
        Changelog
        Config
        DebFile
        DependencyInfo
        DownloadProgress
        Globals
        History
        MarkingErrorInfo
        Package
        SourceEntry
        SourcesList
        Transaction

  REQUIRED_HEADERS QAPT_HEADERS
  PREFIX QApt)

install(FILES ${QAPT_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/qapt COMPONENT Devel)
install(FILES ${QAPT_CAMEL_CASE_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR}/QApt COMPONENT Devel)
