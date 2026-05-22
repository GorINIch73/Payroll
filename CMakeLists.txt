cmake_minimum_required(VERSION 3.20)
project(Payroll)

include(GNUInstallDirs)

# Настройка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
# add_link_options(-fsanitize=address)

if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

add_definitions(-DGLFW_USE_WAYLAND=1)

include(FetchContent)

# Ищем необходимые библиотеки  
find_package(OpenGL REQUIRED)  

# SQLite встраиваем в приложение: локальный amalgamation или автоматическая загрузка.
if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/sqlite3/sqlite3.c")
    set(SQLITE3_SOURCE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/sqlite3")
else()
    FetchContent_Declare(
        sqlite3_amalgamation
        URL https://www.sqlite.org/2025/sqlite-amalgamation-3500400.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(sqlite3_amalgamation)
    set(SQLITE3_SOURCE_DIR "${sqlite3_amalgamation_SOURCE_DIR}")
endif()

add_library(sqlite3 STATIC "${SQLITE3_SOURCE_DIR}/sqlite3.c")
target_include_directories(sqlite3 PUBLIC "${SQLITE3_SOURCE_DIR}")
target_compile_definitions(sqlite3 PRIVATE
    SQLITE_DQS=0
    SQLITE_THREADSAFE=1
    SQLITE_DEFAULT_FOREIGN_KEYS=1
)

find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
    )
    FetchContent_MakeAvailable(glfw)
endif()

#add_subdirectory(thirdparty/glad)  
# для GLAD
# set(GLAD_DIR ${CMAKE_SOURCE_DIR}/thirdparty/glad)  # Путь к ImGui
# file(GLOB GLAD_SOURCES 
#     "${GLAD_DIR}/src/*.cpp"
# )
#


# Для буфкра обмена
# clip  -  git cgit clone https://github.com/dacap/clip.git thirdparty/clip 
# add_compile_definitions(CLIP_WIC_SUPPORT=false)
# add_compile_definitions(CLIP_WIC_SUPPORT=false)
# target_compile_definitions(clip PRIVATE CLIP_WIC_SUPPORT=false)
# Отключаем WIC поддержку в clip

# add_compile_definitions(CLIP_WIC_SUPPORT=false)

# option(CLIP_ENABLE_IMAGE "CPIL IMAGE" off)

# add_subdirectory(thirdparty/clip)
# Указываем пути к установленной библиотеке
# set(CLIP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/clip)



# Для IMGUI: сначала используем локальный thirdparty, иначе скачиваем через CMake
if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/imgui/imgui.h")
    set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/thirdparty/imgui)
else()
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking
    )
    FetchContent_MakeAvailable(imgui)
    set(IMGUI_DIR ${imgui_SOURCE_DIR})
endif()

file(GLOB IMGUI_SOURCES 
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/misc/cpp/*.cpp"

    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)


# Для ImGuiFileDialog: сначала используем локальный thirdparty, иначе скачиваем
if(EXISTS "${CMAKE_SOURCE_DIR}/thirdparty/ImGuiFileDialog/ImGuiFileDialog.h")
    set(ImGuiFileDialog_DIR ${CMAKE_SOURCE_DIR}/thirdparty/ImGuiFileDialog)
else()
    FetchContent_Declare(
        imguifiledialog
        GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(imguifiledialog)
    set(ImGuiFileDialog_DIR ${imguifiledialog_SOURCE_DIR})
endif()

file(GLOB ImGuiFileDialog_SOURCES 
    "${ImGuiFileDialog_DIR}/*.cpp"
)


# Основной исполняемый файл
add_executable(Payroll

    src/main.cpp  
    src/Database.cpp  
    src/GUI.cpp  
    src/imgui_components.cpp
    src/EmployeesPanel.cpp
    src/IndividualsPanel.cpp
    src/DivisionsPanel.cpp
    src/PositionsPanel.cpp
    src/AccrualsPanel.cpp
    src/OrdersPanel.cpp
    src/StatementsPanel.cpp
    src/ReviewPanel.cpp
    src/Manager.cpp
    src/SettingsPanel.cpp

    src/Settings.cpp  
    src/HtmlExporter.cpp  
 
    ${IMGUI_SOURCES}
    # ${GLAD_SOURCES}
	${ImGuiFileDialog_SOURCES}

)

# Подключаем зависимости
target_include_directories(Payroll  PRIVATE
    ${CMAKE_SOURCE_DIR}/src  #наши заголовкиi
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${IMGUI_DIR}/misc/cpp
    # ${GLAD_DIR}/include

    ${OPENGL_INCLUDE_DIR}    
    ${GLFW3_INCLUDE_DIR}
    # ${CLIP_INCLUDE_DIR}

	${ImGuiFileDialog_DIR}

)


target_link_libraries(Payroll PRIVATE
    glfw
    sqlite3
    # clip 
    ${OPENGL_LIBRARIES}
)

if(WIN32)
    target_link_libraries(Payroll PRIVATE Shell32)
endif()

# Копируем Фонты после сборки
add_custom_command(
    TARGET Payroll POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/fonts/"
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/src/fonts/NotoSans-Regular.ttf
        ${CMAKE_CURRENT_BINARY_DIR}/fonts/
)

add_custom_command(
    TARGET Payroll POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/fonts/"
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/src/fonts/fontawesome-webfont.ttf
        ${CMAKE_CURRENT_BINARY_DIR}/fonts/
)


# копируем библиотеку clip - пока не нужен!
# if(WIN32)
#     # Копируем DLL после сборки
#     add_custom_command(
#         TARGET Payroll POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy
#             ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/clip/libclip.dll
#             ${CMAKE_CURRENT_BINARY_DIR}/
#     )
# else()
#
#     # Копируем .so в папку сборки
#     add_custom_command(
#         TARGET Payroll POST_BUILD
#         COMMAND ${CMAKE_COMMAND} -E copy
#             ${CMAKE_CURRENT_BINARY_DIR}/thirdparty/clip/libclip.so
#             ${CMAKE_CURRENT_BINARY_DIR}/
#     )
# endif()
# Для Windows: копирование DLL GLFW
if(WIN32)
    find_file(GLFW_DLL
        NAMES glfw3.dll
        PATHS ${GLFW_ROOT}/lib-mingw-w64
        REQUIRED
    )
    add_custom_command(TARGET Payroll POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${GLFW_DLL}"
        $<TARGET_FILE_DIR:Payroll>
    )
endif()


configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/data/payroll.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/payroll.desktop
    @ONLY
)

# Установка приложения и desktop-интеграции
install(TARGETS Payroll
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/payroll.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/payroll.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)

# Настройка CPack для создания .tar.gz и .deb/.rpm (если нужно)
include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "TGZ")  # Для Arch Linux можно использовать TGZ или STGZ
set(CPACK_PACKAGE_NAME "Payroll-app")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION "Payroll ImGUI + embedded SQLite App")
set(CPACK_PACKAGE_CONTACT "ggorinich@@gmail.com")
include(CPack)

# Установка шрифтов
install(DIRECTORY src/fonts/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/fonts
)
