from conans import ConanFile, CMake, tools


class AdvancedFlexServerConan(ConanFile):
    name = "advanced-flex-server"
    version = "0.9"
    license = "Apache 2.0"
    author = "vortarian vortarian@systemicai.com"
    url = "https://gitlab.com/k8s.makerlabs.us/advanced-flex-server"
    description = "Evolution of Beast Advanced Flex Server for experimentation"
    topics = ("http", "cpp", "c++", "systemicai")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    generators = "cmake"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def source(self):
        self.run("git clone git@gitlab.com:k8s.makerlabs.us/advanced-flex-server.git")

    def build(self):
        #cmake = CMake(self)
        #cmake.configure(source_folder=self.name)
        #cmake.build()

        # Explicit way:
        self.run('bash %s/%s/install-dependencies.sh' % (self.source_folder, self.name))
        self.run('cmake %s/%s -DCMAKE_BUILD_TYPE=Release' % (self.source_folder, self.name))
        self.run("make")

    def package(self):
        self.copy("*", dst="cfg", src="dist/cfg", keep_path=False)
        self.copy("*", dst="bin", src="dist/bin", keep_path=False)
        self.copy("*", dst="html", src="dist/html", keep_path=False)
        self.copy("*", dst="info", src="dist/info", keep_path=False)

    def package_info(self):
        self.cpp_info.bindirs = ["bin"]
        self.cpp_info.resdirs= ["cfg", "html", "info"]

