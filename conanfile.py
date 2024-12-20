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
        """
        Add required dependencies when building with backend support.
        
        This method configures the package dependencies based on build options.
        When backend support is enabled, it adds Boost and Reflect-CPP libraries
        as requirements.
        
        Dependencies:
            - boost 1.86.0 (when with_backend=True)
            - reflect-cpp 0.16.0 (when with_backend=True)
        
        Note:
            This method is called internally during the conan package configuration process.
        """
        if self.options.with_backend:
            self.requires("boost/1.86.0")
            self.requires("reflect-cpp/0.16.0")

    def build_requirements(self):
        """
        Build and configure test-related requirements for the project.
        
        This method sets up test dependencies when tests are enabled through build options.
        
        Parameters:
            self: The builder instance containing configuration options
        
        Note:
            This method is only executed when with_tests option is enabled.
            Currently adds doctest and trompeloeil as test dependencies.
        """
        if self.options.with_tests:
            self.test_requires("doctest/2.4.11")
            self.test_requires("trompeloeil/49")
