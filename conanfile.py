from conans import ConanFile, Meson, tools

class RapidTmxConan(ConanFile):
    name = "rapid-tmx"
    version = "0.1"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "pkg_config"
    exports_sources = "meson.build", "include/*", "src/*", "external/*", "tests/*"
    requires = "zlib/1.2.11", "boost/1.69.0", "rapidxml/1.13"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def build(self):
        meson = Meson(self)
        meson.configure(build_folder="build")
        meson.build()

    def package(self):
        self.copy("*.h", dst="include", src="include")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["rapid-tmx"]
