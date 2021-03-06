    { "L", OPT_EXIT, {(void*)opt_license}, "show license" },
    { "h", OPT_EXIT, {(void*)opt_help}, "show help" },
    { "?", OPT_EXIT, {(void*)opt_help}, "show help" },
    { "help", OPT_EXIT, {(void*)opt_help}, "show help" },
    { "-help", OPT_EXIT, {(void*)opt_help}, "show help" },
    { "version", OPT_EXIT, {(void*)opt_version}, "show version" },
    { "formats"  , OPT_EXIT, {(void*)opt_formats  }, "show available formats" },
    { "codecs"   , OPT_EXIT, {(void*)opt_codecs   }, "show available codecs" },
    { "bsfs"     , OPT_EXIT, {(void*)opt_bsfs     }, "show available bit stream filters" },
    { "protocols", OPT_EXIT, {(void*)opt_protocols}, "show available protocols" },
    { "filters",   OPT_EXIT, {(void*)opt_filters  }, "show available filters" },
    { "pix_fmts" , OPT_EXIT, {(void*)opt_pix_fmts }, "show available pixel formats" },
#if _MSC_VER 
    { "sample_fmts", OPT_EXIT, {(void*)show_sample_fmts }, "show available audio sample formats" },
#else
    { "sample_fmts", OPT_EXIT, {.func_arg = show_sample_fmts }, "show available audio sample formats" },
#endif
    { "loglevel", HAS_ARG, {(void*)opt_loglevel}, "set libav* logging level", "loglevel" },
    { "v", HAS_ARG, {(void*)opt_loglevel}, "set libav* logging level", "loglevel" },
    { "debug", HAS_ARG, {(void*)opt_codec_debug}, "set debug flags", "flags" },
    { "report", 0, {(void*)opt_report}, "generate a report" },
