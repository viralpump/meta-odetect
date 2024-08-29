# CMake script to generate model_list.cpp

set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/model_list.cpp")
set(MODEL_DIR "${CMAKE_CURRENT_LIST_DIR}/include/models")
file(WRITE ${OUTPUT_FILE} "// Generated model list implementation. DO NOT CHANGE!\n\n")

file(GLOB MODEL_FILES "${MODEL_DIR}/*.hpp")

foreach(FILE_PATH ${MODEL_FILES})
    get_filename_component(FILE_NAME ${FILE_PATH} NAME)
    file(APPEND ${OUTPUT_FILE} "#include \"models/${FILE_NAME}\"\n")
endforeach()

file(APPEND ${OUTPUT_FILE} "#include \"factories/ModelFactory.hpp\"\n")

file(APPEND ${OUTPUT_FILE} "\nstd::map<std::string, std::unique_ptr<IModelDnnDetector>(*)(const std::string&, const ODCaps, const void*)> ModelFactory::factory = {\n")

foreach(FILE_PATH ${MODEL_FILES})
    get_filename_component(FILE_NAME ${FILE_PATH} NAME_WE)
    file(APPEND ${OUTPUT_FILE} "    {\"${FILE_NAME}\", &${FILE_NAME}::Construct},\n")
endforeach()

file(APPEND ${OUTPUT_FILE} "};\n")

