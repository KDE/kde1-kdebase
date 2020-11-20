function(create_kde1_config_header)
    include(CheckIncludeFiles)
    include(CheckFunctionExists)
    include(CheckStructHasMember)
    include(CheckCSourceCompiles)

    check_include_files(rpc/rpc.h HAVE_RPC_RPC_H)
    check_include_files(crypt.h HAVE_CRYPT_H)
    check_include_files(rpc/key_prot.h HAVE_RPC_KEY_PROT_H)

    configure_file(${PROJECT_SOURCE_DIR}/common/config.h.in ${PROJECT_BINARY_DIR}/config.h)
    include_directories(${PROJECT_BINARY_DIR})
    add_definitions(-DHAVE_CONFIG_H)
endfunction()
