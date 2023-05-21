
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})
add_executable(chat chat.cpp header.h utils.h parse_json.h chat_header.h server.h ../gpt4all-backend/llmodel_c.h)
target_link_libraries(chat PRIVATE  llmodel llama OpenSSL::SSL OpenSSL::Crypto Threads::Threads)
