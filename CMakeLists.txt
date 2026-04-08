cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(BIOAnRF5340)

# 1. Definiciones globales
target_compile_definitions(app PRIVATE CHIPSEL_594X)

# 2. Asegúrate de incluir la carpeta de cabeceras para que todos se vean entre sí
target_include_directories(app PRIVATE 
    src/AD5940 
    src
)

# 3. Lista de fuentes (Asegúrate de que los nombres coincidan exactamente con los archivos en disco)
target_sources(app PRIVATE
    src/main.c
    src/AD5940/ad5940.c
    src/AD5940/BodyImpedance.c
    src/AD5940/ad5940_port.c   # <--- Verifica que el archivo se llame así en la carpeta
)