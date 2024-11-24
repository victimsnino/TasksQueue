from conan import ConanFile

class TasksQueueConan(ConanFile):
    name = "TasksQueue"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"
    extension_properties = {"compatibility_cppstd": False}

    options = {
        
    }
    default_options = {
        
    }

    def requirements(self):
        pass