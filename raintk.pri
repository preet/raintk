
RAINTK_BUILD_INFO_CONFIG = Unknown
RAINTK_BUILD_INFO_PLATFORM = Unknown
RAINTK_BUILD_INFO_SPEC = $$[QMAKE_XSPEC]
RAINTK_BUILD_INFO_ARCH = $${QT_ARCH}

CONFIG(debug,debug|release) {
    RAINTK_BUILD_INFO_CONFIG = Debug
    DEFINES += RAINTK_BUILD_DEBUG
}

CONFIG(release,debug|release) {
    RAINTK_BUILD_INFO_CONFIG = Release
}

android {
    RAINTK_BUILD_INFO_PLATFORM = Android
}
linux {
    !android {
        RAINTK_BUILD_INFO_PLATFORM = Linux
    }
}
macx {
    RAINTK_BUILD_INFO_PLATFORM = OSX
}
ios {
    RAINTK_BUILD_INFO_PLATFORM = iOS
}
win32 {
    RAINTK_BUILD_INFO_PLATFORM = Windows
}

message("raintk: Build Info: $${RAINTK_BUILD_INFO_PLATFORM}, $${RAINTK_BUILD_INFO_SPEC}, $${RAINTK_BUILD_INFO_ARCH}, $${RAINTK_BUILD_INFO_CONFIG}")

linux_x11 {
    DEFINES += RAINTK_ENV_LINUX_X11
}
android {
    DEFINES += RAINTK_ENV_ANDROID
}




INCLUDEPATH += $${PATH_RAINTK}

# thirdparty
INCLUDEPATH += $${PATH_RAINTK}/raintk/thirdparty

# clipper
HEADERS += \
    $${PATH_RAINTK}/raintk/thirdparty/clipper/clipper.hpp

SOURCES += \
    $${PATH_RAINTK}/raintk/thirdparty/clipper/clipper.cpp

