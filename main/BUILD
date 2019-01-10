cc_library(
        name = "libquod",
        hdrs = [ "include/qlib.h", ],
        )


cc_binary(
        name = "quodigious",
        srcs = [ "octalLikeEncoding.cc" ],
        copts = [ "-std=c++17", "-march=native", "-O3", "-fwhole-program", 
                 "-Imain/include",
                 ],
        linkopts = [ "-lpthread", "-O3", "-std=c++17", "-march=native", "-fwhole-program" ],
        deps = [":libquod"],
        )
cc_library(
        # these files are included in other files, they are not directly compilable
        # don't allow bazel to believe they are either
        name = "libquod64_oldloops",
        hdrs = [ "Specialization10Digits.cc",
                "Specialization2Digits.cc",
                "Specialization3Digits.cc",
                "Specialization4Digits.cc",
                "Specialization5Digits.cc",
                "Specialization6Digits.cc",
                "Specialization7Digits.cc",
                "Specialization8Digits.cc",
                "Specialization9Digits.cc",
                ],
        )
cc_binary(
        name = "quodigious64_OldLoops",
        srcs = [ "loops64.cc", ],
        copts = [ "-std=c++17", "-march=native", "-O3", "-fwhole-program", "-Imain/include", ],
        linkopts = [ "-lpthread", "-O3", "-std=c++17", "-march=native", "-fwhole-program" ],
        deps = [":libquod", ":libquod64_oldloops", ],
        )

cc_binary(
        name = "product-compute",
        srcs = [ "product-compute.cc", ],
        copts = [ "-std=c++17", "-march=native", "-O3", "-fwhole-program", "-Imain/include", ],
        linkopts = [ "-lpthread", "-O3", "-std=c++17", "-march=native", "-fwhole-program" ],
        deps = [":libquod" ],
        )
cc_binary(
        name = "sum-compute",
        srcs = [ "sum-compute.cc", ],
        copts = [ "-std=c++17", "-march=native", "-O3", "-fwhole-program", "-Imain/include", ],
        linkopts = [ "-lpthread", "-O3", "-std=c++17", "-march=native", "-fwhole-program" ],
        deps = [":libquod" ],
        )
cc_binary(
        name = "quodigiousSimple",
        srcs = [ "simpleLoops.cc", ],
        copts = [ "-std=c++17", "-march=native", "-O3", "-fwhole-program", "-Imain/include", ],
        linkopts = [ "-lpthread", "-O3", "-std=c++17", "-march=native", "-fwhole-program" ],
        deps = [":libquod" ],
        )
