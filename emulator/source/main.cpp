#include <cstdio>
#include <cstdlib>
#include <memory>

#include <fern.h>

static void print_usage();
static void assert_exit(bool cond, const std::string& str);

int main(int argc,const char *argv[]) {
	if(argc <= 1) {
		std::puts("error: not enough arguments");
		print_usage();
		std::exit(-1);
	}

	// argument handling --------------------------------@/
	int arg_index = 1;

	auto arg_valid = [&]() {
		return (arg_index < argc);
	};
	auto arg_get = [&]() { 
		if(!arg_valid()) {
			std::puts("internal error: argument index out of range");
			std::exit(-1);
		}
		auto arg = std::string(argv[arg_index]);
		return arg;
	};
	auto arg_read = [&]() {
		auto arg = arg_get();
		arg_index++;
		return arg;
	};

	// read user arguments ------------------------------@/
	std::string filename_rom;
	bool flag_verbose = false;
	bool flag_debug = false;

	while(arg_index < argc) {
		auto arg1 = arg_read();

		if(arg1 == "--help") {
			print_usage();
			std::exit(0);
		} 
		else if(arg1 == "-g") {
			flag_debug = true;
		} 
		else if(arg1 == "-v") {
			//assert_exit(arg_valid(),"mapconv: error: no output file specified");
			//filename_output = arg_read();
			flag_verbose = true;
		} 
		else {
			if(!filename_rom.empty()) {
				std::printf("error: unknown argument '%s'\n",
					arg1.c_str()
				);
				std::exit(-1);
			}
			filename_rom = arg1;
		}
	}

	assert_exit(!filename_rom.empty(),"error: no rom specified");
	std::printf("verbose: %d\n",flag_verbose);

	auto emu = std::make_shared<fern::CEmulator>();
	emu->debug_set(flag_debug);
	emu->load_romfile(filename_rom);
	emu->boot();

	return 0;
}

static void assert_exit(bool cond, const std::string& str) {
	if(!cond) {
		std::puts(str.c_str());
		std::exit(-1);
	}
}
static void print_usage() {
	std::puts(
		"fern 0.01\n"
		"usage: fern <source rom> <options>\n"
		"\t-g        debug flag\n"
		"\t-v        verbose flag\n"
		"\t--help    Display help"
	);
}

