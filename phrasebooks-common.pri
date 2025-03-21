isEmpty(TARGET) {
    error("TARGET is not defined")
}

lessThan(QT_MAJOR_VERSION, 5) {
    error("This project requires Qt 5.1 or greater")
} else {
    equals(QT_MAJOR_VERSION, 5) {
        lessThan(QT_MINOR_VERSION, 1) {
            error("This project requires Qt 5.1 or greater")
        }
    }
}

win32 {
    LANGUAGES=$$system(dir /B \"$${_PRO_FILE_PWD_}\\ts\\*.ts\")
} else {
    LANGUAGES=$$system(find \"$${_PRO_FILE_PWD_}/ts/\" -name \"*.ts\" -printf \"%f \")
}

LANGUAGES=$$replace(LANGUAGES, .ts, )

CONFIG += warn_on c++11

VERSION=$$sprintf("%1.%2.%3", $$VERSION_MAJOR, $$VERSION_MINOR, $$VERSION_PATCH)

DEFINES += VERSION_MAJOR=$$VERSION_MAJOR
DEFINES += VERSION_MINOR=$$VERSION_MINOR
DEFINES += VERSION_PATCH=$$VERSION_PATCH
DEFINES += VERSION_STRING=$$sprintf("\"\\\"%1\\\"\"", $$VERSION)

DEFINES += TARGET=$$TARGET
DEFINES += TARGET_STRING=$$sprintf("\"\\\"%1\\\"\"", $$TARGET)

# require at least Windows XP
win32 {
    DEFINES += _WIN32_WINNT=0x0501 WINVER=0x0501
}

# use fast string concat
DEFINES *= QT_USE_QSTRINGBUILDER

contains(QMAKE_HOST.arch, x86_64) {
    HOST64="-x86_64"
}

# search an executable in PATH
defineReplace(findexe) {
    return ( $$system(for %i in ($$1) do @echo. %~$PATH:i) )
}

# escape command to allow multiple lines in Makefile
defineReplace(mle) {
    return ( $$1$$escape_expand(\\n\\t) )
}

# check for gcc
win32-g++ {
    GCC=$$findexe("gcc.exe")
    GCCDIR=$$dirname(GCC)

    isEmpty(GCCDIR) {
        message("MinGW is not found in PATH. Custom dist targets are disabled")
    } else {
        message("MinGW is found in PATH. Custom dist targets are enabled")
    }

    # check for 7z
    ZIP=$$findexe("7z.exe")
}

for(ts, LANGUAGES) {
    win32 {
        LRELEASE="$$[QT_INSTALL_BINS]\\lrelease.exe"
        LUPDATE="$$[QT_INSTALL_BINS]\\lupdate.exe"
        MTRANSLATIONS += $${_PRO_FILE_PWD_}\\ts\\$${ts}.ts
    } else {
        LRELEASE=$$system(which lrelease)
        LUPDATE=$$system(which lupdate)
        MTRANSLATIONS += $${_PRO_FILE_PWD_}/ts/$${ts}.ts
    }
}

message(Translations: $$MTRANSLATIONS)

# copy translations
!isEmpty(LUPDATE) {
    message("lupdate is found, will update the translations")
    QMAKE_POST_LINK += $$mle($$LUPDATE -no-obsolete $$_PRO_FILE_ -ts $$MTRANSLATIONS)
}

# lrelease for each ts
!isEmpty(LRELEASE) {
    win32 {
        TRANSLATIONS_DIR="$${OUT_PWD}/$(DESTDIR_TARGET)/../translations"
        QMAKE_POST_LINK += $$mle(if not exist \"$$TRANSLATIONS_DIR\" mkdir \"$$TRANSLATIONS_DIR\")

        for(ts, LANGUAGES) {
            QMAKE_POST_LINK += $$mle($$LRELEASE \"$${_PRO_FILE_PWD_}\\ts\\$${ts}.ts\" -qm \"$$TRANSLATIONS_DIR\\$${ts}.qm\")
            QMAKE_POST_LINK += $$mle(copy /Y \"$${_PRO_FILE_PWD_}\\ts\\$${ts}.png\" \"$$TRANSLATIONS_DIR\\$${ts}.png\")
        }

        QMAKE_POST_LINK += $$mle(copy /Y \"$${_PRO_FILE_PWD_}\\ts\\translations.conf\" \"$$TRANSLATIONS_DIR\\\")
    } else {
        TRANSLATIONS_DIR="$${OUT_PWD}/translations"
        QMAKE_POST_LINK += $$mle(mkdir -p \"$$TRANSLATIONS_DIR\")

        for(ts, LANGUAGES) {
            QMAKE_POST_LINK += $$mle($$LRELEASE \"$${_PRO_FILE_PWD_}/ts/$${ts}.ts\" -qm \"$$TRANSLATIONS_DIR/$${ts}.qm\")
            QMAKE_POST_LINK += $$mle(cp -f \"$${_PRO_FILE_PWD_}/ts/$${ts}.png\" \"$$TRANSLATIONS_DIR/$${ts}.png\")
        }

        QMAKE_POST_LINK += $$mle(cp -f \"$${_PRO_FILE_PWD_}/ts/translations.conf\" \"$$TRANSLATIONS_DIR/\")
    }
}

win32 {
    SIGNTOOL=$$findexe("signtool.exe")
    CERT=$$dirname(_PRO_FILE_)-certs\\cert.pfx
    RFC3161_SERVER="http://timestamp.comodoca.com/rfc3161"

    !isEmpty(SIGNTOOL):exists($$CERT) {
        message("Signtool and the certificate are found, will sign the $$TARGET executable")
    } else {
        warning("Signtool or the certificate is not found, will not sign the $$TARGET executable")
    }

    # sign
    !isEmpty(SIGNTOOL):exists($$CERT) {
        QMAKE_POST_LINK += $$mle($$SIGNTOOL sign /d \"Phrasebooks\" /du \"$$HTTPROOT\" /f \"$$CERT\" /tr \"$$RFC3161_SERVER\" /v \"$${OUT_PWD}/$(DESTDIR_TARGET)\")
    }
}

# INNO setup
win32 {
    INNO=$$system(echo %ProgramFiles(x86)%)\\Inno Setup 5\\iscc.exe

    !exists($$INNO) {
        INNO=$$system(echo %ProgramFiles%)\\Inno Setup 5\\iscc.exe
    }
}
