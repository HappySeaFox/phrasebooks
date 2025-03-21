TARGET = phrasebooks
TEMPLATE = app

VERSION_MAJOR=1
VERSION_MINOR=0
VERSION_PATCH=2

QT += core gui network widgets

unix {
    QT += x11extras
}

INCLUDEPATH += . src qtsingleapplication
DEPENDPATH += . src qtsingleapplication

SOURCES += src/main.cpp \
    qtsingleapplication/qtsingleapplication.cpp \
    qtsingleapplication/qtlockedfile.cpp \
    qtsingleapplication/qtlocalpeer.cpp \
    src/utils.cpp \
    src/phrasebooks.cpp \
    src/settings.cpp \
    src/list.cpp \
    src/target.cpp \
    src/miniinput.cpp \
    src/networkaccess.cpp \
    src/updatechecker.cpp \
    src/numericlabel.cpp \
    src/englishvalidator.cpp \
    src/about.cpp \
    src/booksandchapters.cpp \
    src/selectchapter.cpp \
    src/chaptersview.cpp \
    src/options.cpp \
    src/bookfileiconprovider.cpp \
    src/chapterlabel.cpp

HEADERS += qtsingleapplication/qtsingleapplication.h \
    qtsingleapplication/qtlockedfile.h \
    qtsingleapplication/qtlocalpeer.h \
    src/utils.h \
    src/phrasebooks.h \
    src/settings.h \
    src/list.h \
    src/target.h \
    src/miniinput.h \
    src/networkaccess.h \
    src/updatechecker.h \
    src/numericlabel.h \
    src/englishvalidator.h \
    src/about.h \
    src/booksandchapters.h \
    src/selectchapter.h \
    src/chaptersview.h \
    src/options.h \
    src/bookfileiconprovider.h \
    src/chapterlabel.h

win32 {
    SOURCES += qtsingleapplication/qtlockedfile_win.cpp \
                src/windowmarker.cpp

    HEADERS += src/windowmarker.h \
                src/x11.h
} else {
    SOURCES += qtsingleapplication/qtlockedfile_unix.cpp \
                src/x11.cpp
}

RESOURCES += phrasebooks.qrc

win32 {
    RC_FILE = phrasebooks.rc
    LIBS += -lpsapi -lgdi32 -luser32
} else:unix {
    LIBS += -lX11 -lXtst
}

OTHER_FILES += \
    phrasebooks.rc \
    README.txt \
    LICENSE.txt \
    $${TARGET}-version.tag

include($${TARGET}-common.pri)

# network data
VCSROOT_FOR_DOWNLOAD="http://sourceforge.net/p/phrasebooks/code/ci/master/tree"
HTTPROOT="http://sourceforge.net/p/phrasebooks"
DOWNLOADROOT="http://sourceforge.net/projects/phrasebooks"

# files to copy to the distribution
BEARERPLUGINS=
IMAGEPLUGINS=qico.dll
SQLPLUGINS=
QTLIBS=Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll Qt5Network.dll
SSLLIBS=libeay32.dll ssleay32.dll

isEmpty(HOST64) {
    MINGW_BUILD_TYPE=dw2
} else {
    MINGW_BUILD_TYPE=seh
}

MINGWLIBS=libgcc_s_$${MINGW_BUILD_TYPE}-1.dll libwinpthread-1.dll libstdc++-6.dll
LICENSES=LICENSE.txt

DEFINES += VCSROOT_FOR_DOWNLOAD=$$sprintf("\"\\\"%1\\\"\"", $$VCSROOT_FOR_DOWNLOAD)
DEFINES += HTTPROOT=$$sprintf("\"\\\"%1\\\"\"", $$HTTPROOT)
DEFINES += DOWNLOADROOT=$$sprintf("\"\\\"%1\\\"\"", $$DOWNLOADROOT)

