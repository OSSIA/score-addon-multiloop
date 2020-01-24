project(score_addon_multiloop LANGUAGES CXX)

# General initialization
score_common_setup()

# Source files
set(HDRS
    multiloop/Executor.hpp
    multiloop/Inspector.hpp
    multiloop/LocalTree.hpp
    multiloop/Metadata.hpp
    multiloop/Presenter.hpp
    multiloop/Process.hpp
    multiloop/View.hpp
    multiloop/Layer.hpp
    multiloop/CommandFactory.hpp

    score_addon_multiloop.hpp
)
set(SRCS
    multiloop/Executor.cpp
    multiloop/Inspector.cpp
    multiloop/LocalTree.cpp
    multiloop/Presenter.cpp
    multiloop/Process.cpp
    multiloop/View.cpp

    score_addon_multiloop.cpp
)


# Creation of the library
add_library(${PROJECT_NAME} ${SRCS} ${HDRS})

# Code generation
score_generate_command_list_file(${PROJECT_NAME} "${HDRS}")

# Link
target_link_libraries(${PROJECT_NAME} PUBLIC score_plugin_engine)

# Target-specific options
setup_score_plugin(${PROJECT_NAME})
