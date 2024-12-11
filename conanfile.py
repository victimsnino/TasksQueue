from conan import ConanFile

class TasksQueueConan(ConanFile):
    name = "tasks_queue"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    extension_properties = {"compatibility_cppstd": False}

    options = {
        "with_tests": [True, False],
        "with_backend": [True, False]
    }
    default_options = {
        "with_tests": False,
        "with_backend": False
    }

    def requirements(self):
        if self.options.with_backend:
            self.requires("grpc/1.67.1")
            self.requires("protobuf/5.27.0")
            self.requires("googleapis/cci.20230501")

    def build_requirements(self):
        if self.options.with_tests:
            self.test_requires("doctest/2.4.11")
            self.test_requires("trompeloeil/48")
