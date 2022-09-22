

cc_library(
    name = "lox_compiler",
    srcs = [
            "chunk.cpp",
            "compiler.cpp",
            "error.cpp",
            "heap.cpp",
            "scanner.cpp",
            "source.cpp",
            "value.cpp",
            "virtual_machine.cpp",
    ],
    hdrs = [
            "chunk.h",
            "compiler.h",
            "error.h",
            "heap.h",
            "object.h",
            "scanner.h",
            "source.h",
            "value.h",
            "virtual_machine.h",
    ],
    linkstatic = True,
    includes = ["."],
    copts=["-Wall", 
           "-Wextra", 
           "-Werror", 
           "-fno-exceptions"] + 
            select({":debug_build": ["-fsanitize=address", "-fsanitize=undefined"], "//conditions:default": []}),
    linkopts = select({":debug_build": ["-fsanitize=address", "-fsanitize=undefined"], "//conditions:default": []}),
    visibility = ["//visibility:public"],
    deps = ["@fmt//:fmt"]
)

cc_binary(
    name = "lox_cpp",
    srcs = [
            "main.cpp",
    ],
    copts=["-Wall", 
           "-Wextra", 
           "-Werror", 
           "-fno-exceptions"] + 
            select({":debug_build": ["-fsanitize=address", "-fsanitize=undefined"], "//conditions:default": []}),
    visibility = ["//visibility:public"],
    deps = ["@fmt//:fmt", ":lox_compiler"]

)


config_setting(
    name = "debug_build",
    values = {        
        "compilation_mode": "dbg",
    },
)
