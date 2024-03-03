add_executable(em_papilio "papilio_embind.cpp")
target_link_libraries(em_papilio PUBLIC papilio)
target_link_options(em_papilio PUBLIC "-lembind")
