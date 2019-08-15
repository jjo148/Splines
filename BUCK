load('//:subdir_glob.bzl', 'subdir_glob')

cxx_library(
    name = "splines",
    header_namespace = "",
    exported_headers = subdir_glob([('src', '*.hh')], prefix=""),
    srcs = glob([('src/*.cc')]),
    exported_platform_preprocessor_flags = [
        ('macosx.*', ['-DSPLINES_DO_NOT_USE_GENERIC_CONTAINER=1']),
        ('linux.*', ['-DSPLINES_DO_NOT_USE_GENERIC_CONTAINER=1']),
        ('win.*', ['/DSPLINES_DO_NOT_USE_GENERIC_CONTAINER=1']),
    ],
    visibility = [ "PUBLIC" ]
)
