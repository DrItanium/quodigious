cc_library(
        name = "libquod",
        hdrs = [ "include/qlib.h", ],
        )
        

cc_binary(
        name = "quodigiousOctalEncoding",
        srcs = [ "octalLikeEncoding.cc" ],
        copts = [ "-std=c++17", "-march=native", "-O3", "-fwhole-program", 
                "-Imain/include",
                 ],
        linkopts = [ "-lpthread", "-O3", "-std=c++17", "-march=native", "-fwhole-program" ],
        deps = [":libquod"],
        )
