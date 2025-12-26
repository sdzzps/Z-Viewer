cmake_minimum_required(VERSION 3.16)

project(ZViewerCMake1 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON) #load resource file

# enable Objective-C++ on Mac OS
if(APPLE)
  enable_language(OBJCXX)
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Qml #for qml style theme
    Quick
    QuickControls2 #for qml style theme
    Svg
)

qt_standard_project_setup(REQUIRES 6.10)

#set icon for Windows version .exe file
if (WIN32)
    set(APP_ICON_RESOURCE app_icon.rc) #load icon
endif()

qt_add_executable(appZViewerCMake1
    main.cpp
    resource.qrc #resource file
    ${APP_ICON_RESOURCE} #load icon
)

#set output program name
set_target_properties(appZViewerCMake1 PROPERTIES
    OUTPUT_NAME "ZViewer"
)

#set icon for Mac OS version .app package
if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)

    #put incs into .app/Contents/Resources
    set(APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/AppIcon.icns")
    set_source_files_properties("${APP_ICON}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )

    # link to project
    target_sources(appZViewerCMake1 PRIVATE "${APP_ICON}")
endif()

#disable qDebug for release version
target_compile_definitions(appZViewerCMake1
    PRIVATE $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>
)

#Only add cross-platform sources here
qt_add_qml_module(appZViewerCMake1
    URI ZViewerCMake1
    RESOURCE_PREFIX /qt/qml
    QML_FILES
        Main.qml CollapsedPart.qml FileThumb.qml InfoPanel.qml ThumbPanel.qml ZBorderlessWindow.qml
        EntryRow.qml ZButton.qml ZSwitch.qml ZButtonIcon.qml BasicInfo.qml BasicInfoTag.qml
        EditMenu.qml AboutWindow.qml
    SOURCES
        backend.h backend.cpp getExif.cpp getExif.h thumbImage.h thumbImage.cpp
        thumbImage.cpp thumbImage.h frontEndModels.h frontEndModels.cpp platform.h
    RESOURCES
        resource.qrc
)

target_link_libraries(appZViewerCMake1 PRIVATE
    Qt6::Core
    Qt6::Quick
    Qt6::Qml #used for theme
    Qt6::QuickControls2 #used for theme
    Qt6::Svg
)

#Platform specific settings
#Windows version sources
if(WIN32)
    set_target_properties(appZViewerCMake1 PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    target_sources(appZViewerCMake1 PRIVATE
        windowsShellThumbProvider.h
        windowsShellThumbProvider.cpp
    )
endif()

#Mac OS version sources and settings
if(APPLE)

    # Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
    # If you are developing for iOS or macOS you should consider setting an
    # explicit, fixed bundle identifier manually though.
    set_target_properties(appZViewerCMake1 PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "ZViewer"
        MACOSX_BUNDLE_ICON_FILE "AppIcon.icns"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"

        #    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appZViewerCMake1
    )

    target_sources(appZViewerCMake1 PRIVATE
        macThumbProvider.h macThumbProvider.cpp
        MacQLThumbnail.mm
    )

  #link Mac OS frameworks
  target_link_libraries(appZViewerCMake1 PRIVATE
    "-framework Cocoa"
    "-framework QuickLookThumbnailing"
    "-framework QuartzCore"
    "-framework CoreServices"
  )

  #enable ARC for Objective-C++ file
  set_source_files_properties(MacQLThumbnail.mm PROPERTIES COMPILE_FLAGS "-fobjc-arc")
endif()

include(GNUInstallDirs)
install(TARGETS appZViewerCMake1
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