tag.commands += $$mle(echo "$$VERSION"> "\"$${_PRO_FILE_PWD_}/$${TARGET}-version.tag\"")
tag.commands += $$mle(git -C \"$${_PRO_FILE_PWD_}\" commit -m "\"$$VERSION file tag\"" "\"$${_PRO_FILE_PWD_}/$${TARGET}-version.tag\"")
tag.commands += $$mle(git -C \"$${_PRO_FILE_PWD_}\" tag -a "\"$$VERSION\"" -m "\"$$VERSION\"")
tag.commands += $$mle(git -C \"$${_PRO_FILE_PWD_}\" push -u origin master)
QMAKE_EXTRA_TARGETS += tag

!isEmpty(GCCDIR):!isEmpty(ZIP) {
    message("7Z is found, will create custom dist targets")

    # source archive
    T="$${OUT_PWD}/$${TARGET}-$$VERSION"
    distsrc.commands += $$mle(if exist \"$$T\" rd /S /Q \"$$T\")
    distsrc.commands += $$mle(xcopy /s /q /y /i /h \"$${_PRO_FILE_PWD_}\" \"$$T\\\")
    distsrc.commands += $$mle(git -C \"$$T\" clean -dfx)
    distsrc.commands += $$mle(rd /S /Q \"$$T/.git\")
    distsrc.commands += $$mle(del /F /Q $${TARGET}-$${VERSION}.zip)
    distsrc.commands += $$mle($$ZIP a -r -tzip -mx=9 $${TARGET}-$${VERSION}.zip \"$$T\")
    distsrc.commands += $$mle(rd /S /Q \"$$T\")

    QMAKE_EXTRA_TARGETS += distsrc

    # portable binary
    T="$${OUT_PWD}/$${TARGET}-portable-$$VERSION"

    distbin.commands += $$mle(if exist \"$$T\" rd /S /Q \"$$T\")
    distbin.commands += $$mle(mkdir \"$$T\")
    distbin.commands += $$mle(mkdir \"$$T/imageformats\")
    distbin.commands += $$mle(mkdir \"$$T/platforms\")
    distbin.commands += $$mle(mkdir \"$$T/translations\")

    # binary & necessary files
    distbin.commands += $$mle(copy /y \"$${OUT_PWD}/$(DESTDIR_TARGET)\" \"$$T\")

    # platforms plugin
    distbin.commands += $$mle(copy /y \"$$[QT_INSTALL_PLUGINS]\\platforms\\qwindows.dll\" \"$$T/platforms\")

    for(ql, QTLIBS) {
        distbin.commands += $$mle(copy /y \"$$[QT_INSTALL_BINS]\\$$ql\" \"$$T\")
    }

    for(sl, SSLLIBS) {
        distbin.commands += $$mle(copy /y \"$${_PRO_FILE_PWD_}\\extra\\$$sl\" \"$$T\")
    }

    for(ml, MINGWLIBS) {
        distbin.commands += $$mle(copy /y \"$$GCCDIR\\$$ml\" \"$$T\")
    }

    for(bp, BEARERPLUGINS) {
        distbin.commands += $$mle(copy /y \"$$[QT_INSTALL_PLUGINS]\\bearer\\$$bp\" \"$$T/bearer\")
    }

    for(ip, IMAGEPLUGINS) {
        distbin.commands += $$mle(copy /y \"$$[QT_INSTALL_PLUGINS]\\imageformats\\$$ip\" \"$$T/imageformats\")
    }

    for(sp, SQLPLUGINS) {
        distbin.commands += $$mle(copy /y \"$$[QT_INSTALL_PLUGINS]\\sqldrivers\\$$sp\" \"$$T/sqldrivers\")
    }

    # translations
    distbin.commands += $$mle(copy /y \"$$TRANSLATIONS_DIR\\*.qm\" \"$$T/translations\")
    distbin.commands += $$mle(copy /y \"$$TRANSLATIONS_DIR\\*.png\" \"$$T/translations\")
    distbin.commands += $$mle(copy /y \"$$TRANSLATIONS_DIR\\translations.conf\" \"$$T/translations\")

    for(l, LANGUAGES) {
        qtl=$$[QT_INSTALL_TRANSLATIONS]\\qt_$${l}.qm $$[QT_INSTALL_TRANSLATIONS]\\qtbase_$${l}.qm

        for(l, qtl) {
            exists($$l) {
                distbin.commands += $$mle(copy /y \"$$l\" \"$$T/translations\")
            }
        }
    }

    for(lc, LICENSES) {
        distbin.commands += $$mle(copy /y \"$${_PRO_FILE_PWD_}\\$$lc\" \"$$T\")
    }

    # compress
    distbin.commands += $$mle(del /F /Q $${TARGET}-portable-$${VERSION}$${HOST64}.zip)
    distbin.commands += $$mle($$ZIP a -r -tzip -mx=9 $${TARGET}-portable-$${VERSION}$${HOST64}.zip \"$$T\")
    distbin.commands += $$mle(rd /S /Q \"$$T\")

    QMAKE_EXTRA_TARGETS += distbin
} else {
    warning("7Z is not found, will not create custom dist targets")
}

!isEmpty(GCCDIR):exists($$INNO) {
    message("Inno Setup is found, will create a setup file in a custom dist target")

    INNO_APPID="{{C498F818-43DB-438C-8DAA-272DE94FFCE1}"
    LANGS=$$system(dir /B \"$$INNO\\..\\Languages\")

    ISS="$${TARGET}-$${VERSION}.iss"

    iss.commands += $$mle(echo $${LITERAL_HASH}define MyAppName \"Phrasebooks\" > $$ISS)
    iss.commands += $$mle(echo $${LITERAL_HASH}define MyAppPublisher \"Dmitry Baryshev\" >> $$ISS)
    iss.commands += $$mle(echo $${LITERAL_HASH}define MyAppURL \"$$HTTPROOT\" >> $$ISS)

    iss.commands += $$mle(echo [Setup] >> $$ISS)

    !isEmpty(HOST64) {
        iss.commands += $$mle(echo ArchitecturesAllowed=x64 >> $$ISS)
        iss.commands += $$mle(echo ArchitecturesInstallIn64BitMode=x64 >> $$ISS)
    }

    iss.commands += $$mle(echo AppId=$$INNO_APPID >> $$ISS)
    iss.commands += $$mle(echo AppName={$${LITERAL_HASH}MyAppName} >> $$ISS)
    iss.commands += $$mle(echo AppVersion=$$VERSION >> $$ISS)
    iss.commands += $$mle(echo AppPublisher={$${LITERAL_HASH}MyAppPublisher} >> $$ISS)
    iss.commands += $$mle(echo AppPublisherURL={$${LITERAL_HASH}MyAppURL} >> $$ISS)
    iss.commands += $$mle(echo AppSupportURL={$${LITERAL_HASH}MyAppURL}/wiki/howto >> $$ISS)
    iss.commands += $$mle(echo AppUpdatesURL={$${LITERAL_HASH}MyAppURL} >> $$ISS)
    iss.commands += $$mle(echo DefaultDirName={pf}\\{$${LITERAL_HASH}MyAppName} >> $$ISS)
    iss.commands += $$mle(echo DefaultGroupName={$${LITERAL_HASH}MyAppName} >> $$ISS)
    iss.commands += $$mle(echo LicenseFile=$${_PRO_FILE_PWD_}\\LICENSE.txt >> $$ISS)
    iss.commands += $$mle(echo OutputDir=. >> $$ISS)
    iss.commands += $$mle(echo OutputBaseFilename=$${TARGET}-setup-$${VERSION}$${HOST64} >> $$ISS)
    iss.commands += $$mle(echo SetupIconFile=$${_PRO_FILE_PWD_}\\images\\phrasebooks.ico >> $$ISS)
    iss.commands += $$mle(echo Compression=lzma >> $$ISS)
    iss.commands += $$mle(echo SolidCompression=yes >> $$ISS)
    iss.commands += $$mle(echo UninstallDisplayName={$${LITERAL_HASH}MyAppName} >> $$ISS)
    iss.commands += $$mle(echo UninstallDisplayIcon={app}\\$${TARGET}.exe >> $$ISS)
    iss.commands += $$mle(echo MinVersion="0,5.1" >> $$ISS)

    !isEmpty(SIGNTOOL):exists($$CERT) {
        iss.commands += $$mle(echo SignTool=bps sign /d \$\$qPhrasebooks\$\$q /du \$\$q$$HTTPROOT\$\$q /f \$\$q$$CERT\$\$q /tr \$\$q$$RFC3161_SERVER\$\$q /v \$\$q\$\$f\$\$q >> $$ISS)
    }

    iss.commands += $$mle(echo [Languages] >> $$ISS)
    iss.commands += $$mle(echo Name: \"Default\"; MessagesFile: \"compiler:Default.isl\" >> $$ISS)

    for(lng, LANGS) {
        lng = $$find(lng, .isl$)

        !isEmpty(lng) {
            lngname=$$lng
            lngname ~= s/\\./x
            iss.commands += $$mle(echo Name: \"$$lngname\"; MessagesFile: \"compiler:Languages\\$$lng\" >> $$ISS)
        }
    }

    iss.commands += $$mle(echo [Tasks] >> $$ISS)
    iss.commands += $$mle(echo Name: \"desktopicon\"; Description: \"{cm:CreateDesktopIcon}\"; GroupDescription: \"{cm:AdditionalIcons}\" >> $$ISS)
    iss.commands += $$mle(echo Name: \"quicklaunchicon\"; Description: \"{cm:CreateQuickLaunchIcon}\"; GroupDescription: \"{cm:AdditionalIcons}\"; OnlyBelowVersion: "0,6.1" >> $$ISS)

    iss.commands += $$mle(echo [Files] >> $$ISS)
    iss.commands += $$mle(echo Source: \"$${OUT_PWD}/$(DESTDIR_TARGET)\"; DestDir: \"{app}\"; Flags: ignoreversion >> $$ISS)

    # platforms plugin
    iss.commands += $$mle(echo Source: \"$$[QT_INSTALL_PLUGINS]\\platforms\\qwindows.dll\"; DestDir: \"{app}\\platforms\"; Flags: ignoreversion >> $$ISS)

    for(lc, LICENSES) {
        iss.commands += $$mle(echo Source: \"$${_PRO_FILE_PWD_}\\$$lc\"; DestDir: \"{app}\"; Flags: ignoreversion >> $$ISS)
    }

    iss.commands += $$mle(echo Source: \"$$TRANSLATIONS_DIR/*.qm\"; DestDir: \"{app}/translations\"; Flags: ignoreversion >> $$ISS)
    iss.commands += $$mle(echo Source: \"$$TRANSLATIONS_DIR/*.png\"; DestDir: \"{app}/translations\"; Flags: ignoreversion >> $$ISS)
    iss.commands += $$mle(echo Source: \"$$TRANSLATIONS_DIR/translations.conf\"; DestDir: \"{app}/translations\"; Flags: ignoreversion >> $$ISS)

    for(l, LANGUAGES) {
        qtl=$$[QT_INSTALL_TRANSLATIONS]\\qt_$${l}.qm $$[QT_INSTALL_TRANSLATIONS]\\qtbase_$${l}.qm

        for(l, qtl) {
            exists($$l) {
                iss.commands += $$mle(echo Source: \"$$l\"; DestDir: \"{app}/translations\"; Flags: ignoreversion >> $$ISS)
            }
        }
    }

    for(ql, QTLIBS) {
        iss.commands += $$mle(echo Source: \"$$[QT_INSTALL_BINS]\\$$ql\"; DestDir: \"{app}\"; Flags: ignoreversion >> $$ISS)
    }

    for(sl, SSLLIBS) {
        iss.commands += $$mle(echo Source: \"$${_PRO_FILE_PWD_}\\extra\\$$sl\"; DestDir: \"{app}\"; Flags: ignoreversion >> $$ISS)
    }

    for(bp, BEARERPLUGINS) {
        iss.commands += $$mle(echo Source: \"$$[QT_INSTALL_PLUGINS]\\bearer\\$$bp\"; DestDir: \"{app}\\bearer\"; Flags: ignoreversion >> $$ISS)
    }

    for(ip, IMAGEPLUGINS) {
        iss.commands += $$mle(echo Source: \"$$[QT_INSTALL_PLUGINS]\\imageformats\\$$ip\"; DestDir: \"{app}\\imageformats\"; Flags: ignoreversion >> $$ISS)
    }

    for(sp, SQLPLUGINS) {
        iss.commands += $$mle(echo Source: \"$$[QT_INSTALL_PLUGINS]\\sqldrivers\\$$sp\"; DestDir: \"{app}\\sqldrivers\"; Flags: ignoreversion >> $$ISS)
    }

    for(ml, MINGWLIBS) {
        iss.commands += $$mle(echo Source: \"$$GCCDIR\\$$ml\"; DestDir: \"{app}\"; Flags: ignoreversion >> $$ISS)
    }

    iss.commands += $$mle(echo [Icons] >> $$ISS)
    iss.commands += $$mle(echo Name: \"{group}\\{$${LITERAL_HASH}MyAppName}\"; Filename: \"{app}\\$${TARGET}.exe\" >> $$ISS)
    iss.commands += $$mle(echo Name: \"{group}\\{cm:UninstallProgram","{$${LITERAL_HASH}MyAppName}}\"; Filename: \"{uninstallexe}\" >> $$ISS)
    iss.commands += $$mle(echo Name: \"{commondesktop}\\{$${LITERAL_HASH}MyAppName}\"; Filename: \"{app}\\$${TARGET}.exe\"; Tasks: desktopicon >> $$ISS)
    iss.commands += $$mle(echo Name: \"{userappdata}\\Microsoft\\Internet Explorer\\Quick Launch\\{$${LITERAL_HASH}MyAppName}\"; Filename: \"{app}\\$${TARGET}.exe\"; Tasks: quicklaunchicon >> $$ISS)

    iss.commands += $$mle(echo [Registry] >> $$ISS)
    iss.commands += $$mle(echo Root: HKLM; Subkey: \"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\"; ValueType: String; ValueName: \"{app}\\$${TARGET}.exe\"; ValueData: \"RUNASADMIN\"; Flags: uninsdeletekeyifempty uninsdeletevalue; MinVersion: "0,6.1" >> $$ISS)

    iss.commands += $$mle(echo [Run] >> $$ISS)
    iss.commands += $$mle(echo Filename: \"{$${LITERAL_HASH}MyAppURL}/wiki/howto\"; Flags: nowait shellexec >> $$ISS)

    iss.commands += $$mle(echo [Code] >> $$ISS)
    iss.commands += $$mle(echo procedure CurStepChanged(CurStep: TSetupStep); >> $$ISS)
    iss.commands += $$mle(echo var >> $$ISS)
    iss.commands += $$mle(echo ResultCode: Integer; >> $$ISS)
    iss.commands += $$mle(echo Uninstall: String; >> $$ISS)
    iss.commands += $$mle(echo UninstallQuery : String; >> $$ISS)
    iss.commands += $$mle(echo begin >> $$ISS)
    iss.commands += $$mle(echo   UninstallQuery := ExpandConstant(\'Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{$${LITERAL_HASH}emit SetupSetting(\"AppId\")}_is1\'); >> $$ISS)

    iss.commands += $$mle(echo   if (CurStep = ssInstall) then begin >> $$ISS)
    iss.commands += $$mle(echo     if RegQueryStringValue(HKLM, UninstallQuery, \'UninstallString\', Uninstall) >> $$ISS)
    iss.commands += $$mle(echo        or RegQueryStringValue(HKCU, UninstallQuery, \'UninstallString\', Uninstall) then begin >> $$ISS)
    iss.commands += $$mle(echo       Uninstall := RemoveQuotes(Uninstall) >> $$ISS)
    iss.commands += $$mle(echo       if (FileExists(Uninstall)) AND (not Exec(RemoveQuotes(Uninstall), \'/VERYSILENT\', \'\', SW_SHOWNORMAL, ewWaitUntilTerminated, ResultCode)) then begin >> $$ISS)
    iss.commands += $$mle(echo         MsgBox(SysErrorMessage(ResultCode), mbCriticalError, MB_OK); >> $$ISS)
    iss.commands += $$mle(echo         Abort(); >> $$ISS)
    iss.commands += $$mle(echo       end; >> $$ISS)
    iss.commands += $$mle(echo     end; >> $$ISS)
    iss.commands += $$mle(echo   end; >> $$ISS)
    iss.commands += $$mle(echo end; >> $$ISS)

    iss.commands += $$mle(echo function FullInstall(Param: String) : String; >> $$ISS)
    iss.commands += $$mle(echo begin >> $$ISS)
    iss.commands += $$mle(echo result := SetupMessage(msgFullInstallation); >> $$ISS)
    iss.commands += $$mle(echo end; >> $$ISS)

    iss.commands += $$mle(echo function CustomInstall(Param: String) : String; >> $$ISS)
    iss.commands += $$mle(echo begin >> $$ISS)
    iss.commands += $$mle(echo result := SetupMessage(msgCustomInstallation); >> $$ISS)
    iss.commands += $$mle(echo end; >> $$ISS)

    iss.commands += $$mle(echo function ReadyMemoComponents(Param: String) : String; >> $$ISS)
    iss.commands += $$mle(echo begin >> $$ISS)
    iss.commands += $$mle(echo result := SetupMessage(msgReadyMemoComponents); >> $$ISS)
    iss.commands += $$mle(echo end; >> $$ISS)

    QMAKE_EXTRA_TARGETS += iss
    QMAKE_EXTRA_TARGETS *= distbin

    !isEmpty(SIGNTOOL):exists($$CERT) {
        ADD="\"/sbps=$$SIGNTOOL \$\$p\""
    }

    distbin.depends += iss
    distbin.commands += $$mle(\"$$INNO\" /o. $$ADD \"$$ISS\")
    distbin.commands += $$mle(del /F /Q \"$$ISS\")
} else {
    warning("Inno Setup is not found, will not create a setup file in a custom dist target")
}

FORMS += \
    src/phrasebooks.ui \
    src/list.ui \
    src/miniinput.ui \
    src/about.ui \
    src/booksandchapters.ui \
    src/selectchapter.ui \
    src/options.ui
