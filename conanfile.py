from conans import ConanFile, Meson, tools
import re, os

class RapidTmxConan(ConanFile):
    name = "rapid-tmx"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "pkg_config"
    exports_sources = "meson.build", "include/*", "src/*", "external/*", "tests/*"
    requires = "zlib/1.2.11", "rapidxml/1.13"

    def set_version(self):
        content = tools.load(os.path.join(self.recipe_folder, "meson.build"))
        version = re.search(r"version\s*:\s*'([\d.]+)'", content).group(1)
        self.version = version.strip()

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
