#include <cstdio>
#include <cstdlib>
#include <memory>

#include <fern.h>

#include <windows.h>
#include <dirent.h>

static void print_usage();
static void assert_exit(bool cond, const std::string& str);

int io_promptFileOpen(char filename[],int filename_size,void* parent_window,const char *filter) {
	OPENFILENAME ofn;      			// common dialog box structure

	char old_dir[512] = {};
	getcwd(old_dir,sizeof(old_dir));	// copy cur directory to old_dir
	
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.lpstrFile = filename;
	ofn.nMaxFile = filename_size;
	ofn.lpstrFilter = filter;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = old_dir;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.hwndOwner = static_cast<HWND>(parent_window);

	ZeroMemory(filename,filename_size);
	int success = (GetOpenFileName(&ofn) == true);
	//chdir(old_dir);
	
	return success;
}

int main(int argc,const char *argv[]) {
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
	bool flag_vsync = false;

	while(arg_index < argc) {
		auto arg1 = arg_read();

		if(arg1 == "--help") {
			print_usage();
			std::exit(0);
		}
		else if(arg1 == "-g") {
			flag_debug = true;
		} 
		else if(arg1 == "-vs") {
			flag_vsync = true;
		} 
		else if(arg1 == "-v") {
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

	if(filename_rom.empty()) {
		const char* filter = "Monochrome GB ROM (*.gb)\0*.gb\0Color GB ROM (*.gbc)\0*.gbc\0All Files (*.*)\0*.*\0\0";
		std::array<char,512> filename_buf;
		if(io_promptFileOpen(filename_buf.data(),filename_buf.size(),NULL,filter)) {
			filename_rom = std::string(filename_buf.data());
		} else {
			print_usage();
			std::exit(0);
		}
	}
	assert_exit(!filename_rom.empty(),"error: no rom specified");

	fern::CEmuInitFlags flags;
	flags.debug = flag_debug;
	flags.vsync = flag_vsync;
	flags.verbose = flag_verbose;

	auto emu = std::make_shared<fern::CEmulator>(&flags);
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
		"fern 0.6\n"
		"usage: fern <source rom> <options>\n"
		"\t-vs       enable vsync\n"
		"\t-g        enable debugger\n"
		"\t-v        verbose flag\n"
		"\t--help    Display help\n"
		"\tcontrols:\n"
		"\t\tarrow keys - d-pad\n"
		"\t\ts - A button\n"
		"\t\ta - B button\n"
		"\t\tv - select\n"
		"\t\tb - start"
	);
}

