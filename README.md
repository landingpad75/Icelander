# Icelander

Icelander is a simple, lightweight, and easy-to-use C++ library for UDP communication.

## Notes

- This is my first time utilizing multiple cmakelists.txt, so if it doesn't work as expected don't blame me
- This code was made in around 14 hours, so expect bugs
- This is a library for me to use in my own programs, so expect 0 support if you want to use it in your own programs
- You shall NOT act like this library is yours, provide proper attribution.

## Building

- Clone the repository:
  ```bash
  git clone https://github.com/yourusername/icelander.git
  cd icelander
  ```
- Build the library:
  ```bash
  mkdir build
  cd build
  cmake ..
  cmake --build .
  ```

## Usage

### With CMake

- Add the repository as a submodule:
  ```bash
  git submodule add https://github.com/yourusername/icelander.git
  ```

- Add the repository as a dependency:
  ```cmake
  add_subdirectory(icelander)
  target_link_libraries(your_target PRIVATE icelander)
  ```
