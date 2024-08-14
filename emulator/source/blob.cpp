#include "blob.h"
#include <cstdio>
#include <cstdlib>

// construct ----------------------------------------------------------------@/
Blob::Blob() {
	m_data.clear();
	m_data.reserve(32);
}
Blob::Blob(const Blob& other) {
	m_data.clear();
	m_data.reserve(32);
	write_blob(other);
}
Blob::Blob(const std::string& source_str) {
	m_data.clear();
	m_data.reserve(32);
	write_str(source_str);
}

bool Blob::load_file(const std::string& filename, bool allow_fail) {
	auto file = std::fopen(filename.c_str(),"rb");
	if(!file) {
		if(allow_fail) {
			return false;
		} else {
			std::printf("Blob::load_file: error: unable to open file '%s'\n",
				filename.c_str()
			);
			std::exit(-1);
		}
	}

	// get file size ------------------------------------@/
	std::fseek(file,0,SEEK_END);
	const size_t filesize = ftell(file);
	std::rewind(file);

	// read entire file ---------------------------------@/
	uint8_t chrbuffer = 0;
	for(size_t i=0; i<filesize; i++) {
		std::fread(&chrbuffer,1,sizeof(chrbuffer),file);
		write_u8(chrbuffer);
	}

	std::fclose(file);
	return true;
}

// writing ------------------------------------------------------------------@/
void Blob::write_u8(const uint32_t num) {
	m_data.push_back(num & 0xFF);
}
void Blob::write_u16(const uint32_t num) {
	write_u8(num);
	write_u8(num>>8);
}
void Blob::write_u32(const uint32_t num) {
	write_u16(num);
	write_u16(num>>16);
}
void Blob::write_str(const std::string& source_str) {
	for(auto& cur_chr : source_str) {
		write_u8(cur_chr);
	}
	write_u8(0);
}

auto Blob::write_raw(const void* source, size_t size) -> void {
	if(!source) {
		std::puts("Blob::write_raw: error: NULL source");
		std::exit(-1);
	}

	auto src_ptr = static_cast<const uint8_t*>(source);
	for(size_t i=0; i<size; i++) {
		write_u8(*src_ptr++);
	}
}
auto Blob::write_blob(const Blob& other) -> void {
	write_raw(other.data(),other.size());
}
auto Blob::write_file(const std::string& filename, bool allow_fail) -> bool {
	auto file = std::fopen(filename.c_str(),"wb");
	if(!file) {
		if(!allow_fail) {
			std::printf("Blob::write_raw: error: file %s is unavailable\n",
				filename.c_str()
			);
			std::exit(-1);
		}
		return false;
	}
	std::fwrite(data(),size(),1,file);
	std::fclose(file);
	return true;
}

// misc ---------------------------------------------------------------------@/
auto Blob::align(size_t alignment, uint8_t filler) -> void {
	auto newsize = ((size() + (alignment-1)) / alignment) * alignment;
	auto to_write = newsize - size();
	for(size_t i=0; i<to_write; i++) {
		write_u8(filler);
	}
}

