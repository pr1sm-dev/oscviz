file(GLOB IMGUI_SOURCES imgui/*.cpp)

add_library(imgui STATIC ${IMGUI_SOURCES})