# shaders
HEADERS += \
    $${PATH_RAINTK}/raintk/shaders/*.hpp

# core
HEADERS += \
    $${PATH_RAINTK}/raintk/RainTkConfig.hpp \
    $${PATH_RAINTK}/raintk/RainTkGlobal.hpp \
    $${PATH_RAINTK}/raintk/RainTkUnits.hpp \
    $${PATH_RAINTK}/raintk/RainTkLog.hpp \
    $${PATH_RAINTK}/raintk/RainTkProperty.hpp \
    $${PATH_RAINTK}/raintk/RainTkSceneKey.hpp \
    $${PATH_RAINTK}/raintk/RainTkComponents.hpp \
    $${PATH_RAINTK}/raintk/RainTkDrawKey.hpp \
    $${PATH_RAINTK}/raintk/RainTkAnimation.hpp \
    $${PATH_RAINTK}/raintk/RainTkTween.hpp \
    $${PATH_RAINTK}/raintk/RainTkPropertyAnimation.hpp \
    $${PATH_RAINTK}/raintk/RainTkMainDrawStage.hpp \
    $${PATH_RAINTK}/raintk/RainTkInputListener.hpp \
    $${PATH_RAINTK}/raintk/RainTkInputRecorder.hpp \
    $${PATH_RAINTK}/raintk/RainTkInputSystem.hpp \
    $${PATH_RAINTK}/raintk/RainTkAnimationSystem.hpp \
    $${PATH_RAINTK}/raintk/RainTkTransformSystem.hpp \
    $${PATH_RAINTK}/raintk/RainTkDrawSystem.hpp \
    $${PATH_RAINTK}/raintk/RainTkScene.hpp

SOURCES += \
    $${PATH_RAINTK}/raintk/RainTkUnits.cpp \
    $${PATH_RAINTK}/raintk/RainTkComponents.cpp \
    $${PATH_RAINTK}/raintk/RainTkDrawKey.cpp \
    $${PATH_RAINTK}/raintk/RainTkAnimation.cpp \
    $${PATH_RAINTK}/raintk/RainTkTween.cpp \
    $${PATH_RAINTK}/raintk/RainTkLog.cpp \
    $${PATH_RAINTK}/raintk/RainTkMainDrawStage.cpp \
    $${PATH_RAINTK}/raintk/RainTkInputListener.cpp \
    $${PATH_RAINTK}/raintk/RainTkInputRecorder.cpp \
    $${PATH_RAINTK}/raintk/RainTkInputSystem.cpp \
    $${PATH_RAINTK}/raintk/RainTkAnimationSystem.cpp \
    $${PATH_RAINTK}/raintk/RainTkTransformSystem.cpp \
    $${PATH_RAINTK}/raintk/RainTkDrawSystem.cpp \
    $${PATH_RAINTK}/raintk/RainTkScene.cpp

# models
HEADERS += \
    $${PATH_RAINTK}/raintk/RainTkListModel.hpp \
    $${PATH_RAINTK}/raintk/RainTkListModelSTLVector.hpp \
    $${PATH_RAINTK}/raintk/RainTkListDelegate.hpp

SOURCES += \
    $${PATH_RAINTK}/raintk/RainTkListModel.cpp

# helpers
HEADERS += \
    $${PATH_RAINTK}/raintk/RainTkAlignment.hpp \
    $${PATH_RAINTK}/raintk/RainTkColorConv.hpp \
    $${PATH_RAINTK}/raintk/RainTkImageAtlas.hpp

SOURCES += \
    $${PATH_RAINTK}/raintk/RainTkAlignment.cpp \
    $${PATH_RAINTK}/raintk/RainTkColorConv.cpp \
    $${PATH_RAINTK}/raintk/RainTkImageAtlas.cpp


# widget
HEADERS += \
    $${PATH_RAINTK}/raintk/RainTkWidget.hpp

SOURCES += \
    $${PATH_RAINTK}/raintk/RainTkWidget.cpp

# widgets (basic)
HEADERS += \
    $${PATH_RAINTK}/raintk/RainTkDrawableWidget.hpp \
    $${PATH_RAINTK}/raintk/RainTkRectangle.hpp \
    $${PATH_RAINTK}/raintk/RainTkImage.hpp \
    $${PATH_RAINTK}/raintk/RainTkAtlasImage.hpp \
    $${PATH_RAINTK}/raintk/RainTkText.hpp

SOURCES += \
    $${PATH_RAINTK}/raintk/RainTkDrawableWidget.cpp \
    $${PATH_RAINTK}/raintk/RainTkRectangle.cpp \
    $${PATH_RAINTK}/raintk/RainTkImage.cpp \
    $${PATH_RAINTK}/raintk/RainTkAtlasImage.cpp \
    $${PATH_RAINTK}/raintk/RainTkText.cpp

# widgets (inputs)
HEADERS += \
    $${PATH_RAINTK}/raintk/RainTkInputArea.hpp \
    $${PATH_RAINTK}/raintk/RainTkSinglePointArea.hpp \
    $${PATH_RAINTK}/raintk/RainTkScrollArea.hpp \
    $${PATH_RAINTK}/raintk/RainTkScrollBar.hpp \
    $${PATH_RAINTK}/raintk/RainTkTextInput.hpp
#    $${PATH_RAINTK}/raintk/RainTkMouseArea.hpp \

SOURCES += \
    $${PATH_RAINTK}/raintk/RainTkInputArea.cpp \
    $${PATH_RAINTK}/raintk/RainTkSinglePointArea.cpp \
    $${PATH_RAINTK}/raintk/RainTkScrollArea.cpp \
    $${PATH_RAINTK}/raintk/RainTkTextInput.cpp
#    $${PATH_RAINTK}/raintk/RainTkMouseArea.cpp \

# widgets (layout)
HEADERS += \
    $${PATH_RAINTK}/raintk/RainTkColumn.hpp \
    $${PATH_RAINTK}/raintk/RainTkRow.hpp \
    $${PATH_RAINTK}/raintk/RainTkGrid.hpp

SOURCES += \
    $${PATH_RAINTK}/raintk/RainTkColumn.cpp \
    $${PATH_RAINTK}/raintk/RainTkRow.cpp \
    $${PATH_RAINTK}/raintk/RainTkGrid.cpp

# widget (views)
HEADERS += \
    $${PATH_RAINTK}/raintk/RainTkListView.hpp

SOURCES += \
    $${PATH_RAINTK}/raintk/RainTkListView.cpp


# test

contains(RAINTK_BUILD_INFO_CONFIG,Debug) {
    DEFINES += RAINTK_DEBUG_TEST_DRAW_SYSTEM
    DEFINES += RAINTK_DEBUG_LIST_VIEW_GUIDELINES
}

# test resources
SOURCES += \
    $${PATH_RAINTK}/raintk/test/RainTkTestFontResource.cpp \
    $${PATH_RAINTK}/raintk/test/RainTkTestImageResources.cpp

# individual tests
HEADERS += \
    $${PATH_RAINTK}/raintk/test/RainTkTestContext.hpp

SOURCES += \
#    $${PATH_RAINTK}/raintk/test/RainTkTestDrawSystemOpaqueSingle.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestDrawSystemOpaqueMultiple.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestDrawSystemClipping.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestDrawSystemTransparency.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestDrawableIds.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestBoundingBoxes.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestTransforms.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestRectangle.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestAnimation.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestClipping.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestClippingPolys.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestImage.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestRow.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestGrid.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestInputCanceling.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestSinglePointArea.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestMouseArea.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestScrollArea.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestScrollAreaInputPassThrough.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestScrollAreaInputPassThrough2.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestText.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestInputFocus.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestTextInput.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestTextDims.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestPropertiesAndUpdateOrder.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestUpdateHierarchy.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestAlignment.cpp
    $${PATH_RAINTK}/raintk/test/RainTkTestListView.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestDrag.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestImageAtlas.cpp
#    $${PATH_RAINTK}/raintk/test/RainTkTestOpacityHierarchy.cpp


