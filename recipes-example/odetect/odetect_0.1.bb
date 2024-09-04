DESCRIPTION = "Application for detecting various objects."
SECTION = "examples"

LIC_FILES_CHKSUM="file://LICENSE;md5=3da9cfbcb788c80a0384361b4de20420"
LICENSE = "MIT"

# Because of x264 codec and gstreamer1.0-plugins-ugly
LICENSE_FLAGS = "commercial"

SRC_URI = "file://odetect"

PREFERRED_VERSION_opencv = "4.5.5"
DEPENDS = "opencv gstreamer1.0 gstreamer1.0-plugins-base gstreamer1.0-plugins-bad gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly x264"

inherit cmake pkgconfig

B = "${WORKDIR}/build"
S = "${WORKDIR}/odetect"

do_configure() {
    cmake ${S} -B${B} -DCMAKE_INSTALL_PREFIX=${D}
}

do_compile() {
    cmake --build ${B}
}

do_install() {
    cmake --install ${B} --prefix ${D}
    install -d ${D}/usr/share/odetect
    cp -r ${S}/resourses/* ${D}/usr/share/odetect/
}

FILES_${PN} = "${bindir}/odetect"