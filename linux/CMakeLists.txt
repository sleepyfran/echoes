find_package(ECM 6.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(FeatureSummary)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMDeprecationSettings)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)
find_package(KF6 REQUIRED COMPONENTS I18n Kirigami)

qt_policy(SET QTP0001 NEW)

qt_add_executable(echoes-linux-ui
    src/main.cpp
)

qt_add_qml_module(echoes-linux-ui
    URI me.sleepyfran.echoes
    VERSION 1.0
    QML_FILES
        qml/Main.qml
)

target_link_libraries(echoes-linux-ui
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        KF6::I18n
        KF6::Kirigami
        echoes::core
)

install(TARGETS echoes-linux-ui ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS ${CMAKE_SOURCE_DIR}/me.sleepyfran.echoes.desktop DESTINATION ${KDE_INSTALL_APPDIR})
