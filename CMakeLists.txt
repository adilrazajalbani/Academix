cmake_minimum_required(VERSION 3.21)

project(Academix VERSION 1.0 LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt's tools (moc) run automatically for classes that use Q_OBJECT
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(Academix
    main.cpp
    student.h
    student.cpp
    studentmanager.h
    studentmanager.cpp
    mainwindow.h
    mainwindow.cpp
)

target_link_libraries(Academix PRIVATE Qt6::Widgets)

# students.csv is stored in the project folder (the folder that contains this
# CMakeLists.txt), so the program always finds it no matter where it is started.
target_compile_definitions(Academix PRIVATE
    ACADEMIX_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
