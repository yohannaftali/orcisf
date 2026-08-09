add_library(orcisf_engine STATIC
    src/StiffnessSolver.cpp
    src/StructuralAnalysis.cpp
    src/BeamDesign.cpp
    src/ColumnDesign.cpp
    src/CostConstraint.cpp
    src/LegacyIO.cpp
    src/Optimizer.cpp
    src/Engine.cpp
)

target_include_directories(orcisf_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(orcisf_engine PUBLIC cxx_std_20)

find_package(Threads REQUIRED)
target_link_libraries(orcisf_engine PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(orcisf_engine PRIVATE /W4)
else()
    target_compile_options(orcisf_engine PRIVATE -Wall -Wextra)
endif()

# Headless validation/batch CLI -- no GUI dependencies, so it builds even
# before the GUI's heavier dependencies (Dear ImGui/GLFW/etc.) are fetched.
# Used to validate this port against Optimasi Beton/Example/* datasets.
add_executable(orcisf_cli tools/orcisf_cli.cpp)
target_link_libraries(orcisf_cli PRIVATE orcisf_engine)
if(MSVC)
    target_compile_options(orcisf_cli PRIVATE /W4)
else()
    target_compile_options(orcisf_cli PRIVATE -Wall -Wextra)
endif()
