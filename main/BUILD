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
        copts = [ "-std=c++17", "-march=native", "-O3", "-fwhole-program", "-Imain/include", "-Imain/", ],
        linkopts = [ "-lpthread", "-O3", "-std=c++17", "-march=native", "-fwhole-program" ],
        deps = [":libquod", ":libquod64_oldloops", ],
        )
