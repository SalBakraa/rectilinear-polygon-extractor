#define NOBUILD_IMPLEMENTATION
#include "./nobuild.h"

// Overridable macros

#ifndef DESTDIR
#	define DESTDIR ""
#endif

#ifndef PREFIX
#	define PREFIX PATH_SEP "usr" PATH_SEP "local"
#endif

#ifndef BINARY_NAME
#	define BINARY_NAME "rectilinearize"
#endif

// Paths

#define SRC_DIR "src"

#define BUILD_DIR "build"

#define LIB_DIR "lib"

// Build macros

#ifdef DEBUG
#	define EXTRA_CFLAGS "-ggdb", "-Og"
#else
#	define EXTRA_CFLAGS "-O2"
#endif

#ifdef __linux__
#	define CC "cc"
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#	define CC "cl.exe"
#endif

#define WARNING_FLAGS "-Wall", "-Wextra", "-Wshadow", "-Wconversion", "-Wduplicated-cond", "-Wduplicated-branches", "-Wrestrict", "-Wnull-dereference", "-Wjump-misses-init", "-Wimplicit-fallthrough"
#define CFLAGS EXTRA_CFLAGS, WARNING_FLAGS, CONCAT("-I", LIB_DIR), "-lm"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// LSP stuff
static void create_ccls_file(Cstr binary_path) {
	if (IS_NEWER(binary_path, PATH(SRC_DIR, ".ccls"))) {
		CHAIN(CHAIN_CMD("echo", JOIN("\n", "clang", CFLAGS)), CHAIN_OUT(PATH(SRC_DIR, ".ccls")));
	}
}

static Cstr build_stb_lib(Cstr lib_file, Cstr impl_macro) {
	Cstr out_file = PATH(BUILD_DIR, CONCAT(NOEXT(BASENAME(lib_file)), ".o"));
	if (IS_NEWER(lib_file, out_file)) {
		CMD(CC, EXTRA_CFLAGS, CONCAT("-D", impl_macro), "-xc", "-o", out_file, "-c", lib_file);
	}
	return out_file;
}

// Build the project
static void build(void) {
	// Copy and generate necassry source files into intermediate build directory
	if (!PATH_EXISTS(BUILD_DIR)) {
		MKDIRS(BUILD_DIR);
	}

	Cstr in_file = PATH(SRC_DIR, "main.c");
	Cstr_Array source_files = CSTR_ARRAY_MAKE(in_file,
		build_stb_lib(PATH(LIB_DIR, "stb_image.h"), "STB_IMAGE_IMPLEMENTATION"),
		build_stb_lib(PATH(LIB_DIR, "stb_ds.h"), "STB_DS_IMPLEMENTATION"),
		build_stb_lib(PATH(LIB_DIR, "nobuild", "nobuild.h"), "NOBUILD_IMPLEMENTATION")
	);

	int should_build_bin = 0;
	Cstr bin_name = PATH(BUILD_DIR, BINARY_NAME);
	FOREACH_ARRAY(Cstr , file, source_files, {
		should_build_bin = should_build_bin || IS_NEWER(*file, bin_name);
	});

	if (should_build_bin) {
		Cmd build_cmd = {
			.line = cstr_array_concat(CSTR_ARRAY_MAKE(CC, CFLAGS, "-DBINARY", "-o", bin_name), source_files),
		};
		INFO("CMD: %s", cmd_show(build_cmd));
		cmd_run_sync(build_cmd);
	}

	INFO("Building static library:");
	Cstr out_file = PATH(BUILD_DIR, CONCAT(NOEXT(BASENAME(in_file)), ".o"));
	if (IS_NEWER(in_file, out_file)) {
		CMD(CC, CFLAGS, "-o", out_file, "-c", in_file);
	}

	Cstr lib_name = PATH(BUILD_DIR, CONCAT("lib", BINARY_NAME, ".a"));
	if (IS_NEWER(out_file, lib_name)) {
		CMD("ar", "rcs", lib_name, out_file);
	}

	if (IS_NEWER(PATH(SRC_DIR, "main.h"), PATH(BUILD_DIR, CONCAT(BINARY_NAME, ".h")))) {
		COPY(PATH(SRC_DIR, "main.h"), PATH(BUILD_DIR, CONCAT(BINARY_NAME, ".h")));
	}
}

int main(int argc, char **argv)
{
	GO_REBUILD_URSELF(argc, argv);

	int clean_build_files = 0;
	int dump_cflags = 0;
	for (int i = 1; i < argc; ++i) {
		if (STARTS_WITH(argv[i], "--clean")) {
			clean_build_files = 1;
			continue;
		}

		if (STARTS_WITH(argv[i], "--make-ccls-file")) {
			dump_cflags = 1;
			continue;
		}
	}

	if (clean_build_files) {
		RM(BUILD_DIR);
		RM(PATH(SRC_DIR, ".ccls"));
	}

	if (dump_cflags) {
		create_ccls_file(argv[0]);
	}

	build();

	return 0;
}
