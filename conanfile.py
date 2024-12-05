from conan import ConanFile

class TasksQueueConan(ConanFile):
    name = "tasks_queue"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    extension_properties = {"compatibility_cppstd": False}

    options = {
        "with_tests": [True, False],
    }
    default_options = {
        "with_tests": False
    }

    def requirements(self):
        self.requires("boost/1.86.0")
        self.requires("openssl/3.3.2")

    def build_requirements(self):
        if self.options.with_tests:
            self.test_requires("doctest/2.4.11")
            self.test_requires("trompeloeil/48")
