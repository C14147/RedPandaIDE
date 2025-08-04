# Because of the qt6 static library on MSYS2 is not standard,
# so there will include some libs customly.

contains(QT, widgets){
    MSYSTEM_PREFIX=$$(MSYSTEM_PREFIX)
    greaterThan(MSYSTEM_PREFIX,' '){
        contains(CONFIG, static) {
	        message("STATIC Qt6 with MSYS2. Extra patch should be introduced.");
            CONFIG += no_lflags_merge
            DEFINES+=BUILD_MODERN
	        QMAKE_LIBS += -ltiff  -lmng.dll -ljpeg -ljbig -ldeflate  -lzstd -llerc -llzma  -lgraphite2 -lbz2 -lusp10 -lRpcrt4 -lsharpyuv -lOleAut32
        }
    }
}
