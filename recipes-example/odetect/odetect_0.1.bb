DESCRIPTION = "Simple application using OpenCV"
SECTION = "examples"
LICENSE = "CLOSED"

SRC_URI = "file://odetect"

DEPENDS = "opencv gstreamer1.0 gstreamer1.0-plugins-base gstreamer1.0-plugins-bad gstreamer1.0-plugins-good"

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