cmake_minimum_required(VERSION 4.0)
project(Payroll)

# Настройка компилятора для Windows
set(CMAKE_CXX_STANDARD 17)

if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()


# Ищем необходимые библиотеки  
find_package(OpenGL REQUIRED)  
find_package(glfw3 REQUIRED)
find_package(SQLite3 REQUIRED)
#find_package(HPDF REQUIRED)


# LibHaru для pdf git clone --depth=1 https://github.com/libharu/libharu.git thirdparty/libharu
add_subdirectory(thirdparty/libharu)
# include(ExternalProject)
# Указываем пути
set(LIBHARU_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/libharu/include)
# if (WIN32)
#     set(LIBHARU_LIBRARY ${CMAKE_SOURCE_DIR}/thirdparty/libharu/libhpdf.dll.a)
# else()
#     #set(LIBHARU_LIBRARY ${CMAKE_BINARY_DIR}/libharu-install/lib/libhpdf.so)
#     set(LIBHARU_LIBRARY ${CMAKE_SOURCE_DIR}/thirdparty/libharu/libhpdf.so)
# endif()


# json для файла конфигурации git clone --depth=1 https://github.com/nlohmann/json.git thirdparty/json 
add_subdirectory(thirdparty/json)

# Указываем пути к установленной библиотеке
set(JSON_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/thirdparty/json/include)

#add_subdirectory(thirdparty/glad)  
# для GLAD
# set(GLAD_DIR ${CMAKE_SOURCE_DIR}/thirdparty/glad)  # Путь к ImGui
# file(GLOB GLAD_SOURCES 
#     "${GLAD_DIR}/src/*.cpp"
# )
#


# для IMGUI
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/thirdparty/imgui)  # Путь к ImGui
file(GLOB IMGUI_SOURCES 
    "${IMGUI_DIR}/*.cpp"
    "${IMGUI_DIR}/misc/cpp/*.cpp"

    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)


# для ImGuiFileDialog git clone https://github.com/aiekick/ImGuiFileDialog.git thirdparty/ImGuiFileDialog
set(ImGuiFileDialog_DIR ${CMAKE_SOURCE_DIR}/thirdparty/ImGuiFileDialog)  # Путь
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
    src/PositionsPanel.cpp
    #src/EmployeeListPanel.cpp
    #src/EmployeeEditPanel.cpp

    #    src/Settings.cpp  
    #    src/PdfExporter.cpp  
 
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
    ${SQLite3_INCLUDE_DIR} 
	${LIBHARU_INCLUDE_DIR}
	${JSON_INCLUDE_DIR}

	${ImGuiFileDialog_DIR}

)


target_link_libraries(Payroll PRIVATE
    glfw
    hpdf
    ${OPENGL_LIBRARIES}
    ${SQLite3_LIBRARY} 
)

if(WIN32)
    # хз почему не копирует под винду
else()
add_custom_command(
    TARGET Payroll  # Имя вашего target (исполняемого файла/библиотеки)
    POST_BUILD     # Копировать после сборки
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/src/font/*.ttf
        ${CMAKE_CURRENT_BINARY_DIR}/
    COMMENT "Копирование ресурсов в каталог сборки..."
)
endif()

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
