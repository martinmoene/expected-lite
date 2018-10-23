from conans import ConanFile

class ExpectedLiteConan(ConanFile):
    version = "0.0.0"
    name = "expected-lite"
    description = "Expected objects for C++11 and later"
    license = "Boost Software License - Version 1.0. http://www.boost.org/LICENSE_1_0.txt"
    url = "https://github.com/martinmoene/expected-lite"
    exports_sources = "include/nonstd/*", "LICENSE.txt"
    author = "Martin Moene"

    def package_info(self):
        self.info.header_only()
